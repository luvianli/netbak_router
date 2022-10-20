#if 0 /*comment by huayi,no need in new platform*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/sched.h>/*jiffies*/
#include <linux/proc_fs.h>
#include <net/netfilter/vsf/vsf_uci.h>
#include <net/netfilter/vsf/vsf_lock.h>
#include <net/netfilter/vsf/vsf_status.h>

static struct stt uci_stt;
static int uci_file_num = 0;
static char current_uci_file[32];
struct proc_dir_entry *vsf_proc_dir;
static struct list_head uci_package_head;

static int uci_link_scan(void)
{
	PT_uci_package  uci_package;
	PT_uci_sections uci_sections;
	PT_uci_option   uci_option;

	if (list_empty(&uci_package_head))
	{
		printk("uci list is empty!\n");
		return 0;
	}

	cste_uci_read_lock();
	/* package head */
	list_for_each_entry(uci_package, &uci_package_head, uci_package_list){
		printk(" %s :\n", uci_package->uci_package_name);
		
		/* sections node */
		list_for_each_entry(uci_sections, &(uci_package->uci_sections_head), uci_sections_list){
			printk(" config %s %s\n", uci_sections->uci_sections, uci_sections->uci_sections_name);

			/* option node */
			list_for_each_entry(uci_option, &(uci_sections->uci_option_head), uci_option_list){
				printk("\t");
				printk("option %s ", uci_option->uci_option);
				printk(" %s \n", uci_option->uci_value);
			}
			printk("\n");
		}
		printk("\n");
	}	
	cste_uci_read_unlock();
	
	return 0;
}

static int uci_link_free(void)
{
	PT_uci_package  uci_package,  uci_package_next;
	PT_uci_sections uci_sections, uci_sections_next;
	PT_uci_option   uci_option,   uci_option_next;
	
	cste_uci_read_lock();
	
	list_for_each_entry_safe(uci_package, uci_package_next, &uci_package_head, uci_package_list){
		
		list_for_each_entry_safe(uci_sections, uci_sections_next, &(uci_package->uci_sections_head), uci_sections_list){

			list_for_each_entry_safe(uci_option, uci_option_next, &(uci_sections->uci_option_head), uci_option_list){

				list_del(&uci_option->uci_option_list);
				kfree(uci_option);
			}
			
			list_del(&uci_sections->uci_sections_list);
			kfree(uci_sections);
		}
		
		list_del(&uci_package->uci_package_list);
		kfree(uci_package);
	}
	cste_uci_read_unlock();
	return 0;
}


static PT_uci_package cste_uci_find_entry_by_package(T_uci_ioctl_entity *new_entry) 
{
	PT_uci_package p_entry, p_entry_next;

	cste_uci_read_lock();	
	/* find uci file */
	list_for_each_entry_safe(p_entry, p_entry_next, &uci_package_head, uci_package_list){
	
		if (strcmp(p_entry->uci_package_name, new_entry->uci_package_name) == 0) {
			cste_uci_read_unlock();
			return p_entry;
		}
	}
	
	cste_uci_read_unlock();
	return NULL;
}

static PT_uci_sections cste_uci_find_entry_by_sections(T_uci_ioctl_entity *new_entry, PT_uci_package p_entry) 
{
	PT_uci_sections s_entry, s_entry_next;

	cste_uci_read_lock();

	/* find uci sections */
	list_for_each_entry_safe(s_entry, s_entry_next, &(p_entry->uci_sections_head), uci_sections_list){
		
		if (strcmp(s_entry->uci_sections, new_entry->uci_sections) == 0 && \
			strcmp(s_entry->uci_sections_name, new_entry->uci_sections_name) == 0) {
			cste_uci_read_unlock();
			return s_entry;
		}
	}
	
	cste_uci_read_unlock();
	return NULL;
}

static PT_uci_option cste_uci_find_entry_by_option(T_uci_ioctl_entity *new_entry, PT_uci_sections s_entry) 
{
	PT_uci_option   o_entry, o_entry_next;
	
	cste_uci_read_lock();

	/* find uci option */
	list_for_each_entry_safe(o_entry, o_entry_next, &(s_entry->uci_option_head), uci_option_list){
		
		if (strcmp(o_entry->uci_option, new_entry->uci_option) == 0) {
			cste_uci_read_unlock();
			return o_entry;
		}
	}
	
	cste_uci_read_unlock();
	return NULL;
}

