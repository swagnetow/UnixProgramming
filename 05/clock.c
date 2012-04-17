/* Modified demo code from Dr. Murphy's 615 website */
#include <asm/siginfo.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jpv");

#define FILE "myclock"
#define SIG_TEST 44

struct siginfo info;
static int delay;
static struct timer_list timer;
static struct task_struct *t;

void timer_callback(unsigned long data) {
    int return_value;

    memset(&info, 0, sizeof(struct siginfo));

    info.si_signo = SIG_TEST;
    info.si_code = SI_QUEUE;
    info.si_int = delay;
    return_value = send_sig_info(SIG_TEST, &info, t);

    if(return_value < 0) {
        printk("kobject error sending signal\n");
    }
}

static ssize_t myclock2_show(struct kobject* kobj, struct kobj_attribute* attr, char* buffer) {
    printk("myclock2 (myclock2_show): pid = %d", current->pid);

    t = current;

    return sprintf(buffer, "%d %d", (int)CURRENT_TIME.tv_sec, (int)CURRENT_TIME.tv_nsec/1000);
}

static ssize_t myclock2_store(struct kobject *kobj, struct kobj_attribute *attr, const char* buffer, size_t count) {
    printk("myclock2 (myclock2_store): pid = %d.\n", current->pid);

    return count;
}

static ssize_t delay_show(struct kobject* kobj, struct kobj_attribute* attr, char* buffer) {
    int return_value;

    printk("delay (delay_show): pid = %d, delay = %d.\n", current->pid, delay);

    t = current;

    printk("delay (delay_show): Starting timer to fire in %d seconds.\n", delay);

    return_value = mod_timer(&timer, jiffies + msecs_to_jiffies(delay * 1000));

    if(return_value) {
        printk("delay (delay_show): Error in setting timer!\n");
    }

    return sprintf(buffer, "%d\n", delay);
}

static ssize_t delay_store(struct kobject *kobj, struct kobj_attribute *attr, const char* buffer, size_t count) {
    sscanf(buffer, "%du", &delay);

    printk("delay (delay_store): pid = %d, delay = %d.\n", current->pid, delay);

    return count;
}

static struct kobj_attribute delay_attribute = __ATTR(delay, 0666, delay_show, delay_store);

static struct kobj_attribute myclock2_attribute = __ATTR(myclock2, 0666, myclock2_show, myclock2_store);

/* Returns a single ASCII string with two numerical substrings seperated by a delayngle space correseponding
 * to the number of seconds in the current epoch and the number of microseconds in the current second.
 */
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

static struct attribute *delay_attrs[] = {
    &delay_attribute.attr,
    NULL,
};

static struct attribute_group delay_attr_group = {
    .attrs = delay_attrs,
};

static struct attribute *myclock2_attrs[] = {
    &myclock2_attribute.attr,
    NULL,
};

static struct attribute_group myclock2_attr_group = {
    .attrs = myclock2_attrs,
};

static struct kobject *kobj_delay;
static struct kobject *kobj_myclock2;

static int __init mod_init(void) {
    int delay_value;
    int myclock2_value;
    struct proc_dir_entry *file;

    /* proc filesystem */
    printk(KERN_ALERT "Creating a /proc/myclock file.\n");

    file = create_proc_entry(FILE, 0644, NULL);

    if(file == NULL) {
        remove_proc_entry(FILE, NULL);
        printk(KERN_ALERT "Error in creating /proc/myclock entry!\n");
        return -ENOMEM;
    }

    file->read_proc = mod_file_reader;
    file->mode = S_IFREG | S_IRUGO;
    file->uid = 42;
    file->gid = 42;
    file->size = 0;

    printk(KERN_ALERT "/proc/myclock has been created!\n");

    /* sys filesystem */
    setup_timer(&timer, timer_callback, 0);

    kobj_delay = kobject_create_and_add("delay", kernel_kobj);
    kobj_myclock2 = kobject_create_and_add("myclock2", kernel_kobj);

    if(!kobj_delay) {
        return -ENOMEM;
    }

    if(!kobj_myclock2) {
        return -ENOMEM;
    }

    myclock2_value = sysfs_create_group(kobj_myclock2, &myclock2_attr_group);
    delay_value = sysfs_create_group(kobj_delay, &delay_attr_group);

    if(delay_value) {
        kobject_put(kobj_delay);
    }

    if(myclock2_value) {
        kobject_put(kobj_myclock2);
    }

    return 0;
}

static void __exit mod_exit(void) {
    int return_value;

    /* proc filesystem */
    printk(KERN_ALERT "Goodbye!\n");
    remove_proc_entry(FILE, NULL);

    /* sys filesystem */
    return_value = del_timer(&timer);

    if(return_value) {
        printk("myclock2: The timer is still in use!\n");
    }

    printk("myclock2: Uninstalling!\n");

    kobject_put(kobj_myclock2);
    kobject_put(kobj_delay);
}

module_init(mod_init);
module_exit(mod_exit);
