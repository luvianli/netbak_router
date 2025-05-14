#include "std_c.h"
#include <dirent.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/usbdevice_fs.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 20)
#include <linux/usb/ch9.h>
#else
#include <linux/usb_ch9.h>
#endif
#include "usbfs.h"

typedef struct
{
    char DEVNAME[32];
    char DEVTYPE[32];
    char DRIVER[32];
    int MAJOR;
    int MINOR;
    char PRODUCT[32];
} UEVENT_T;

#define MAX_USBFS_BULK_SIZE     (16 * 1024)
#define MAX_PATH                256


static int endpoint_in  = 1;
static int endpoint_out = 1;
static int endpoint_len = 512;


void dump_data(const unsigned char *data, size_t len, char c)
{
#if 0
    unsigned i;

    for (i = 0; i < len; i++)
    {
        if (i > 128 && (i+32) < len)
            continue;

        if (i%16 == 0)
        {
            printf("\n%c%04x  ", c, i);
        }
        printf(" %02X", data[i]);
    }
    printf("\n");
#endif
}

int strStartsWith(const char *line, const char *prefix)
{
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++)
    {
        if (*line != *prefix)
        {
            return 0;
        }
    }

    return *prefix == '\0';
}

int sendSync(int usbfd, const void *data, size_t len, int need_zlp)
{
    int ret;
    struct usbdevfs_bulktransfer bulk;
    int last_len = 0;
    const void *last_data = 0;

    if (need_zlp && ((len%endpoint_len) == 0))
    {
        last_len = 1;
        len -= last_len;
        last_data = (char *)data + len;
        printf("zlp\n");
    }

    bulk.ep = endpoint_out;
    bulk.len = len;
    bulk.data = (void *)data;
    bulk.timeout = 3000;

    dump_data((unsigned char*)data, len, '>');
    ret = ioctl(usbfd, USBDEVFS_BULK, &bulk);
    if ((size_t)ret != len) printf("%s bluk out = %d, errno: %d (%s)\n", get_time(), ret, errno, strerror(errno));

    if ((size_t)ret == len && last_len)
    {
        bulk.len = last_len;
        bulk.data = (void *)last_data;
        ioctl(usbfd, USBDEVFS_BULK, &bulk);
    }

    return ret;
}

int recvSync(int usbfd, void *data, size_t len, unsigned timeout)
{
    struct usbdevfs_bulktransfer bulk;
    int ret;

    len = (len > MAX_USBFS_BULK_SIZE) ? MAX_USBFS_BULK_SIZE : len;
    bulk.ep = 0x80 | endpoint_in;
    bulk.len = len;
    bulk.data = data;
    bulk.timeout = timeout;

    ret = ioctl(usbfd, USBDEVFS_BULK, &bulk);
    if (ret == -1 && errno != ETIMEDOUT)
        printf("%s bluk in = %d, errno: %d (%s)\n", get_time(), ret, errno, strerror(errno));

    if (ret > 0)
        dump_data((unsigned char*)data, ret, '<');
    return ret;
}

