#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "strings.h"
#include "time.h"
#include "unistd.h"
#include "stdbool.h"
#include "stdint.h"
#include <sys/stat.h>
#include <huayi_uci_fun.h>

/* ====================== 阈值配置宏，现场可调 ====================== */
#define SAMPLE_INTERVAL_SEC     60      /* 已联网时的巡检周期 60秒 */
#define STARTUP_GRACE_SEC       90      /* 启动后等待系统自身完成中继连接 */
#define SCAN_WAIT_SEC           10      /* 触发扫描后等待结果的时间 */
#define ASSOC_WAIT_SEC          60      /* 切换候选AP后等待关联的最长时间(huayi-reset wireless内部sleep 10s+) */
#define NET_WAIT_SEC            30      /* 关联后等待联网(ping)的最长时间 */
#define RETRY_INTERVAL_SEC      120     /* 所有记录均失败后，重新扫描的间隔 */

#define REP_MAX_SLOT            5       /* 最多保存5条中继记录 */
#define SCAN_LIST_FILE          "/tmp/WispRepScanList"
#define APCLI_IFNAME            "apcli0"
#define APCLI_SECTION           "radio0_cli0"

#define SSID_LEN                64
#define WPAPSK_LEN              128
#define BSSID_LEN               18
#define MAX_SCAN_AP             64

/* 日志宏：与 CHECK_WISP_LOG 同样的写法，输出到 /tmp/log/wisp_rep_log */
#define WISP_REP_LOG_SIN 1
#define WISP_REP_LOG(format, args...) \
					do{ \
						if(WISP_REP_LOG_SIN) \
						{ \
							FILE *fp = fopen("/tmp/log/wisp_rep_log", "a"); \
							if(fp) \
							{ \
								fprintf(fp, "%s@%s,%d] " format "\n", \
									__FILE__, __func__,__LINE__, ##args ); \
							} \
							fclose(fp);\
						} \
						struct stat buf;\
						stat("/tmp/log/wisp_rep_log", &buf);\
						int size=buf.st_size;\
						if(size > 20000)\
						{ \
							system("rm -fr /tmp/log/wisp_rep_log");\
						}\
					}while(0)

/* 一条中继记录 */
typedef struct {
	char ssid[SSID_LEN];       /* SSID */
	char wpapsk[WPAPSK_LEN];   /* 密码 */
	char bssid[BSSID_LEN];     /* BSSID */
} RepRecord;

/* 扫描到的AP */
typedef struct {
	char bssid[BSSID_LEN];
	char ssid[SSID_LEN];
} ScanAp;

/* 读取全部 5 条记录 */
static void rep_read_all(RepRecord *recs)
{
	char cmd[128];
	char buf[256];
	FILE *fp;
	int i;

	memset(recs, 0, sizeof(RepRecord) * REP_MAX_SLOT);

	for (i = 0; i < REP_MAX_SLOT; i++) {
		char sec[16];
		snprintf(sec, sizeof(sec), "rep%d", i + 1);

		/* ssid */
		memset(buf, 0, sizeof(buf));
		snprintf(cmd, sizeof(cmd), "uci -q get wisp_rep.%s.ssid", sec);
		fp = popen(cmd, "r");
		if (fp) {
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				char *nl = strchr(buf, '\n');
				if (nl) *nl = '\0';
				if (buf[0]) strncpy(recs[i].ssid, buf, SSID_LEN - 1);
			}
			pclose(fp);
		}

		/* wpapsk */
		memset(buf, 0, sizeof(buf));
		snprintf(cmd, sizeof(cmd), "uci -q get wisp_rep.%s.wpapsk", sec);
		fp = popen(cmd, "r");
		if (fp) {
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				char *nl = strchr(buf, '\n');
				if (nl) *nl = '\0';
				if (buf[0]) strncpy(recs[i].wpapsk, buf, WPAPSK_LEN - 1);
			}
			pclose(fp);
		}

		/* bssid */
		memset(buf, 0, sizeof(buf));
		snprintf(cmd, sizeof(cmd), "uci -q get wisp_rep.%s.bssid", sec);
		fp = popen(cmd, "r");
		if (fp) {
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				char *nl = strchr(buf, '\n');
				if (nl) *nl = '\0';
				if (buf[0]) strncpy(recs[i].bssid, buf, BSSID_LEN - 1);
			}
			pclose(fp);
		}
	}
}

