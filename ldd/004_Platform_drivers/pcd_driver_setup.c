#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include "platform.h"

#define MAX_DEVICE_NUMBER 10

int check_permission(int,int);
int pcd_open (struct inode *, struct file *);
int pcd_release(struct inode * , struct file *);
ssize_t pcd_write (struct file *, const char __user *, size_t, loff_t *);
ssize_t pcd_read(struct file * , char __user *, size_t, loff_t *);
ssize_t pcd_write (struct file *, const char __user *, size_t, loff_t *);
loff_t pcd_llseek (struct file *, loff_t,int);
loff_t pcd_llseek (struct file *, loff_t, int);

// file operations of the driver
struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_llseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};

//When matched platform device is found
int pcd_platform_driver_probe(struct platform_device *);

//When the device gets removed
void pcd_platform_driver_remove(struct platform_device *);

struct platform_driver pcd_platform_driver = {

    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .driver = {
        .name = "pseudo-char-device"
    }

};


struct pcdev_private_data{
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_number;
    struct cdev pcd_cdev;
};

struct pcdrv_private_data{
    int total_devices;
    dev_t device_base_number;
    struct class *class_pcd;
    struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data;

static int __init pcd_driver_init(void) {

    /*1. Dynamically allocate a device number of max devices*/
    pr_info("1. Allocating device number for MAX devices\n");
    
    int ret = alloc_chrdev_region(&pcdrv_data.device_base_number , 0 , MAX_DEVICE_NUMBER , "pcd_devices");

    if(ret < 0){
        pr_err("Alloc chrdev failed\n");
        return ret;
    }

    /*2. Create device class under /sys/class */
    
    pr_info("2. Creating device class under /sys/class \n");
    pcdrv_data.class_pcd = class_create("pcd_class");

    if(IS_ERR(pcdrv_data.class_pcd)){
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_base_number , MAX_DEVICE_NUMBER);
        return ret;
    }


    /*3. Register a platform driver*/
    platform_driver_register(&pcd_platform_driver);
    pr_info("%s , Driver registered" , __func__);
    return 0;
 
}

static void __exit pcd_driver_exit(void) {

    platform_driver_unregister(&pcd_platform_driver);
    pr_info("Driver unregistered");

}


module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

int pcd_platform_driver_probe(struct platform_device *pdev){
    pr_info("%s , A device is detected\n" , __func__);
    return 0;
}

void pcd_platform_driver_remove(struct platform_device *pdev){
    //return 0;
    pr_info("%s , A device is removed\n" , __func__);
}

int check_permission(int dev_perm , int mode){
    
    if(dev_perm == RDRW){
        return 0;
    }
    if((dev_perm == RD_ONLY) && (mode && FMODE_READ) && !(mode && FMODE_WRITE)){
        return 0;
    }
    if((dev_perm == WR_ONLY) && (mode && FMODE_WRITE) && !(mode && FMODE_READ)){
        return 0;
    }

    return -EPERM;
}

loff_t pcd_llseek (struct file *pcd_file, loff_t offset, int whence){
    return 0;
}

int pcd_open (struct inode *pcd_inode, struct file *pcd_file){
    
    return 0;
}

int pcd_release(struct inode *pcd_inode , struct file *pcd_file){
 
    pr_info("Release this device file\n");
    return 0;
}

ssize_t pcd_read(struct file *pcd_file , char __user *buff , size_t count , loff_t *offset){

    return count;
}

ssize_t pcd_write (struct file *pcd_file, const char __user *buff, size_t count, loff_t *offset)
{
    return count;
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

