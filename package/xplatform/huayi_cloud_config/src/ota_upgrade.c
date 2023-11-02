/*******************************************************************************************
 * Project:  huayi_clound_config                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20210114 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_config.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <string.h>
#include "libutils.h"


extern int need_upgrade;

int huayi_check_clound_firmware(char *imagefile, int offset, int len)
{
	struct stat sbuf;

	int  data_len;
	char *data;
	unsigned char *ptr;
	unsigned long checksum;

	image_header_t header;
	image_header_t *hdr = &header;
	int upgrade_no_check = 0;
	char product_model[OPTION_STR_LEN] = {0};

	int ifd;

	if ((unsigned)len < sizeof(image_header_t)) {
		return -1;
	}

	ifd = open(imagefile, O_RDONLY);
	if(!ifd){
		return -1;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		return -1;
	}

	ptr = (unsigned char *) mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if ((caddr_t)ptr == (caddr_t)-1) {
		close(ifd);
		return -1;
    }
	ptr += offset;

	/*
	 *  handle Header CRC32
	 */
    memcpy (hdr, ptr, sizeof(image_header_t));
	
	huayi_uci_get_int(PKG_SYSTEM,"global","upgrade_no_check",&upgrade_no_check);
	if(!upgrade_no_check)
	{
		huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
		if((strlen(product_model) != strlen(hdr->ih_name)) 
			|| strcmp(product_model,(char *)(hdr->ih_name)))
		{
			munmap(ptr, len);
			close(ifd);
			return -1;
		}
	}
	
    if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		munmap(ptr, len);
		close(ifd);
		return -1;
	}

	data = (char *)hdr;

    checksum = ntohl(hdr->ih_hcrc);
    hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

    if (crc32 ((const unsigned char *)data, sizeof(image_header_t)) != checksum) {
		munmap(ptr, len);
		close(ifd);
		return -1;
    }
	
	/*
	 *  handle Data CRC32
	 */
    data = (char *)(ptr + sizeof(image_header_t));
    data_len  = ntohl(hdr->ih_size) ;

    if (crc32 ((const unsigned char *)data, data_len) != ntohl(hdr->ih_dcrc)) {
		munmap(ptr, len);
		close(ifd);
		return -1;
    }

	munmap(ptr, len);
	close(ifd);

	return 0;
}


static int process_downloadimage(char *fwurl, char *md5)
{
    int iState = -1, len = 0;
    char *fileMd5 = NULL;
    char szCmd[CMD_STR_LEN];
	
    if ((strlen(fwurl) == 0))
		return -1;
	need_upgrade = 1;
	/*dowload timeout is 60s*/
	sprintf(szCmd, "curl \"%s\" -s -L -o %s --connect-timeout 60 -m 43200 --speed-limit 5000 --speed-time 300", fwurl,IMAGE_PATH_STOR);
    iState = system(szCmd);
    
    if ((WIFEXITED(iState)) && (WEXITSTATUS(iState) == 0) && (iState != -1))
    {
        /*
    	fileMd5  = MD5_file(IMAGE_PATH_STOR, 32);
		if(fileMd5 == NULL) 
			return -1;
		if(strcmp(fileMd5,md5) != 0) {
			CLOUD_CLIENT_LOG("%s check firmware md5 fail. download image md5:%s receive param md5:%s\n",__FUNCTION__, fileMd5, md5);
			free(fileMd5);
			fileMd5 = NULL;
			return -1;
		} else 
            */
       {
			CLOUD_CLIENT_LOG("%s check firmware md5 ok\n",__FUNCTION__);
			len = get_file_size(IMAGE_PATH_STOR);
			if(!huayi_check_clound_firmware(IMAGE_PATH_STOR, 0, len))
			{
				system("huayi_cmd set system upgrade_keep_setting 1");
				CLOUD_CLIENT_LOG("%s check firmware ok, do upgrade !\n",__FUNCTION__);
				system("huayi-reset upgrade");
				return 0;
			}
		}
    }            
   
	CLOUD_CLIENT_LOG("[firmwire] download image file fail, download:%d\n",iState);
	
    return iState;    
}


void huayi_remote_upgrade(cJSON *response_root)
{
	char firmware_md5[OPTION_STR_LEN] = {0}, firmware_url[LIST_STR_LEN] = {0};
	int ret = -1;
	
	if(NULL == response_root) 
	{
		return ;
	}
	/*OTA upgrade is doing...*/
    CLOUD_CLIENT_LOG("OTA upgrade is doing...");
	if(1 == need_upgrade)
		return;
	if(!huayi_json_get_str(response_root,"firmware_url", firmware_url, LIST_STR_LEN)) {
		printf("firmware_url param is not respon !\n");
		return;
	} else {
		if(0 == strlen(firmware_url)) {
			printf("firmware_url len is %d !\n", strlen(firmware_url));
            CON_LOG("firmware_url len is %d !\n", strlen(firmware_url));
			return;
		} else {
			huayi_uci_set_str(PKG_SYSTEM, "server","firmwareurl", firmware_url);
		}
	}
	/*
	if(!huayi_json_get_str(response_root,"firmware_md5", firmware_md5, OPTION_STR_LEN)) {
		printf("firmware_md5 param is not respon !\n");
		return;
	} else {
		if(0 == strlen(firmware_md5)) {
			printf("firmware_md5 len is %d !\n", strlen(firmware_md5));
			return;
		} else {
			huayi_uci_set_str(PKG_SYSTEM, "server","firmwaremd5", firmware_md5);
		}
	}	
	huayi_uci_commit(PKG_SYSTEM);
	CLOUD_CLIENT_LOG("%s, %d, url = %s md5 = %d!\n", __FUNCTION__, __LINE__, firmware_url, firmware_md5);
	*/
	CLOUD_CLIENT_LOG("%s, %d, url = %s \n", __FUNCTION__, __LINE__, firmware_url);
	ret = process_downloadimage(firmware_url, firmware_md5);
    if ( 0 != ret) {
		need_upgrade = 0;
		remove(IMAGE_PATH_STOR);
        printf("downloadimage failed\n");
    }

	return;
}

