#ifndef __HUAYI_CTL_H__
#define __HUAYI_CTL_H__
#include "huayi_universal.h"
#include "huayi_dal.h"

#define TYPE_CHAR       0x01
#define TYPE_STREAM   0x02
#define TYPE_INT 	   0x04
#define TYPE_SHORT	   0x08
#define FORMAT_D         0x10
#define FORMAT_X         0x20
#define FORMAT_S         0x40

#define TBL_FORMAT    0x00f
#define TBL_FORMAT_D  0x001
#define TBL_FORMAT_X  0x002
#define TBL_FORMAT_S  0x004
#define TBL_ARG_ONE   0x010    /* function takes xxx# */
#define TBL_ARG_TWO   0x020    /* function takes xxx# and yyy# */
#define TBL_TYPE_CHAR  0x200
#define TBL_TYPE_INT   0x400
#define TBL_TYPE_SHORT 0x800

#define HUAYI_CTL_IOC_MAGIC 0xfb  //ioctl command number magic
#define HUAYI_CTL_IOC_W  _IOW(HUAYI_CTL_IOC_MAGIC, 16, huayi_control_t)
#define HUAYI_CTL_IOC_R  _IOR(HUAYI_CTL_IOC_MAGIC, 17, huayi_control_t)


typedef struct{
	char type;           /* Type of value */
	char op[10];        /* Operation */
	char intrface[2][20];       /* Keep interface */
	char ifname[40];  /* Name of the command */
	char section[20];   /* Name of the section */
	union{
		char buf_c[2];
		int buf_i[2];
		short buf_h[2];
		char buf_s[512];
	}buf;
}huayi_control_t;


typedef struct{
	char *name;
	int flags;
	HUAYI_STATUS (*get)();
	HUAYI_STATUS (*set)();
}control_tbl;

#define g(x)      ((HUAYI_STATUS(*)())(x))
#define s(x)      ((HUAYI_STATUS(*)())(x))

typedef struct{
	char module[20];
	control_tbl *control_table;
}huayi_control_tbl;


struct dal_ctrl_ops {
    HUAYI_STATUS (*control_detach_table)(char *module);
    HUAYI_STATUS (*control_attach_table)(char *module, control_tbl *control_table);
    control_tbl *(*control_get_table)(char *module);
};
/* get huayi ctrl ops handle */
#define HUAYI_CTRL_ATTACH_TABLE(module, control_table)   \
    do { if (dal_get_huayi_ctrl_hooks())  \
            dal_get_huayi_ctrl_hooks()->control_attach_table(module, control_table);} while (0)
#define HUAYI_CTRL_DETACH_TABLE(module)     \
    do { if (dal_get_huayi_ctrl_hooks())  \
            dal_get_huayi_ctrl_hooks()->control_detach_table(module);}while (0)
#define HUAYI_CTRL_GEE_TABLE(module) ({ \
    control_tbl *ptbl = NULL; if (dal_get_huayi_ctrl_hooks())  \
            ptbl = dal_get_huayi_ctrl_hooks()->control_get_table(module); ptbl;})
#endif /* __HUAYI_CTL_H__ */
