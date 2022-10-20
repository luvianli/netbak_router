#ifndef __HUAYI_PROCFS_H__
#define __HUAYI_PROCFS_H__

#ifdef __KERNEL__
#include <linux/proc_fs.h>

#define HUAYI_PROC_ROOT           "huayi"

#define HUAYI_PROC_MAX_BUFFER		128
#define HUAYI_PROC_PERM  0444


extern struct proc_dir_entry *huayi_proc;

extern int huayi_proc_read_int32(char *buffer, char **buffer_location, 
                                off_t offset, int buffer_length, 
                                int *eof, void *data);
extern int huayi_proc_write_int32(struct file *file, const char *buffer,
                                 unsigned long count, void *data);

extern int huayi_proc_read_string(char *buffer, char **buffer_location, 
                                off_t offset, int buffer_length, 
                                int *eof, void *data);
extern int huayi_proc_write_string(struct file *file, const char *buffer,
                                  unsigned long count, void *data);

extern int huayi_proc_read_macaddr13(char *buffer, char **buffer_location, 
                                    off_t offset, int buffer_length, 
                                    int *eof, void *data);
extern int huayi_proc_write_macaddr13(struct file *file, const char *buffer,
                                     unsigned long count, void *data);

#endif
#endif /* __HUAYI_PROCFS_H__ */