/* 写回全部 5 条记录并 commit */
static int rep_write_all(RepRecord *recs)
{
	char cmd[256];
	int i;

	for (i = 0; i < REP_MAX_SLOT; i++) {
		char sec[16];
		snprintf(sec, sizeof(sec), "rep%d", i + 1);

		snprintf(cmd, sizeof(cmd), "uci set wisp_rep.%s.ssid='%s'", sec, recs[i].ssid);
		system(cmd);

		snprintf(cmd, sizeof(cmd), "uci set wisp_rep.%s.wpapsk='%s'", sec, recs[i].wpapsk);
		system(cmd);

		snprintf(cmd, sizeof(cmd), "uci set wisp_rep.%s.bssid='%s'", sec, recs[i].bssid);
		system(cmd);
	}

	system("uci commit wisp_rep");
	return 0;
}

/*
 * 当前中继连接成功：保存记录。
 * 记录按 rep1(最旧) -> rep5(最新) 的时间顺序保存：
 *  - 已存在的记录(bssid/ssid匹配)：更新后移到最新位置；
 *  - 新记录：未满5条则追加到最新位置；
 *  - 已满5条：删除第一条，其余前移，新记录写到rep5。
 */
static void rep_upsert(const char *ssid, const char *bssid, const char *wpapsk)
{
	RepRecord recs[REP_MAX_SLOT + 1]; /* 多一个槽用于临时存放 */
	RepRecord nr;
	int i, n = 0, idx = -1;

	if (!ssid[0] && !bssid[0]) {
		return;
	}

	rep_read_all(recs);

	/* 统计有效记录数 */
	for (i = 0; i < REP_MAX_SLOT; i++) {
		if (recs[i].ssid[0]) {
			n++;
		}
	}

	/* 查找是否已存在：优先按 bssid，其次按 ssid */
	for (i = 0; i < n; i++) {
		if (bssid[0] && recs[i].bssid[0] &&
			strcasecmp(recs[i].bssid, bssid) == 0) {
			idx = i;
			break;
		}
	}
	if (idx < 0) {
		for (i = 0; i < n; i++) {
			if (ssid[0] && recs[i].ssid[0] &&
				strcmp(recs[i].ssid, ssid) == 0) {
				idx = i;
				break;
			}
		}
	}

	/* 构造新记录 */
	memset(&nr, 0, sizeof(nr));
	strncpy(nr.ssid, ssid, SSID_LEN - 1);
	strncpy(nr.bssid, bssid, BSSID_LEN - 1);
	strncpy(nr.wpapsk, wpapsk, WPAPSK_LEN - 1);

	if (idx >= 0) {
		/* 已存在：移出原位置，后面记录前移，追加到最新位置 */
		for (i = idx; i < n - 1; i++) {
			recs[i] = recs[i + 1];
		}
		recs[n - 1] = nr;
		/* 清空有效记录之后的槽位 */
		for (i = n; i < REP_MAX_SLOT; i++) {
			memset(&recs[i], 0, sizeof(recs[i]));
		}
		WISP_REP_LOG("[REP] record exists, move ssid=%s to slot %d", nr.ssid, n);
	} else if (n < REP_MAX_SLOT) {
		/* 未满：追加到最新位置 */
		recs[n] = nr;
		/* 清空之后的槽位 */
		for (i = n + 1; i < REP_MAX_SLOT; i++) {
			memset(&recs[i], 0, sizeof(recs[i]));
		}
		WISP_REP_LOG("[REP] new record ssid=%s saved to slot %d", nr.ssid, n + 1);
	} else {
		/* 已满5条：删除第一条，其余前移，新记录写到rep5 */
		WISP_REP_LOG("[REP] slots full, evict rep1 (ssid=%s), shift up",
				recs[0].ssid);
		for (i = 0; i < REP_MAX_SLOT - 1; i++) {
			recs[i] = recs[i + 1];
		}
		recs[REP_MAX_SLOT - 1] = nr;
	}

	if (rep_write_all(recs) == 0) {
		WISP_REP_LOG("[REP] records committed");
	} else {
		WISP_REP_LOG("[REP] commit records failed");
	}
}

