/*=========================================================================*
 * FileName: huayi-kalarm.c
 * Author: hpluo
 * Description: Alarm Kernel Module
 * Version: 1
 *    Notify if there are events via huayi-events
 * Modified History:
 *=========================================================================*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <huayi-universal/huayi_universal.h>
#include <huayi-universal/huayi_types.h>
#include <huayi-universal/huayi_events.h>
#include <huayi-universal/huayi_dal.h>
#include "huayi-kalarm.h"


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kAlarm Module");
MODULE_AUTHOR("gerry_li");

extern struct timezone sys_tz;
static int V = 0; /* 0: donot print debug info(1. update status no support.. 2. notification)
                   * otherwise: print those two debug info
                   */
module_param(V, int, S_IRUGO);
/*====================
 * Member Declarations
 *====================*/

static struct huayi_kalarm s_kalarm;

static char s_alarm_syslog[256];
static void kalarm_alarm_tosyslog(const struct huayi_notification *e, char* syslog) {
    switch (e->notification_id) {
    case CNOTE_STA_ASSOCIATION:
        snprintf(syslog, 256, "[%s:%d] %02x:%02x:%02x:%02x:%02x:%02x [%s:%d] [%d %d %d %d]",
                 e->info.sta_association.device_id,
                 e->info.sta_association.vap_id,
                 e->info.sta_association.client_macaddr[0],
                 e->info.sta_association.client_macaddr[1],
                 e->info.sta_association.client_macaddr[2],
                 e->info.sta_association.client_macaddr[3],
                 e->info.sta_association.client_macaddr[4],
                 e->info.sta_association.client_macaddr[5],
                 HUAYI_NOTE_ASSOSTAT_TOSTR(e->info.sta_association.status),
                 e->info.sta_association.reason_code,
                 e->info.sta_association.auth_mode,
                 e->info.sta_association.channel,
                 e->info.sta_association.rssi,
                 e->info.sta_association.qos);
        break;
	#if 0
    case CNOTE_RADIO_DOWN:
        snprintf(syslog, 256, "ap_macaddr:%02x:%02x:%02x:%02x:%02x:%02x device_id:%s reason:%s",
                 e->info.radio_down.ap_macaddr[0],
                 e->info.radio_down.ap_macaddr[1],
                 e->info.radio_down.ap_macaddr[2],
                 e->info.radio_down.ap_macaddr[3],
                 e->info.radio_down.ap_macaddr[4],
                 e->info.radio_down.ap_macaddr[5],
                 e->info.radio_down.device_id,
                 e->info.radio_down.reason);
        break;
    case CNOTE_RADIO_DOWN_RECOV:
        snprintf(syslog, 256, "ap_macaddr:%02x:%02x:%02x:%02x:%02x:%02x device_id:%s reason:%s",
                 e->info.radio_down_recov.ap_macaddr[0],
                 e->info.radio_down_recov.ap_macaddr[1],
                 e->info.radio_down_recov.ap_macaddr[2],
                 e->info.radio_down_recov.ap_macaddr[3],
                 e->info.radio_down_recov.ap_macaddr[4],
                 e->info.radio_down_recov.ap_macaddr[5],
                 e->info.radio_down_recov.device_id,
                 e->info.radio_down_recov.reason);
        break;
	#endif
    default:
        snprintf(syslog, 256, "-");
        break;
    }
    return;
}

static void kalarm_syslog(const struct huayi_notification *e) {
    switch (e->notification_id) {
    case CNOTE_STA_ASSOCIATION:
        ALARM_LOG(HUAYILOG_INFO, "Asso - %s\n", s_alarm_syslog);
        break;
    }
    return;
}

