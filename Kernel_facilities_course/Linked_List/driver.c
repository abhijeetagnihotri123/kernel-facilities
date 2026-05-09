/*
 * Copyright © 2025 pyjamabrah.com. All Rights Reserved.
 * Author: Piyush Itankar <piyush@pyjamabrah.com>
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/list.h>

static LIST_HEAD(head);

struct book_keeping{
    int id;
    int long_id;
    struct list_head list;
};

static int __init dog_init(void) {
    printk(KERN_INFO "Linked list initialized\n");

    

    return 0;
}

static void __exit dog_exit(void) {
    printk(KERN_INFO "module unloaded!\n");
}

module_init(dog_init);
module_exit(dog_exit);

MODULE_AUTHOR("Street Dogg <streetdogg@pyjamabrah.com>");
MODULE_LICENSE("GPL");