/* 从 wireless.radio0_cli0 读取当前中继配置 */
static void get_apcli_config(char *ssid, size_t slen,
			     char *bssid, size_t blen,
			     char *wpapsk, size_t plen)
{
	char buf[256];

	ssid[0] = '\0';
	bssid[0] = '\0';
	wpapsk[0] = '\0';

	memset(buf, 0, sizeof(buf));
	if (huayi_uci_get_str(PKG_WIRELESS, APCLI_SECTION, "apcli_ssid", buf)) {
		strncpy(ssid, buf, slen - 1);
	}

	memset(buf, 0, sizeof(buf));
	if (huayi_uci_get_str(PKG_WIRELESS, APCLI_SECTION, "apcli_bssid", buf)) {
		strncpy(bssid, buf, blen - 1);
	}

	memset(buf, 0, sizeof(buf));
	if (huayi_uci_get_str(PKG_WIRELESS, APCLI_SECTION, "apcli_wpapsk", buf)) {
		strncpy(wpapsk, buf, plen - 1);
	}
}

/* 检查接口是否真正关联到AP */
static bool is_interface_associated(const char *ifname)
{
	char cmd[128];
	char output[256] = {0};
	FILE *fp;
	bool associated = false;

	snprintf(cmd, sizeof(cmd),
		"iwconfig %s 2>/dev/null | grep 'Access Point' | grep -v 'Not-Associated'",
		ifname);

	fp = popen(cmd, "r");
	if (!fp) {
		return false;
	}

	if (fgets(output, sizeof(output), fp) != NULL) {
		associated = true;
	}
	pclose(fp);

	return associated;
}

/* 解析 iwpriv apcli0 get_site_survey 的扫描结果 */
static int parse_scan_list(ScanAp *aps, int max)
{
	FILE *fp;
	char line[512];
	int n = 0;

	fp = fopen(SCAN_LIST_FILE, "r");
	if (!fp) {
		WISP_REP_LOG("[REP] open scan list %s failed", SCAN_LIST_FILE);
		return 0;
	}

	while (fgets(line, sizeof(line), fp) && n < max) {
		char *p;
		char bssid[BSSID_LEN] = {0};
		char ssid[SSID_LEN] = {0};

		/* BSSID: xx:xx:xx:xx:xx:xx（可能有前导空格） */
		p = strrstr(line, "BSSID:");
		if (!p) {
			continue;
		}
		p += strlen("BSSID:");
		while (*p == ' ' || *p == '\t') p++;
		strncpy(bssid, p, BSSID_LEN - 1);
		if (bssid[2] != ':' || bssid[5] != ':' || bssid[8] != ':') {
			continue;
		}

		/* SSID:"xxxx"（隐藏SSID时为空，仍保留bssid用于匹配） */
		p = strstr(line, "SSID:\"");
		if (p) {
			char *q;
			size_t l;
			p += strlen("SSID:\"");
			q = strchr(p, '"');
			if (q) {
				l = (size_t)(q - p);
				if (l >= SSID_LEN) {
					l = SSID_LEN - 1;
				}
				memcpy(ssid, p, l);
			}
		}

		strncpy(aps[n].bssid, bssid, BSSID_LEN - 1);
		strncpy(aps[n].ssid, ssid, SSID_LEN - 1);
		n++;
	}

	fclose(fp);
	return n;
}

/* 记录是否在扫描结果中：优先 bssid 匹配，其次 ssid 匹配 */
static bool ap_in_scan(ScanAp *aps, int nap, const RepRecord *rec)
{
	int i;

	for (i = 0; i < nap; i++) {
		if (rec->bssid[0] && aps[i].bssid[0] &&
			strcasecmp(rec->bssid, aps[i].bssid) == 0) {
			return true;
		}
		if (rec->ssid[0] && aps[i].ssid[0] &&
			strcmp(rec->ssid, aps[i].ssid) == 0) {
			return true;
		}
	}
	return false;
}