#if 0
static int kalarm_update_alarm(const struct huayi_notification *e) {
    struct list_head *ptr, *n;
    struct huayi_kalarm_alarm *entry;

    switch (e->notification_id) {
	case CNOTE_STA_ASSOCIATION:
		break;
	case CNOTE_SYS_REBOOT:
		break;
	case CNOTE_RADIO_DOWN:
		/* raise alarm - device level */
		list_for_each(ptr, &s_kalarm.list_alarm) {
			entry = list_entry(ptr, struct huayi_kalarm_alarm, list);
			if (e->notification_id == entry->alarm.notification_id &&
				strcmp(e->info.device.device_id, entry->alarm.info.device.device_id) == 0) {
				// alarm already raised
				KALARM_LOG(HUAYILOG_INFO, "alarm already raised(%d) device(%s)\n",
						   e->notification_id, e->info.device.device_id);
				return 0;
			}
		}
		entry = HUAYI_MALLOC(sizeof(struct huayi_kalarm_alarm));
		BCOPY((void*)e, (void*)&entry->alarm, sizeof(struct huayi_notification));
		entry->alarm_id = s_kalarm.curr_alarm_id;
		snprintf(entry->alarm.info.device.device_id, CNOTE_MAX_DEVICE_ID,
				 "%s", e->info.device.device_id);
		s_kalarm.curr_alarm_id++;
		s_kalarm.active++;
		list_add_tail(&entry->list, &s_kalarm.list_alarm);
		ALARM_LOG(HUAYILOG_INFO, "Alarm Raise - %s(%d) - %s\n",
				  HUAYI_NOTE_ID_TOSTR(e->notification_id), e->notification_id, s_alarm_syslog);
		break;
		
    case CNOTE_RADIO_DOWN_RECOV:
        /* clear alarm - device level */
        list_for_each_safe(ptr, n, &s_kalarm.list_alarm) {
            entry = list_entry(ptr, struct huayi_kalarm_alarm, list);
            // +1 match the raise event
            if (e->notification_id == (entry->alarm.notification_id+1) &&
                strcmp(e->info.device.device_id, entry->alarm.info.device.device_id) == 0) {
                // alarm found
                s_kalarm.active--;
                list_del(&entry->list);
                HUAYI_FREE(entry);
                ALARM_LOG(HUAYILOG_INFO, "Alarm Clear - %s(%d) - %s\n",
                          HUAYI_NOTE_ID_TOSTR(e->notification_id), e->notification_id, s_alarm_syslog);
                break;
            }
        }
        if (ptr == &s_kalarm.list_alarm) {
            // alarm already cleared
            KALARM_LOG(HUAYILOG_INFO, "alarm already cleared(%d)\n",
                       e->notification_id);
            return 0;
        }
        break;

	case CNOTE_WIRELESS_INSTRUSION_DETECTED:
		printk("CNOTE_WIRELESS_INSTRUSION_DETECTED\n");
		list_for_each(ptr, &s_kalarm.list_alarm) {
			entry = list_entry(ptr, struct huayi_kalarm_alarm, list);
			if (e->notification_id == entry->alarm.notification_id &&
				(memcmp(e->info.wireless_instrusion_detected.attack_mac, entry->alarm.info.wireless_instrusion_detected.attack_mac, CNOTE_MAC_LEN) == 0)
				&& (memcmp(e->info.wireless_instrusion_detected.bssid, entry->alarm.info.wireless_instrusion_detected.bssid, CNOTE_MAC_LEN) == 0)) {
				// wireless_instrusion_detected already raised
				KALARM_LOG(HUAYILOG_INFO, "alarm already raised(%d) wireless_instrusio_attackmac(%s) bssid(%s)\n",
						   e->notification_id, e->info.wireless_instrusion_detected.attack_mac, \
						   e->info.wireless_instrusion_detected.bssid);
				return 0;
			}
		}
		entry = HUAYI_MALLOC(sizeof(struct huayi_kalarm_alarm));
		BCOPY((void*)e, (void*)&entry->alarm, sizeof(struct huayi_notification));
		entry->alarm_id = s_kalarm.curr_alarm_id;
		
		memcpy(entry->alarm.info.wireless_instrusion_detected.ap_macaddr, 
				e->info.wireless_instrusion_detected.ap_macaddr, CNOTE_MAC_LEN);
		snprintf(entry->alarm.info.wireless_instrusion_detected.device_id, CNOTE_MAX_DEVICE_ID,
				 "%s", e->info.wireless_instrusion_detected.device_id);
		memcpy(entry->alarm.info.wireless_instrusion_detected.bssid,
				e->info.wireless_instrusion_detected.bssid,  CNOTE_MAC_LEN);
		memcpy(entry->alarm.info.wireless_instrusion_detected.attack_mac, 
				e->info.wireless_instrusion_detected.attack_mac, CNOTE_MAC_LEN);
		entry->alarm.info.wireless_instrusion_detected.detect_type = \
							e->info.wireless_instrusion_detected.detect_type;
		
		s_kalarm.curr_alarm_id++;
		s_kalarm.active++;
		list_add_tail(&entry->list, &s_kalarm.list_alarm);
		ALARM_LOG(HUAYILOG_INFO, "wireless_instrusion_detected Raise - %s(%d) - %s\n",
				  HUAYI_NOTE_ID_TOSTR(e->notification_id), e->notification_id, s_alarm_syslog);
		break;

	case CNOTE_BUTTON_PRESS:
		break;

	case CNOTE_BUTTON_RELEASE:
		break;
	case CNOTE_AP_LOCATION_BEGIN:
		break;
	case CNOTE_AP_LOCATION_END:
		break;
    default:
        if (V) {
            KALARM_LOG(HUAYILOG_DEBUG, "update status not supported for %d(%s)\n",
                    e->notification_id, HUAYI_NOTE_ID_TOCMD(e->notification_id));
        }
        break;
    }
    return 0;
}
#endif