static void add_new_option(T_uci_ioctl_entity *new_uci_entry, struct list_head *option_head, int append_flag)
{
	PT_uci_option new_uci_option = NULL;

	if (append_flag == NO_APPEND)
	{
		INIT_LIST_HEAD(option_head);
	}

	new_uci_option = (PT_uci_option)kmalloc(sizeof(T_uci_option), GFP_KERNEL);
	
	memset(new_uci_option, 0x00, sizeof(T_uci_option));
	list_add_tail(&(new_uci_option->uci_option_list), option_head);
	
	strcpy(new_uci_option->uci_option, new_uci_entry->uci_option);
	
	strcpy(new_uci_option->uci_value,  new_uci_entry->uci_value);
	
	return;
}

static void add_new_sections(T_uci_ioctl_entity *new_uci_entry, struct list_head *sections_head, int append_flag)
{
	PT_uci_sections new_uci_sections = NULL;
	
	if (append_flag == NO_APPEND)
	{
		INIT_LIST_HEAD(sections_head);
	}
	
	new_uci_sections = (PT_uci_sections)kmalloc(sizeof(T_uci_sections), GFP_KERNEL);
	
	memset(new_uci_sections, 0x00, sizeof(T_uci_sections));
	list_add_tail(&(new_uci_sections->uci_sections_list), sections_head);
	
	strcpy(new_uci_sections->uci_sections, new_uci_entry->uci_sections);
	strcpy(new_uci_sections->uci_sections_name, new_uci_entry->uci_sections_name);
	add_new_option(new_uci_entry, &(new_uci_sections->uci_option_head), NO_APPEND);
	
	return;
}

static void add_new_package(T_uci_ioctl_entity *new_uci_entry, int append_flag)
{
	PT_uci_package new_uci_package = NULL;

	if (append_flag == NO_APPEND)
	{
		INIT_LIST_HEAD(&uci_package_head);
	}
	
	new_uci_package = (PT_uci_package)kmalloc(sizeof(T_uci_package), GFP_KERNEL);
	
	memset(new_uci_package, 0x00, sizeof(T_uci_package));
	list_add_tail(&(new_uci_package->uci_package_list), &uci_package_head);
	strcpy(new_uci_package->uci_package_name, new_uci_entry->uci_package_name);
	
	add_new_sections(new_uci_entry, &(new_uci_package->uci_sections_head), NO_APPEND);
	return;
}

static int get_package_num(void)
{
	int num = 0;
	PT_uci_package p_entry, p_entry_next;

	if (list_empty(&uci_package_head))
	{
		return 0;
	}

	cste_uci_read_lock();	
	/* find uci file */
	list_for_each_entry_safe(p_entry, p_entry_next, &uci_package_head, uci_package_list){
		num ++;
	}
	cste_uci_read_unlock();
	
	return num;
}

static void *uci_file_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct list_head *uci_file_list = &uci_package_head;
	
	cste_uci_read_lock();
	
	if(uci_file_list == NULL)
	{
		return NULL;
	}

    return seq_list_start(uci_file_list, *pos);
} 

static void *uci_file_seq_next(struct seq_file *seq, void *v, loff_t *pos) 
{
	struct list_head *uci_file_list = &uci_package_head;
	
	if(v== NULL)
	{
		return NULL;
	}
	
    return seq_list_next(v, uci_file_list, pos);
}

static void uci_file_seq_stop(struct seq_file *seq, void *v) 
{
    cste_uci_read_unlock();
}

