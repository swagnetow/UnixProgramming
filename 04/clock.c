/* Modified demo code from Dr. Murphy's 615 website */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

#define FILE "myclock"

MODULE_LICENSE("Dual BSD/GPL");

/* Returns a single ASCII string with two numerical substrings seperated by a single space correseponding
 * to the number of seconds in the current epoch and the number of microseconds in the current second.
 */
int mod_file_reader(char* buffer, char** buffer_location, off_t offset, int length, int* eof, void* data) {
    int return_value;

    if(offset > 0) {
        return_value = 0;
    }
    else {
        return_value = sprintf(buffer, "%i %i", (int)CURRENT_TIME.tv_sec, (int)CURRENT_TIME.tv_nsec/1000);
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
