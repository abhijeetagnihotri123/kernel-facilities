
#define RDRW 0x11
#define RD_ONLY 0x01
#define WR_ONLY 0x10

struct pcdev_platform_data{
    int size;
    int perm;
    const char *serial_number;
};