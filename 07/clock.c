/* Modified demo code from Dr. Murphy's 615 website */
#include <asm/siginfo.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jpv");

#define FILE "myclock2"

/* Lab 6 stuff */
static int delay[5];
static struct timer_list timer[5];
static int n;

struct proc_dir_entry *file;
struct siginfo info;
static struct task_struct *t;
static struct kmem_cache *pending_alarm_cache;

struct pending_alarm {
    int signal;
    struct list_head alarm_list;
};

LIST_HEAD(my_alarm_list);

void timer_callback(unsigned long data) {
    int element = -1;
    int i;
    int return_value;

    for(i = 0; i < 5 && element == -1; i++) {
        if(delay[i] == (int)data) {
            element = i;
        }
    }

    memset(&info, 0, sizeof(struct siginfo));

    info.si_signo = SIGUSR1;
    info.si_code = SI_QUEUE;
    info.si_int = delay[element];
    return_value = send_sig_info(SIGUSR1, &info, t);

    if(return_value < 0) {
        printk(KERN_ALERT "kobject error sending signal\n");
    }

    n++;
    delay[element] = -1;
}

static ssize_t myclock2_show(struct kobject* kobj, struct kobj_attribute* attr, char* buffer) {
    printk(KERN_ALERT "myclock2 (myclock2_show): pid = %d", current->pid);

    t = current;

    return sprintf(buffer, "%d %d", (int)CURRENT_TIME.tv_sec, (int)CURRENT_TIME.tv_nsec/1000);
}

static ssize_t myclock2_store(struct kobject *kobj, struct kobj_attribute *attr, const char* buffer, size_t count) {
    printk(KERN_ALERT "myclock2 (myclock2_store): pid = %d.\n", current->pid);

    return count;
}

static ssize_t delay_show(struct kobject* kobj, struct kobj_attribute* attr, char* buffer) {
    int element;
    int return_value;
    struct list_head *pos;
    struct pending_alarm *alarm;

    element = n++;

    printk(KERN_ALERT "delay (delay_show): pid = %d, delay = %d.\n", current->pid, delay[element]);

    t = current;

    printk(KERN_ALERT "delay (delay_show): Starting timer to fire in %d seconds.\n", delay[element]);

    return_value = mod_timer(&timer[element], jiffies + msecs_to_jiffies(delay[element] * 1000));

    list_for_each(pos, &my_alarm_list) {
        alarm = list_entry(pos, struct pending_alarm, alarm_list);
        printk("%d\n", alarm->signal);
    }

    if(return_value) {
        printk(KERN_ALERT "delay (delay_show): Error in setting timer!\n");
    }

    return sprintf(buffer, "%d\n", delay[element]);
}

static ssize_t delay_store(struct kobject *kobj, struct kobj_attribute *attr, const char* buffer, size_t count) {
    int element = -1;
    int i;
    struct pending_alarm *alarm;
    alarm = kmem_cache_alloc(pending_alarm_cache, GFP_KERNEL);

    for(i = 0; i < 5 && element == -1; i++) {
        if(delay[i] == -1) {
            element = i;
        }
    }

    sscanf(buffer, "%d", &delay[element]);

    alarm->signal = delay[element];

    list_add(&alarm->alarm_list, &my_alarm_list);

    setup_timer(&timer[element], timer_callback, alarm->signal);

    printk(KERN_ALERT "delay (delay_store): pid = %d, delay = %d.\n", current->pid, alarm->signal);

    return count;
}

static struct kobject *kobj;
static struct kobj_attribute delay_attribute = __ATTR(delay, 0666, delay_show, delay_store);
static struct kobj_attribute myclock2_attribute = __ATTR(myclock2, 0666, myclock2_show, myclock2_store);

static struct attribute *attrs[] = {
    &delay_attribute.attr,
    &myclock2_attribute.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

int mod_file_reader(char* buffer, char** buffer_location, off_t offset, int length, int* eof, void* data) {
    int return_value;

    if(offset > 0) {
        return_value = 0;
    }
    else {
        return_value = sprintf(buffer, "%d %d", (int)CURRENT_TIME.tv_sec, (int)CURRENT_TIME.tv_nsec/1000);
    }

    return return_value;
}

static int __init mod_init(void) {
    int i;
    int return_value;
    n = 0;
    pending_alarm_cache = kmem_cache_create("pending_alarm", sizeof(struct pending_alarm), 0, SLAB_HWCACHE_ALIGN, NULL);

    for(i = 0; i < 5; i++) {
        delay[i] = -1;
    }

    /* procfs */
    printk(KERN_ALERT "Creating a /proc/myclock2 file.\n");

    file = create_proc_entry(FILE, 0644, NULL);

    if(file == NULL) {
        remove_proc_entry(FILE, NULL);
        printk(KERN_ALERT "Error in creating /proc/myclock2 entry!\n");
        return -ENOMEM;
    }

    file->read_proc = mod_file_reader;
    file->mode = S_IFREG | S_IRUGO;
    file->uid = 0;
    file->gid = 0;
    file->size = 0;

    printk(KERN_ALERT "/proc/myclock2 has been created!\n");

    /* sysfs */
    kobj = kobject_create_and_add("myclock2", kernel_kobj);

    if(!kobj) {
        return -ENOMEM;
    }

    return_value = sysfs_create_group(kobj, &attr_group);

    if(return_value) {
        kobject_put(kobj);
    }

    printk(KERN_ALERT "/sys/kernel/myclock2/myclock2 has been created!\n");
    printk(KERN_ALERT "/sys/kernel/myclock2/delay has been created!\n");

    return 0;
}

static void __exit mod_exit(void) {
    int i;
    int return_value;

    /* proc filesystem */
    printk(KERN_ALERT "Goodbye!\n");
    remove_proc_entry(FILE, NULL);

    /* sys filesystem */
    for(i = 0; i < 5; i++) {
        return_value = del_timer(&timer[i]);
    }

    if(return_value) {
        printk("myclock2: The timer is still in use!\n");
    }

    printk("myclock2: Uninstalling!\n");

    kobject_put(kobj);

    if(pending_alarm_cache) {
        kmem_cache_destroy(pending_alarm_cache);
    }
}

module_init(mod_init);
module_exit(mod_exit);