static int uci_file_seq_show(struct seq_file *seq, void *v)
{
	PT_uci_sections uci_sections;
	PT_uci_option   uci_option;
	PT_uci_package  uci_package = list_entry(v, struct uci_package, uci_package_list);

	if( v == SEQ_START_TOKEN)
	{
		return 0;
	}

	if (strcmp(current_uci_file, uci_package->uci_package_name) == 0)
	{
		/* sections node */
		list_for_each_entry(uci_sections, &(uci_package->uci_sections_head), uci_sections_list){

			if (strlen(uci_sections->uci_sections_name))
				seq_printf(seq, "config '%s' '%s'\n", uci_sections->uci_sections, uci_sections->uci_sections_name);
			else
				seq_printf(seq, "config '%s'\n", uci_sections->uci_sections);
			
			/* option node */
			list_for_each_entry(uci_option, &(uci_sections->uci_option_head), uci_option_list){
				seq_printf(seq, "\t");
				seq_printf(seq, "option '%s' ", uci_option->uci_option);
				seq_printf(seq, " '%s' \n", uci_option->uci_value);
			}
			seq_printf(seq, "\n");
		}
	}
	
    return 0;
}

static struct seq_operations stt_uci_file_seq_ops = {
	.start = uci_file_seq_start,
	.next  = uci_file_seq_next,
	.stop  = uci_file_seq_stop, 
	.show  = uci_file_seq_show,
};

int uci_proc_file_open(struct inode *inode, struct file *file) 
{
    int res = -ENOMEM;
    res = seq_open(file, &stt_uci_file_seq_ops);
	memset(current_uci_file, 0x00, sizeof(current_uci_file));
	strcpy(current_uci_file, file->f_path.dentry->d_iname);
    return res;
}

static int uci_proc_file_release(struct inode *inode, struct file *file)
{
    struct seq_file *seq = file->private_data;

	if(seq->private != NULL)
        VSF_FREE(seq->private);
	
    return seq_release(inode, file);
}

static struct file_operations stt_uci_file_stats_fops = { 
    .owner = THIS_MODULE, 
    .open = uci_proc_file_open, 
    .read = seq_read, 
    .llseek = seq_lseek, 
    .release = uci_proc_file_release 
}; 

int init_uci_proc_dir(struct proc_dir_entry *dir)
{
	vsf_proc_dir = dir;
	return 0;
}

int creat_uci_proc_file(char *uci_file_name)
{
	uci_stt.proc_dir = vsf_proc_dir;
	snprintf(uci_stt.uci_file[uci_file_num].name, STT_MAX_NAME, "%s", uci_file_name);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	uci_stt.uci_file[uci_file_num].proc = proc_create_data(uci_stt.uci_file[uci_file_num].name,
												 0, uci_stt.proc_dir, &stt_uci_file_stats_fops, NULL);
	if (uci_stt.uci_file[uci_file_num].proc == NULL) 
	{
		printk("Initialize /proc/%s/%s\n failed",CSTE_PROC_ROOT,uci_stt.uci_file[uci_file_num].name);
		return VSF_ERROR;
	}
#else
	uci_stt.uci_file[uci_file_num].proc = proc_create(uci_stt.uci_file[uci_file_num].name,
									 0444, uci_stt.proc_dir,&stt_uci_file_stats_fops);

	if (uci_stt.uci_file[uci_file_num].proc == NULL) 
	{
		printk("Initialize /proc/%s/%s failed\n",CSTE_PROC_ROOT,uci_stt.uci_file[uci_file_num].name);
		return VSF_ERROR;
	}

	uci_stt.uci_file[uci_file_num].proc->proc_fops = &stt_uci_file_stats_fops;
#endif
	
	if ((++uci_file_num) > UCI_MAX_FILE)
	{
		printk("uci file Numbers is over %d \n!", UCI_MAX_FILE);
		return -1;
	}

	return 0;
}

int uci_add(PT_uci_ioctl_entity new_uci_entry) 
{
	PT_uci_package  uci_package  = NULL;
	PT_uci_sections uci_sections = NULL;
	PT_uci_option   uci_option   = NULL;
	
	uci_package = cste_uci_find_entry_by_package(new_uci_entry);
	if (uci_package == NULL)
	{
		add_new_package(new_uci_entry, NEED_APPEND);
		creat_uci_proc_file(new_uci_entry->uci_package_name);
		return 0;
	}

	uci_sections = cste_uci_find_entry_by_sections(new_uci_entry, uci_package);
	if (uci_package != NULL && uci_sections == NULL)
	{
		add_new_sections(new_uci_entry, &(uci_package->uci_sections_head), NEED_APPEND);
		return 0;
	}

	uci_option = cste_uci_find_entry_by_option(new_uci_entry, uci_sections);
	if (uci_package != NULL && uci_sections != NULL && uci_option == NULL)
	{
		add_new_option(new_uci_entry, &(uci_sections->uci_option_head), NEED_APPEND);
		return 0;
	}
	else if (uci_package != NULL && uci_sections != NULL && uci_option != NULL)
	{
		strcpy(uci_option->uci_value, new_uci_entry->uci_value);
		return 0;
	}
	
	
	return 0;
}

