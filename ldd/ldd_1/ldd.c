#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");


static int ldd_init(void) {
    printk(KERN_INFO "LDD module initialized\n");
    return 0;
}

static void ldd_exit(void) {
    printk(KERN_INFO "LDD module exited\n");
}

module_init(ldd_init);
module_exit(ldd_exit);