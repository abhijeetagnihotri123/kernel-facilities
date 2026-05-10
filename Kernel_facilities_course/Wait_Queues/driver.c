/*
 * Copyright © 2025 pyjamabrah.com. All Rights Reserved.
 * Author: Piyush Itankar <piyush@pyjamabrah.com>
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static DECLARE_WAIT_QUEUE_HEAD(dogg_wait_queue);
static int wake_condition = 0;

/* declare a work queue*/
static struct work_struct work_queue;

static void simulate_io(struct work_struct *w) {
    printk(KERN_INFO "simulate_io(): entry\n");

    printk(KERN_INFO "simulate_io(): setting wake_condition == 1\n");

    msleep(2500);

    wake_condition = 1;
    wake_up_interruptible(&dogg_wait_queue);

    printk(KERN_INFO "simulate_io(): exit\n");
    return;
}

static int __init dog_init(void) {
    printk(KERN_INFO "dog_init(): entry!\n");

    INIT_WORK(&work_queue, simulate_io);
    schedule_work(&work_queue);

    printk(KERN_INFO "dog_init(): starting to wait on wake_condition != 0\n");
    wait_event_interruptible(dogg_wait_queue, wake_condition != 0);
    printk(KERN_INFO "dog_init(): exited wait state, wake_condition != 0\n");

    printk(KERN_INFO "dog_init(): exit!\n");
    return 0;
}

static void __exit dog_exit(void) {
    printk(KERN_INFO "Good bye!\n");
}

module_init(dog_init);
module_exit(dog_exit);

MODULE_AUTHOR("Street Dogg <streetdogg@pyjamabrah.com>");
MODULE_LICENSE("GPL");