static DAL_STATUS kalarm_raise_notification(struct huayi_notification e) {
    int len;

    switch (e.notification_id) {
    case CNOTE_STA_ASSOCIATION:
        e.type = CNOTE_TYPE_STA;
        e.status = 0;
        break;
	case CNOTE_CHECK_IP_ADDR_CHG:
        e.type = CNOTE_TYPE_SYSTEM;
        e.status = 0;
        break;
	#if 0
    case CNOTE_SYS_REBOOT:
        e.type = CNOTE_TYPE_SYSTEM;
        e.status = 0;
        break;
    case CNOTE_RADIO_DOWN_RECOV:
        e.type = CNOTE_TYPE_WIRELESS;
        e.status = 1;
        break;
    case CNOTE_RADIO_DOWN:
        e.type = CNOTE_TYPE_WIRELESS;
        e.status = 0;
        break;
    case CNOTE_CHANNEL_SCAN_COMPLETED:
        e.type = CNOTE_TYPE_WIRELESS;
        e.status = 0;
        break;
    case CNOTE_WIRELESS_INSTRUSION_DETECTED:
        e.type = CNOTE_TYPE_WIRELESS;
        e.status = 0;
        break;
	case CNOTE_BUTTON_PRESS:
        e.type = CNOTE_TYPE_SYSTEM;
        e.status = 0;
        break;
	case CNOTE_BUTTON_RELEASE:
        e.type = CNOTE_TYPE_SYSTEM;
        e.status = 0;
        break;
	case CNOTE_AP_LOCATION_BEGIN:
		e.type = CNOTE_TYPE_SYSTEM;
		e.status = 0;
		break;
	case CNOTE_AP_LOCATION_END:
		e.type = CNOTE_TYPE_SYSTEM;
		e.status = 0;
		break;
	#endif
	case CNOTE_USB_STORAGE_ADD:
	case CNOTE_USB_STORAGE_REMOVE:
	case CNOTE_HUAYI_BUTTON_PRESSED:
	case CNOTE_HUAYI_BUTTON_RELEASED:
	case CNOTE_HUAYI_SCHE_REBOOT_UPDATE:
	case CNOTE_HUAYI_UPDATE_AC_CONFIG:
		e.type = CNOTE_TYPE_SYSTEM;
		e.status = 0;
		break;
    default:
        // not supported notification_id
        printk("notification ID %d (%s) not supported\n",
                   e.notification_id, HUAYI_NOTE_ID_TOCMD(e.notification_id));
        return DAL_ERROR;
    }

    kalarm_alarm_tosyslog(&e, s_alarm_syslog);

    kalarm_syslog(&e);

    // update alarm status
   // kalarm_update_alarm(&e);
    if (V) {
        printk("notification %d(%s)\n",
               e.notification_id, HUAYI_NOTE_ID_TOSTR(e.notification_id));
    }

    len = huayi_events_send(s_kalarm.event_handle,
                           HUAYI_EVENTS_NOTE, (char*)&e, sizeof(e));

    if (len != sizeof(e)) {
        printk("sending size(%d) does not match the event size(%u)\n",
                   len, sizeof(e));
    }
    return DAL_OK;
}

