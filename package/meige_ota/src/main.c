#include "std_c.h"
#include <getopt.h>
#include "usbfs.h"
#include <sys/ipc.h>
#include <sys/shm.h>


#define OTA_SOFT_VER    "V1.05"

static USBFS_T usbfs_dev[8] = {0};
static const struct usb_modem_info modemList[] =
{
    {"SLM320",          "2dee/4d41/",   0, "AT^FORCEDNLD\r"},   //AT
    {"SLM320",          "2dee/4d42/",   2, "AT^FORCEDNLD\r"},   //AT
    {"8910_DIAG",       "525/a4a7/",    1, NULL},
    {"8850_DIAG",       "1782/4d16/",   1, NULL},
    {"SLM770U",         "1782/4d23/",   2, "AT^FORCEDNLD\r"},   //AT
    {"SLM770U",         "1782/4d00/",   0, NULL},               //Down
    {NULL, NULL, 0, NULL},
};

const struct usb_modem_info * find_usb_modem(const char *PRODUCT)
{
    int i = 0;
    while (modemList[i].modem_name)
    {
        if (strStartsWith(PRODUCT, modemList[i].PRODUCT))
        {
            return &modemList[i];
        }
        i++;
    }
    return NULL;
}

const char *get_time(void)
{
    static char str[64];
    static unsigned start = 0;
    unsigned now;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    now = (unsigned)ts.tv_sec*1000 + (unsigned)(ts.tv_nsec / 1000000);
    if (start == 0)
        start = now;

    now -= start;
    snprintf(str, sizeof(str), "[%03d:%03d]", now/1000, now%1000);

    return str;
}

static void switch_to_download(int usbfd, const char *download_atc, const char *dev_modem_name)
{
    sendSync(usbfd, "ATI\r", 4, 0);
    sendSync(usbfd, "AT+SGSW\r", 8, 0);
    sendSync(usbfd, "AT+EGMR=2,7\r", strlen("AT+EGMR=2,7\r"), 0);

    while (!access(dev_modem_name, F_OK))
    {
        int ret;
        char at_rsp[256];

        dprintf("AT > %s\n", download_atc);
        ret = sendSync(usbfd, download_atc, strlen(download_atc), 0);
        if (ret < 1) break;

        do  {
            ret = recvSync(usbfd, at_rsp, sizeof(at_rsp), 1000);
            if (ret > 0)
            {
                int i, len = 0;

                for (i = 0; i < ret; i++)
                {
                    uint8_t ch = at_rsp[i];

                    if (ch != '\r' && ch != '\n')
                        at_rsp[len++] = ch;
                    else if (len > 1)
                    {
                        at_rsp[len++] = 0;
                        dprintf("AT < %s\n", at_rsp);
                        len = 0;
                    }
                }
            }
        } while (ret > 0);

        if (errno != ETIMEDOUT)
        {
            sleep(1);
            break;
        }
    }
}

static void dispResult(int ret)
{
    dprintf("================================\r\n");
    if (ret == 0)
    {
        dprintf(" mmmmm    mm    mmmm   mmmm \n");
        dprintf(" #    #   ##   #      #     \n");
        dprintf(" #mmm#   #  #   #mmm   #mmm \n");
        dprintf(" #       #mm#       #      #\n");
        dprintf(" #      #    #  mmm#   mmm# \r\n");
    }
    else
    {
        dprintf(" mmmmmm   mm   mmmmm  m     \n");
        dprintf(" #        ##     #    #     \n");
        dprintf(" #mmmmm  #  #    #    #     \n");
        dprintf(" #       #mm#    #    #     \n");
        dprintf(" #      #    # mm#mm  #mmmmm\r\n");
    }
    dprintf("================================\r\n");
}

static void waitReboot(const USBFS_T *udev)
{
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        if (access(udev->dev_name, F_OK))
            break;
        usleep(10*1000);
    }
}

int main(int argc, char *argv[])
{
    char *unpacket_file = NULL;
    int i, ret = -1, opt, count, usbfd;
    const USBFS_T *udev;
    const usb_modem_info *mdev;
    FILE *fw_fp = NULL;
    int iErrRebootCnt = 0;

    //init data
    memset(&g_downInfo, 0, sizeof(g_downInfo));

    /*------------------------ get opt ------------------------------*/
    optind = 1;
    while (-1 != (opt = getopt(argc, argv, "f:evbh")))
    {
        switch (opt)
        {
        case 'f':
            unpacket_file = strdup(optarg);
            break;
        case 'e':
            g_downInfo.erase_all = 1;
            break;
        case 'b':
            g_downInfo.backupNV = 1;
            break;
        case 'v':
            openLog(1);
            break;
        default:
            printf("%s -f fw_image\n", argv[0]);
            printf("fw image is '.pac',         can be found in the FW package\n");
            return 0;
            break;
        }
    }

    dprintf("Version: MEIG_QDloder_Linux_%s\n", OTA_SOFT_VER);

    iErrRebootCnt = 0;
_RE_PLAY:
    /*------------------------ read ota file -----------------------*/
    if (!unpacket_file)
    {
        dprintf("[ ERR ] fw image is NULL\n");
        dispResult(-1);
        return -1;
    }

    dprintf("ota file: %s\n", unpacket_file);
    fw_fp = fopen(unpacket_file, "rb");
    if (!fw_fp)
    {
        dprintf("[ ERR ] open ota file fail, errno: %d (%s)\n", errno, strerror(errno));
        dispResult(-1);
        return -1;
    }

_scan_modem:
    /*------------------------ scan usb com -----------------------*/
    dprintf("scan meig modem\n");
    for (i = 0; i < 10; i++)
    {
        count = usbfs_find_meig_modules(usbfs_dev, 8);
        if (count > 0) break;

        usleep(1000*1000);
    }

    if (count == 0)
    {
        fclose(fw_fp);
        dprintf("[ ERR ]not meig modem find!\n");
        dispResult(-1);
        return -1;
    }
    udev = &usbfs_dev[0];

    mdev = find_usb_modem(udev->PRODUCT);
    if (!mdev)
        goto _scan_modem;

    usbfd = usbfs_open(udev, mdev->usb_interface);
    if (usbfd == -1)
    {
        fclose(fw_fp);
        return -1;
    }

    /*------------------------ download  ------------------------*/
    if (mdev->download_atc)
    {
        switch_to_download(usbfd, mdev->download_atc, udev->dev_name);
        usbfs_close(usbfd, mdev->usb_interface);
        goto _scan_modem;
    }

    ret = dloader_main(usbfd, fw_fp, mdev->modem_name, &iErrRebootCnt);
    fclose(fw_fp);
    usbfs_close(usbfd, mdev->usb_interface);
    if (!ret) waitReboot(udev);

    dprintf("Upgrade module %s\n", (ret == 0) ? "successfully" : "fail");
    dispResult(ret);
    closeLog();

    if (iErrRebootCnt > 0 && iErrRebootCnt < 2)
    {
        waitReboot(udev);
        goto _RE_PLAY;
    }
    return ret;
}

