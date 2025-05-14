#include "std_c.h"
#include "usbfs.h"
#include <getopt.h>
#include "pac.h"


typedef struct
{
    uint32_t code;
    const char *name;
} MG_MODE_NAME_T;

typedef struct
{
    uint32_t cmd;
    const char *name;
} BSL_CMD_NAME_T;

static const MG_MODE_NAME_T mg_mode_name[] =
{
    {MODE_8910,     "UIX8910_MODEM"},
    {MODE_8850,     "8850_MODEM"},
    {MODE_8310,     "UIS8310"},
};

#define bsl_cmd_item(type) {type, #type}
static const BSL_CMD_NAME_T bsl_cmd_name[] =
{
    bsl_cmd_item(BSL_CMD_CONNECT),
    bsl_cmd_item(BSL_CMD_START_DATA),
    bsl_cmd_item(BSL_CMD_MIDST_DATA),
    bsl_cmd_item(BSL_CMD_END_DATA),
    bsl_cmd_item(BSL_CMD_EXEC_DATA),
    bsl_cmd_item(BSL_CMD_NORMAL_RESET),
    bsl_cmd_item(BSL_CMD_READ_FLASH),
    bsl_cmd_item(BSL_CMD_CHANGE_BAUD),
    bsl_cmd_item(BSL_CMD_ERASE_FLASH),
    bsl_cmd_item(BSL_CMD_REPARTITION),
    bsl_cmd_item(BSL_CMD_START_READ),
    bsl_cmd_item(BSL_CMD_READ_MIDST),
    bsl_cmd_item(BSL_CMD_END_READ),
    bsl_cmd_item(BSL_CMD_DISABLE_TRANSCODE),
    bsl_cmd_item(BSL_CMD_WRITE_APR_INFO),
    bsl_cmd_item(BSL_CMD_ENABLE_DEBUG_MODE),

    bsl_cmd_item(BSL_REP_ACK),
    bsl_cmd_item(BSL_REP_VER),
    bsl_cmd_item(BSL_REP_INVALID_CMD),
    bsl_cmd_item(BSL_REP_UNKNOW_CMD),
    bsl_cmd_item(BSL_REP_OPERATION_FAILED),
    bsl_cmd_item(BSL_REP_NOT_SUPPORT_BAUDRATE),
    bsl_cmd_item(BSL_REP_DOWN_NOT_START),
    bsl_cmd_item(BSL_REP_DOWN_MUTI_START),
    bsl_cmd_item(BSL_REP_DOWN_EARLY_END),
    bsl_cmd_item(BSL_REP_DOWN_DEST_ERROR),
    bsl_cmd_item(BSL_REP_DOWN_SIZE_ERROR),
    bsl_cmd_item(BSL_REP_VERIFY_ERROR),
    bsl_cmd_item(BSL_REP_NOT_VERIFY),
    bsl_cmd_item(BSL_REP_READ_FLASH),
    bsl_cmd_item(BSL_REP_INCOMPATIBLE_PARTITION)
};

static uint32_t frame_sz        = 0;
static int use_crc16BootCode    = 0;


#if 0
#define bsl_exec(_c) do{if (_c) {dprintf("failed: %s (%s: %s: %d)\n", #_c, __FILE__, __func__, __LINE__); return -1;}} while(0)
#else
#define bsl_exec(_c) do{if (_c) {dprintf("failed: %s (%s: %d)\n", #_c, __func__, __LINE__); return -1;}} while(0)
#endif

#define OTA_LOG_FILE    "ota.log"

#define OTA_OK  0
#define OTA_NOK 1

typedef struct
{
    char fileID[32];
    int bOK;
}ts_scheme;

typedef struct
{
    int bOK;
    int bNVBackup;
    ts_scheme schemeList[15];
}ts_ota_status;

static int bNVBackup = OTA_NOK;
static ts_ota_status g_otaStatus = {0};

int readFile_OTA(void)
{
    int fd = -1;

    DBPrintf("read ota log file\n");
    memset(&g_otaStatus, 0, sizeof(g_otaStatus));
    g_otaStatus.bOK = OTA_NOK;
    g_otaStatus.bNVBackup = OTA_NOK;

    fd = open(OTA_LOG_FILE, O_RDONLY);
    if (fd < 0)
    {
        goto DO_ERR;
    }
    if (lseek(fd, 0, SEEK_SET) < 0)
        goto DO_ERR;

    if (read(fd, &g_otaStatus, sizeof(g_otaStatus)) < 0)
        goto DO_ERR;

    close(fd);
    
    bNVBackup = g_otaStatus.bNVBackup;
    return 0;

DO_ERR:
    if (fd)
        close(fd);

    return -1;
}

int writeFile_OTA(void)
{
    int fd = -1;

    DBPrintf("write ota log file\n");
    fd = open(OTA_LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 664);
    if (fd < 0)
        goto DO_ERR;

    if (lseek(fd, 0, SEEK_SET) < 0)
        goto DO_ERR;

    if (write(fd, (char *)&g_otaStatus, sizeof(g_otaStatus)) < 0)
        goto DO_ERR;

    close(fd);
    return 0;

DO_ERR:
    if (fd)
        close(fd);

    return -1;
}

