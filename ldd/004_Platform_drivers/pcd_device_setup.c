#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform.h"

//Creating two platform data which will function as private data for the devices

void pcdev_release(struct device *);

struct pcdev_platform_data pcdev_pdata[2] = {
    [0] = {
        .size = 512 , .perm = RDRW , .serial_number = "PCDEV_5033"
    },
    [1] = {
        .size = 1024 , .perm = RDRW , .serial_number = "PCDEV_5034"
    }
};

//Creating two platform devices

struct platform_device platform_pcdev_1 = {
    .name = "PCDEV-A1X",
    .id = 0,
    .dev = {
        .platform_data = &pcdev_pdata[0],
        .release = pcdev_release
    }
};

struct platform_device platform_pcdev_2 = {
    .name = "PCDEV-B1X",
    .id = 1,
    .dev = {
        .platform_data = &pcdev_pdata[1],
        .release = pcdev_release
    }
};


static int __init pcdev_platform_init(void){
    
    platform_device_register(&platform_pcdev_1);
    platform_device_register(&platform_pcdev_2);

#if 0
    //Can also be done as below

    struct platform_device *platform_devices[] = {
        &platform_pcdev_1,
        &platform_pcdev_2
    };


    
    platform_add_devices(platform_devices , 2);

#endif
    
    pr_info("Device setup module inserted\n");

    return 0;
}

static void __exit pcdev_platform_exit(void){

    platform_device_unregister(&platform_pcdev_1);
    platform_device_unregister(&platform_pcdev_2);
    pr_info("Device setup module removed\n");
}

void pcdev_release(struct device *dev){
    pr_info("Device released\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers platform devices.");