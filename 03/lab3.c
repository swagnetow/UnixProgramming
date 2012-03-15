/* Modified demo code from Dr. Murphy's 615 website */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/utsname.h>

#define FILE "hello"

MODULE_LICENSE("Dual BSD/GPL");

/* Code from mod_demo2.c */
int mod_file_reader(char* buffer, char** buffer_location, off_t offset, int length, int* eof, void* data) {
    int return_value;

    if(offset > 0) {
        return_value = 0;
    }
    else {
        /* Print hello world in the /proc filesystem */
        return_value = sprintf(buffer, "Hello world!\n");
    }

    return(return_value);
}

static int mod_init(void) {
    struct new_utsname *uts;
    struct proc_dir_entry *file;

    uts = utsname();

    printk(KERN_ALERT "Hello from %s on %02d:%02d:%02d GMT!\n", uts->nodename, (int)CURRENT_TIME.tv_sec/3600%24, (int)CURRENT_TIME.tv_sec/60%60, (int)CURRENT_TIME.tv_sec%60);

    printk(KERN_ALERT "Creating a /proc file.\n");

    file = create_proc_entry(FILE, 0644, NULL);

    if(file == NULL) {
        remove_proc_entry(FILE, NULL);
        printk(KERN_ALERT "Error in creating /proc/hello entry!\n");
        return -ENOMEM;
    }

    file->read_proc = mod_file_reader;
    file->mode = S_IFREG | S_IRUGO;
    file->uid = 42;
    file->gid = 42;
    file->size = 0;

    printk(KERN_ALERT "/proc/hello has been created!\n");

    return(0);
}

static void mod_exit(void) {
    printk(KERN_ALERT "Goodbye!\n");
}

module_init(mod_init);
module_exit(mod_exit);
