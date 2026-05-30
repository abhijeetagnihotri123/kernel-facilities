#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/err.h>

#include <linux/spinlock.h>
#include <linux/mutex.h>

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_DEV1 1024
#define MEM_SIZE_MAX_DEV2 1024
#define MEM_SIZE_MAX_DEV3 1024
#define MEM_SIZE_MAX_DEV4 1024


#define RDONLY 0x01
#define WRONLY 0x10
#define RDWW 0x11

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sample Author");
MODULE_DESCRIPTION("A simple LDD module");

char device_buffer_device1[MEM_SIZE_MAX_DEV1];
char device_buffer_device2[MEM_SIZE_MAX_DEV2];
char device_buffer_device3[MEM_SIZE_MAX_DEV3];
char device_buffer_device4[MEM_SIZE_MAX_DEV4];

//This holds the device number
// sudo -i to get permissions to read and write

struct pcdev_private_data{

    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev pcd_cdev;

    spinlock_t pcd_spin_lock;
    struct mutex pcd_mutex_lock;

};

struct pcdrv_private_data{

    int total_devices;
    dev_t device_number;
    struct class *class_pcd;
    struct device *device_pcd;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];

};


struct pcdrv_private_data private_drv_data = {

    .total_devices = NO_OF_DEVICES,
    .pcdev_data = { 
        [0] = {
            .buffer = device_buffer_device1,
            .size = MEM_SIZE_MAX_DEV1,
            .serial_number = "PCDEV5033",
            .perm = RDONLY /*Read only*/
        },
        [1] = {
            .buffer = device_buffer_device2,
            .size = MEM_SIZE_MAX_DEV2,
            .serial_number = "PCDEV5034",
            .perm = WRONLY /*Write only*/
        },
        [2] = {
            .buffer = device_buffer_device3,
            .size = MEM_SIZE_MAX_DEV3,
            .serial_number = "PCDEV5035",
            .perm = RDWW /*Both Read and Write*/
        },
        [3] = {
            .buffer = device_buffer_device4,
            .size = MEM_SIZE_MAX_DEV4,
            .serial_number = "PCDEV5036",
            .perm = RDWW /*Both Read and Write*/
        }
    }

};


int check_permission(int dev_perm , int mode){
    
    if(dev_perm == RDWW){
        return 0;
    }
    if((dev_perm == RDONLY) && (mode && FMODE_READ) && !(mode && FMODE_WRITE)){
        return 0;
    }
    if((dev_perm == WRONLY) && (mode && FMODE_WRITE) && !(mode && FMODE_READ)){
        return 0;
    }

    return -EPERM;
}

int pcd_open (struct inode *pcd_inode, struct file *pcd_file){
    pr_info("Open this device file\n");

    int ret;
    int minor_n;

    struct pcdev_private_data *pcdev_data;

    minor_n = MINOR(pcd_inode->i_rdev);
    pr_info("Minor number of the device %d\n" , minor_n);

    pcdev_data = container_of(pcd_inode->i_cdev , struct pcdev_private_data , pcd_cdev);

    /*To supply device private data to other methods of the driver*/

    pcd_file->private_data = pcdev_data;

    /*Check permission*/

    ret = check_permission(pcdev_data->perm , pcd_file->f_mode);

    (!ret)?pr_info("Open was successful\n"):pr_info("Open was unsuccessful\n");

    return ret;
}

int pcd_release(struct inode *pcd_inode , struct file *pcd_file){
 
    pr_info("Release this device file\n");
    return 0;
}

ssize_t pcd_read(struct file *pcd_file , char __user *buff , size_t count , loff_t *offset){

    pr_info("Read requested for %zu bytes\n" , count);
    
    pr_info("Current file position = %lld\n" , *offset);

    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)pcd_file->private_data;

    mutex_lock(&pcdev_data->pcd_mutex_lock);

    int max_size = pcdev_data->size;

    if((*offset + count) > max_size){
        count = max_size - *offset;
    }
    if(copy_to_user(buff , &pcdev_data->buffer[*offset] , count)){
        mutex_unlock(&pcdev_data->pcd_mutex_lock);
        return -EFAULT;
    }
    
    *offset += count;
    
    pr_info("Number of bytes successfully read = %zu\n" , count);
    pr_info("Updated file position = %lld\n" , *offset);
    mutex_unlock(&pcdev_data->pcd_mutex_lock);
    
    return count;
}

