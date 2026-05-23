#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#define DEV_MEM_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

char device_buffer[DEV_MEM_SIZE];

//This holds the device number
dev_t device_number;

int pcd_open (struct inode *pcd_inode, struct file *pcd_file){
    return 0;
}

int pcd_release(struct inode *pcd_inode , struct file *pcd_file){
    return 0;
}

ssize_t pcd_read(struct file *pcd_file , char __user *buff , size_t count , loff_t *offset){
    return 0;
}

size_t pcd_write(struct file *pcd_file , char __user *buff , size_t count , loff_t *offset){
    return 0;
}

loff_t pcd_llseek (struct file *pcd_file, loff_t offset, int whence){
    return 0;
}



// Cdev variable
struct cdev pcd_cdev;

// file operations of the driver
struct file_operations pcd_fops;

static int __init pcd_driver_init(void) {
    printk(KERN_INFO "LDD module initialized\n");
    
    //1. Dynamically allocate a device number

    alloc_chrdev_region(&device_number , 0 , 1 , "pcd_driver");
    

    //2. Initialize the cdev structure with fops
    cdev_init(&pcd_cdev , &pcd_fops);

    pcd_cdev.owner = THIS_MODULE;

    return 0;
}

static void __exit pcd_driver_exit(void) {
    printk(KERN_INFO "LDD module exited\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);
