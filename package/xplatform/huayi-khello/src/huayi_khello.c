/**
 * @file huayi_khello.c
 * 
 * 
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/kernel.h>
#include <linux/string.h> 
#include <asm/cputime.h>
#include <linux/tick.h>
#include "huayi_khello.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HUAYI HelloWorld Module");
MODULE_AUTHOR("gerry_li");

static int32_t __init module_huayi_khello_init(void){
	
	printk("huayi khello init\n");
    return 0;
}
module_init(module_huayi_khello_init);

static void __exit module_huayi_khello_exit(void){
	
	printk("huayi khello module exit\n");
    return;
}
module_exit(module_huayi_khello_exit);

