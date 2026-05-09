#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

static struct proc_dir_entry *proc_entry;

ssize_t	(*proc_read)(struct file *, char __user *, size_t, loff_t *);

ssize_t LDD_READ(struct file *file_pointer,
                 char *user_space_buffer,
                 size_t count,
                 loff_t *offset) {
    printk(KERN_INFO "LDD_READ called\n");

    char msg[] = "Ack!\n";
    size_t msg_len = sizeof(msg);

    if(*offset >= msg_len){
        return 0;
    }

    int result = copy_to_user(
        user_space_buffer,
        msg,
        msg_len
    );

    *offset += msg_len;

    return msg_len;
}

static struct proc_ops driver_proc_ops ={
    .proc_read = LDD_READ
};

static int ldd_init(void) {

    printk(KERN_INFO "LDD module loaded\n");

    proc_entry = proc_create("LDD_PROC_MODULE", 0 , NULL, &driver_proc_ops);

    printk(KERN_INFO "Proc entry created\n");
    return 0;
}

static void ldd_exit(void) {

    printk(KERN_INFO "LDD module exiting\n");

    proc_remove(proc_entry);

    printk(KERN_INFO "Proc entry removed\n");
}

module_init(ldd_init);
module_exit(ldd_exit);