static int usbfs_decode(int usbfd, int interface_no)
{
    uint8_t *desc_p;
    int desc_len, len = 0;
    int bInterfaceNumber = -1;

    endpoint_in = -1;
    endpoint_out = -1;

    desc_p = (uint8_t *)malloc(2048);
    if (!desc_p)
        return -1;

    desc_len = read(usbfd, desc_p, 2048);

    while (len < desc_len)
    {
        struct usb_descriptor_header *h = (struct usb_descriptor_header *)(desc_p + len);

        if (h->bLength == sizeof(struct usb_device_descriptor) && h->bDescriptorType == USB_DT_DEVICE)
        {
            struct usb_device_descriptor *device = (struct usb_device_descriptor *)h;

            DBPrintf("P: idVendor=%04x idProduct=%04x\n", device->idVendor, device->idProduct);
        }
        else if (h->bLength == sizeof(struct usb_config_descriptor) && h->bDescriptorType == USB_DT_CONFIG)
        {
            struct usb_config_descriptor *config = (struct usb_config_descriptor *)h;

            DBPrintf("C: bNumInterfaces: %d\n", config->bNumInterfaces);
        }
        else if (h->bLength == sizeof(struct usb_interface_descriptor) && h->bDescriptorType == USB_DT_INTERFACE)
        {
            struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)h;

            DBPrintf("I: If#= %d Alt= %d #EPs= %d Cls=%02x Sub=%02x Prot=%02x\n",
                interface->bInterfaceNumber, interface->bAlternateSetting, interface->bNumEndpoints,
                interface->bInterfaceClass, interface->bInterfaceSubClass, interface->bInterfaceProtocol);
            bInterfaceNumber = interface->bInterfaceNumber;
        }
        else if (h->bLength == USB_DT_ENDPOINT_SIZE && h->bDescriptorType == USB_DT_ENDPOINT)
        {
            struct usb_endpoint_descriptor *endpoint = (struct usb_endpoint_descriptor *)h;

            DBPrintf("E: Ad=%02x Atr=%02x MxPS= %d Ivl=%dms\n",
                endpoint->bEndpointAddress, endpoint->bmAttributes, endpoint->wMaxPacketSize, endpoint->bInterval);

            if (bInterfaceNumber == interface_no)
            {
                if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
                {
                    if (endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
                    {
                        endpoint_in = endpoint->bEndpointAddress;
                        DBPrintf("endpoint_in:  %x\n", endpoint_in);
                    }
                    else
                    {
                        endpoint_out = endpoint->bEndpointAddress;
                        endpoint_len = endpoint->wMaxPacketSize;
                        DBPrintf("endpoint_out: %x\n", endpoint_out);
                        DBPrintf("endpoint_len: %d\n", endpoint_len);
                    }
                }
                else if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)
                {

                }
            }
        }
        else
        {

        }

        len += h->bLength;

        if (endpoint_in != -1 && endpoint_out != -1)
            return 0;
    }

    return -1;
}

int usbfs_open(const USBFS_T *usbfs_dev, int interface_no)
{
    int ret;
    int usbfd;
    struct usbdevfs_getdriver usbdrv;

    usbfd = open(usbfs_dev->dev_name, O_RDWR | O_NOCTTY);
    printf("%s open(%s) = %d\n", get_time(), usbfs_dev->dev_name, usbfd);
    if (usbfd == -1)
    {
        printf("%s usbfd = %d, errno: %d (%s)\n", get_time(), usbfd, errno, strerror(errno));
        return -1;
    }

    if (usbfs_decode(usbfd ,interface_no) == -1)
    {
        printf("%s fail to find usb interface %d\n", get_time(), interface_no);
        close(usbfd);
        return -1;
    }

    usbdrv.interface = interface_no;
    ret = ioctl(usbfd, USBDEVFS_GETDRIVER, &usbdrv);
    if (ret && errno != ENODATA)
    {
        printf("%s USBDEVFS_GETDRIVER ret=%d errno: %d (%s)\n", get_time(), ret, errno, strerror(errno));
        close(usbfd);
        return -1;
    }
    else if (ret == 0)
    {
        struct usbdevfs_ioctl operate;

        operate.data = NULL;
        operate.ifno = interface_no;
        operate.ioctl_code = USBDEVFS_DISCONNECT;

        ret = ioctl(usbfd, USBDEVFS_IOCTL, &operate);
        if (ret)
        {
            printf("%s USBDEVFS_DISCONNECT ret=%d errno: %d (%s)\n", get_time(), ret, errno, strerror(errno));
            close(usbfd);
            return -1;
        }
    }

    ret = ioctl(usbfd, USBDEVFS_CLAIMINTERFACE, &interface_no);
    if (ret)
    {
        printf("%s USBDEVFS_CLAIMINTERFACE ret=%d, errno: %d (%s)\n", get_time(), ret, errno, strerror(errno));
        close(usbfd);
        return -1;
    }
    return usbfd;
}

int usbfs_close(int usbfd, int interface_no)
{
    close(usbfd);
    return 0;
}

