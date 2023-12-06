#include "fastcgi_updownload.h"
//#include <zlib.h>

#define MEM_SIZE	1024
#define MEM_HALT	512

static const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};
 
static unsigned int crc32( const unsigned char *buf, unsigned int size)
{
     unsigned int i, crc;
     crc = 0xFFFFFFFF;
 
 
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
 
     return crc^0xFFFFFFFF;
} 

void *memmem(const void *buf, size_t buf_len, const void *byte_line, size_t byte_line_len)
{
    unsigned char *bl = (unsigned char *)byte_line;
    unsigned char *bf = (unsigned char *)buf;
    unsigned char *p  = bf;

    while (byte_line_len <= (buf_len - (p - bf))){
        unsigned int b = *bl & 0xff;
        if ((p = (unsigned char *) memchr(p, b, buf_len - (p - bf))) != NULL){
            if ( (memcmp(p, byte_line, byte_line_len)) == 0)
                return p;
            else
                p++;
        }else{
            break;
        }
    }
    return NULL;
}

int findStrInFile(char *filename, int offset,char *str, int str_len)
{
	int pos = 0, rc;
	FILE *fp;
	unsigned char mem[MEM_SIZE];

	if(str_len > MEM_HALT)
	{
		return -1;
	}
		
	if(offset <0)
	{
		return -1;
	}
		

	fp = fopen(filename, "rb");
	if(!fp)
	{
		return -1;
	}
		

	rewind(fp);
	fseek(fp, offset + pos, SEEK_SET);
	rc = fread(mem, 1, MEM_SIZE, fp);
	while(rc){
		unsigned char *mem_offset;
		mem_offset = (unsigned char*)memmem(mem, rc, str, str_len);
		if(mem_offset){
			fclose(fp);	//found it
			return (mem_offset - mem) + pos + offset;
		}

		if(rc == MEM_SIZE){
			pos += MEM_HALT;	// 8
		}else
			break;
		
		rewind(fp);
		fseek(fp, offset+pos, SEEK_SET);
		rc = fread(mem, 1, MEM_SIZE, fp);
	}

	fclose(fp);
	return -1;
}

/*
 *  ps. callee must free memory...
 */
void *getMemInFile(char *filename, int offset, int len)
{
    void *result;
    FILE *fp;
    if( (fp = fopen(filename, "r")) == NULL ){
        return NULL;
    }
	fseek(fp, offset, SEEK_SET);
    result = malloc(sizeof(unsigned char) * len );
	if(!result)
		return NULL;
    if( fread(result, 1, len, fp) != len){
        free(result);
        return NULL;
    }
    return result;
}

int getFileSize(char *filename)
{
	int file_len = 0;
    FILE *fp = NULL;
	
    fp = fopen(filename,"r");
	if(NULL == fp)
	{
		printf("file open faild\n");
		return 0;
	}

	fseek(fp,0L,SEEK_END);
	
	file_len = ftell(fp) + 1;

	fclose(fp);
	
	return file_len;
}

FCGI_BOOL get_upload_content(char * filename,int *rest_code)
{
	FILE *f=NULL;
	int ilen=0,itry=0;
	
	char tmp_text[LONG_BUFF_LEN]={0};

	system("echo 3 > /proc/sys/vm/drop_caches");
	
	f = fopen(filename, "w+");

	if(!f)
	{
		*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
		return FCGI_FALSE;
	}
	
	while(1)
	{
		memset(tmp_text, 0, LONG_BUFF_LEN);
		itry=fread(tmp_text, 1, LONG_BUFF_LEN, stdin);
		if(!itry)
		{
           break;
		}
		
		ilen = fwrite(tmp_text, 1,itry, f);
		if(ilen!=itry)
		{
			fclose(f);
			*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
			return FCGI_FALSE;
		}
	}

	fclose(f);
	f = NULL;
	
	return FCGI_TRUE;
}

