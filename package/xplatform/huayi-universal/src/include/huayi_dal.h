#ifndef __HUAYI_DAL_H__
#define __HUAYI_DAL_H__

#define DAL_MAX_DEVICE_ID       10
#define DAL_MAX_IFNAME          16
#define DAL_MAX_DEVICE_DESC     32
#define DAL_MAX_CLIENT			128

#include "huayi_universal.h"
#include "huayi_dal_stat.h"
#include "huayi_control.h"

/**
 * Some common macros
 */
/*TODO: we now reuse HUAYI_STATUS, need to define DAL's own error code */
typedef enum{
    DAL_OK = HUAYI_OK,
    DAL_ERROR = HUAYI_ERROR,
    DAL_NODE_NOT_EXIST = -2,
}DAL_STATUS;


typedef enum sys_hooks_types {
    EN_PRODUCT_INFO = 0,
    EN_HUAYI_STATUS  = 1,
    EN_HUAYI_STATUS_DID = 2,
    EN_HUAYI_FILTER_CONFIG = 3,
} SYS_HOOKS_TYPES;


#define huayi_dal_MAX_SYS_HOOKS 16
typedef void* (*dal_sys_mod_ops)(void*addr, int len);
struct dal_sys_hooks{
    dal_sys_mod_ops sys_mod_ops[huayi_dal_MAX_SYS_HOOKS];
};

struct dal_kalarm_hooks{
    DAL_STATUS (*raise_notification)(struct huayi_notification e);
};

extern struct dal_wifi_ops      *dal_get_wifi_ops(void);
extern DAL_STATUS dal_attach_wifi_ops(struct dal_wifi_ops *ops);
extern DAL_STATUS dal_detach_wifi_ops(void);


extern DAL_STATUS dal_attach_huayi_ctrl_hooks(struct dal_ctrl_ops *ops);
extern DAL_STATUS dal_detach_huayi_ctrl_hooks(void);
extern struct dal_ctrl_ops *dal_get_huayi_ctrl_hooks(void);


extern DAL_STATUS dal_attach_kalarm_hooks(struct dal_kalarm_hooks *hooks);
extern DAL_STATUS dal_detach_kalarm_hooks(void);


extern struct dal_kalarm_hooks  *dal_get_kalarm_hooks(void);


//struct dal_ctrl_ops;
//DAL_STATUS dal_attach_huayi_ctrl_hooks(struct dal_ctrl_ops *ops);
//DAL_STATUS dal_detach_huayi_ctrl_hooks(void);
//struct dal_ctrl_ops *dal_get_huayi_ctrl_hooks(void);


/****************************************
 *  sys hooks
 ****************************************/
void dal_reg_sys_hooks(dal_sys_mod_ops, enum sys_hooks_types idx);
void dal_unreg_sys_hooks(enum sys_hooks_types idx);
dal_sys_mod_ops dal_get_sys_hooks(enum sys_hooks_types idx);

/** product info hook **/
#define proinfo_dev_dal_reg_hook(sys_hooks)  dal_reg_sys_hooks(sys_hooks,EN_PRODUCT_INFO)
#define proinfo_dev_dal_unreg_hook()         dal_unreg_sys_hooks(EN_PRODUCT_INFO)
/** huayi status info hook **/
#define HUAYI_STATUS_dal_reg_hook(sys_hooks)  dal_reg_sys_hooks(sys_hooks,EN_HUAYI_STATUS)
#define HUAYI_STATUS_dal_unreg_hook()         dal_unreg_sys_hooks(EN_HUAYI_STATUS)
#define HUAYI_STATUS_did_dal_reg_hook(sys_hooks)  dal_reg_sys_hooks(sys_hooks,EN_HUAYI_STATUS_DID)
#define HUAYI_STATUS_did_dal_unreg_hook()         dal_unreg_sys_hooks(EN_HUAYI_STATUS_DID)

#define huayi_filter_dal_reg_hook(sys_hooks)  dal_reg_sys_hooks(sys_hooks,EN_HUAYI_FILTER_CONFIG)
#define huayi_filter_dal_unreg_hook()         dal_unreg_sys_hooks(EN_HUAYI_FILTER_CONFIG)



#define DAL_WIFI_GET_IDX_BY_DEV (dal_get_wifi_ops()->get_idx_by_dev)
#define DAL_WIFI_GET_IDX_BY_OL_SCN (dal_get_wifi_ops()->get_idx_by_ol_scn)
#define DAL_WIFI_IS_UP          (dal_get_wifi_ops()->is_up)
#define DAL_WIFI_GET_DEV_BY_IDX (dal_get_wifi_ops()->get_dev_by_idx)
#define DAL_WIFI_GET_OL_SCN_BY_IDX (dal_get_wifi_ops()->get_ol_scn_by_idx)
#define DAL_WIFI_GET_SCAN_STATS  (dal_get_wifi_ops()->get_scan_stats)
#define DAL_GET_REG_CNT  (dal_get_wifi_ops()->get_reg_cnt)
#define DAL_WIFI_GET_DEVICE_STATS  (dal_get_wifi_ops()->get_device_stats)
#define DAL_WIFI_GET_VAP_STATS  (dal_get_wifi_ops()->get_vap_stats)
#define DAL_WIFI_GET_STA_STATS	(dal_get_wifi_ops()->get_sta_stats)	
#define DAL_WIFI_GET_STA_LIST	(dal_get_wifi_ops()->get_sta_list)
#define DAL_WIFI_GET_NF         (dal_get_wifi_ops()->get_nf)
#define DAL_WIFI_GET_CHANNEL_USAGE	(dal_get_wifi_ops()->get_channel_usage)
#define DAL_WIFI_IFNAME_TO_VAPIDX  (dal_get_wifi_ops()->ifname_to_vapidx)



#define DAL_GET_DID_BY_IDX(_idx)    (dal_get_config()->wifi[_idx].device_id)

#endif /* __HUAYI_DAL_H__ */