PDL_CMD_T * pdl_cmd_reset(PDL_CMD_T *pdl_cmd, uint16_t cmd)
{
    pdl_cmd->ucTag = 0xae;
    pdl_cmd->nDataSize= htole32(sizeof(struct pdl_pkt_tag));
    pdl_cmd->ucFlowID = 0xFF;
    pdl_cmd->wReserved = htole16(0);

    pdl_cmd->pkt[0].dwCmdType = htole32(cmd);
    pdl_cmd->pkt[0].dwDataAddr = htole32(0);
    pdl_cmd->pkt[0].dwDataSize = htole32(0);

    return pdl_cmd;
}

static int pdl_send_cmd(pac_ctx_t *ctx, PDL_CMD_T *pdl_cmd)
{
    int ret;

    ret = sendSync(ctx->usbfd, &pdl_cmd->ucTag, sizeof(PDL_CMD_T), 0);
    if (ret == sizeof(PDL_CMD_T))
    {
        ret = sendSync(ctx->usbfd, pdl_cmd->pkt, sizeof(struct pdl_pkt_tag), 0);
        if (ret == sizeof(struct pdl_pkt_tag))
        {
            int size = le32toh(pdl_cmd->nDataSize) - sizeof(struct pdl_pkt_tag);
            if (size > 0)
                ret = sendSync(ctx->usbfd, pdl_cmd->pkt[0].data, size, 0);
        }
    }

    return ret;
}

static PDL_CMD_T * pdl_read_cmd(pac_ctx_t *ctx, unsigned timeout)
{
    int ret;
    PDL_CMD_T *pdl_rsp = ctx->pdl_rsp;

    memset(pdl_rsp, 0, sizeof(PDL_CMD_T));
    ret = recvSync(ctx->usbfd, pdl_rsp, sizeof(ctx->bsl_rsp_data), timeout ? timeout : 15000);

    return (ret > 0) ? pdl_rsp : NULL;
}

int pdl_send_cmd_wait_ack(pac_ctx_t *ctx, PDL_CMD_T *pdl_req, uint16_t pdl_cmd, unsigned timeout)
{
    int ret;
    PDL_CMD_T *pdl_rsp;

    if (!pdl_req)
    {
        pdl_req = pdl_cmd_reset(ctx->pdl_req, pdl_cmd);
    }

    ret = pdl_send_cmd(ctx, pdl_req);
    if (ret <= 0) return -1;

    pdl_rsp = pdl_read_cmd(ctx, timeout);
    if (pdl_rsp == NULL) return -1;

    if (le16toh(pdl_rsp->pkt[0].dwCmdType) != PDL_RSP_ACK)
        return -1;

    return 0;
}

static int pdl_flash_id(pac_ctx_t *ctx, Scheme_t *File)
{
    int ret;
    size_t curSize = 0, fileSize;
    PDL_CMD_T *pdl_req = ctx->pdl_req;
    int index = 0;
    uint8_t end_flag[] = {0x1c, 0x3c, 0x6e, 0x06};

    dprintf("%-8s %s\n",  "Flash", File->FileID);

    bsl_exec(pac_open_bin(ctx, File->image));
    fileSize = File->image->dwLoFileSize;

    pdl_cmd_reset(pdl_req, PDL_CMD_START_DATA);
    pdl_req->pkt[0].dwDataAddr = htole32(strtoul(File->Base, NULL, 16));
    pdl_req->pkt[0].dwDataSize = htole32(fileSize);
    memcpy(pdl_req->pkt[0].data, "PDL1", 5);
    pdl_req->nDataSize= htole32(sizeof(struct pdl_pkt_tag) + 5);

    bsl_exec(pdl_send_cmd_wait_ack(ctx, pdl_req, 0, 1000));

    curSize = 0;
    while (curSize < fileSize)
    {
        ret = pac_read_bin(ctx, pdl_req->pkt[0].data, frame_sz);
        bsl_exec(ret <= 0);
        pdl_cmd_reset(pdl_req, PDL_CMD_MID_DATA);
        pdl_req->nDataSize= htole32(sizeof(struct pdl_pkt_tag) + ret);
        pdl_req->pkt[0].dwDataAddr = htole32(index++);
        pdl_req->pkt[0].dwDataSize = htole32(ret);

        curSize += ret;
        bsl_exec(pdl_send_cmd_wait_ack(ctx, pdl_req, 0, 0));
    }

    pdl_cmd_reset(pdl_req, PDL_CMD_END_DATA);
    memcpy(pdl_req->pkt[0].data, end_flag, 4);
    pdl_req->nDataSize= htole32(sizeof(struct pdl_pkt_tag) + 4);
    bsl_exec(pdl_send_cmd_wait_ack(ctx, pdl_req, 0, 0));

    return 0;
}

static BSL_CMD_T * bsl_cmd_reset(BSL_CMD_T * bsl_cmd, uint16_t cmd)
{
    bsl_cmd->magic = 0x7e;
    bsl_cmd->cmd = htobe16(cmd);
    bsl_cmd->len = htobe16(0);

    return bsl_cmd;
}

