#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "stdbool.h"
#include "stdint.h"
#include <huayi_uci_fun.h>

/* ====================== 阈值配置宏，现场可调 ====================== */
#define SAMPLE_INTERVAL_SEC     60      /* 采样周期 60秒 */
#define HANG_TRIGGER_CNT        3       /* 连续多少次判定死机 */
#define RESTART_COOL_SEC        600     /* 重启冷却 10分钟 */

#define TX_PER_THRESHOLD        35.0f   /* TX PER告警阈值 */
#define CRC_PER_THRESHOLD       45.0f   /* CRC PER告警阈值 */
#define WISP_RSSI_THRESHOLD     -70     /* WISP信号下限 */
#define TEMP_OVER_THRESHOLD     80      /* 过温告警温度 */

#define LINE_BUF_SIZE           1024
#define CMD_BUF_SIZE            128

/* 故障类型 */
typedef enum {
    WIFI_OK = 0,
    WIFI_DRIVER_HANG,       /* 驱动挂死 */
    WIFI_HIGH_TX_PER,       /* 发送高丢包 */
    WIFI_HIGH_CRC_ERR,      /* CRC错误高 */
    WIFI_WISP_RSSI_LOW,     /* WISP信号差 */
    WIFI_OVER_TEMP          /* 芯片过温 */
} WifiFaultType;

/* iwpriv stat 统计快照 */
typedef struct {
    int32_t  temperature;
    uint64_t tx_success;
    uint64_t tx_fail;
    double   tx_per;

    uint64_t rx_success;
    uint64_t rx_crc_err;
    double   rx_crc_per;

    int rssi[3];
} WifiIfStat;

/* 单接口检测结果 */
typedef struct {
    char ifname[16];
    WifiFaultType fault;
    WifiIfStat stat;
    bool is_associated;
    int hang_detect_cnt;
} WifiCheckResult;

/* 监控全局上下文 */
typedef struct {
    WifiIfStat prev_apcli0;
    WifiIfStat prev_ra0;

    int hang_apcli0;
    int hang_ra0;

    time_t last_restart_ts;
    bool first_run;
    
    /* 记录上一次关联状态，用于检测变化 */
    bool last_apcli0_assoc;
    bool last_ra0_assoc;
} WifiMonitorCtx;

int first_connect = 1;
/* 初始化上下文 */
static void wifi_monitor_ctx_init(WifiMonitorCtx *ctx)
{
    memset(ctx, 0, sizeof(WifiMonitorCtx));
    ctx->first_run = true;
    ctx->last_restart_ts = 0;
    ctx->last_apcli0_assoc = false;
    ctx->last_ra0_assoc = false;
}

/* 检查接口是否真正关联到AP */
static bool is_interface_associated(const char *ifname)
{
    char cmd[CMD_BUF_SIZE];
    char output[256] = {0};
    FILE *fp;
    bool associated = false;
    
    snprintf(cmd, sizeof(cmd), "iwconfig %s 2>/dev/null | grep 'Access Point' | grep -v 'Not-Associated'", ifname);
    
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

/* 执行 iwpriv <ifname> stat，获取原始输出 */
static bool exec_iwpriv_stat(const char *ifname, char *out_buf, size_t buf_len)
{
    char cmd[CMD_BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "iwpriv %s stat 2>/dev/null", ifname);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        CHECK_WISP_LOG("[MON] popen fail: %s", cmd);
        return false;
    }

    memset(out_buf, 0, buf_len);
    size_t offset = 0;
    char tmp[256];
    while (fgets(tmp, sizeof(tmp), fp) != NULL) {
        size_t tlen = strlen(tmp);
        if ((offset + tlen) >= buf_len)
            break;
        memcpy(out_buf + offset, tmp, tlen);
        offset += tlen;
    }
    pclose(fp);

    if (offset == 0)
        return false;
    return true;
}

/* 解析iwpriv stat输出到WifiIfStat */
static bool parse_stat(const char *raw, WifiIfStat *st)
{
    memset(st, 0, sizeof(WifiIfStat));
    char line[256];
    const char *p = raw;

    while (*p != '\0') {
        /* 提取一行 */
        size_t i = 0;
        while (*p != '\0' && *p != '\n' && i < sizeof(line)-1) {
            line[i++] = *p++;
        }
        line[i] = '\0';
        if (*p == '\n') p++;

        if (strstr(line, "CurrentTemperature")) {
            sscanf(line, "CurrentTemperature              = %d", &st->temperature);
        } else if (strstr(line, "Tx success")) {
            sscanf(line, "Tx success                      = %llu", &st->tx_success);
        } else if (strstr(line, "Tx fail count")) {
            sscanf(line, "Tx fail count                   = %llu, PER=%lf%%",
                   &st->tx_fail, &st->tx_per);
        } else if (strstr(line, "Rx success")) {
            sscanf(line, "Rx success                      = %llu", &st->rx_success);
        } else if (strstr(line, "Rx with CRC")) {
            sscanf(line, "Rx with CRC                     = %llu, PER=%lf%%",
                   &st->rx_crc_err, &st->rx_crc_per);
        } else if (strstr(line, "RSSI")) {
            sscanf(line, "RSSI                            = %d %d %d",
                   &st->rssi[0], &st->rssi[1], &st->rssi[2]);
        }
    }
    return true;
}