static ssize_t kalarm_proc_write(struct file *file, const char *buffer,
                                 size_t count, loff_t *data) {
    int ret;
    char str[HUAYI_PROC_MAX_BUFFER];
    struct huayi_notification e;
    char *pEnd;
    char *pStr;
    int nid;
    struct timex txc;

    BZERO((void*)&e, sizeof(struct huayi_notification));
	
    do_gettimeofday(&(txc.time));
	
    e.time = txc.time.tv_sec;
    ret = huayi_proc_write_string(file, buffer, count, str);
    nid = simple_strtol(str, &pEnd, 10);
    pStr = strsep(&pEnd, "\n");
    pEnd = pStr;
    switch(nid) {
    case CNOTE_USB_STORAGE_ADD:
   	case CNOTE_USB_STORAGE_REMOVE:
	case CNOTE_HUAYI_BUTTON_PRESSED:
	case CNOTE_HUAYI_BUTTON_RELEASED:
	case CNOTE_HUAYI_SCHE_REBOOT_UPDATE:
	case CNOTE_HUAYI_UPDATE_AC_CONFIG:
	case CNOTE_CHECK_IP_ADDR_CHG:
		// no parameters;
        break;
	#if 0
	case CNOTE_CHECK_IP_ADDR_CHG:
        pStr = strsep(&pEnd, " ");
        while (pStr != NULL) {
            if (*pStr != '-') {
                pStr = strsep(&pEnd, " ");
                continue;
            }
            pStr++;

            switch (*pStr) {
            case 'p':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    e.info.ip_addr_chg.pre_ip_addr = A_INET_ADDR(pStr);
                    pStr = strsep(&pEnd, " ");
                }
                break;
            case 'c':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    e.info.ip_addr_chg.cur_ip_addr = A_INET_ADDR(pStr);
                    pStr = strsep(&pEnd, " ");
                }
                break;
            default:
                pStr = strsep(&pEnd, " ");
                break;
            }
        }
        break;

    case CNOTE_HUAYI_BUTTON_EVENT:
        pStr = strsep(&pEnd, " ");
        while (pStr != NULL) {
            if (*pStr != '-') {
                pStr = strsep(&pEnd, " ");
                continue;
            }
            pStr++;

            switch (*pStr) {
            case 'a':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    sscanf(pStr, "%d", &(e.info.button_event.action));
                    pStr = strsep(&pEnd, " ");
                }
                break;
            default:
                pStr = strsep(&pEnd, " ");
                break;
            }
        }
        break;
	
    case CNOTE_WIRELESS_INSTRUSION_DETECTED:
        pStr = strsep(&pEnd, " ");
        while (pStr != NULL) {
            if (*pStr != '-') {
                pStr = strsep(&pEnd, " ");
                continue;
            }
            pStr++;

            switch (*pStr) {
            case 'm':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    A_MACADDR_FROMSTR((A_MACADDR*)e.info.wireless_instrusion_detected.ap_macaddr, pStr);
                    pStr = strsep(&pEnd, " ");
                }
                break;
            case 'd':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    snprintf(e.info.wireless_instrusion_detected.device_id, CNOTE_MAX_DEVICE_ID, "%s", pStr);
                    pStr = strsep(&pEnd, " ");
                }
                break;
		   case 'v':
	           pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    A_MACADDR_FROMSTR((A_MACADDR*)e.info.wireless_instrusion_detected.bssid, pStr);
                    pStr = strsep(&pEnd, " ");
                }
			case 's':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    A_MACADDR_FROMSTR((A_MACADDR*)e.info.wireless_instrusion_detected.attack_mac,pStr);
                    pStr = strsep(&pEnd, " ");
                }
                break;
            case 't':
                pStr = strsep(&pEnd, " ");
                if (pStr != NULL) {
                    sscanf(pStr, "%d", &(e.info.wireless_instrusion_detected.detect_type));
                    pStr = strsep(&pEnd, " ");
                }
            default:
                pStr = strsep(&pEnd, " ");
                break;
            }
        }
        break;
	#endif
    default:
        printk("non-supported notification %d (userspace)\n", nid);
        return ret;
    }
    e.notification_id = nid;
    kalarm_raise_notification(e);
    return ret;
}