FCGI_BOOL get_upload_nohead_content(char * filename,int *rest_code)
{
	FILE *f=NULL;
	int ilen=0,itry=0;
	int first_read = 1;
	int first_read_len = 0;
	int data_len = 0;
	char *text_head = NULL;
	char tmp_text[4096]={0};

	system("echo 3 > /proc/sys/vm/drop_caches");

	data_len = strtol(getenv("CONTENT_LENGTH"), NULL, 10);

	if(data_len <= (LONG_BUFF_LEN*1024))
	{
		*rest_code = FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		return FCGI_FALSE;
	}
	
	f = fopen(filename, "w+");

	if(!f)
	{
		*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
		return FCGI_FALSE;
	}
	
	while(1)
	{
		memset(tmp_text, 0, LONG_BUFF_LEN);
		itry=fread(tmp_text, 1, LONG_BUFF_LEN, stdin);
		if(itry <= 0)
		{
           	fclose(f);
			*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
			return FCGI_FALSE;
		}

		while(first_read == 1 && itry < LONG_BUFF_LEN)
		{
			first_read_len = fread(tmp_text+itry, 1, LONG_BUFF_LEN-itry, stdin);
			if (first_read_len <= 0)
			{
				fclose(f);
				*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
				return FCGI_FALSE;
			}
			itry += first_read_len;
		}

		if (first_read == 1)
		{			
			if ((text_head = strstr(tmp_text,"\r\n\r\n")) == NULL)
			{
				fclose(f);
				*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
				return FCGI_FALSE;
			}
			text_head += 4;
			itry = itry - (text_head - tmp_text);

			data_len = data_len - (text_head - tmp_text);

			ilen = fwrite(text_head, 1,itry, f);
			if(ilen!=itry)
			{
				fclose(f);
				*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
				return FCGI_FALSE;
			}

			data_len = data_len - ilen;
			
			first_read = 0;
			continue;
		}

		if (data_len <= (LONG_BUFF_LEN*2))
		{
			while(itry < data_len)
			{
				first_read_len = fread(tmp_text+itry, 1, data_len-itry, stdin);
				if (first_read_len <= 0)
				{
					fclose(f);
					*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
					return FCGI_FALSE;
				}
				itry += first_read_len;
			}
			
			if ((text_head = memmem(tmp_text, itry, "\r\n", strlen("\r\n"))) != NULL)
			{
				
				itry = (text_head - tmp_text);
			}
			
			ilen = fwrite(tmp_text, 1,itry, f);
			if(ilen!=itry)
			{
				fclose(f);
				*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
				return FCGI_FALSE;
			}
			break;
		}
		
		ilen = fwrite(tmp_text, 1,itry, f);
		if(ilen!=itry)
		{
			fclose(f);
			*rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
			return FCGI_FALSE;
		}

		data_len = data_len - ilen;
	}
	
	fclose(f);
	
	return FCGI_TRUE;
}

