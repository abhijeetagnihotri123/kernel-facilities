#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kdev_t.h>

#define DEV_MEM_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

char device_buffer[DEV_MEM_SIZE];

//This holds the device number
dev_t device_number;

int pcd_open (struct inode *pcd_inode, struct file *pcd_file){
    pr_info("Open this device file\n");
    return 0;
}

int pcd_release(struct inode *pcd_inode , struct file *pcd_file){
 
    pr_info("Release this device file\n");
    return 0;
}

ssize_t pcd_read(struct file *pcd_file , char __user *buff , size_t count , loff_t *offset){
    
    pr_info("Read requested for %zu bytes\n" , count);
    return 0;
}

ssize_t pcd_write (struct file *pcd_file, const char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Write requested for %zu bytes\n" , count);
    return 0;
}


loff_t pcd_llseek (struct file *pcd_file, loff_t offset, int whence){
    pr_info("Lseek requested\n");
    return 0;
}



// Cdev variable
struct cdev pcd_cdev;

// file operations of the driver
struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_llseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};

struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_driver_init(void) {
    printk(KERN_INFO "LDD module initialized\n");
    
    //1. Dynamically allocate a device number

    alloc_chrdev_region(&device_number , 0 , 1 , "pcd_devices");

    pr_info("%s : Device number <major>:<minor> = %d:%d\n" , __func__ ,  MAJOR(device_number) , MINOR(device_number));

    //2. Initialize the cdev structure with fops
    cdev_init(&pcd_cdev , &pcd_fops);

    pcd_cdev.owner = THIS_MODULE;

    //3. Register a device(cdev structure) with VFS
    cdev_add(&pcd_cdev , device_number , 1);

    //4. Create a device class under /sys/class
    class_pcd = class_create("pcd_class");

    //5. Device file creation or populate sysfs with device information
    device_pcd = device_create(class_pcd , NULL , device_number , NULL , "pcd");

    pr_info("Module init was successful\n");

    return 0;
}

static void __exit pcd_driver_exit(void) {

    device_destroy(class_pcd , device_number);
    class_destroy(class_pcd);

    cdev_del(&pcd_cdev);

    unregister_chrdev_region(device_number , 1);

    printk(KERN_INFO "LDD module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);