void bsl_cmd_dump(BSL_CMD_T * bsl_cmd)
{
    const char *name = NULL;
    uint16_t cmd = be16toh(bsl_cmd->cmd);
    size_t i;

    for (i = 0; i < (sizeof(bsl_cmd_name)/sizeof(bsl_cmd_name[0])); i++)
    {
        if (cmd == bsl_cmd_name[i].cmd)
        {
            name = bsl_cmd_name[i].name;
            break;
        }
    }

    if (!name)
    {
        dprintf("unknow bsl_cmd: cmd=%04x, len=%d\n", cmd, be16toh(bsl_cmd->len));
        return;
    }

    dprintf("\t%c %s\n", (cmd&0x80) ? '<' : '>', name);
}

void bsl_cmd_add_be32(BSL_CMD_T * bsl_cmd, uint32_t value)
{
    size_t bsl_len = be16toh(bsl_cmd->len);

    //*((uint32_t *)(&bsl_cmd->data[bsl_len])) = htobe32(value);
    inttochar((char *)&bsl_cmd->data[bsl_len], htobe32(value));
    bsl_cmd->len = htobe16(bsl_len + 4);
}

void bsl_cmd_add_le32(BSL_CMD_T * bsl_cmd, uint32_t value)
{
    size_t bsl_len = be16toh(bsl_cmd->len);

    //*((uint32_t *)&bsl_cmd->data[bsl_len]) = htole32(value);
    inttochar((char *)&bsl_cmd->data[bsl_len], htobe32(value));
    bsl_cmd->len = htobe16(bsl_len + 4);
}

void bsl_cmd_add_array(BSL_CMD_T * bsl_cmd, uint8_t *pBuf, size_t size)
{
    size_t bsl_len = be16toh(bsl_cmd->len);

    memcpy(&bsl_cmd->data[bsl_len], pBuf, size);
    bsl_cmd->len = htobe16(bsl_len + size);
}

void bsl_cmd_add_partition(BSL_CMD_T * bsl_cmd, const char*pStr)
{
    char str[0x48];
    size_t size =  strlen(pStr);
    size_t i;

    for (i = 0; i < size; i++)
    {
        str[i*2 + 0] = pStr[i];
        str[i*2 + 1] = 0;
    }
    for (i = size*2; i < sizeof(str); i++)
    {
        str[i] = 0;
    }

    bsl_cmd_add_array(bsl_cmd, (uint8_t *)str, sizeof(str));
}

void bsl_cmd_add_crc16(BSL_CMD_T * bsl_cmd, int BootCode)
{
    uint8_t *src = &bsl_cmd->magic + 1;
    size_t len = be16toh(bsl_cmd->len);
    uint16_t crc = BootCode ? crc16BootCode(src, len + 4) : crc16FDL((uint16_t *)src, len + 4);

    //*((uint16_t *)&bsl_cmd->data[len]) = htobe16(crc);
    shorttochar((char * )&bsl_cmd->data[len], htobe16(crc));
}

static int bsl_send_cmd(pac_ctx_t *ctx, BSL_CMD_T *bsl_cmd)
{
    uint8_t *buf = ctx->data_7e;
    size_t i, size = 0;
    size_t total = be16toh(bsl_cmd->len) + 2; //add crc16
    uint16_t req_cmd =  be16toh(bsl_cmd->cmd);

    if (req_cmd == BSL_CMD_READ_MIDST
         || req_cmd == BSL_CMD_MIDST_DATA
         || req_cmd == BSL_CMD_READ_FLASH)
        ;
    else
        bsl_cmd_dump(bsl_cmd);

    /*转义包含: 类型、包长度、包数据、CRC 校验位*/
    {
        uint8_t m_tmp[8] = {0};
        memcpy(m_tmp + size, bsl_cmd, 5);

        //
        i = 0;
        buf[size++] = m_tmp[i++];

        //
        for (; i < 5; i++)
        {
            uint8_t ch = m_tmp[i];
            if (ch == 0x7E || ch == 0x7D)
            {
                buf[size++] = 0x7D;
                buf[size++] = (0x20 ^ ch);
            }
            else
            {
                buf[size++] = ch;
            }
        }
    }

    for (i = 0; i < total; i++)
    {
        uint8_t ch = bsl_cmd->data[i];

        if (ch == 0x7E || ch == 0x7D)
        {
            buf[size++] = 0x7D;
            buf[size++] = (0x20 ^ ch);
        }
        else
        {
            buf[size++] = ch;
        }
    }

    buf[size++] = 0x7e;
    return sendSync(ctx->usbfd, buf, size, 1);
}