/* 采集单个接口统计 */
static bool collect_if(const char *ifname, WifiIfStat *out)
{
    char raw[LINE_BUF_SIZE];
    if (!exec_iwpriv_stat(ifname, raw, sizeof(raw))) {
        CHECK_WISP_LOG("[MON] collect %s failed", ifname);
        return false;
    }
    return parse_stat(raw, out);
}

/* 单接口检测逻辑 */
static WifiCheckResult check_interface(const char *ifname,
                                       const WifiIfStat *prev,
                                       const WifiIfStat *curr,
                                       int old_hang_cnt)
{
    WifiCheckResult res;
    memset(&res, 0, sizeof(res));
    strncpy(res.ifname, ifname, sizeof(res.ifname)-1);
    res.stat = *curr;
    res.fault = WIFI_OK;
    res.hang_detect_cnt = old_hang_cnt;

    /* 判断是否关联上级AP：结合 iwconfig 和 RSSI */
    bool iwconfig_assoc = is_interface_associated(ifname);
    bool rssi_valid = (curr->rssi[0] != 0) || (curr->rssi[1] != 0);

    /* 只有 iwconfig 显示已关联 且 RSSI 有效，才认为真正关联 */
    res.is_associated = iwconfig_assoc && rssi_valid;

    /* 如果 iwconfig 显示未关联，但 RSSI 还有值，说明是假连接（驱动未刷新） */
    if (!iwconfig_assoc && rssi_valid) {
        CHECK_WISP_LOG("[MON] %s: iwconfig says Not-Associated but RSSI=%d,%d (stale data, treat as NOT associated)",
                       ifname, curr->rssi[0], curr->rssi[1]);
    }

    uint64_t delta_tx = curr->tx_success - prev->tx_success;
    uint64_t delta_rx = curr->rx_success - prev->rx_success;
    uint64_t delta_crc = curr->rx_crc_err - prev->rx_crc_err;

    /* 计算增量 CRC PER（反映当前时刻的通信质量） */
    double delta_crc_per = (delta_rx > 0) ? (double)delta_crc / delta_rx * 100 : 0;

    /* 只有已关联，才做死机判定；未关联计数为0属于正常 */
    if (res.is_associated) {
        if (delta_tx == 0 || delta_rx == 0) {
            res.hang_detect_cnt++;
        } else {
            res.hang_detect_cnt = 0;
        }
        if (res.hang_detect_cnt >= HANG_TRIGGER_CNT) {
            res.fault = WIFI_DRIVER_HANG;
        }
    } else {
        res.hang_detect_cnt = 0;
    }

    /* 过温 */
    if (curr->temperature >= TEMP_OVER_THRESHOLD) {
        res.fault = WIFI_OVER_TEMP;
    }
    /* 高TX PER - 只在已关联时检查（使用累计值） */
    if (res.is_associated && curr->tx_per > TX_PER_THRESHOLD) {
        res.fault = WIFI_HIGH_TX_PER;
    }
    /* 高CRC - 只在已关联时检查（使用增量值，反映当前实时通信质量） */
    if (res.is_associated && delta_crc_per > CRC_PER_THRESHOLD) {
        res.fault = WIFI_HIGH_CRC_ERR;
    }
    /* WISP RSSI过低，仅对apcli0 */
    if ((strcmp(ifname, "apcli0") == 0) && res.is_associated) {
        int min_rssi = (curr->rssi[0] < curr->rssi[1]) ? curr->rssi[0] : curr->rssi[1];
        if (min_rssi <= WISP_RSSI_THRESHOLD) {
            res.fault = WIFI_WISP_RSSI_LOW;
        }
    }
    return res;
}

/* 执行无线重启，带冷却 */
static void trigger_wireless_restart(WifiMonitorCtx *ctx)
{
    time_t now = time(NULL);
    if ((now - ctx->last_restart_ts) < RESTART_COOL_SEC) {
        CHECK_WISP_LOG("[MON] skip restart, cooling %lds remain",
                (long)(RESTART_COOL_SEC - (now - ctx->last_restart_ts)));
        return;
    }
    CHECK_WISP_LOG("[MON] ######## WIRELESS RESTART TRIGGER ########");
    system("huayi-reset wireless");
    ctx->last_restart_ts = now;
}

