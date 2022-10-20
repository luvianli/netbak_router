/**
 * @file huayi_procfs.c
 * 
 * 
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include "include/huayi_universal.h"
#include "include/huayi_types.h"


#define APROCLOG(level, fmt, arg...) do { \
     }while (0)

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HUAYI procfs module");
MODULE_AUTHOR("Gerry-li");

static char s_buffer[HUAYI_PROC_MAX_BUFFER];

struct proc_dir_entry *huayi_proc = NULL;


EXPORT_SYMBOL(huayi_proc);

int huayi_proc_read_int32(char *buffer, char **buffer_location, 
                         off_t offset, int buffer_length, 
                         int *eof, void *data) {
    int ret;
    int32_t *p32 = (int32_t*)data;

    ret = sprintf(buffer, "%d\n", *p32);
    *eof = 1;
    return ret;
}
EXPORT_SYMBOL(huayi_proc_read_int32);

int huayi_proc_write_int32(struct file *file, const char *buffer,
                          unsigned long count, void *data) {
    int32_t *p32 = (int32_t*)data;
    int size = count;

    if (size > HUAYI_PROC_MAX_BUFFER) {
        size = HUAYI_PROC_MAX_BUFFER;
    }

    /* write data to the buffer */
    if (copy_from_user(s_buffer, buffer, size)) {
        return -EFAULT;
    }
    s_buffer[HUAYI_PROC_MAX_BUFFER-1] = '\0';

    sscanf(s_buffer, "%d", p32);
    return size;
}
EXPORT_SYMBOL(huayi_proc_write_int32);

int huayi_proc_read_string(char *buffer, char **buffer_location, 
                          off_t offset, int buffer_length, 
                          int *eof, void *data) {
    int ret;
    char *str = (char*)data;

    ret = snprintf(buffer, buffer_length,"%s\n", str);
    *eof = 1;
    return ret;
}
EXPORT_SYMBOL(huayi_proc_read_string);

int huayi_proc_write_string(struct file *file, const char *buffer,
                           unsigned long count, void *data) {
    char *str = (char*)data;
    int size = count;

    if (size > HUAYI_PROC_MAX_BUFFER) {
        size = HUAYI_PROC_MAX_BUFFER;
    }

    /* write data to the buffer */
    if (copy_from_user(s_buffer, buffer, size)) {
        return -EFAULT;
    }
    s_buffer[HUAYI_PROC_MAX_BUFFER-1] = '\0';

    sprintf(str, "%s", s_buffer);  // !!! assume size of str > size
    return size;
}
EXPORT_SYMBOL(huayi_proc_write_string);

extern int huayi_proc_read_macaddr13(char *buffer, char **buffer_location, 
                                    off_t offset, int buffer_length, 
                                    int *eof, void *data) {
    int ret;
    char str[13];
    A_MACADDR *macaddr = (A_MACADDR*)data;

    A_MACADDR_TOSTR_13(macaddr, str, 13);
    ret = snprintf(buffer, buffer_length,"%s\n", str);
    *eof = 1;
    return ret;

}
EXPORT_SYMBOL(huayi_proc_read_macaddr13);

static int htoi(char ch) {
    if (ch>='a' && ch <='f') {
        return ch - 'a' + 10;
    } else if (ch>='A' && ch <='F') {
        return ch - 'A' + 10;
    } else if (ch>='1' && ch <='9') {
        return ch - '0';
    } else {
        // invalid value;
        return 0;
    }
}
extern int huayi_proc_write_macaddr13(struct file *file, const char *buffer,
                                     unsigned long count, void *data) {
    A_MACADDR *macaddr = (A_MACADDR*)data;
    int size = count;

    if (size > HUAYI_PROC_MAX_BUFFER) {
        size = HUAYI_PROC_MAX_BUFFER;
    }

    /* write data to the buffer */
    if (copy_from_user(s_buffer, buffer, size)) {
        return -EFAULT;
    }

    if (size < 13) {
        // invalid mac address
        BZERO(macaddr, sizeof(A_MACADDR));
        return size;
    }

    macaddr->octets[0] = (htoi(s_buffer[0]) << 4)  | htoi(s_buffer[1]);
    macaddr->octets[1] = (htoi(s_buffer[2]) << 4)  | htoi(s_buffer[3]);
    macaddr->octets[2] = (htoi(s_buffer[4]) << 4)  | htoi(s_buffer[5]);
    macaddr->octets[3] = (htoi(s_buffer[6]) << 4)  | htoi(s_buffer[7]);
    macaddr->octets[4] = (htoi(s_buffer[8]) << 4)  | htoi(s_buffer[9]);
    macaddr->octets[5] = (htoi(s_buffer[10]) << 4) | htoi(s_buffer[11]); 
    return size;
}
EXPORT_SYMBOL(huayi_proc_write_macaddr13);

static int32_t __init huayi_procfs_module_init(void){

	APROCLOG(HUAYILOG_INFO, "Module Init\n");
	
    huayi_proc = proc_mkdir(HUAYI_PROC_ROOT, NULL);
    if (huayi_proc == NULL) {
        APROCLOG(HUAYILOG_CRIT, "Initialize /proc/%s failed\n",
                                HUAYI_PROC_ROOT);
        return HUAYI_ERROR;
    }
   
    return 0;
}
module_init(huayi_procfs_module_init);

static void __exit huayi_procfs_module_exit(void){
    if (huayi_proc) {
        remove_proc_entry(HUAYI_PROC_ROOT, NULL);        
        huayi_proc = NULL;
    }

    APROCLOG(HUAYILOG_INFO, "Module Exit\n");
    return;
}
module_exit(huayi_procfs_module_exit);