static BSL_CMD_T * bsl_read_cmd(pac_ctx_t *ctx, unsigned timeout)
{
    int ret,i;
    BSL_CMD_T *bsl_rsp = ctx->bsl_rsp;

    memset(bsl_rsp, 0, sizeof(BSL_CMD_T));
    ret = recvSync(ctx->usbfd, bsl_rsp, sizeof(ctx->bsl_rsp_data), timeout ? timeout : 15000);

    if (ret > 0)
    {
        uint16_t req_cmd =  be16toh(ctx->bsl_req[0].cmd);
        uint16_t rsp_cmd =  be16toh(ctx->bsl_rsp[0].cmd);

        if ((req_cmd == BSL_CMD_READ_MIDST && rsp_cmd == BSL_REP_READ_FLASH)
            || (req_cmd == BSL_CMD_MIDST_DATA && rsp_cmd == BSL_REP_ACK)
            || (req_cmd == BSL_CMD_READ_FLASH && rsp_cmd == BSL_REP_READ_FLASH)
            || (rsp_cmd == BSL_REP_ACK))
            ;
        else
            bsl_cmd_dump(bsl_rsp);

        if (be16toh(bsl_rsp->cmd) == BSL_REP_READ_FLASH)
        {
            uint32_t need_len = (be16toh(bsl_rsp->len) + 1 + 2 + 2 + 2 + 1);
            uint32_t read_len = ret;


            uint8_t *data = (uint8_t *)bsl_rsp;
            uint32_t cnt_7d = 0;

            for (i = 0; i < ret; i++)
            {
                if (data[i] == 0x7d) cnt_7d++;
            }

            while ((read_len < (need_len + cnt_7d)) && (ret == 128 || ret == 129))
            {
                ret = recvSync(ctx->usbfd, data + read_len, sizeof(ctx->bsl_rsp_data), 1000);
                if (ret > 0)
                {
                    for (i = 0; i < ret; i++)
                    {
                        if (data[read_len+ i] == 0x7d) cnt_7d++;
                    }
                    read_len += ret;
                }
                else
                {
                    return NULL;
                }
            }

            if (data[0] != 0x7e)
            {
                dprintf("%s not start with 0x7e\n", __func__);
                return NULL;
            }
            if (data[read_len - 1] != 0x7e)
            {
                dprintf("%s not end with 0x7e\n", __func__);
                return NULL;
            }

            cnt_7d = 1;
            uint32_t i;
            for (i = cnt_7d; i < (read_len - 1); i++)
            {
                if (data[i] == 0x7d)
                {
                    i++;
                    data[cnt_7d++] = data[i] ^ 0x20;
                }
                else
                {
                    data[cnt_7d++] = data[i];
                }
            }
            data[cnt_7d++] = data[read_len - 1];

            if (read_len != cnt_7d)
            {
                //printf("cnt_7d = %d\n", (read_len - cnt_7d));
                read_len = cnt_7d;
            }

            if (need_len != read_len)
            {
                dprintf("read: %d / need: %d\n", read_len, need_len);
                return NULL;
            }


            uint16_t crc16_1 = crc16FDL((uint16_t *)(&bsl_rsp->magic + 1), be16toh(bsl_rsp->len) + 4);
            //uint16_t crc16_2 =  be16toh(*((uint16_t *)(&(bsl_rsp->data[be16toh(bsl_rsp->len)]))));
            uint16_t crc16_2 =  be16toh(chartoshort((char *) &(bsl_rsp->data[be16toh(bsl_rsp->len)])));
            if (crc16_1 != crc16_2)
            {
                dprintf("%s crc16 un-match!\n", __func__);
                return NULL;
            }

            if (need_len != read_len)
                dprintf("read: %d / need: %d\n", read_len, need_len);
        }
    }

    return (ret > 0) ? bsl_rsp : NULL;
}

int bsl_send_cmd_wait_ack(pac_ctx_t *ctx, BSL_CMD_T *bsl_req, uint16_t bsl_cmd, unsigned timeout)
{
    int ret;
    BSL_CMD_T *bsl_rsp;

    if (!bsl_req)
    {
        bsl_req = bsl_cmd_reset(ctx->bsl_req, bsl_cmd);
        bsl_cmd_add_crc16(bsl_req, use_crc16BootCode);
    }

    ret = bsl_send_cmd(ctx, bsl_req);
    if (ret <= 0) return -1;

    bsl_rsp = bsl_read_cmd(ctx, timeout);
    if (bsl_rsp == NULL) return -1;

    if (be16toh(bsl_rsp->cmd) != BSL_REP_ACK && be16toh(bsl_rsp->cmd) != BSL_REP_INCOMPATIBLE_PARTITION)
        return -1;

    return 0;
}

static int bsl_check_baud(pac_ctx_t *ctx)
{
    int ret;
    uint8_t hello = BSL_CMD_CHECK_BAUD;
    int i;
    BSL_CMD_T *bsl_rsp;

    dprintf("\t%c %s\n", '>', "BSL_CMD_CHECK_BAUD");
    for (i = 0; i < 5; i++)
    {
        ret = sendSync(ctx->usbfd, &hello, 1, 0);
        if (ret != 1) return -1;

        bsl_rsp = bsl_read_cmd(ctx, 1000);
        if (bsl_rsp && be16toh(bsl_rsp->cmd) == BSL_REP_VER)
            return 0;
    }

    return -1;
}

static int bsl_change_baud(pac_ctx_t *ctx)
{
    BSL_CMD_T *bsl_req = ctx->bsl_req;

    bsl_cmd_reset(bsl_req, BSL_CMD_CHANGE_BAUD);
    bsl_cmd_add_be32(bsl_req, 115200);
    bsl_cmd_add_crc16(bsl_req, 0);
    bsl_exec(bsl_send_cmd_wait_ack(ctx, bsl_req, 0, 0));

    return 0;
}

static int bsl_file_is_nv(Scheme_t *File)
{
    return (!strcasecmp(File->FileID, "NV"));
}

