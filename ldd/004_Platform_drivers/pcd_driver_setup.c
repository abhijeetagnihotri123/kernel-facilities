#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
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

    /*1. Unregister the platform driver */
    platform_driver_unregister(&pcd_platform_driver);

    /*2. Destroy class */
    class_destroy(pcdrv_data.class_pcd);


    /*3. Unregister the device number for max devices*/
    unregister_chrdev_region(pcdrv_data.device_base_number , MAX_DEVICE_NUMBER);
    

    pr_info("Driver unregistered");

}


module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

int pcd_platform_driver_probe(struct platform_device *pdev){
    
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data *pdata;

    int ret;

    /*1. Get the platform data*/
    // or (struct pcdev_platform_data*)dev_get_platdata(&pdev->data)

    pr_info("A device is detected\n");

    pr_info("1. Getting the platform device data");
    pdata = pdev->dev.platform_data;

    if(!pdata){
        pr_info("No platform data available\n");
        ret = -EINVAL;
        goto out;
    }

    /*2. Dynamically allocate memory for the device private data*/
    
    dev_data = kzalloc(sizeof(struct pcdev_private_data) , GFP_KERNEL);

    pr_info("2. KZAlloc to create memory for device specifications");

    if(!dev_data){
        pr_info("Cannot allocate memory\n");
        ret = -ENOMEM;
        goto out;
    }

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.perm = pdata->perm;
    dev_data->pdata.serial_number = pdata->serial_number;

    pr_info("Size of the device buffer : %d" , dev_data->pdata.size);
    pr_info("Serial number of the device : %s" , dev_data->pdata.serial_number);
    pr_info("Permissions the device buffer : %d" , dev_data->pdata.perm);
    
    pr_info("3. KZAlloc to create data\n");

    dev_data->buffer = kzalloc(dev_data->pdata.size , GFP_KERNEL);

    if(!dev_data->buffer){
        pr_info("Cannot allocate memory for device buffer\n");
        ret = -ENOMEM;
        goto dev_data_free;
    }

    pr_info("4. Get the base device number + index number\n");
    dev_data->dev_number = pcdrv_data.device_base_number + pdev->id;

    pr_info("5. Do Cdev init and cdev add\n");

    cdev_init(&dev_data->pcd_cdev , &pcd_fops);

    ret = cdev_add(&dev_data->pcd_cdev , dev_data->dev_number , 1);

    if(ret < 0){
        pr_err("Cdev add failed\n");
        goto buffer_free;
    }

    pr_info("6. Create device files\n");
    pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd , NULL , dev_data->dev_number , NULL , "pcd%d" , pdev->id);
    
    if(IS_ERR(pcdrv_data.device_pcd)){
        pr_err("Device creation failed\n");
        ret = PTR_ERR(pcdrv_data.device_pcd);
        goto cdev_deletion;
    }

    pr_info("The probe was successful\n");

    return 0;

cdev_deletion:
    cdev_del(&dev_data->pcd_cdev);

buffer_free:
    kfree(dev_data->buffer);


dev_data_free:
    kfree(dev_data);

out:
    pr_info("Device probe failed\n");
    return ret;

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