int cut_upload_file(char *filename,char *realfile)
{
	int file_begin, file_end;
    int line_begin, line_end;
    char *boundary; int boundary_len;
	FILE *f=NULL;
	FILE *fp=NULL;

	line_begin = 0;
	if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
		return -1;
	}
	boundary_len = line_end - line_begin;
	boundary = getMemInFile(filename, line_begin, boundary_len);

	// sth like this..
	// Content-Disposition: form-data; name="filename"; filename="\\192.168.3.171\tftpboot\a.out"
	//
	char *line, *semicolon, *user_filename;
	line_begin = line_end + 2;
	if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
		free(boundary);
		return -1;
	}
	line = getMemInFile(filename, line_begin, line_end - line_begin);
	if(strncasecmp(line, "content-disposition: form-data;", strlen("content-disposition: form-data;"))){
		free(boundary);
		free(line);
		return -1;
	}
	semicolon = line + strlen("content-disposition: form-data;") + 1;
	if(! (semicolon = strchr(semicolon, ';'))  ){
		free(boundary);
		free(line);
		return -1;
	}
	user_filename = semicolon + 2;
	if( strncasecmp(user_filename, "filename=", strlen("filename="))  ){
		free(boundary);
		free(line);
		return -1;
	}
	user_filename += strlen("filename=");
	//until now we dont care about what the true filename is.
	free(line);

	/* We may check a string  "Content-Type: application/octet-stream" here,
	 but if our firmware extension name is the same with other known ones,
	 the browser would use other content-type instead.
	 So we dont check Content-type here...
	*/
	line_begin = line_end;
	if((line_end = findStrInFile(filename, line_begin, "\r\n\r\n", 4)) == -1){
		free(boundary);
		free(line);
		return -1;
	}

	file_begin = line_end + 4;

	if( (file_end = findStrInFile(filename, file_begin, boundary, boundary_len)) == -1){
		free(boundary);
		free(line);
		return -1;
	}

	file_end -= 2;
	free(boundary);
	int ilen=0,try=0,file_len=0;
	char tmp_text[1024]={0};
	file_len=file_end-file_begin;
	f = fopen(realfile, "w+");
	if(!f)
	{
		return -1;
	}
	fp = fopen(filename, "rb");
	if(!fp)
	{
		fclose(f);
		return -1;
	}
	fseek(fp, file_begin, SEEK_SET);
	
	while(1){
		memset(tmp_text, 0, 1024);
		try=fread(tmp_text, 1, 1024, fp);
		if(file_len<=try){
			ilen = fwrite(tmp_text, 1, file_len, f);
			if(ilen!=file_len)
			{
				fclose(fp);
				fclose(f);
				return -1;
			}
            fclose(fp);
			fclose(f);
			return file_end-file_begin;
		}else{
			ilen = fwrite(tmp_text, 1, try, f);
			if(ilen!=try)
			{
				fclose(f);
				fclose(fp);
				return -1;
			}
		}
		file_len=file_len-ilen;
	}
	
}

FCGI_BOOL check_upload_file(char *filename)
{
	char cmd[CMD_STR_LEN] = {0};
	char buf[OPTION_STR_LEN] = {0};
	char check_model[OPTION_STR_LEN] = {0};
	FCGI_BOOL ret = FCGI_FALSE;
	
	if(NULL == filename || 0 == strlen(filename))
	{
		ret = FCGI_FALSE;
		goto end;
	}

	snprintf(cmd, sizeof(cmd), "rm -rf %s", CHECK_BACKUP_FILE_PATH);
	system(cmd);

	snprintf(cmd, sizeof(cmd), "mkdir %s", CHECK_BACKUP_FILE_PATH);
	system(cmd);
	
	snprintf(cmd, sizeof(cmd), "tar -zxf %s -C %s", filename, CHECK_BACKUP_FILE_PATH);
	system(cmd);

	snprintf(cmd, sizeof(cmd), "uci -q -c %s get system.upgrade.check_model", CHECK_PATH);

	get_cmd_result(cmd,buf,OPTION_STR_LEN);

	if(!huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",check_model))
	{
		ret = FCGI_FALSE;
		goto end;
	}

//	if(strlen(buf) == 0 || 0 != strcmp(check_model, buf))
    if(strlen(buf) == 0)
	{
		ret = FCGI_FALSE;
		goto end;
	}

	ret = FCGI_TRUE;
	
end:
	remove(CHECK_BACKUP_FILE_PATH);
	return ret;
}

int check_system_firmware(char *imagefile, int offset, int len)
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
    //upgrade_no_check =1;
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

/*read file and write to stdout */
void huayi_common_download_file(char *read_file,FCGI_BOOL need_remove)
{
	FILE *fp = NULL;
	char *output = NULL;
	int  file_size;
	
	fp=fopen(read_file,"r");
	
	if(!fp)
	{
		return;
	}

	fseek(fp,0L,SEEK_END);
	
	file_size = ftell(fp);

	output = malloc(file_size);

	fseek(fp,0L,SEEK_SET);
		
	fread(output,file_size,1,fp);

	fclose(fp);

	FCGI_fwrite(output, 1,file_size, stdout);

	free(output);

	if(need_remove== FCGI_TRUE)
	{
		remove(read_file);
	}

	return;
}

void do_download_supermode(char *super_file)
{
	char file_name[TEMP_STR_LEN] ={0};
	char date[OPTION_STR_LEN] = { 0 };
	
	if(!get_cmd_result("date +%Y%m%d%H%M%S",date,OPTION_STR_LEN))
	{
		return;
	}

	snprintf(file_name,sizeof(file_name),"superfile-%s",date);
	
	FCGI_printf("Content-type:application/octet-stream\n");

	FCGI_printf("Content-Disposition: attachment; filename=%s\n\n",file_name);

	huayi_common_download_file(super_file,FCGI_FALSE);
	
	return;	
}

void do_download_syslog()
{
	char file_name[TEMP_STR_LEN] ={0};
	char product_name[OPTION_STR_LEN]={0};
	char date[OPTION_STR_LEN] = { 0 };
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);

	if(!get_cmd_result("date +%Y%m%d%H%M%S",date,OPTION_STR_LEN))
	{
		return;
	}

	snprintf(file_name,sizeof(file_name),"%s-%s.log",product_name,date);

	system("logread > /tmp/syslog 2>/dev/null");
	
	FCGI_printf("Content-type:application/octet-stream\n");
	
	FCGI_printf("Content-Disposition: attachment; filename=%s\n\n",file_name);

	huayi_common_download_file("/tmp/syslog",FCGI_TRUE);
	
	return;	
}


