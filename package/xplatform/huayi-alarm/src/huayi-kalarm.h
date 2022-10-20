#ifndef HUAYI_KALARM_H_
#define HUAYI_KALARM_H_

#include <linux/proc_fs.h>
#include <linux/list.h>
//#include <huayi-universal/huayi_log.h>
#include <huayi-universal/huayi_proc.h>
#include <huayi-universal/huayi_control.h>
#include <huayi-universal/huayi_alarm.h>


#define HUAYI_MODULE_KALARM      "kalarm"
#define HUAYI_ALARM_LOG          "alarm"
#define HUAYI_KALARM_PROC_STATUS "status_alarm"

#define KALARM_LOG(level, fmt, arg...) do { \
         } while(0)

#define ALARM_LOG(level, fmt, arg...) do { \
      } while(0)

typedef struct huayi_kalarm_alarm {
    struct list_head list;
    int alarm_id;
    struct huayi_notification alarm;
} HUAYI_KALARM_ALARM;

typedef struct huayi_kalarm {
    struct proc_dir_entry *proc_kalarm;
    struct proc_dir_entry *proc_status;
    int event_handle;
    int curr_alarm_id;
    int active;
    struct list_head list_alarm;
} HUAYI_KALARM;

#endif