static const char* fault_str(WifiFaultType f)
{
    switch(f) {
        case WIFI_OK: return "OK";
        case WIFI_DRIVER_HANG: return "DRIVER_HANG";
        case WIFI_HIGH_TX_PER: return "HIGH_TX_PER";
        case WIFI_HIGH_CRC_ERR: return "HIGH_CRC_ERR";
        case WIFI_WISP_RSSI_LOW: return "WISP_RSSI_LOW";
        case WIFI_OVER_TEMP: return "OVER_TEMP";
        default: return "UNKNOWN";
    }
}

/* 执行一轮完整巡检 */
static void do_scan(WifiMonitorCtx *ctx)
{
    WifiIfStat curr_apcli0, curr_ra0;
    bool ok_apcli = collect_if("apcli0", &curr_apcli0);
    bool ok_ra0   = collect_if("ra0", &curr_ra0);

    if (ctx->first_run) {
        if (ok_apcli) {
            ctx->prev_apcli0 = curr_apcli0;
            ctx->last_apcli0_assoc = is_interface_associated("apcli0");
        }
        if (ok_ra0) {
            ctx->prev_ra0 = curr_ra0;
            ctx->last_ra0_assoc = is_interface_associated("ra0");
        }
        ctx->first_run = false;
        CHECK_WISP_LOG("[MON] first sample done, skip check");
        return;
    }

    if (ok_apcli) {
        bool current_assoc = is_interface_associated("apcli0");
        
        /* 从未连接变为已连接，刷新统计 */
        if (!ctx->last_apcli0_assoc && current_assoc) {
            CHECK_WISP_LOG("[MON apcli0] status changed: DISCONNECTED -> CONNECTED, resetting stats");
            system("iwpriv apcli0 set ResetCounter=1");
            /* 重新采集一次，获取刷新后的数据 */
            collect_if("apcli0", &curr_apcli0);
        }
        ctx->last_apcli0_assoc = current_assoc;

        WifiCheckResult res = check_interface("apcli0", &ctx->prev_apcli0, &curr_apcli0, ctx->hang_apcli0);
        ctx->hang_apcli0 = res.hang_detect_cnt;

        CHECK_WISP_LOG("[MON apcli0] temp:%d tx_per:%.1f crc_per:%.1f rssi0:%d rssi1:%d hang_cnt:%d assoc:%s fault:%s",
               curr_apcli0.temperature,
               curr_apcli0.tx_per,
               curr_apcli0.rx_crc_per,
               curr_apcli0.rssi[0],
               curr_apcli0.rssi[1],
               res.hang_detect_cnt,
               res.is_associated ? "Y":"N",
               fault_str(res.fault));

        if (res.fault == WIFI_DRIVER_HANG) {
            trigger_wireless_restart(ctx);
        }
        ctx->prev_apcli0 = curr_apcli0;
    }

    if (ok_ra0) {
        bool current_assoc = is_interface_associated("ra0");
        
        /* 从未连接变为已连接，刷新统计 */
        if (!ctx->last_ra0_assoc && current_assoc) {
            CHECK_WISP_LOG("[MON ra0] status changed: DISCONNECTED -> CONNECTED, resetting stats");
            /* 重新采集一次，获取刷新后的数据 */
            collect_if("ra0", &curr_ra0);
        }
        ctx->last_ra0_assoc = current_assoc;

        WifiCheckResult res = check_interface("ra0", &ctx->prev_ra0, &curr_ra0, ctx->hang_ra0);
        ctx->hang_ra0 = res.hang_detect_cnt;

        CHECK_WISP_LOG("[MON ra0] temp:%d tx_per:%.1f crc_per:%.1f rssi0:%d rssi1:%d hang_cnt:%d fault:%s",
               curr_ra0.temperature,
               curr_ra0.tx_per,
               curr_ra0.rx_crc_per,
               curr_ra0.rssi[0],
               curr_ra0.rssi[1],
               res.hang_detect_cnt,
               fault_str(res.fault));

        if (res.fault == WIFI_DRIVER_HANG) {
            trigger_wireless_restart(ctx);
        }
        ctx->prev_ra0 = curr_ra0;
    }
}

int main(void)
{
    WifiMonitorCtx ctx;
    wifi_monitor_ctx_init(&ctx);
    CHECK_WISP_LOG("WISP WiFi Monitor(C) start, interval=%d sec", SAMPLE_INTERVAL_SEC);

    for (;;) {
        do_scan(&ctx);
        sleep(SAMPLE_INTERVAL_SEC);
    }
    return 0;
}