int uci_del(PT_uci_ioctl_entity new_uci_entry) 
{
	PT_uci_package  uci_package  = NULL;
	PT_uci_sections uci_sections = NULL;
	PT_uci_option   uci_option   = NULL;
	
	uci_package = cste_uci_find_entry_by_package(new_uci_entry);
	if (uci_package == NULL)
	{
		printk("file %s not exit!\n", new_uci_entry->uci_package_name);
		return 0;
	}

	uci_sections = cste_uci_find_entry_by_sections(new_uci_entry, uci_package);
	if (uci_package != NULL && uci_sections == NULL)
	{
		printk("file %s sections %s not exit!\n", new_uci_entry->uci_package_name, new_uci_entry->uci_sections);
		return 0;
	}

	uci_option = cste_uci_find_entry_by_option(new_uci_entry, uci_sections);
	if (uci_package != NULL && uci_sections != NULL && uci_option == NULL)
	{
		printk("file %s sections %s option %s is not exit!\n",new_uci_entry->uci_package_name, new_uci_entry->uci_sections, new_uci_entry->uci_option);
		return 0;
	}
	else if (uci_package != NULL && uci_sections != NULL && uci_option != NULL)
	{
		list_del(&uci_option->uci_option_list);
		if (uci_option != NULL)
		{
			kfree(uci_option);
		}
	}
	
	return 0;
}

int uci_default_config_init(void)
{
	int i;
	PT_uci_package  uci_package  = NULL;
	PT_uci_sections uci_sections = NULL;
	PT_uci_option   uci_option   = NULL;
	
	INIT_LIST_HEAD(&uci_package_head);
	
	for (i=0; i<ARRAY_SIZE(uci_default_value); i++)
	{
		uci_package = cste_uci_find_entry_by_package(&uci_default_value[i]);
		if (uci_package == NULL)
		{
			add_new_package(&uci_default_value[i], NEED_APPEND);
			continue;
		}

		uci_sections = cste_uci_find_entry_by_sections(&uci_default_value[i], uci_package);
		if (uci_package != NULL && uci_sections == NULL)
		{
			add_new_sections(&uci_default_value[i], &(uci_package->uci_sections_head), NEED_APPEND);
			continue;
		}

		uci_option = cste_uci_find_entry_by_option(&uci_default_value[i], uci_sections);
		if (uci_package != NULL && uci_sections != NULL && uci_option == NULL)
		{
			add_new_option(&uci_default_value[i], &(uci_sections->uci_option_head), NEED_APPEND);
			continue;
		}
	}
	
	return  0;	
}

int uci_config_ctl_exit(void)
{
	printk("uci config parse exit succed !\n");
	
	uci_link_free();
	return 0;
}

struct list_head * get_uci_config_file_list_head(void)
{
	struct list_head *head = NULL; 
	
	cste_uci_read_lock();
	if (!list_empty(&uci_package_head))
	{
		head = &uci_package_head; 
	}
	cste_uci_read_unlock();
	
	return head;
}

/*****************************************************
 *   uci config file contrl ops
 ****************************************************/
EXPORT_SYMBOL_GPL(init_uci_proc_dir);
EXPORT_SYMBOL_GPL(creat_uci_proc_file);
EXPORT_SYMBOL_GPL(uci_default_config_init);
EXPORT_SYMBOL_GPL(uci_config_ctl_exit);
EXPORT_SYMBOL_GPL(get_uci_config_file_list_head);
EXPORT_SYMBOL_GPL(uci_add);
EXPORT_SYMBOL_GPL(uci_del);
#endif