static int get_sysinfo_by_uevent(const char *uevent, UEVENT_T *pSysInfo)
{
    FILE *fp;
    char line[MAX_PATH];

    memset(pSysInfo, 0x00, sizeof(UEVENT_T));

    fp = fopen(uevent, "r");
    if (fp == NULL)
    {
        printf("%s fail to fopen %s, errno: %d (%s)\n", get_time(), uevent, errno, strerror(errno));
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')
        {
            line[strlen(line) - 1] = '\0';
        }

        if (strStartsWith(line, "MAJOR="))
        {
            pSysInfo->MAJOR = atoi(&line[strlen("MAJOR=")]);
        }
        else if (strStartsWith(line, "MINOR="))
        {
            pSysInfo->MINOR = atoi(&line[strlen("MINOR=")]);
        }
        else if (strStartsWith(line, "DEVNAME="))
        {
            if(pSysInfo)
                strncpy(pSysInfo->DEVNAME, &line[strlen("DEVNAME=")], sizeof(pSysInfo->DEVNAME));
        }
        else if (strStartsWith(line, "DEVTYPE="))
        {
            strncpy(pSysInfo->DEVTYPE, &line[strlen("DEVTYPE=")], sizeof(pSysInfo->DEVTYPE));
        }
        else if (strStartsWith(line, "DRIVER="))
        {
            strncpy(pSysInfo->DRIVER, &line[strlen("DRIVER=")], sizeof(pSysInfo->DRIVER));
        }
        else if (strStartsWith(line, "PRODUCT="))
        {
            strncpy(pSysInfo->PRODUCT, &line[strlen("PRODUCT=")], sizeof(pSysInfo->PRODUCT));
        }
    }

    fclose(fp);
    return 1;
}

// the return value is the number of modules
int usbfs_find_meig_modules(USBFS_T *usbfs_dev, size_t max)
{
    const char *base = "/sys/bus/usb/devices";
    DIR *busdir = NULL;
    struct dirent *de = NULL;
    size_t count = 0;

    busdir = opendir(base);
    if (busdir == NULL)
        return -1;

    while ((de = readdir(busdir)))
    {
        static char uevent[MAX_PATH+9];
        UEVENT_T sysinfo;

        if (!isdigit(de->d_name[0])) continue;

        snprintf(uevent, sizeof(uevent), "%s/%.16s/uevent", base, de->d_name);
        if (!get_sysinfo_by_uevent(uevent, &sysinfo))
            continue;

        if (!sysinfo.MAJOR || !sysinfo.PRODUCT[0]|| !sysinfo.DEVTYPE[0])
            continue;

        if (sysinfo.MAJOR != 189 || strcmp(sysinfo.DEVTYPE, "usb_device"))
            continue;

#if 0
        dprintf("--------------------------------\n");
        dprintf("uevent: %s\n", uevent);
        dprintf(">>>sysinfo.DEVNAME=%s\n", sysinfo.DEVNAME);
        dprintf(">>>sysinfo.DEVTYPE=%s\n", sysinfo.DEVTYPE);
        dprintf(">>>sysinfo.DRIVER=%s\n", sysinfo.DRIVER);
        dprintf(">>>sysinfo.PRODUCT=%s\n", sysinfo.PRODUCT);
        dprintf(">>>sysinfo.MAJOR=%d\n", sysinfo.MAJOR);
        dprintf(">>>sysinfo.MINOR=%d\n", sysinfo.MINOR);
        dprintf("--------------------------------\n");
#endif
        if (find_usb_modem(sysinfo.PRODUCT))
        {
            snprintf(usbfs_dev[count].sys_path, sizeof(usbfs_dev[count].sys_path),
                      "%s/%.16s", base, de->d_name);
            snprintf(usbfs_dev[count].dev_name, sizeof(usbfs_dev[count].dev_name),
                      "/dev/%.16s", sysinfo.DEVNAME);
            strncpy(usbfs_dev[count].PRODUCT, sysinfo.PRODUCT, 32);
            printf("%s %s %s\n", get_time(), usbfs_dev[count].sys_path, usbfs_dev[count].PRODUCT);
            count++;
            if (count == max)
               break;
        }
    }

    closedir(busdir);

    return count;
}

