#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kdev_t.h>

#include <linux/uaccess.h>
#include <linux/err.h>

#define DEV_MEM_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

char device_buffer[DEV_MEM_SIZE];

//This holds the device number
// sudo -i to get permissions to read and write
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
    
    pr_info("Current file position = %lld\n" , *offset);

    if((*offset + count) > DEV_MEM_SIZE){
        count = DEV_MEM_SIZE - *offset;
    }
    if(copy_to_user(buff , &device_buffer[*offset] , count)){
        return -EFAULT;
    }
    
    *offset += count;
    
    pr_info("Number of bytes successfully read = %zu\n" , count);
    pr_info("Updated file position = %lld\n" , *offset);

    return count;
}

ssize_t pcd_write (struct file *pcd_file, const char __user *buff, size_t count, loff_t *offset)
{
    pr_info("Write requested for %zu bytes\n" , count);
    pr_info("Current file position = %lld\n" , *offset);

    if((*offset + count) > DEV_MEM_SIZE){
        count = DEV_MEM_SIZE - *offset; 
    }

    if(count == 0){
        return -ENOMEM;
    }

    if(copy_from_user(&device_buffer[*offset] , buff , count)){
        return -EFAULT;
    }

    *offset += count;


    pr_info("Number of bytes successfully written = %zu\n" , count);
    pr_info("Updated file position = %lld\n" , *offset);

    return count;
}


loff_t pcd_llseek (struct file *pcd_file, loff_t offset, int whence){
    pr_info("Lseek requested\n");

    pr_info("current file position = %lld\n" , pcd_file->f_pos);

    loff_t temp;

    switch(whence){
        case SEEK_SET:
            if((offset > DEV_MEM_SIZE) || (offset < 0)){
                return -EINVAL;
            }
            pcd_file->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = pcd_file->f_pos + offset;
            if(temp > DEV_MEM_SIZE || temp < 0){
                return -EINVAL;
            }
            pcd_file->f_pos += temp;
            break;
        case SEEK_END:
            temp = pcd_file->f_pos + offset;
            if(temp > DEV_MEM_SIZE || temp < 0){
                return -EINVAL;
            }
            pcd_file->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }

    pr_info("New value of the file position = %lld\n" , pcd_file->f_pos);

    return pcd_file->f_pos;
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
    
    int ret;

    //1. Dynamically allocate a device number

    ret = alloc_chrdev_region(&device_number , 0 , 1 , "pcd_devices");

    if(ret < 0){
        goto out;
    }

    pr_info("%s : Device number <major>:<minor> = %d:%d\n" , __func__ ,  MAJOR(device_number) , MINOR(device_number));

    //2. Initialize the cdev structure with fops
    cdev_init(&pcd_cdev , &pcd_fops);

    pcd_cdev.owner = THIS_MODULE;

    //3. Register a device(cdev structure) with VFS
    ret = cdev_add(&pcd_cdev , device_number , 1);

    if(ret < 0){
        goto unregister_chrdev;
    }

    //4. Create a device class under /sys/class
    class_pcd = class_create("pcd_class");

    if(IS_ERR(class_pcd)){
        pr_err("Class creation failed\n");
        ret = PTR_ERR(class_pcd);
        goto cdev_del;
    }

    //5. Device file creation or populate sysfs with device information
    device_pcd = device_create(class_pcd , NULL , device_number , NULL , "pcd");

    if(IS_ERR(device_pcd)){
        pr_err("Device creation failed");
        ret = PTR_ERR(device_pcd);
        goto class_del;
    }

    pr_info("Module init was successful\n");

    return 0;
class_del:
    device_destroy(class_pcd , device_number);

cdev_del:
    cdev_del(&pcd_cdev);

unregister_chrdev:
    unregister_chrdev_region(device_number , 1);

out:
    pr_info("Module insertion failed\n");
    return ret;

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
