#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

static struct proc_dir_entry *proc_entry;

static struct proc_ops driver_proc_ops ={

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