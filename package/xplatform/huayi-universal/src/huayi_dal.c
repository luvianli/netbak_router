/**
 * @file dal.c 
 * 
 *  
 * TODO: 
 * Need a way to prevent any access to huayi_dal when the DAL 
 * module is removed from the system (eg. when user change 
 * configuration and unload/re-load the module. 
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "include/huayi_dal.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DAL Interface");
MODULE_AUTHOR("Gerry_li");


static struct dal{
	struct dal_sys_hooks sys_hooks;
	struct dal_ctrl_ops* ctrl_ops;	
	struct dal_kalarm_hooks kalarm_hooks;
}huayi_dal;

/* huayi ctrl ops */
DAL_STATUS dal_attach_huayi_ctrl_hooks(struct dal_ctrl_ops *ops)
{
    huayi_dal.ctrl_ops = ops;
    return HUAYI_OK;
}
EXPORT_SYMBOL(dal_attach_huayi_ctrl_hooks);

DAL_STATUS dal_detach_huayi_ctrl_hooks(void)
{
    huayi_dal.ctrl_ops = NULL;
    return HUAYI_OK;
}
EXPORT_SYMBOL(dal_detach_huayi_ctrl_hooks);
struct dal_ctrl_ops *dal_get_huayi_ctrl_hooks(void)
{
    return huayi_dal.ctrl_ops;
}
EXPORT_SYMBOL(dal_get_huayi_ctrl_hooks);


/*****************************************************
 *      sys hook need register respectively
 ****************************************************/
void dal_reg_sys_hooks(dal_sys_mod_ops sys_hooks, enum sys_hooks_types idx)
{
    huayi_dal.sys_hooks.sys_mod_ops[idx] = sys_hooks;
}
void dal_unreg_sys_hooks(enum sys_hooks_types idx)
{
    huayi_dal.sys_hooks.sys_mod_ops[idx] = NULL;
}
dal_sys_mod_ops dal_get_sys_hooks(enum sys_hooks_types idx)
{
    return huayi_dal.sys_hooks.sys_mod_ops[idx];
}
EXPORT_SYMBOL(dal_reg_sys_hooks);
EXPORT_SYMBOL(dal_unreg_sys_hooks);
EXPORT_SYMBOL(dal_get_sys_hooks);

struct dal_kalarm_hooks *dal_get_kalarm_hooks(void){
    return &huayi_dal.kalarm_hooks;
}
EXPORT_SYMBOL(dal_get_kalarm_hooks);


DAL_STATUS dal_attach_kalarm_hooks(struct dal_kalarm_hooks *hooks){
    huayi_dal.kalarm_hooks = *hooks;
    printk("Attached DAL kalarm_hooks\n");
    return HUAYI_OK;
}
EXPORT_SYMBOL(dal_attach_kalarm_hooks);

DAL_STATUS dal_detach_kalarm_hooks(void){
	BZERO(&huayi_dal.kalarm_hooks, sizeof(huayi_dal.kalarm_hooks));
    printk("Detached DAL kalarm_hooks\n");
    return HUAYI_OK;
}
EXPORT_SYMBOL(dal_detach_kalarm_hooks);


static int32_t __init dal_init(void){
    BZERO((char*)&huayi_dal, sizeof(huayi_dal));
	//printk("huayi basic dal module init\n");
    return 0;
}
module_init(dal_init);

static void __exit dal_exit(void){
    BZERO((char*)&huayi_dal, sizeof(huayi_dal));
	//printk("huayi basic dal module exit\n");
    return;
}
module_exit(dal_exit);