static void *kalarm_seq_start(struct seq_file *seq, loff_t *pos)
{
    if (*pos == 0) {
        return SEQ_START_TOKEN;
    }

    if (*pos < CNOTE_MAX_ID) {
        return pos;
    }
    return NULL;
}
static void *kalarm_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    if (v == SEQ_START_TOKEN) {
        return pos;
    }
    ++*pos;
    if (*pos < CNOTE_MAX_ID) {
        return pos;
    }
    return NULL;
}
static void kalarm_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do (release), we use pos directly */
}
static int kalarm_seq_show(struct seq_file *s, void *v)
{
    loff_t *pos;

    if (v == SEQ_START_TOKEN) {
        // print header here
        return 0;
    }

    pos = (loff_t *) v;
    seq_printf(s, "%lld:%s:%s:%s\n", *pos,
               HUAYI_NOTE_ID_TOCMD(*pos),
               HUAYI_NOTE_ID_TOUSAGE(*pos),
               HUAYI_NOTE_ID_TOSTR(*pos));
    return 0;
}

static struct seq_operations kalarm_seq_ops = {
    .start = kalarm_seq_start,
    .next = kalarm_seq_next,
    .stop = kalarm_seq_stop,
    .show = kalarm_seq_show
};

int kalarm_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &kalarm_seq_ops);
}

static struct file_operations kalarm_proc_fops = {
    .owner = THIS_MODULE,
    .open = kalarm_proc_open,
    .read = seq_read,
    .write = kalarm_proc_write,
    .llseek = seq_lseek,
    .release = seq_release
};

#if 0
static void* seq_next_status_alarm(struct seq_file *s, void *v, loff_t *pos);
static void* seq_start_status_alarm(struct seq_file *s, loff_t *pos) {
    if (*pos == 0) {
        s->private = (void*)&(s_kalarm.list_alarm);
        return s->private;
    }
    return seq_next_status_alarm(s, NULL, pos);
}

static void* seq_next_status_alarm(struct seq_file *s, void *v, loff_t *pos) {
    struct list_head *ptr = (struct list_head*)s->private;

    if (ptr == NULL) {
        KALARM_LOG(HUAYILOG_ERR,"invalid ptr in status_alarm next\n");
        return NULL;
    }
    if (ptr->next == &(s_kalarm.list_alarm)) {
        // no next
        return NULL;
    }
    s->private = (void*)ptr->next;
    return s->private;
}

static void seq_stop_status_alarm(struct seq_file *s, void *v) {
}