/* 把候选记录写入 wireless.radio0_cli0 并重启无线生效 */
static void apply_apcli_candidate(const RepRecord *rec)
{
	/* 确保中继接口已启用 */
	huayi_uci_set_int(PKG_WIRELESS, APCLI_SECTION, "disabled", 0);
	huayi_uci_set_int(PKG_WIRELESS, APCLI_SECTION, "apcli_enable", 1);

	huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_ssid", (char *)rec->ssid);

	if (rec->bssid[0]) {
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_bssid", (char *)rec->bssid);
	}

	if (rec->wpapsk[0]) {
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_wpapsk", (char *)rec->wpapsk);
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_authmode", "WPA2PSK");
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_encryptype", "AES");
	} else {
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_authmode", "OPEN");
		huayi_uci_set_str(PKG_WIRELESS, APCLI_SECTION, "apcli_encryptype", "NONE");
	}

	huayi_uci_commit(PKG_WIRELESS);

	WISP_REP_LOG("[REP] apply candidate ssid=%s bssid=%s, restart wireless+network",
			rec->ssid, rec->bssid);
	system("huayi-reset wireless");
	sleep(5);
	system("huayi-reset network");
	sleep(10);
}

/*
 * 当前中继未连成功：先WiFi扫描，扫描到已保存的记录则按 rep1->rep5 顺序
 * 一个个连接，找到能联网(ping成功)的WiFi即停止。
 * 返回 1：已通过某条记录联网；0：所有记录均失败。
 */
static int do_failover(void)
{
	RepRecord recs[REP_MAX_SLOT];
	ScanAp aps[MAX_SCAN_AP];
	int nap, i;

	WISP_REP_LOG("[REP] === failover start: site survey ===");
	system("iwpriv " APCLI_IFNAME " set SiteSurvey=");
	sleep(SCAN_WAIT_SEC);
	system("iwpriv " APCLI_IFNAME " get_site_survey > " SCAN_LIST_FILE " 2>/dev/null");

	nap = parse_scan_list(aps, MAX_SCAN_AP);
	WISP_REP_LOG("[REP] scan finished, %d AP(s) found", nap);
	if (nap <= 0) {
		return 0;
	}

	rep_read_all(recs);

	/* 按记录顺序 1 -> 5 逐个尝试 */
	for (i = 0; i < REP_MAX_SLOT; i++) {
		int waited;
		bool assoc = false;

		if (!recs[i].ssid[0]) {
			WISP_REP_LOG("[REP] slot %d empty, skip", i + 1);
			continue;
		}
		if (!ap_in_scan(aps, nap, &recs[i])) {
			WISP_REP_LOG("[REP] slot %d ssid=%s bssid=%s not found in scan, skip",
					i + 1, recs[i].ssid, recs[i].bssid);
			continue;
		}

		WISP_REP_LOG("[REP] trying slot %d: ssid=%s bssid=%s",
				i + 1, recs[i].ssid, recs[i].bssid);
		apply_apcli_candidate(&recs[i]);

		/* 等待关联 */
		waited = 0;
		while (waited < ASSOC_WAIT_SEC) {
			if (is_interface_associated(APCLI_IFNAME)) {
				assoc = true;
				break;
			}
			sleep(2);
			waited += 2;
		}
		if (!assoc) {
			WISP_REP_LOG("[REP] slot %d associate timeout, try next", i + 1);
			continue;
		}
		WISP_REP_LOG("[REP] slot %d associated, checking internet...", i + 1);

		/* 关联成功后等待获取IP并检测联网(ping) */
		waited = 0;
		while (waited < NET_WAIT_SEC) {
			if (internet_do_ping_detect(INTERNET_DOMAIN, NULL) == 1) {
				WISP_REP_LOG("[REP] slot %d ssid=%s ONLINE, stop failover",
						i + 1, recs[i].ssid);
				/* 连接成功：移到最新位置 */
				rep_upsert(recs[i].ssid, recs[i].bssid, recs[i].wpapsk);
				return 1;
			}
			sleep(5);
			waited += 5;
		}
		WISP_REP_LOG("[REP] slot %d ssid=%s no internet, try next",
				i + 1, recs[i].ssid);
	}

	WISP_REP_LOG("[REP] === failover finished: no usable record ===");
	return 0;
}