static int bsl_backup_id(pac_ctx_t *ctx, Scheme_t *File);
static int bsl_flash_id(pac_ctx_t *ctx, Scheme_t *File)
{
    int ret = 0;
    int bDispFlag = 0;
    size_t curSize = 0, fileSize;
    BSL_CMD_T *bsl_req = ctx->bsl_req;
    const char *base = NULL;
    uint32_t crc32 = 0;
    uint16_t crc16 = 0;
    int nv_crc = bsl_file_is_nv(File);

    dprintf("%-8s %s\n",  "Flash", File->FileID);

    // backup nv
    if ((g_downInfo.backupNV || g_downInfo.erase_all == 0) && nv_crc)
    {
        bsl_backup_id(ctx, File);
    }
#if 0 //for reduce test time
    if (!strcasecmp(File->FileID, "AP") || !strcasecmp(File->FileID, "PS"))
        return 0;
#endif

    bsl_cmd_reset(bsl_req, BSL_CMD_START_DATA);

    base = File->Block[0] ? File->Block : File->Base;
    fileSize = File->image->dwLoFileSize;
    if (nv_crc && File->nv_buf)
    {
        fileSize = File->nv_size;
        DBPrintf(" -- file size: %ld\n", fileSize);
    }
    else if (File->backup_fp)
    {
        DBPrintf("get backup file size\n");
        bsl_exec(fseek(File->backup_fp, 0, SEEK_END));
        fileSize = ftell(File->backup_fp);
        bsl_exec(fseek(File->backup_fp, 0, SEEK_SET));
        DBPrintf("-- file size: %ld\n", fileSize);
    }
    else
    {
        DBPrintf("get file size\n");
        bsl_exec(pac_open_bin(ctx, File->image));
    }

    if (!strncasecmp(base, "0x", 2))
    {
        bsl_cmd_add_be32(bsl_req, strtoul(base, NULL, 16));
        bsl_cmd_add_be32(bsl_req, fileSize);
    }
    else
    {
        bsl_cmd_add_partition(bsl_req, base);
        bsl_cmd_add_le32(bsl_req, fileSize);
    }

    if (nv_crc)
    {
        if (File->nv_buf)
        {
            nv_checksum_buf((uint8_t*)File->nv_buf, File->nv_size, &crc32, &crc16);
        }
        else
        {
            DBPrintf("read backup file seek\n");
            nv_checksum_file(ctx->pac_fp, fileSize, &crc32, &crc16);
            fseek(ctx->pac_fp, File->image->dwLoDataOffset, SEEK_SET);
        }

        if (!strncasecmp(base, "0x", 2))
        {
            bsl_cmd_add_be32(bsl_req, crc32);
        }
        else
        {
            bsl_cmd_add_le32(bsl_req, crc32);
        }
    }

    bsl_cmd_add_crc16(bsl_req, use_crc16BootCode);
    bsl_exec(bsl_send_cmd_wait_ack(ctx, bsl_req, 0, 0));

    bDispFlag = 0;
    curSize = 0;
    while (curSize < fileSize)
    {
        if (!bDispFlag)
        {
            DBPrintf("\t  downloading ...\n");
            bDispFlag = 1;
        }
        bsl_cmd_reset(bsl_req, BSL_CMD_MIDST_DATA);
        if (nv_crc && File->nv_buf)
        {
            ret = fileSize - curSize;
            if (ret > (int)frame_sz)
                ret = frame_sz;
            memcpy(bsl_req->data, (char *)File->nv_buf + curSize, frame_sz);
        }
        else if (File->backup_fp)
        {
            ret = fread(bsl_req->data, 1, frame_sz, File->backup_fp);
        }
        else
        {
            ret = pac_read_bin(ctx, bsl_req->data, frame_sz);
        }

        if (ret <= 0)
            break;

        if (nv_crc && (curSize == 0))
        {
            shorttochar((char * )bsl_req->data, htobe16(crc16));
        }

        if ((ret&1) == 1)
        {
            uint8_t *p = bsl_req->data;
            DBPrintf("\t  ret:%d\n", ret);
            //bsl_req->data[ret++] = 0;
            p[ret++] = 0;
        }

        bsl_req->len = htobe16(ret);
        bsl_cmd_add_crc16(bsl_req, use_crc16BootCode);

        curSize += ret;
        bsl_exec(bsl_send_cmd_wait_ack(ctx, bsl_req, 0, 0));
    }

    if (File->backup_fp)
    {
        fclose(File->backup_fp);
        File->backup_fp = NULL;
    }

    if (File->nv_buf)
    {
        free(File->nv_buf);
        File->nv_buf = NULL;
    }

    bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_END_DATA, 0));

    return 0;
}

int checkDir(const char *appPath, const char *path)
{
    int iRet = -1;
    int iLen = 5;
    char *p = NULL;

    if (appPath)
        iLen += strlen(appPath);
    if (path)
        iLen += strlen(path);

    p = (char *)malloc((sizeof(char)*iLen));
    if (p)
    {
        iRet = 0;
        memset(p, 0, iLen);
        sprintf(p, "%s%s", appPath ? appPath : "", path ? path : "");
        if (access(p, W_OK) && errno == ENOENT)
            iRet = -1;
    }

    if (p)
        free(p);
    return iRet;
}