static int seq_show_status_alarm(struct seq_file *s, void *v) {
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x) \
((u8*)(x))[0],((u8*)(x))[1],\
((u8*)(x))[2],((u8*)(x))[3],\
((u8*)(x))[4],((u8*)(x))[5]

    struct huayi_kalarm_alarm *entry;
    struct list_head *ptr = (struct list_head*)s->private;

    // from seq_start_status_alarm, print general
    if (ptr == &(s_kalarm.list_alarm)) {
        seq_printf(s, "config 'general' 'general'\n");
        seq_printf(s, "        option 'active' '%d'\n", s_kalarm.active);
        seq_printf(s, "\n");
        return 0;
    }

    if (ptr == NULL) {
        KALARM_LOG(HUAYILOG_ERR,"invalid ptr in status_alarm show\n");
        return 1;
    }
    entry = list_entry(ptr, struct huayi_kalarm_alarm, list);
    if (entry == NULL) {
        KALARM_LOG(HUAYILOG_ERR,"cannot get list data in status_alarm show\n");
        return 1;
    }

    seq_printf(s, "config 'alarm' 'alarm_%d'\n", entry->alarm_id);
    seq_printf(s, "        option 'id' '%d'\n", entry->alarm_id);
    seq_printf(s, "        option 'nid' '%d'\n", entry->alarm.notification_id);
    seq_printf(s, "        option 'description' '%s(%d)'\n",
               HUAYI_NOTE_ID_TOSTR(entry->alarm.notification_id),
               entry->alarm.notification_id);
    seq_printf(s, "        option 'type' '%d'\n", entry->alarm.type);
    seq_printf(s, "        option 'timestamp' '%ld'\n", entry->alarm.time);
    seq_printf(s, "        option 'status' '%lld'\n", entry->alarm.status);
    seq_printf(s, "        option 'cause' '%s'\n", entry->alarm.reason);

    switch (entry->alarm.notification_id) {
    case CNOTE_RADIO_DOWN:
        seq_printf(s, "        option 'ap_macaddr' '"MAC_FMT"'\n",
                   MAC_ARG(entry->alarm.info.radio_down.ap_macaddr));
        seq_printf(s, "        option 'device_id' '%s'\n",
                   entry->alarm.info.radio_down.device_id);
        seq_printf(s, "        option 'reason' '%s'\n",
                   entry->alarm.info.radio_down.reason);
        break;
#if 0	
    case CNOTE_WIRELESS_INSTRUSION_DETECTED:
        seq_printf(s, "        option 'ap_macaddr' '"MAC_FMT"'\n",
                   MAC_ARG(entry->alarm.info.wireless_instrusion_detected.ap_macaddr));
        seq_printf(s, "        option 'device_id' '%s'\n",
                   entry->alarm.info.wireless_instrusion_detected.device_id);
        seq_printf(s, "        option 'bssid' '"MAC_FMT"'\n",
                   MAC_ARG(entry->alarm.info.wireless_instrusion_detected.bssid));
        seq_printf(s, "        option 'attack_mac' '"MAC_FMT"'\n",
                   MAC_ARG(entry->alarm.info.wireless_instrusion_detected.attack_mac));
		
        seq_printf(s, "        option 'reason' '%d'\n",
                   entry->alarm.info.wireless_instrusion_detected.detect_type);
        break;
#endif

    default:
        KALARM_LOG("unsupported alarm in status_alarm show %d\n",
                   entry->alarm.notification_id);
        return 1;  // return error
    }
    seq_printf(s, "\n");

    // buffer full, move iterator backward
    if (s->count == s->size) {
        s->private = ptr->prev;
    }
    return 0;
#undef MAC_FMT
#undef MAC_ARG
}

static struct seq_operations seq_ops_status_alarm = {
    .start = seq_start_status_alarm,
    .next  = seq_next_status_alarm,
    .stop  = seq_stop_status_alarm,
    .show  = seq_show_status_alarm
};

