/*
 * driver/char/huayi-control
 *driver for huayi control
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#include <huayi-universal/huayi_control.h>
#include <huayi-universal/huayi_dal.h>

#define DEV_HUAYI_CTL_MAJOR_NUMBER  243

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HUAYI control Interface");
MODULE_AUTHOR("gerry_li");

int gerry_control_dev_open (struct inode *inode, struct file *file);
int gerry_control_dev_release (struct inode *inode, struct file *file);
long huayi_control_dev_ioctl (struct file *flip, unsigned int cmd, unsigned long arg);


#define TBL_MAX_ENTRY 32

static huayi_control_tbl huayi_control_table[TBL_MAX_ENTRY];
static int tbl_max = 0;

int huayi_get_index_from_interface(char *stream)
{
   	long index;
    char *cp = NULL;
	int ret = 0;
	
	for (cp = stream; *cp != '\0';cp++)
	{
		if('0' <= *cp && *cp <= '9')
		 break;
	}
	
    ret=kstrtol(cp,10,&index);
	if(ret < 0)
	{
		return -1;
	}
	
    return (int)index;
}

HUAYI_STATUS control_detach_table(char *module){
    int i, j;
	
    for (i = 0; i < tbl_max; i ++){
        if (!strcmp(module, huayi_control_table[i].module)){
            memset(&huayi_control_table[i], 0, sizeof(huayi_control_table[i]));
            if (i != (tbl_max-1)){
                for (j=i; j<(tbl_max-1); j++){
                    memcpy (&huayi_control_table[j], &huayi_control_table[j+1], sizeof(huayi_control_table[j]));
                }
            }
            tbl_max--;
            return HUAYI_OK;
        }
    }
	
    return HUAYI_ERROR;
}
//EXPORT_SYMBOL(control_detach_table);

HUAYI_STATUS control_attach_table(char *module, control_tbl *control_table){
    int i;

    if (tbl_max == TBL_MAX_ENTRY){
        printk("The huayi control table is full, can't attach more.\n");
        return HUAYI_ERROR;
    }

    for (i = 0; i < tbl_max; i++){
        if (!strcmp(module, huayi_control_table[i].module)){
            printk("%s command table already attached.\n", module);
            return HUAYI_OK;
        }
    }
    strcpy(huayi_control_table[tbl_max].module, module);
    huayi_control_table[tbl_max].control_table = control_table;
    tbl_max++;
	
    return HUAYI_OK;
}
//EXPORT_SYMBOL(control_attach_table);

control_tbl *control_get_table(char *module){
    int i;
	
    for (i = 0; i < tbl_max; i ++){
        if (!strcmp(module, huayi_control_table[i].module)){
            return huayi_control_table[i].control_table;
        }
    }
    printk("no %s command table!\n", module);
    return 0;
}
//EXPORT_SYMBOL(control_get_table);


int gerry_control_dev_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

long huayi_control_dev_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    huayi_control_t huayi;
    control_tbl *control_t = NULL;
    control_tbl *Control_t;
    char buf_c;
    int buf_i;
    short buf_h;
    char buf_s[512];
    int idx1, idx2;
    HUAYI_STATUS rc = 0;
	
    switch(cmd)
    {
    case HUAYI_CTL_IOC_R:
        copy_from_user(&huayi, (huayi_control_t __user *)arg, sizeof(huayi_control_t));

        Control_t = control_get_table(huayi.section);
        if (Control_t == NULL) {
            printk("failed to get command table\n");
            return -1;
        }
        for (control_t = Control_t; control_t->name != NULL; control_t++) {
            if (strcmp(control_t->name, huayi.ifname) == 0) {
                switch (control_t->flags & TBL_FORMAT) {
                case TBL_FORMAT_D:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(idx1, &buf_i);
                            huayi.buf.buf_i[1] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(idx1, &buf_h);
                            huayi.buf.buf_h[1] = buf_h;
                        } else {
                            rc = control_t->get(idx1, &buf_c);
                            huayi.buf.buf_c[1] = buf_c;
                        }
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(idx1, idx2, &buf_i);
                            huayi.buf.buf_i[1] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(idx1, idx2, &buf_h);
                            huayi.buf.buf_h[1] = buf_h;
                        } else {
                            rc = control_t->get(idx1, idx2, &buf_c);
                            huayi.buf.buf_c[1] = buf_c;
                        }
                    } else {
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(&buf_i);
                            huayi.buf.buf_i[1] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(&buf_h);
                            huayi.buf.buf_h[1] = buf_h;
                        } else {
                            rc = control_t->get(&buf_c);
                            huayi.buf.buf_c[1] = buf_c;
                        }
                    } 
                    break;
                case TBL_FORMAT_X:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(idx1, &buf_i);
                            huayi.buf.buf_i[0] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(idx1, &buf_h);
                            huayi.buf.buf_h[0] = buf_h;
                        } else {
                            rc = control_t->get(idx1, &buf_c);
                            huayi.buf.buf_c[0] = buf_c;
                        }
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(idx1, idx2, &buf_i);
                            huayi.buf.buf_i[0] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(idx1, idx2, &buf_h);
                            huayi.buf.buf_h[0] = buf_h;
                        } else {
                            rc = control_t->get(idx1, idx2, &buf_c);
                            huayi.buf.buf_c[0] = buf_c;
                        }
                    } else {
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->get(&buf_i);
                            huayi.buf.buf_i[0] = buf_i;
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->get(&buf_h);
                            huayi.buf.buf_h[0] = buf_h;
                        } else {
                            rc = control_t->get(&buf_c);
                            huayi.buf.buf_c[0] = buf_c;
                        }
                    } 
                    break;
                case TBL_FORMAT_S:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        rc = control_t->get(idx1, buf_s);
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        rc = control_t->get(idx1, idx2, buf_s);
                    } else {
                        rc = control_t->get(buf_s);
                    }
                    strcpy (huayi.buf.buf_s, buf_s);
                    break;
                default:
                    break;
                }
            }
        }
        copy_to_user((huayi_control_t __user *)arg, &huayi, sizeof(huayi_control_t));
        break;
    case HUAYI_CTL_IOC_W:
        copy_from_user(&huayi, (huayi_control_t __user *)arg, sizeof(huayi_control_t));
        Control_t = control_get_table(huayi.section);
        if (Control_t == NULL){
            printk("failed to get command table\n");
            return -1;
        }
        for (control_t = Control_t; control_t->name != NULL; control_t++){
            if (strcmp(control_t->name, huayi.ifname) == 0) {
                switch (control_t->flags & TBL_FORMAT) {
                case TBL_FORMAT_D:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(idx1, huayi.buf.buf_i[1]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(idx1, huayi.buf.buf_h[1]);
                        } else {
                            rc = control_t->set(idx1, huayi.buf.buf_c[1]);
                        }
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_i[1]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_h[1]);
                        } else {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_c[1]);
                        }
                    } else {
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(huayi.buf.buf_i[1]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(huayi.buf.buf_h[1]);
                        } else {
                            rc = control_t->set(huayi.buf.buf_c[1]);
                        }
                    }
                    break;
                case TBL_FORMAT_X:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(idx1, huayi.buf.buf_i[0]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(idx1, huayi.buf.buf_h[0]);
                        } else {
                            rc = control_t->set(idx1, huayi.buf.buf_c[0]);
                        }
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_i[0]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_h[0]);
                        } else {
                            rc = control_t->set(idx1, idx2, huayi.buf.buf_c[0]);
                        }
                    } else {
                        if (control_t->flags & TBL_TYPE_INT) {
                            rc = control_t->set(huayi.buf.buf_i[0]);
                        } else if (control_t->flags & TBL_TYPE_SHORT) {
                            rc = control_t->set(huayi.buf.buf_h[0]);
                        } else {
                            rc = control_t->set(huayi.buf.buf_c[0]);
                        }
                    }
                    break;
                case TBL_FORMAT_S:
                    if (control_t->flags & TBL_ARG_ONE) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        rc = control_t->set(idx1, huayi.buf.buf_s);
                    } else if (control_t->flags & TBL_ARG_TWO) {
                        idx1 = huayi_get_index_from_interface(huayi.intrface[0]);
                        if (idx1 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        idx2 = huayi_get_index_from_interface(huayi.intrface[1]);
                        if (idx2 == -1) {
                            printk("invalid index\n");
                            return -1;
                        }
                        rc = control_t->set(idx1, idx2, huayi.buf.buf_s);
                    } else {
                        rc = control_t->set(huayi.buf.buf_s);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    default:
        break;
    }
    return rc;
}

int huayi_control_deva_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static struct file_operations huayi_control_dev_fops = {
    .owner = THIS_MODULE,
    .open = gerry_control_dev_open,
    .unlocked_ioctl  = huayi_control_dev_ioctl,
    .release = huayi_control_deva_release,
};
static struct class *huayi_control_class;
static struct dal_ctrl_ops huayi_ctrl_ops = {
    .control_detach_table = control_detach_table,
    .control_attach_table = control_attach_table,
    .control_get_table = control_get_table,
};

static int __init huayi_control_init(void)
{	
    if (register_chrdev(DEV_HUAYI_CTL_MAJOR_NUMBER, "huayi_control", &huayi_control_dev_fops))
        printk (KERN_ERR "Failed to register driver for /dev/huayi_control\n");
	
    huayi_control_class=class_create(THIS_MODULE, "huayi_control");
	
    if (IS_ERR(huayi_control_class)){
        printk(KERN_ERR "Failed to create class.\n");
    }
	
    device_create (huayi_control_class, NULL, MKDEV(DEV_HUAYI_CTL_MAJOR_NUMBER, 0), NULL, "huayi_control");
    printk (KERN_INFO "Registerd character driver\n");

    dal_attach_huayi_ctrl_hooks(&huayi_ctrl_ops);
	
	//printk("huayi ctl module init!!\n");
    return 0;
}

static void __exit huayi_control_exit(void)
{
    dal_detach_huayi_ctrl_hooks();
    unregister_chrdev(DEV_HUAYI_CTL_MAJOR_NUMBER, "huayi_control");
	//printk ("unregister driver for /dev/huayi_control\n");
    device_destroy (huayi_control_class, MKDEV (DEV_HUAYI_CTL_MAJOR_NUMBER,0));
    class_destroy (huayi_control_class);
	//printk("huayi ctl module exit!!\n");
}

module_init(huayi_control_init);
module_exit(huayi_control_exit);
