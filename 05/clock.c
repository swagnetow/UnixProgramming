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

#define FILE "myclock"
#define SIG_TEST 44

struct siginfo info;
static int si;
static struct timer_list timer;
static struct task_struct *t;

void timer_callback(unsigned long data) {
    int return_value;

    memset(&info, 0, sizeof(struct siginfo));

    info.si_signo = SIG_TEST;
    info.si_code = SI_QUEUE;
    info.si_int = si;
    return_value = send_sig_info(SIG_TEST, &info, t);

    if(return_value < 0) {
        printk("kobject error sending signal\n");
    }
}

/* Returns a single ASCII string with two numerical substrings seperated by a single space correseponding
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

    return(return_value);
}

static int mod_init(void) {
    struct proc_dir_entry *file;

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

    return(0);
}

static void mod_exit(void) {
    printk(KERN_ALERT "Goodbye!\n");
    remove_proc_entry(FILE, NULL);
}

module_init(mod_init);
module_exit(mod_exit);