static int fops_open_status_alarm(struct inode *inode, struct file *file) {
    return seq_open(file, &seq_ops_status_alarm);
}

static struct file_operations proc_fops_status_alarm = {
    .owner = THIS_MODULE,
    .open = fops_open_status_alarm,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

#endif

static struct dal_kalarm_hooks kalarm_hooks = {
    .raise_notification = kalarm_raise_notification,
};

static int kalarm_events_cb(char* buf, uint32_t len) {
    // not using
    return 1;
}

static void kalarm_init(void) {
    HUAYI_EVENTS_TYPE eid[1] = {HUAYI_EVENTS_NOTE};

	// alarm list
    s_kalarm.curr_alarm_id = 0;
    s_kalarm.active = 0;
    INIT_LIST_HEAD(&s_kalarm.list_alarm);
    // proc
    //s_kalarm.proc_kalarm = proc_create(HUAYI_MODULE_KALARM,
    //                                   HUAYI_PROC_PERM, huayi_proc,
    //                                   &kalarm_proc_fops);

	s_kalarm.proc_kalarm = proc_create_data(HUAYI_MODULE_KALARM,
											 HUAYI_PROC_PERM, huayi_proc, &kalarm_proc_fops, NULL);

    if (s_kalarm.proc_kalarm == NULL) {
        printk("Initialize /proc/%s/%s failed\n",
                   HUAYI_PROC_ROOT, HUAYI_MODULE_KALARM);
        return;
    }

#if 0
    //s_kalarm.proc_status = create_proc_entry(HUAYI_KALARM_PROC_STATUS,
    //                                  HUAYI_PROC_PERM, HUAYI_STATUS_proc);

	s_kalarm.proc_status = proc_create_data(HUAYI_KALARM_PROC_STATUS,
											 HUAYI_PROC_PERM, huayi_proc, &proc_fops_status_alarm, NULL);
	
    if (s_kalarm.proc_status == NULL) {
        printk("Initialize /proc/%s/%s failed\n",
                   HUAYI_PROC_ROOT, HUAYI_KALARM_PROC_STATUS);
        return;
    }
#endif
    // event hook
    s_kalarm.event_handle = huayi_events_create(kalarm_events_cb);
    huayi_events_add(eid, 1, s_kalarm.event_handle);
    dal_attach_kalarm_hooks(&kalarm_hooks);
    return;
}

static void kalarm_deinit(void) {
    struct huayi_kalarm_alarm* entry;

    // event hook
    HUAYI_EVENTS_TYPE eid[1] = {HUAYI_EVENTS_NOTE};
    huayi_events_del(eid, 1, s_kalarm.event_handle);
    huayi_events_destroy(s_kalarm.event_handle);
    dal_detach_kalarm_hooks();

    // proc
    if (s_kalarm.proc_status != NULL) {
        remove_proc_entry(HUAYI_KALARM_PROC_STATUS, huayi_proc);
        s_kalarm.proc_status = NULL;
    }
    if (s_kalarm.proc_kalarm != NULL) {
        remove_proc_entry(HUAYI_MODULE_KALARM, huayi_proc);
        s_kalarm.proc_kalarm = NULL;
    }

    // alarm list
    while (!list_empty(&(s_kalarm.list_alarm))) {
        entry = list_entry(s_kalarm.list_alarm.next,
                           struct huayi_kalarm_alarm, list);
        list_del(s_kalarm.list_alarm.next);
        HUAYI_FREE(entry);
    }

    return;
}

static int __init module_kalarm_init(void)
{
    kalarm_init();
	printk("Alarm Module Init\n");
    return 0;
}
module_init(module_kalarm_init);

static void __exit module_kalarm_exit(void)
{
    kalarm_deinit();
	printk("Alarm Module Exit\n");
    return;
}
module_exit(module_kalarm_exit);