void do_backup_system_config()
{
	char file_name[TEMP_STR_LEN] ={0};
	char product_name[OPTION_STR_LEN]={0};
	char date[OPTION_STR_LEN] = { 0 };
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);

	if(!get_cmd_result("date +%Y%m%d%H%M%S",date,OPTION_STR_LEN))
	{
		return;
	}

	snprintf(file_name,sizeof(file_name),"backup-%s-%s.tar.gz",product_name,date);

	system("sysupgrade --create-backup /tmp/backup 2>/dev/null");
	
	FCGI_printf("Content-type:application/octet-stream\n");
	
	FCGI_printf("Content-Disposition: attachment; filename=%s\n\n",file_name);

	huayi_common_download_file("/tmp/backup",FCGI_TRUE);
	
	return;	
}

int do_restore_system_config(int *lock_time)
{
	int rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
	int len=0;

	if(get_upload_content(UPLOAD_BACKUP_FILE,&rest_code))
	{	
		if((len=cut_upload_file(UPLOAD_BACKUP_FILE,REAL_BACKUP_FILE))!=-1)
		{	
			if(FCGI_TRUE == check_upload_file(REAL_BACKUP_FILE))
			{
				*lock_time = HUAYI_SYSTEM_UPGRADE_TIME;
				rest_code = FCGI_RESP_CODE_SUCCESS;
				system("huayi-reset restore_config");
			}
			else
			{
				remove(REAL_BACKUP_FILE);
				rest_code = FCGI_RESP_CODE_FILE_FORMAT_ERROR;
			}			
		}else{
			rest_code = FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
		
		remove(UPLOAD_BACKUP_FILE);
	}else
	{
		*lock_time = 0;
		return rest_code;
	}
	
	return rest_code;	
}


int do_system_upgrade(int *lock_time)
{
	int 	len=0;
	int 	rest_code = FCGI_RESP_CODE_SERVER_FAILURE;
	
	if(get_upload_nohead_content(UPGRADE_FILE,&rest_code))
	{
		len = getFileSize(UPGRADE_FILE);
		if(!check_system_firmware(UPGRADE_FILE,0,len))
		{
			system("huayi-reset upgrade");
			*lock_time = HUAYI_SYSTEM_UPGRADE_TIME;
			rest_code = FCGI_RESP_CODE_SUCCESS;
			return rest_code;
		}else
		{
			rest_code = FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
	}
	
	remove(UPGRADE_FILE);
	
	return rest_code;
}


char* MemSrch (char* src, char* sub, int scrSize, int subSize)
{    
	char *bp;    
	char *sp;    
	int restScrSz = scrSize; 
	int restSubSz =subSize;
	
	if(!scrSize || !subSize) 
	{        
		return NULL;    
	}     
	
	while(restScrSz > 0) 
	{           
		bp = src;
		sp = sub;
		while(restSubSz > 0) 
		{            
			if(*bp == *sp) 
			{        
				bp++;                
				sp++;                
				restSubSz--;                
				if(!restSubSz)
				{               
					return src;                
				}            
			} else 
			{                
				restSubSz = subSize;                
				break;           
			}        
		}
		src++;        
		restScrSz--;
	}    
	return NULL;
}

int uploade_multipart_file(int * lock_time)
{
	int content_len=0;
	char *content_type = NULL;
	char *query_data = NULL;
	char *boundary = NULL;
	int  boundary_len = NULL;
	int  offset =0;
	char *start = NULL, *end = NULL;
	char *quote,*pname;
	int attr_len,filename_len, file_size = 0;
	int content_position_len = 0, content_type_len = 0;
	char attr[OPTION_STR_LEN];
	char filename[OPTION_STR_LEN];
	char tmp_file[OPTION_STR_LEN];
	FILE *fp = NULL;
	
	system("echo 3 > /proc/sys/vm/drop_caches");
	
	content_len = strtol(getenv("CONTENT_LENGTH"), NULL, 10);
	
	content_type =getenv("CONTENT_TYPE");

	if(!strstr(content_type,"multipart/form-data"))
	{
		return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
	}
	
	query_data = malloc(content_len);
	
	if(query_data == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	boundary = strstr(content_type,"boundary=");

	boundary+=strlen("boundary=");
	
	boundary_len =strlen(boundary); 
	
	memset(query_data,0,content_len);

	start = NULL;
	
	fread(query_data, 1, content_len, stdin);
	
	start = (char *)query_data;

	offset = 0;

	end = strstr(start, boundary);
	if(end)
	{
		offset+=(end-start);
		start = end + boundary_len;
		offset+=boundary_len;
	}else
	{
		free(query_data);
		return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
	}
	
	while (offset < content_len) 
	{
		if (0 == memcmp(start, "--\r\n", 4))
		{
			offset+=4;
        	break;  
    	}
		
    	start += 2; //strlen("\r\n") = 2;
    	offset +=2;
    	
		/*content_Disposition start from here*/
		pname = NULL;
		quote = NULL;
		memset(attr,0,OPTION_STR_LEN);
		attr_len = 0;
		end = strstr(start,"\r\n");
		if(!end)
		{
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}

		//
		pname = strstr(start, "name=\"");
		if(pname == NULL)
		{
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
		
		quote= strchr(pname + 6, '\"');    //strlen("name=\"") = 6;
		if(quote == NULL)
		{
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
	
		
		attr_len = quote - (pname + 6);        
		strncpy(attr, pname + 6, attr_len);        
		attr[attr_len] = '\0';
		pname = strstr(start, "filename=\"");        
		if (NULL == pname) 
		{    
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
		
		quote = strchr(pname + 10, '\"'); //strlen("filename=\"") = 10;            
		if(NULL == quote) 
		{                
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;          
		}
		
		filename_len = quote - (pname + 10);            
		strncpy(filename, pname + 10, filename_len);            
		filename[filename_len] = '\0';            
		//
		
		content_position_len = (end-start);
		offset+=content_position_len;
		start +=content_position_len;

		start += 2; //strlen("\r\n") = 2;
    	offset +=2;

		/*content_type start from here*/
		end = strstr(start,"\r\n\r\n");
		if(!end)
		{
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}

		content_type_len = (end-start);
		offset+=content_type_len;
		start +=content_type_len;

		start += 4; //strlen("\r\n\r\n") = 4;
    	offset += 4;

		/*file data from here*/
		end = MemSrch(start,boundary,content_len-offset,boundary_len);
		if(end)
		{
			
			file_size = (end-start) -4;
			if(file_size)
			{
				snprintf(tmp_file,sizeof(tmp_file),"/tmp/%s",filename);
				
                fp = fopen(tmp_file, "w+");
				if (NULL != fp) 
				{
					fwrite(start, 1, file_size, fp);                    
					fclose(fp);                
				}
			}
			
			offset+=(end-start);
			start = end + boundary_len;
			offset+=boundary_len;
		}else
		{
			free(query_data);
			return FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		}
	}

	free(query_data);
	
	return FCGI_RESP_CODE_SUCCESS;
}