static int bsl_backup_id(pac_ctx_t *ctx, Scheme_t *File)
{
    size_t curSize = 0, fileSize;
    BSL_CMD_T *bsl_req = ctx->bsl_req;
    BSL_CMD_T *bsl_rsp;
    FILE *fp = NULL;
    char *pAppPath = NULL, *pPath = NULL;
    char *backup_dir = (char *)"/data";
    char backup_file[1024] = {0};
    int nv_crc = bsl_file_is_nv(File);

    pAppPath = getcwd(NULL, 0);
    pPath = pAppPath;
    if (checkDir(pPath, backup_dir))
    {
        pPath = NULL;
        backup_dir = (char *)"/tmp";
        if (checkDir(pPath, backup_dir))
        {
            backup_dir = (char *)"/data";
            if (checkDir(pPath, backup_dir))
                backup_dir = (char *)"/cache";
        }
    }

    snprintf(backup_file, sizeof(backup_file), "%s%s/meig_back_%s", !pPath ? "" : pPath, backup_dir, File->FileID);
    if (pAppPath)
    {
        free(pAppPath);
        pAppPath = NULL;
    }
    //----------------------------------------------

    fileSize = strtoul(File->Size, NULL, 16);
    if (fileSize <= 0 && File->image)
    {
        fileSize = File->image->dwLoFileSize;
    }

    dprintf("Backup %s--> %s\tsize:%ld\n", File->FileID, backup_file, fileSize);

    curSize = 0;
    while (curSize < fileSize)
    {
        bsl_cmd_reset(bsl_req, BSL_CMD_READ_FLASH);
        bsl_cmd_add_be32(bsl_req, strtoul(File->Base, NULL, 16));
        bsl_cmd_add_be32(bsl_req, min(FRAMESZ_DATA, fileSize - curSize));
        bsl_cmd_add_be32(bsl_req, curSize);
        bsl_cmd_add_crc16(bsl_req, 0);

        bsl_exec(bsl_send_cmd(ctx, bsl_req) < 1);

        bsl_rsp = bsl_read_cmd(ctx, 0);
        if (bsl_rsp == NULL && !curSize && OTA_OK == g_otaStatus.bNVBackup)
            goto DO_READ;
        bsl_exec(bsl_rsp == NULL);

        if (be16toh(bsl_rsp->cmd) != BSL_REP_READ_FLASH)
        {
            if (!curSize && OTA_OK == g_otaStatus.bNVBackup)
                goto DO_READ;
            return -1;
        }

        if (curSize == 0)
        {
            fp = fopen(backup_file, "wb");
            if (fp == NULL)
            {
                DBPrintf("[ ERR ] fopen %s, errno: %d (%s)\n", backup_file, errno, strerror(errno));
                return -1;
            }

            if (nv_crc)
            {
                uint32_t tmp = nv_get_size(bsl_rsp->data, be16toh(bsl_rsp->len));
                if (tmp && tmp < fileSize) fileSize = tmp;
            }
        }

        if (fwrite(bsl_rsp->data, 1, be16toh(bsl_rsp->len), fp) != be16toh(bsl_rsp->len))
        {
            DBPrintf("[ ERR ] faill to save %zd bytes\n", File->nv_size);
            fclose(fp);
            return -1;
        }

        curSize += be16toh(bsl_rsp->len);
    }

DO_READ:
    if (fp)
        fclose(fp);

    if (nv_crc)
    {
        bsl_exec(pac_open_bin(ctx, File->image));
        File->nv_size = File->image->dwLoFileSize; //fileSize; //fix by neil, when old nv file's size < new nv file's size, we should use new nv file's size
        File->nv_buf = malloc(File->nv_size);
        if (!File->nv_buf)
        {
            DBPrintf("[ ERR ] faill to malloc %zd bytes\n", File->nv_size);
            return -1;
        }
        pac_read_bin(ctx, File->nv_buf, File->nv_size);
        bsl_exec(nv_merg_file(backup_file, (uint8_t*)File->nv_buf, File->nv_size, ctx->NVBackupList, ctx->NVBackupNum));
    }
    else
    {
        File->backup_fp = fopen(backup_file, "rb");
    }

    return 0;
}

static int bsl_erase_id(pac_ctx_t *ctx, Scheme_t *File, unsigned timeout)
{
    BSL_CMD_T *bsl_req = ctx->bsl_req;

    dprintf("%-8s %s\n", "Erase", File->FileID);
    bsl_cmd_reset(bsl_req, BSL_CMD_ERASE_FLASH);
    if (File->Block[0])
    {
        bsl_cmd_add_partition(bsl_req, File->Block);
        bsl_cmd_add_le32(bsl_req, 0);
    }
    else
    {
        bsl_cmd_add_be32(bsl_req, strtoul(File->Base, NULL, 16));
        bsl_cmd_add_be32(bsl_req, strtoul(File->Size, NULL, 16));
    }
    bsl_cmd_add_crc16(bsl_req, 0);

    bsl_exec(bsl_send_cmd_wait_ack(ctx, bsl_req, 0, timeout));

    return 0;
}

int bsl_re_parttition(pac_ctx_t *ctx)
{
    BSL_CMD_T *bsl_req = ctx->bsl_req;
    int i;

    bsl_cmd_reset(bsl_req, BSL_CMD_REPARTITION);
    for (i = 0; i < ctx->PartitionNum; i++)
    {
        uint32_t psz =  ctx->PartitionList[i].PartitionSize;
        DBPrintf("bsl_re_parttition psz %d\n", psz);
        if (psz != 0xFFFFFFFF)
           psz = psz / 1024 / 1024;
        bsl_cmd_add_partition(bsl_req,  ctx->PartitionList[i].id);
        bsl_cmd_add_le32(bsl_req, psz);
    }
    bsl_cmd_add_crc16(bsl_req, 0);

    bsl_exec(bsl_send_cmd_wait_ack(ctx, bsl_req, 0, 0));

     return 0;
}