ssize_t pcd_write (struct file *pcd_file, const char __user *buff, size_t count, loff_t *offset)
{

    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)pcd_file->private_data;

    int max_size = pcdev_data->size;

    mutex_lock(&pcdev_data->pcd_mutex_lock);

    pr_info("Write requested for %zu bytes\n" , count);
    pr_info("Current file position = %lld\n" , *offset);

    if((*offset + count) > max_size){
        count = max_size - *offset; 
    }

    if(count == 0){
        pr_err("No space left on the device\n");
        mutex_unlock(&pcdev_data->pcd_mutex_lock);
        return -ENOMEM;
    }

    if(copy_from_user(&pcdev_data->buffer[*offset] , buff , count)){
        return -EFAULT;
    }

    *offset += count;

    pr_info("Number of bytes successfully written = %zu\n" , count);
    pr_info("Updated file position = %lld\n" , *offset);
    mutex_unlock(&pcdev_data->pcd_mutex_lock);
    return count;
}


loff_t pcd_llseek (struct file *pcd_file, loff_t offset, int whence){

    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)pcd_file->private_data;

    int max_size = pcdev_data->size;

    pr_info("Lseek requested\n");
    pr_info("current file position = %lld\n" , pcd_file->f_pos);

    loff_t temp;

    switch(whence){
        case SEEK_SET:
            if((offset > max_size) || (offset < 0)){
                return -EINVAL;
            }
            pcd_file->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = pcd_file->f_pos + offset;
            if(temp > max_size || temp < 0){
                return -EINVAL;
            }
            pcd_file->f_pos += temp;
            break;
        case SEEK_END:
            temp = pcd_file->f_pos + offset;
            if(temp > max_size || temp < 0){
                return -EINVAL;
            }
            pcd_file->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }

    pr_info("New value of the file position = %lld\n" , pcd_file->f_pos);

    return pcd_file->f_pos;

    return 0;
}


// file operations of the driver
struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_llseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};


static int __init pcd_driver_init(void) {
    
    printk(KERN_INFO "LDD module initialized\n");
    
    //1. Dynamically allocate a device number
    int ret;

    ret = alloc_chrdev_region(&private_drv_data.device_number , 0 , NO_OF_DEVICES , "pcd_devices");

    if(ret < 0)
    {
        goto out;
    }

    //4. Create a device class under /sys/class
    private_drv_data.class_pcd = class_create("pcd_class");

    if(IS_ERR(private_drv_data.class_pcd)){
        pr_err("Class creation failed\n");
        ret = PTR_ERR(private_drv_data.class_pcd);
        goto unregister_chrdev;
    }

    int i;

    for(i = 0 ; i < NO_OF_DEVICES ; i++)
    {
        pr_info("%s : Device number <major>:<minor> = %d:%d\n" , __func__ ,  MAJOR(private_drv_data.device_number + i) , MINOR(private_drv_data.device_number + i));
    
        //2. Initialize the cdev structure with fops
        cdev_init(&private_drv_data.pcdev_data[i].pcd_cdev , &pcd_fops);

        private_drv_data.pcdev_data[i].pcd_cdev.owner = THIS_MODULE;

        mutex_init(&private_drv_data.pcdev_data[i].pcd_mutex_lock);

        //3. Register a device(cdev structure) with VFS
        ret = cdev_add(&private_drv_data.pcdev_data[i].pcd_cdev , private_drv_data.device_number + i , 1);

        if(ret < 0){
            goto class_del;
        }

        //5. Device file creation or populate sysfs with device information
        private_drv_data.device_pcd = device_create(private_drv_data.class_pcd , NULL , private_drv_data.device_number + i , NULL , "pcdev-%d" , i);

        if(IS_ERR(private_drv_data.device_pcd)){
            pr_err("Device creation failed");
            ret = PTR_ERR(private_drv_data.device_pcd);
            goto class_del;
        }
    }

    pr_info("Module init was successful\n");
    return 0;

class_del:
    for(; i >= 0 ; i--){
        device_destroy(private_drv_data.class_pcd , private_drv_data.device_number + i);
        cdev_del(&private_drv_data.pcdev_data[i].pcd_cdev);
    }
    class_destroy(private_drv_data.class_pcd);

unregister_chrdev:
    unregister_chrdev_region(private_drv_data.device_number , NO_OF_DEVICES);

out:
    pr_info("Module insertion failed\n");
    return ret;

}

static void __exit pcd_driver_exit(void) {

    for(int i = 0 ; i < NO_OF_DEVICES ; i++){
        device_destroy(private_drv_data.class_pcd , private_drv_data.device_number + i);
        cdev_del(&private_drv_data.pcdev_data[i].pcd_cdev);
    }

    class_destroy(private_drv_data.class_pcd);
    unregister_chrdev_region(private_drv_data.device_number , NO_OF_DEVICES);

    printk(KERN_INFO "LDD module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);