int main(void)
{
	/* 记录上一次已保存的中继，bssid或ssid变化时才写flash */
	char last_key[SSID_LEN + BSSID_LEN] = {0};
	char wan_type[OPTION_STR_LEN] = {0};
	int failover_active = 0;
	int fail_count = 0;
	int cpe_link = 0;

	system("mkdir -p /tmp/log");
	WISP_REP_LOG("WISP repeater record daemon(C) start");

	/* 启动后先等待系统自身完成中继连接 */
	sleep(STARTUP_GRACE_SEC);

	for (;;) {
		memset(wan_type,0,sizeof(wan_type));
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

		if(strcmp(wan_type,"mobile") == 0)//4G有连接时不执行
		{
			sleep(SAMPLE_INTERVAL_SEC);
			continue;
		}else if(strcmp(wan_type,"rep4g") == 0)
		{
			huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &cpe_link);
			if(cpe_link == 1)
			{
				sleep(SAMPLE_INTERVAL_SEC);
				continue;
			}
		}

		if (is_interface_associated(APCLI_IFNAME) &&
			internet_do_ping_detect(INTERNET_DOMAIN, NULL) == 1) {
			char ssid[SSID_LEN] = {0};
			char bssid[BSSID_LEN] = {0};
			char wpapsk[WPAPSK_LEN] = {0};
			char key[SSID_LEN + BSSID_LEN];

			fail_count = 0;
			if (failover_active) {
				WISP_REP_LOG("[REP] uplink recovered");
				failover_active = 0;
			}

			/*
			 * 当前中继连接成功：把
			 * wireless.radio0_cli0.apcli_bssid/apcli_ssid/apcli_wpapsk
			 * 保存到记录文件
			 */
			get_apcli_config(ssid, sizeof(ssid),
					 bssid, sizeof(bssid),
					 wpapsk, sizeof(wpapsk));

			snprintf(key, sizeof(key), "%s@%s",
				 bssid[0] ? bssid : "", ssid);

			if ((ssid[0] || bssid[0]) && strcmp(last_key, key) != 0) {
				WISP_REP_LOG("[REP] uplink ok: save record ssid=%s bssid=%s",
						ssid, bssid);
				rep_upsert(ssid, bssid, wpapsk);
				strncpy(last_key, key, sizeof(last_key) - 1);
			}

			sleep(SAMPLE_INTERVAL_SEC);
		} else {
			/*
			 * 当前中继未连接成功(未关联或无网)：
			 * 如果当前配置与列表最新记录不同（正在切换WiFi），
			 * 重试3次避免误判；如果与最新记录相同，直接进failover
			 */
			char cur_ssid[SSID_LEN] = {0};
			char cur_bssid[BSSID_LEN] = {0};
			char cur_wpapsk[WPAPSK_LEN] = {0};
			RepRecord rep_recs[REP_MAX_SLOT];
			bool same_as_latest = false;
			int ri;

			fail_count++;

			get_apcli_config(cur_ssid, sizeof(cur_ssid),
					 cur_bssid, sizeof(cur_bssid),
					 cur_wpapsk, sizeof(cur_wpapsk));
			rep_read_all(rep_recs);
			for (ri = REP_MAX_SLOT - 1; ri >= 0; ri--) {
				if (rep_recs[ri].ssid[0]) {
					if ((cur_bssid[0] && rep_recs[ri].bssid[0] &&
						strcasecmp(cur_bssid, rep_recs[ri].bssid) == 0) ||
						(cur_ssid[0] && rep_recs[ri].ssid[0] &&
						strcmp(cur_ssid, rep_recs[ri].ssid) == 0)) {
						same_as_latest = true;
					}
					break;
				}
			}

			if (!same_as_latest && fail_count < 3) {
				WISP_REP_LOG("[REP] uplink check fail #%d, current != latest, wait and retry",
						fail_count);
				sleep(20);
				continue;
			}

			if (!failover_active) {
				WISP_REP_LOG("[REP] uplink not ready, enter failover");
				last_key[0] = '\0';
				failover_active = 1;
			}

			if (do_failover() == 1) {
				char ssid[SSID_LEN] = {0};
				char bssid[BSSID_LEN] = {0};
				char wpapsk[WPAPSK_LEN] = {0};

				failover_active = 0;
				fail_count = 0;
				/* do_failover内部已保存记录，这里仅同步去重key */
				get_apcli_config(ssid, sizeof(ssid),
						 bssid, sizeof(bssid),
						 wpapsk, sizeof(wpapsk));
				snprintf(last_key, sizeof(last_key), "%s@%s",
					 bssid[0] ? bssid : "", ssid);
				/* failover成功后正常巡检 */
				sleep(SAMPLE_INTERVAL_SEC);
			} else {
				WISP_REP_LOG("[REP] no usable record, rescan after %d sec",
						RETRY_INTERVAL_SEC);
				sleep(RETRY_INTERVAL_SEC);
			}
		}
	}

	return 0;
}