int findDeal(const char *fileId)
{
    int iCnt = sizeof(g_otaStatus.schemeList)/sizeof(g_otaStatus.schemeList[0]);
    if (!fileId)
        return -1;

    int i;
    for (i = 0; i < iCnt; i++)
    {
        if (!memcmp(g_otaStatus.schemeList[i].fileID, fileId, strlen(fileId)))
        {
            return i;
        }
    }
    
    //dprintf(">>>>>>>>>> no find fileId!!!\n");
    return -1;
}

int saveDeal(const char *fileId, int bStatus)
{
    int iCnt = sizeof(g_otaStatus.schemeList)/sizeof(g_otaStatus.schemeList[0]);
    int i;
    if (!fileId)
        return -1;

    for (i = 0; i < iCnt; i++)
    {
        if (!memcmp(g_otaStatus.schemeList[i].fileID, fileId, strlen(fileId)))
        {
            g_otaStatus.schemeList[i].bOK = bStatus;
            return 0;
        }
    }

    for (i = 0; i < iCnt; i++)
    {
        if (!strlen(g_otaStatus.schemeList[i].fileID))
        {
            memset(g_otaStatus.schemeList[i].fileID, 0, sizeof(g_otaStatus.schemeList[i].fileID));
            memcpy(g_otaStatus.schemeList[i].fileID, fileId, sizeof(g_otaStatus.schemeList[i].fileID));
            g_otaStatus.schemeList[i].bOK = bStatus;
            return 0;
        }
    }

    return -1;
}

int checkOK(const char *fileId)
{
    int iRet = findDeal(fileId);
    if (iRet < 0)
        return iRet;

    return g_otaStatus.schemeList[iRet].bOK == OTA_OK ? 0 : -1;
}

int nvBackup(int numBegin, pac_ctx_t *ctx)
{
    bNVBackup = OTA_NOK;
    Scheme_t *bx = NULL;
    int j;
    for (j = numBegin; j < ctx->SchemeNum; j++)
    {
        bx = &ctx->SchemeList[j];
        if (!strcasecmp(bx->FileID, "NV") && (!strcasecmp(bx->Type, "NV_NAND") || !strcasecmp(bx->Type, "NV")))
        {
            bx->backup[0] = '1';
        }

        //bx->backup[0] = '0';
        if (bx->backup[0] == '1')
        {
            bsl_exec(bsl_backup_id(ctx, bx));
        }
    }
    bNVBackup = OTA_OK;
    return 0;
}

static uint32_t mg_get_mode_name(pac_ctx_t *ctx)
{
    struct pac_header_t *pachdr = &ctx->pac_hdr;
    size_t i;

    for (i = 0; i < (sizeof(mg_mode_name)/sizeof(mg_mode_name[0])); i++)
    {
        if (!strcasecmp(mg_mode_name[i].name, (char *)pachdr->szPrdName)
         && strlen(mg_mode_name[i].name) == strlen((char *)pachdr->szPrdName))
        {
            return mg_mode_name[i].code;
        }
    }

    return 0;
}

static int dloader_bsl(pac_ctx_t *ctx)
{
    int i;
    int fileFlag = 0;
    int fileCheckFlag = 0;
    static char m_erase_all = 0;
    Scheme_t *x = NULL;
    Scheme_t x_tmp;

    g_downInfo.modeType = (e_modeType)mg_get_mode_name(ctx);
    if (MODE_NULL == g_downInfo.modeType)
    {
        DBPrintf("[ ERR ] product name is null.");
        return -1;
    }

    dprintf("SchemeNum:%d\n", ctx->SchemeNum);
    for (i = 0; i < ctx->SchemeNum; i++)
    {
        x = &ctx->SchemeList[i];
        if (g_downInfo.erase_all && g_downInfo.erase_all == m_erase_all)
        {
            memset(&x_tmp, 0, sizeof(Scheme_t));
            snprintf(x_tmp.FileID, sizeof(x->FileID), "%s", "EraseFlashAll");
            snprintf(x_tmp.Base, sizeof(x->Base), "0x%08x", 0x00000000);
            snprintf(x_tmp.Size, sizeof(x->Base), "0x%08x", 0xffffffff);
            m_erase_all = 0;
            i--;
            x = &x_tmp;
        }

        fileFlag = strtoul(x->Flag, NULL, 16);
        fileCheckFlag = strtoul(x->CheckFlag, NULL, 16);

        DBPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        DBPrintf("-- FileID: %-25s %-16s %s\n", x->FileID, x->Block[0] ? x->Block : x->Base, x->Type);
        DBPrintf("-- Flag: %d\tC1heckFlag: %d\tNvFlag: %d\n", fileFlag, fileCheckFlag, bsl_file_is_nv(x));

        if (!strcasecmp(x->FileID, "HOST_FDL"))
        {
            frame_sz = FRAMESZ_PDL;
            pdl_send_cmd_wait_ack(ctx, NULL, PDL_CMD_CONNECT, 1000);
            bsl_exec(pdl_flash_id(ctx, x));
            pdl_send_cmd_wait_ack(ctx, NULL, PDL_CMD_EXEC_DATA, 1000);
            continue;
        }

        if (!strcasecmp(x->FileID, "FDL"))
        {
            use_crc16BootCode = 1;
            frame_sz = FRAMESZ_BOOTCODE;
            bsl_exec(bsl_check_baud(ctx));
            bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_CONNECT, 0));
            bsl_exec(bsl_flash_id(ctx, x));
            bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_EXEC_DATA, 0));
            use_crc16BootCode = 0;
            m_erase_all = g_downInfo.erase_all;
            continue;
        }

        if (!strcasecmp(x->FileID, "FDL2"))
        {
            frame_sz = FRAMESZ_FDL;
            bsl_exec(bsl_check_baud(ctx));
            bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_CONNECT, 0));
            bsl_exec(bsl_change_baud(ctx));
            bsl_exec(bsl_flash_id(ctx, x));
            bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_EXEC_DATA, 0));

            frame_sz = FRAMESZ_PDL;
            continue;
        }

        if (!strcasecmp(x->FileID, "SPL"))
        {
            frame_sz = FRAMESZ_PDL;
            bsl_exec(bsl_check_baud(ctx));
            bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_CONNECT, 0));
            //bsl_exec(bsl_change_baud(ctx));
            bsl_exec(bsl_flash_id(ctx, x));

            frame_sz = FRAMESZ_PDL;
            continue;
        }

        /*-------------------------------------------------------------------------*/
        //Erase Flash
