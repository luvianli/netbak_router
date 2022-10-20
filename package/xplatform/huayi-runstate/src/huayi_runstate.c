/**
 * @file HUAYI_STATUS.c
 * 
 * 
 */
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/time.h>

#include <huayi-universal/huayi_runstate.h>
#include <huayi-universal/huayi_control.h>
#include <huayi-universal/huayi_universal.h>
#include <huayi-universal/huayi_dal.h>
#include <linux/sched.h>/*jiffies*/

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HUAYI RUNSTATE Module");
MODULE_AUTHOR("gerry_li");

static struct huayi_runstate s_runstate;

static HUAYI_STATUS statusGet_system_type(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.system_type);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_system_type(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.system_type, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_cpu_model(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.cpu_model);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_cpu_model(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.cpu_model, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_cpu_cores(int *buf) {
    *buf = s_runstate.about.cpu_cores;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpu_cores(int buf) {
    s_runstate.about.cpu_cores = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpu_processors(int *buf) {
    *buf = s_runstate.about.cpu_processors;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpu_processors(int buf) {
    s_runstate.about.cpu_processors = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_flash_size(int *buf) {
    *buf = s_runstate.about.flash_size;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_flash_size(int buf) {
    s_runstate.about.flash_size = buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_fw_version(char *buf) {
	if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.fw_version);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_fw_version(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.fw_version, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_buildtime(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.buildtime);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_buildtime(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.buildtime, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_uboot_version(char *buf) {
	if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.uboot_version);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_uboot_version(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.uboot_version, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_webui_version(char *buf) {
	if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.webui_version);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_webui_version(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.webui_version, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_product_name(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.product_name);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_product_name(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.product_name, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_product_model(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.product_model);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_product_model(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.product_model, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_product_sn(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.product_sn);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_product_sn(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.product_sn, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_product_desc(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.product_desc);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_product_desc(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.product_desc, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_company_name(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.company_name);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_company_name(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.company_name, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_enterprise_oid(int *buf) {
    *buf = s_runstate.about.enterprise_oid;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_enterprise_oid(int buf) {
    s_runstate.about.enterprise_oid = buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_company_website(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.company_website);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_company_website(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.company_website, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_domain_name(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.domain_name);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_domain_name(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.domain_name, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_company_contacts(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.company_contacts);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_company_contacts(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.company_contacts, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_company_address(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.company_address);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_company_address(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.company_address, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_company_copyright(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.about.company_copyright);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_company_copyright(char *buf) {
    if (buf) {
        snprintf(s_runstate.about.company_copyright, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static control_tbl status_about_control_tbl[] = {
	{"system_type", TBL_FORMAT_S,
        g(statusGet_system_type), s(statusSet_system_type)},
	{"company_copyright", TBL_FORMAT_S,
    	g(statusGet_company_copyright), s(statusSet_company_copyright)},
	{"cpu_model", TBL_FORMAT_S,
        g(statusGet_cpu_model), s(statusSet_cpu_model)},
	{"cpu_cores",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_cpu_cores), s(statusSet_cpu_cores)},
    {"cpu_processors",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_cpu_processors), s(statusSet_cpu_processors)},
    {"flash_size",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_flash_size), s(statusSet_flash_size)},
	{"fw_version",	TBL_FORMAT_S,
			g(statusGet_fw_version), s(statusSet_fw_version)},
	{"buildtime", TBL_FORMAT_S,
        g(statusGet_buildtime), s(statusSet_buildtime)},
	{"uboot_version", TBL_FORMAT_S,
			g(statusGet_uboot_version), s(statusSet_uboot_version)},
	{"webui_version", TBL_FORMAT_S,
			g(statusGet_webui_version), s(statusSet_webui_version)},
    {"product_name", TBL_FORMAT_S,
        g(statusGet_product_name), s(statusSet_product_name)},
    {"product_model", TBL_FORMAT_S,
        g(statusGet_product_model), s(statusSet_product_model)},
    {"product_sn", TBL_FORMAT_S,
        g(statusGet_product_sn), s(statusSet_product_sn)},
    {"product_desc", TBL_FORMAT_S,
        g(statusGet_product_desc), s(statusSet_product_desc)},
	{"company_name", TBL_FORMAT_S,
        g(statusGet_company_name), s(statusSet_company_name)},
    {"enterprise_oid", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_enterprise_oid), s(statusSet_enterprise_oid)},
	{"company_website", TBL_FORMAT_S,
        g(statusGet_company_website), s(statusSet_company_website)},
    {"company_contacts", TBL_FORMAT_S,
        g(statusGet_company_contacts), s(statusSet_company_contacts)},
    {"company_address", TBL_FORMAT_S,
        g(statusGet_company_address), s(statusSet_company_address)},
    {"domain_name", TBL_FORMAT_S,
        g(statusGet_domain_name), s(statusSet_domain_name)},
    {NULL, 0, NULL, NULL}
};


static HUAYI_STATUS statusGet_rec_uptime(int *buf) {
    *buf = s_runstate.system.rec_uptime;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_rec_uptime(int buf) {
    s_runstate.system.rec_uptime = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpu_rt_usage(int *buf) {
    *buf = s_runstate.system.cpu_rt_usage;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpu_rt_usage(int buf) {
    s_runstate.system.cpu_rt_usage = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpu_avg_usage(int *buf) {
    *buf = s_runstate.system.cpu_avg_usage;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpu_avg_usage(int buf) {
    s_runstate.system.cpu_avg_usage = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_mem_avg_usage(int *buf) {
    *buf = s_runstate.system.mem_avg_usage;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_mem_avg_usage(int buf) {
    s_runstate.system.mem_avg_usage = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_flash_usage(int *buf) {
    *buf = s_runstate.system.flash_usage;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_flash_usage(int buf) {
    s_runstate.system.flash_usage = buf;
    return HUAYI_OK;
}

HUAYI_STATUS statusGet_ipv6_address(char* buf) {
    int i;
    int p=0;
    for (i=0; i< CSTATUS_MAX_IPV6_ADDR; i++) {
        if (strlen(s_runstate.system.ipv6_address[i]) == 0) {
            // no more entry
            sprintf(buf + p, "\n");
            return HUAYI_OK;
        }
        p += sprintf(buf + p, "'%s'\n",
                     s_runstate.system.ipv6_address[i]);
    }
    return HUAYI_OK;
}

HUAYI_STATUS statusSet_ipv6_address(char* buf) {
    int i;
    u_int32_t value = memcmp(buf, DEFAULT_IPV6_ADDR, strlen(DEFAULT_IPV6_ADDR));
    if (value == 0 || strlen(buf) == 0) {
		for (i=0; i< CSTATUS_MAX_IPV6_ADDR; i++)
        		BZERO(s_runstate.system.ipv6_address[i], CSTATUS_MAX_IPV6_ADDR_LEN);
        return HUAYI_OK;
    }
	value = strlen(buf);
    for (i=0; i < CSTATUS_MAX_IPV6_ADDR; i++) {
        if (strlen(s_runstate.system.ipv6_address[i]) == 0) {
		memcpy(s_runstate.system.ipv6_address[i], buf, value);
            return HUAYI_OK;
        }
    }
    return HUAYI_ERROR;
}

HUAYI_STATUS statusGet_default_gw_v6(char* buf) {
    sprintf(buf, "'%s'", s_runstate.system.default_gw_ipv6);
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_default_gw_v6(char* buf) {
	BZERO(s_runstate.system.default_gw_ipv6, CSTATUS_MAX_IPV6_ADDR_LEN);
	memcpy(s_runstate.system.default_gw_ipv6, buf, strlen(buf));
    return HUAYI_OK;
}


HUAYI_STATUS statusGet_dns_server_v6(char* buf) {
    int i;
    int p=0;
    for (i=0; i < CSTATUS_MAX_IPV6_DNS_ADDR; i++) {
        if (strlen(s_runstate.system.dns_server_ipv6[i]) == 0) {
            // no more entry
            sprintf(buf + p, "\n");
            return HUAYI_OK;
        }
        p += sprintf(buf + p, "'%s'\n",
                     s_runstate.system.dns_server_ipv6[i]);
    }
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_dns_server_v6(char* buf) {
    int i;
    u_int32_t value=0; 
	value = memcmp(buf, DEFAULT_IPV6_ADDR, strlen(DEFAULT_IPV6_ADDR));
    if (value == 0 || strlen(buf) == 0) {
        for (i=0; i< CSTATUS_MAX_IPV6_DNS_ADDR; i++)
            BZERO(s_runstate.system.dns_server_ipv6[i], CSTATUS_MAX_IPV6_ADDR_LEN);
        return HUAYI_OK;
    }
    value = strlen(buf);
    for (i=0; i< CSTATUS_MAX_IPV6_DNS_ADDR; i++) {
        if (strcmp(s_runstate.system.dns_server_ipv6[i], buf) == 0) {
            // already added
            return HUAYI_OK;
        } else if (strlen(s_runstate.system.dns_server_ipv6[i]) == 0) {
            // find the last one (there is no hold in the array)
            memcpy(s_runstate.system.dns_server_ipv6[i], buf, value);
            return HUAYI_OK;
        }
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_upgrade_keep_setting(int *buf) {
    *buf = s_runstate.system.upgrade_keep_setting;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_upgrade_keep_setting(int buf) {
    s_runstate.system.upgrade_keep_setting = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_usb_storage(int *buf) {
    *buf = s_runstate.system.usb_storage;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_usb_storage(int buf) {
    s_runstate.system.usb_storage = buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_ping_detect(int *buf) {
    *buf = s_runstate.system.ping_detect;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_ping_detect(int buf) {
    s_runstate.system.ping_detect = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_tracert_detect(int *buf) {
    *buf = s_runstate.system.tracert_detect;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_tracert_detect(int buf) {
    s_runstate.system.tracert_detect = buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_dhcp_detect(int *buf) {
    *buf = s_runstate.system.dhcp_detect;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_dhcp_detect(int buf) {
    s_runstate.system.dhcp_detect = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_arp_flush(int *buf) {
    *buf = s_runstate.system.arp_flush;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_arp_flush(int buf) {
    s_runstate.system.arp_flush = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wifi_off(int *buf) {
    *buf = s_runstate.system.wifi_off;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wifi_off(int buf) {
    s_runstate.system.wifi_off = buf;
    return HUAYI_OK;
}

#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
static HUAYI_STATUS statusGet_ddns_state(int *buf) {
    *buf = s_runstate.system.ddns_state;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_ddns_state(int buf) {
    s_runstate.system.ddns_state = buf;
    return HUAYI_OK;
}

HUAYI_STATUS statusGet_ddns_ip(char* buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.system.ddns_ip));
	return HUAYI_OK;
   
}

static HUAYI_STATUS statusSet_ddns_ip(char* buf) {
     s_runstate.system.ddns_ip= A_INET_ADDR(buf);
	 return HUAYI_OK;
}
#endif

static HUAYI_STATUS statusGet_wan_proto(int *buf) {
    *buf = s_runstate.system.wan_proto;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_proto(int buf) {
    s_runstate.system.wan_proto = buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_scan_state(int *buf) {
    *buf = s_runstate.system.scan_state;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_scan_state(int buf) {
    s_runstate.system.scan_state = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_rep_24g_status(int *buf) {
    *buf = s_runstate.system.rep_24g_status;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_rep_24g_status(int buf) {
    s_runstate.system.rep_24g_status= buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_rep_58g_status(int *buf) {
    *buf = s_runstate.system.rep_58g_status;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_rep_58g_status(int buf) {
    s_runstate.system.rep_58g_status= buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wifi_24g_num(int *buf) {
    *buf = s_runstate.system.wifi_24g_num;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wifi_24g_num(int buf) {
    s_runstate.system.wifi_24g_num= buf;
    return HUAYI_OK;
}


static HUAYI_STATUS statusGet_wifi_58g_num(int *buf) {
    *buf = s_runstate.system.wifi_58g_num;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wifi_58g_num(int buf) {
    s_runstate.system.wifi_58g_num= buf;
    return HUAYI_OK;
}


#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
static HUAYI_STATUS statusGet_ac_ipaddr(char* buf) {
	sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.system.ac_ipaddr));
	return HUAYI_OK;
}

static HUAYI_STATUS statusSet_ac_ipaddr(char* buf) {
	s_runstate.system.ac_ipaddr= A_INET_ADDR(buf);
	return HUAYI_OK;
}

static HUAYI_STATUS statusGet_capwap_client_status(int *buf) {
    *buf = s_runstate.system.capwap_client_status;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_capwap_client_status(int buf) {
    s_runstate.system.capwap_client_status = buf;
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_ac_assoc_time(int *buf) {
    *buf = s_runstate.system.ac_assoc_time;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_ac_assoc_time(int buf) {
    s_runstate.system.ac_assoc_time = buf;
    return HUAYI_OK;
}
#endif

#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
static HUAYI_STATUS statusGet_wifi_button(int *buf) {
    *buf = s_runstate.system.wifi_button;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wifi_button(int buf) {
    s_runstate.system.wifi_button = buf;
    return HUAYI_OK;
}
#endif

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
static HUAYI_STATUS statusGet_pptp_link(int *buf) {
    *buf = s_runstate.system.pptp_link;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_pptp_link(int buf) {
    s_runstate.system.pptp_link = buf;
    return HUAYI_OK;
}

HUAYI_STATUS statusGet_pptp_ip(char* buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.system.pptp_ip));
	return HUAYI_OK;
   
}

static HUAYI_STATUS statusSet_pptp_ip(char* buf) {
     s_runstate.system.pptp_ip= A_INET_ADDR(buf);
	 return HUAYI_OK;
}

static HUAYI_STATUS statusGet_xl2tp_link(int *buf) {
    *buf = s_runstate.system.xl2tp_link;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_xl2tp_link(int buf) {
    s_runstate.system.xl2tp_link = buf;
    return HUAYI_OK;
}

HUAYI_STATUS statusGet_xl2tp_ip(char* buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.system.xl2tp_ip));
	return HUAYI_OK;
   
}

static HUAYI_STATUS statusSet_xl2tp_ip(char* buf) {
     s_runstate.system.xl2tp_ip= A_INET_ADDR(buf);
	 return HUAYI_OK;
}

#endif


static control_tbl status_system_control_tbl[] = {
	{"rec_uptime",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_rec_uptime), s(statusSet_rec_uptime)},
    {"cpu_rt_usage",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_cpu_rt_usage), s(statusSet_cpu_rt_usage)},
	{"cpu_avg_usage",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_cpu_avg_usage), s(statusSet_cpu_avg_usage)},
    {"mem_avg_usage",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_mem_avg_usage), s(statusSet_mem_avg_usage)},
	{"flash_usage",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_flash_usage), s(statusSet_flash_usage)},
    {"ipv6_address", TBL_FORMAT_S,
        g(statusGet_ipv6_address), s(statusSet_ipv6_address)},
    {"ipv6_default_gw", TBL_FORMAT_S,
        g(statusGet_default_gw_v6), s(statusSet_default_gw_v6)}, 
    {"ipv6_dns_server", TBL_FORMAT_S,
        g(statusGet_dns_server_v6), s(statusSet_dns_server_v6)},
    {"upgrade_keep_setting",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_upgrade_keep_setting), s(statusSet_upgrade_keep_setting)},
    {"usb_storage",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_usb_storage), s(statusSet_usb_storage)},
    {"ping_detect",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_ping_detect), s(statusSet_ping_detect)},
    {"tracert_detect",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_tracert_detect), s(statusSet_tracert_detect)},
    {"dhcp_detect",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_dhcp_detect), s(statusSet_dhcp_detect)},
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
    {"ddns_state",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_ddns_state), s(statusSet_ddns_state)},
    {"ddns_ip",  TBL_FORMAT_S,
        g(statusGet_ddns_ip), s(statusSet_ddns_ip)},
#endif
    {"wan_proto",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wan_proto), s(statusSet_wan_proto)},
    {"scan_state",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_scan_state), s(statusSet_scan_state)},
    {"rep_24g_status",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_rep_24g_status), s(statusSet_rep_24g_status)},
    {"rep_58g_status",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_rep_58g_status), s(statusSet_rep_58g_status)},
    {"wifi_24g_num",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wifi_24g_num), s(statusSet_wifi_24g_num)},
    {"wifi_58g_num",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wifi_58g_num), s(statusSet_wifi_58g_num)},
#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
    {"ac_ipaddr",  TBL_FORMAT_S,
        g(statusGet_ac_ipaddr), s(statusSet_ac_ipaddr)},
    {"capwap_client_status",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_capwap_client_status), s(statusSet_capwap_client_status)},
    {"ac_assoc_time",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_ac_assoc_time), s(statusSet_ac_assoc_time)},
#endif
#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
	{"wifi_button",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wifi_button), s(statusSet_wifi_button)},
#endif
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	{"pptp_link",  TBL_FORMAT_D | TBL_TYPE_INT,
		g(statusGet_pptp_link), s(statusSet_pptp_link)},
	{"pptp_ip",  TBL_FORMAT_S,
		g(statusGet_pptp_ip), s(statusSet_pptp_ip)},
	{"xl2tp_link",	TBL_FORMAT_D | TBL_TYPE_INT,
	   g(statusGet_xl2tp_link), s(statusSet_xl2tp_link)},
	{"xl2tp_ip",  TBL_FORMAT_S,
		g(statusGet_xl2tp_ip), s(statusSet_xl2tp_ip)},
#endif
	{"arp_flush",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_arp_flush), s(statusSet_arp_flush)},
    {"wifi_off",  TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wifi_off), s(statusSet_wifi_off)},
    {NULL, 0, NULL, NULL}
};

struct huayi_runstate *HUAYI_STATUS_get(void*addr, int len)
{
    return &s_runstate;
}
//EXPORT_SYMBOL(HUAYI_STATUS_get);

static HUAYI_STATUS statusGet_lan_device(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.network.lan_device);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lan_device(char *buf) {
    if (buf) {
        snprintf(s_runstate.network.lan_device, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lan_ipv4(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.lan_ipv4));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_lan_ipv4(char *buf) {
   s_runstate.network.lan_ipv4=A_INET_ADDR(buf);
   return HUAYI_OK;
}


static HUAYI_STATUS statusGet_lan_netmask(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.lan_netmask));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_lan_netmask(char *buf) {
   s_runstate.network.lan_netmask=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_lan_gateway(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.lan_gateway));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_lan_gateway(char *buf) {
   s_runstate.network.lan_gateway=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_lan_linkport(int *buf) {
	*buf = s_runstate.network.linkport;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_lan_linkport(int buf) {
   struct timespec tp;
   
   s_runstate.network.linkport = buf;
   
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wan_ipv4(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.wan_ipv4));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_ipv4(char *buf) {
   s_runstate.network.wan_ipv4=A_INET_ADDR(buf);
   return HUAYI_OK;
}


static HUAYI_STATUS statusGet_wan_netmask(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.wan_netmask));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_netmask(char *buf) {
   s_runstate.network.wan_netmask=A_INET_ADDR(buf);
   return HUAYI_OK;
}


static HUAYI_STATUS statusGet_wan_gateway(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.network.wan_gateway));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_gateway(char *buf) {
   s_runstate.network.wan_gateway=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wan_link(int *buf) {
	*buf = s_runstate.network.wan_link;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_link(int buf) {
   s_runstate.network.wan_link = buf;
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wan_dns_server(char *buf) {
    int i;
    int p=0;
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.network.wan_dns_server[i] == 0) {
            return HUAYI_OK;
        }
        p += sprintf(buf + p, "'"NIPQUAD_FMT"'\n",
                     NIPQUAD(s_runstate.network.wan_dns_server[i]));
    }
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_dns_server(char *buf) {
    int i;
    u_int32_t value = A_INET_ADDR(buf);
	
    if (value == 0) {
        BZERO(&s_runstate.network.wan_dns_server,sizeof(s_runstate.network.wan_dns_server));
        return HUAYI_OK;
    }
	
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.network.wan_dns_server[i] == value) {
            return HUAYI_OK;
        } else if (s_runstate.network.wan_dns_server[i] == 0) {
            s_runstate.network.wan_dns_server[i] = value;
            return HUAYI_OK;
        }
    }
	
    return HUAYI_ERROR;
}	

static HUAYI_STATUS statusGet_wan_rx_bps(int *buf) {
	*buf = s_runstate.network.wan_rx_bps;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_rx_bps(int buf) {
   s_runstate.network.wan_rx_bps = buf;
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wan_tx_bps(int *buf) {
	*buf=s_runstate.network.wan_tx_bps;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_tx_bps(int buf) {
   s_runstate.network.wan_tx_bps=buf;
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wan_device(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.network.wan_device);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_wan_device(char *buf) {
    if (buf) {
        snprintf(s_runstate.network.wan_device, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_wan_up(int *buf) {
	*buf = s_runstate.network.wan_up;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wan_up(int buf) {
   struct timespec tp;
   
   s_runstate.network.wan_up = buf;
   if(buf ==0)
   {
   		s_runstate.network.wan_uptime = 0;
   }else if(buf ==1)
   {	
   		ktime_get_ts(&tp);
   		s_runstate.network.wan_uptime =tp.tv_sec;
   }
   
   return HUAYI_OK;
}


static control_tbl status_network_control_tbl[] = {
    {"lan_device", TBL_FORMAT_S,
        g(statusGet_lan_device), s(statusSet_lan_device)},
	{"lan_ipv4", TBL_FORMAT_S,
        g(statusGet_lan_ipv4), s(statusSet_lan_ipv4)},
    {"lan_netmask", TBL_FORMAT_S,
        g(statusGet_lan_netmask), s(statusSet_lan_netmask)},
    {"lan_gateway", TBL_FORMAT_S,
        g(statusGet_lan_gateway), s(statusSet_lan_gateway)},
    {"linkport", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_lan_linkport), s(statusSet_lan_linkport)},
    {"wan_ipv4", TBL_FORMAT_S,
        g(statusGet_wan_ipv4), s(statusSet_wan_ipv4)},
    {"wan_netmask", TBL_FORMAT_S,
        g(statusGet_wan_netmask), s(statusSet_wan_netmask)},
    {"wan_gateway", TBL_FORMAT_S,
        g(statusGet_wan_gateway), s(statusSet_wan_gateway)},
    {"wan_link", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wan_link), s(statusSet_wan_link)},
    {"wan_dns_server", TBL_FORMAT_S,
        g(statusGet_wan_dns_server), s(statusSet_wan_dns_server)},
    {"wan_rx_bps", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wan_rx_bps), s(statusSet_wan_rx_bps)},
    {"wan_tx_bps", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wan_tx_bps), s(statusSet_wan_tx_bps)},
    {"wan_device", TBL_FORMAT_S,
        g(statusGet_wan_device), s(statusSet_wan_device)},
    {"wan_up", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wan_up), s(statusSet_wan_up)},
    {NULL, 0, NULL, NULL}
};

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
static HUAYI_STATUS statusGet_lte_modemModel(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.modemModel);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_modemModel(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.modemModel, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_registStatus(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.registStatus);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_registStatus(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.registStatus, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_imei(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.imei);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_imei(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.imei, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_imsi(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.imsi);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_imsi(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.imsi, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_phoneNumber(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.phoneNumber);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_phoneNumber(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.phoneNumber, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_center_number(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.center_number);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_center_number(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.center_number, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_lte_bandLte(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.bandLte);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_bandLte(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.bandLte, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_signal(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.signal);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_signal(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.signal, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_lteMode(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.lteMode);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_lteMode(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.lteMode, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_sim_ready(int *buf) {
    *buf = s_runstate.modem.sim_ready;
	
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_lte_sim_ready(int buf) {
	s_runstate.modem.sim_ready = buf;
    
    return HUAYI_OK;
}

static HUAYI_STATUS statusGet_lte_network(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.network);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_network(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.network, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_mobile_type(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.mobile_type);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_mobile_type(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.mobile_type, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_operator(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.operator);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_operator(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.operator, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_lte_iccid(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.iccid);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_iccid(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.iccid, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_lte_lac(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.lac);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_lac(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.lac, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_lte_ci(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.ci);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_ci(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.ci, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_lte_dialresetcount(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.modem.dialresetcount);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_lte_dialresetcount(char *buf) {
    if (buf) {
        snprintf(s_runstate.modem.dialresetcount, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}



static control_tbl status_lte_modem_tbl[] = {
    {"modemModel", TBL_FORMAT_S,
        g(statusGet_lte_modemModel), s(statusSet_lte_modemModel)},
	{"registStatus", TBL_FORMAT_S,
        g(statusGet_lte_registStatus), s(statusSet_lte_registStatus)},
    {"imei", TBL_FORMAT_S,
        g(statusGet_lte_imei), s(statusSet_lte_imei)},
    {"imsi", TBL_FORMAT_S,
        g(statusGet_lte_imsi), s(statusSet_lte_imsi)},
    {"phoneNumber", TBL_FORMAT_S,
        g(statusGet_lte_phoneNumber), s(statusSet_lte_phoneNumber)},
    {"center_number", TBL_FORMAT_S,
        g(statusGet_lte_center_number), s(statusSet_lte_center_number)},
    {"bandLte", TBL_FORMAT_S,
        g(statusGet_lte_bandLte), s(statusSet_lte_bandLte)},
    {"signal", TBL_FORMAT_S,
        g(statusGet_lte_signal), s(statusSet_lte_signal)},
    {"sim_ready", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_lte_sim_ready), s(statusSet_lte_sim_ready)},
    {"network", TBL_FORMAT_S,
        g(statusGet_lte_network), s(statusSet_lte_network)},
    {"mobile_type", TBL_FORMAT_S,
        g(statusGet_lte_mobile_type), s(statusSet_lte_mobile_type)},
    {"lteMode", TBL_FORMAT_S,        
    	g(statusGet_lte_lteMode), s(statusSet_lte_lteMode)},
    {"operator", TBL_FORMAT_S,        
    	g(statusGet_lte_operator), s(statusSet_lte_operator)},
    {"iccid", TBL_FORMAT_S,        
    	g(statusGet_lte_iccid), s(statusSet_lte_iccid)},
    {"lac", TBL_FORMAT_S,        
    	g(statusGet_lte_lac), s(statusSet_lte_lac)},
    {"ci", TBL_FORMAT_S,        
    	g(statusGet_lte_ci), s(statusSet_lte_ci)},
    {"dialresetcount", TBL_FORMAT_S,        
    	g(statusGet_lte_dialresetcount), s(statusSet_lte_dialresetcount)},

    {NULL, 0, NULL, NULL}
};
#endif

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
static HUAYI_STATUS statusGet_wired_device(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.blend.wired_device_id);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_wired_device(char *buf) {
    if (buf) {
        snprintf(s_runstate.blend.wired_device_id, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_wired_ipv4(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.wired_ipv4_addr));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wired_ipv4(char *buf) {
   s_runstate.blend.wired_ipv4_addr=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wired_netmask(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.wired_ipv4_mask));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wired_netmask(char *buf) {
   s_runstate.blend.wired_ipv4_mask=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wired_gateway(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.wired_gateway));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wired_gateway(char *buf) {
   s_runstate.blend.wired_gateway=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wired_link(int *buf) {
	*buf = s_runstate.blend.wired_link;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wired_link(int buf) {
   s_runstate.blend.wired_link = buf;
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_wired_dns_server(char *buf) {
    int i;
    int p=0;
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.blend.wired_dns_server[i] == 0) {
            return HUAYI_OK;
        }
        p += sprintf(buf + p, "'"NIPQUAD_FMT"'\n",
                     NIPQUAD(s_runstate.blend.wired_dns_server[i]));
    }
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_wired_dns_server(char *buf) {
    int i;
    u_int32_t value = A_INET_ADDR(buf);
	
    if (value == 0) {
        BZERO(&s_runstate.blend.wired_dns_server,sizeof(s_runstate.blend.wired_dns_server));
        return HUAYI_OK;
    }
	
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.blend.wired_dns_server[i] == value) {
            return HUAYI_OK;
        } else if (s_runstate.blend.wired_dns_server[i] == 0) {
            s_runstate.blend.wired_dns_server[i] = value;
            return HUAYI_OK;
        }
    }
	
    return HUAYI_ERROR;
}


static HUAYI_STATUS statusGet_cpe_device(char *buf) {
    if (buf) {
        snprintf(buf, CSTATUS_MAX_STRING, "%s", s_runstate.blend.cpe_device_id);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusSet_cpe_device(char *buf) {
    if (buf) {
        snprintf(s_runstate.blend.cpe_device_id, CSTATUS_MAX_STRING, "%s", buf);
        return HUAYI_OK;
    }
    return HUAYI_ERROR;
}

static HUAYI_STATUS statusGet_cpe_ipv4(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.cpe_ipv4_addr));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpe_ipv4(char *buf) {
   s_runstate.blend.cpe_ipv4_addr=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpe_netmask(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.cpe_ipv4_mask));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpe_netmask(char *buf) {
   s_runstate.blend.cpe_ipv4_mask=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpe_gateway(char *buf) {
    sprintf(buf, "'"NIPQUAD_FMT"'", NIPQUAD(s_runstate.blend.cpe_gateway));
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpe_gateway(char *buf) {
   s_runstate.blend.cpe_gateway=A_INET_ADDR(buf);
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpe_link(int *buf) {
	*buf = s_runstate.blend.cpe_link;
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpe_link(int buf) {
   s_runstate.blend.cpe_link = buf;
   return HUAYI_OK;
}

static HUAYI_STATUS statusGet_cpe_dns_server(char *buf) {
    int i;
    int p=0;
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.blend.cpe_dns_server[i] == 0) {
            return HUAYI_OK;
        }
        p += sprintf(buf + p, "'"NIPQUAD_FMT"'\n",
                     NIPQUAD(s_runstate.blend.cpe_dns_server[i]));
    }
    return HUAYI_OK;
}

static HUAYI_STATUS statusSet_cpe_dns_server(char *buf) {
    int i;
    u_int32_t value = A_INET_ADDR(buf);
	
    if (value == 0) {
        BZERO(&s_runstate.blend.wired_dns_server,sizeof(s_runstate.blend.cpe_dns_server));
        return HUAYI_OK;
    }
	
    for (i=0; i<CSTATUS_MAX_DNS_SERVER; i++) {
        if (s_runstate.blend.cpe_dns_server[i] == value) {
            return HUAYI_OK;
        } else if (s_runstate.blend.cpe_dns_server[i] == 0) {
            s_runstate.blend.cpe_dns_server[i] = value;
            return HUAYI_OK;
        }
    }
	
    return HUAYI_ERROR;
}

static control_tbl status_blend_control_tbl[] = {
	{"wired_device", TBL_FORMAT_S,
        g(statusGet_wired_device), s(statusSet_wired_device)},
	{"wired_ipv4", TBL_FORMAT_S,
        g(statusGet_wired_ipv4), s(statusSet_wired_ipv4)},
    {"wired_netmask", TBL_FORMAT_S,
        g(statusGet_wired_netmask), s(statusSet_wired_netmask)},
    {"wired_gateway", TBL_FORMAT_S,
        g(statusGet_wired_gateway), s(statusSet_wired_gateway)},
    {"wired_link", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_wired_link), s(statusSet_wired_link)},
    {"wired_dns_server", TBL_FORMAT_S,
        g(statusGet_wired_dns_server), s(statusSet_wired_dns_server)},

	{"cpe_device", TBL_FORMAT_S,
        g(statusGet_cpe_device), s(statusSet_cpe_device)},
	{"cpe_ipv4", TBL_FORMAT_S,
        g(statusGet_cpe_ipv4), s(statusSet_cpe_ipv4)},
    {"cpe_netmask", TBL_FORMAT_S,
        g(statusGet_cpe_netmask), s(statusSet_cpe_netmask)},
    {"cpe_gateway", TBL_FORMAT_S,
        g(statusGet_cpe_gateway), s(statusSet_cpe_gateway)},
    {"cpe_link", TBL_FORMAT_D | TBL_TYPE_INT,
        g(statusGet_cpe_link), s(statusSet_cpe_link)},
    {"cpe_dns_server", TBL_FORMAT_S,
        g(statusGet_cpe_dns_server), s(statusSet_cpe_dns_server)},
    {NULL, 0, NULL, NULL}
};
#endif

static int HUAYI_STATUS_init(struct huayi_runstate *srunstate) 
{

    BZERO(srunstate, sizeof(struct huayi_runstate));
    HUAYI_CTRL_ATTACH_TABLE(HUAYI_MODULE_STATUS_ABOUT, status_about_control_tbl);
	HUAYI_CTRL_ATTACH_TABLE(HUAYI_MODULE_STATUS_SYSTEM, status_system_control_tbl);
    HUAYI_CTRL_ATTACH_TABLE(HUAYI_MODULE_STATUS_NETWORK, status_network_control_tbl);
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	HUAYI_CTRL_ATTACH_TABLE(HUAYI_MODULE_STATUS_MODEM, status_lte_modem_tbl);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	HUAYI_CTRL_ATTACH_TABLE(HUAYI_MODULE_STATUS_BLEND, status_blend_control_tbl);
#endif
    return 0;
}

static int HUAYI_STATUS_exit(struct huayi_runstate *srunstate) 
{
    HUAYI_CTRL_DETACH_TABLE(HUAYI_MODULE_STATUS_ABOUT);
	HUAYI_CTRL_DETACH_TABLE(HUAYI_MODULE_STATUS_SYSTEM);
    HUAYI_CTRL_DETACH_TABLE(HUAYI_MODULE_STATUS_NETWORK);
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	HUAYI_CTRL_DETACH_TABLE(HUAYI_MODULE_STATUS_MODEM);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	HUAYI_CTRL_DETACH_TABLE(HUAYI_MODULE_STATUS_BLEND);
#endif
    return 0;
}

static int32_t __init module_huayi_runstate_init(void){
    HUAYI_STATUS_init(&s_runstate);
   	HUAYI_STATUS_dal_reg_hook((dal_sys_mod_ops)HUAYI_STATUS_get);
    //printk("huayi runstate module init\n");
    return 0;
}
module_init(module_huayi_runstate_init);

static void __exit module_huayi_runstate_exit(void){
    HUAYI_STATUS_dal_unreg_hook();
    HUAYI_STATUS_exit(&s_runstate);
    //printk("huayi runstate module exit\n");
    return;
}
module_exit(module_huayi_runstate_exit);

