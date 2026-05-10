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
    struct list_head list_meta;
};

static int __init dog_init(void) {
    printk(KERN_INFO "Linked list initialized\n");

    //================= Below code is allocation ==================

    struct book_keeping *node1 =
        kmalloc(sizeof(struct book_keeping) , GFP_KERNEL);

    if(node1 == NULL){
        printk(KERN_ERR "Memory allocation failed for node1\n");
        return -1;
    }

    node1->id = 1;
    node1->long_id = 1001;

    list_add(&node1->list_meta , &head);

    struct book_keeping *node2 =
        kmalloc(sizeof(struct book_keeping) , GFP_KERNEL);

    if(node2 == NULL){
        printk(KERN_ERR "Memory allocation failed for node2\n");
        kfree(node1);
        return -1;
    }

    node2->id = 2;
    node2->long_id = 1002;

    //Below line adds node into list
    struct book_keeping *temp_node;

    list_add(&node2->list_meta , &head);

    //Below line is for traversing the list

    list_for_each_entry(temp_node , &head , list_meta){
        printk(KERN_INFO "Node ID: %d, Long ID: %d\n" , temp_node->id , temp_node->long_id);
    }

    //Below code is for deleting the list and freeing the memory

    printk(KERN_INFO "removing %d\n", node2->id);
    list_del(&(node2->list_meta));

    kfree(node2);

    list_for_each_entry(temp_node, &head, list_meta) {
        printk(KERN_INFO "node id: %d\n", temp_node->id);
    }

    return 0;
}

static void __exit dog_exit(void) {

    struct book_keeping *temp_node;

    //Important to free the memory allocated for the nodes in the list

    list_for_each_entry(temp_node, &head, list_meta) {
        kfree(temp_node);
    }
    printk(KERN_INFO "module unloaded!\n");
}

module_init(dog_init);
module_exit(dog_exit);

MODULE_AUTHOR("Street Dogg <streetdogg@pyjamabrah.com>");
MODULE_LICENSE("GPL");