#if 1
        if(!strcasecmp(x->Type, "EraseFlash") && strcasecmp(x->FileID, "FMT_FSSYS") && g_downInfo.erase_all == 1)
        {
            if (!strtoul(x->Flag, NULL, 16) && !strtoul(x->CheckFlag, NULL, 16))
            {
                bsl_exec(bsl_erase_id(ctx, x, 0));
            }
            continue;
        }
#else
        if(!strcasecmp(x->Type, "EraseFlash"))
        {
            if (!strtoul(x->Flag, NULL, 16) && !strtoul(x->CheckFlag, NULL, 16))
            {
                bsl_exec(bsl_erase_id(ctx, x));
            }
            continue;
        }
#endif
        //down load image
        if (fileFlag == 1)
        {
            //check file
            if (fileCheckFlag == 1 && (!x->image || !x->image->szFileName[0]))
            {
                DBPrintf("[ ERR ] find image\n");
                return -1;
            }

            if (x->image && x->image->szFileName[0])
            {
                if (!checkOK(x->FileID))
                    continue;

                if(!strcasecmp(x->Type, "BOOT_LOADER"))
                {
                    //boot_load = i;
                    //bsl_exec(bsl_erase_id(ctx, x));
                    bsl_exec(bsl_flash_id(ctx, x));
                }
                else
                {
                    saveDeal(x->FileID, OTA_NOK);
                    bsl_exec(bsl_flash_id(ctx, x));
                    saveDeal(x->FileID, OTA_OK);
                }

                continue;
            }
        }
    }

#if 0
    if (boot_load != -1)
    {
        Scheme_t *mx = &ctx->SchemeList[boot_load];
        if (mx->image && mx->image->szFileName[0]) {
            bsl_exec(bsl_flash_id(ctx, mx));
        }
    }
#endif
    if (g_downInfo.backupNV == 1 && g_downInfo.erase_all == 1)
    {
        nvBackup(0, ctx);
    }
    bsl_exec(bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_NORMAL_RESET, 0));

    return 0;
}

static void update_transfer_result(int is_succ)
{
    if (!is_succ)
    {
        memset(&g_otaStatus, 0, sizeof(g_otaStatus));
    }
    else
    {
        g_otaStatus.bOK = OTA_NOK;
        if (OTA_OK == bNVBackup) 
            g_otaStatus.bNVBackup = OTA_OK;
    }
    writeFile_OTA();
}

int dloader_main(int usbfd, FILE *fw_fp, const char *modem_name, int *errRebootCnt)
{
    static pac_ctx_t g_pac_ctx = {0};
    pac_ctx_t *ctx = &g_pac_ctx;
    int ret = -1;

    readFile_OTA();
    DBPrintf("OTA list status:%d\n", g_otaStatus.bOK);
    DBPrintf("OTA list Backup:%d\n", g_otaStatus.bNVBackup);
    g_otaStatus.bNVBackup=g_downInfo.backupNV;
    memset(ctx, 0, sizeof(pac_ctx_t));
    ctx->pdl_req = (PDL_CMD_T *)ctx->bsl_req;
    ctx->pdl_rsp = (PDL_CMD_T *)ctx->bsl_rsp;
    ctx->usbfd = usbfd;

    if (pac_parser(ctx, fw_fp))
        goto out;

    ret = dloader_bsl(ctx);
out:
    if (ctx->xml_buf)
        free(ctx->xml_buf);

    update_transfer_result(ret);
    if (errRebootCnt)
    {
        if (!ret)
            *errRebootCnt = 0;
        else
            *errRebootCnt += 1;
    }

    if (0 != ret)
    {
        bsl_send_cmd_wait_ack(ctx, NULL, BSL_CMD_NORMAL_RESET, 0);
    }

    return ret;
}

