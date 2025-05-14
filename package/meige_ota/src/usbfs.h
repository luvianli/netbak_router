#ifndef _USBFS_H_
#define _USBFS_H_

typedef struct
{
    char sys_path[64];
    char dev_name[32];
    char PRODUCT[32];
} USBFS_T;

typedef struct usb_modem_info
{
    const char *modem_name;
    const char *PRODUCT;
    int usb_interface;
    const char *download_atc;
} usb_modem_info;


extern int usbfs_find_meig_modules(USBFS_T *usbfs_dev, size_t max);
extern int sendSync(int usbfd, const void *pBuf, size_t len, int need_zlp);
extern int recvSync(int usbfd, void *data, size_t len, unsigned timeout);
extern int usbfs_open(const USBFS_T *usbfs_dev, int interface_no);
extern int usbfs_close(int usbfd, int interface_no);
extern int strStartsWith(const char *line, const char *prefix);

extern const struct usb_modem_info * find_usb_modem(const char *PRODUCT);

#endif

