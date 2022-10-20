/*
**
**  Project: Behaviour manager
**  File:    vsf_common.h
**  Author:  rex.tian
**  Date:    2/23/2018
**
**  Purpose:
**    	Behaviour manager.
**
*/

#ifndef VSF_COMMON_H
#define VSF_COMMON_H


/*merge by huayi*/
typedef enum VSF_STATUS{
    VSF_ERROR = -1,
    VSF_OK = 0
}VSF_STATUS;


typedef enum {
	VSF_MAC_FILTER = 0,
	VSF_IP_MAC_BIND,
	VSF_QOS,
	VSF_URL_FILTER, 
	VSF_APP_FILTER,
	VSF_CLASS_MAX,
} e_vsf_class;

typedef enum {
	VSF_NF_DROP = 1,
	VSF_NF_ACCEPT,
	VSF_NF_CONTINUE,
	VSF_NF_ACTION_MAX,
} e_vsf_nf_action;

typedef enum {
		VSF_DISABLE = 0,
		VSF_ENABLE,
} e_vsf_enable;


#define mask_set_bit(mask, bit)				((mask) |= (1 << (bit)))
#define mask_clear_bit(mask, bit)			((mask) &= (~(1 << (bit))))
#define mask_test_bit(mask, bit)			(((mask) & (1 << (bit))) == (1 << (bit)))
#define mask_test_bitmask(mask, bitmask)	((mask) & (bitmask))




#endif

