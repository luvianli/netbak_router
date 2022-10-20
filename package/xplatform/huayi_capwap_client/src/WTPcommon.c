/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/

#include "WTPcommon.h"

unsigned char WTPSetSeqNum(struct VcapwapAgent *Agent,WTPBool is_retry)
{
	if(WTP_FALSE == is_retry)
	{
		Agent->seqNum++;
	}
	
	return Agent->seqNum;
}

int ac_ip_to_proc(char *ac_ip)
{
	char cmd[CMD_STR_LEN] = {0};

	snprintf(cmd,CMD_STR_LEN, "huayi_cmd set system ac_ipaddr %s", ac_ip);
	
	system(cmd);
	
	return 0;
}

int wtp_status_to_proc(AC_STATE state)
{
	char cmd[CMD_STR_LEN] = {0};

	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system capwap_client_status %d", state);
	
	system(cmd);
	
	return 0;
}

int wtp_name_to_uci(char *wtp_name)
{

	huayi_uci_set_str(PKG_THINAP,"global","wtp_name",wtp_name);
	
	huayi_uci_commit(PKG_THINAP);
	
	return 0;
}

int current_uptime_to_proc(void)
{
	int uptime = 0;
	char cmd[CMD_STR_LEN] = {0};
	
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);

	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system ac_assoc_time %d", uptime);

	system(cmd);
	
	return 0;
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
                       fclose(fp);     //found it
                       return (mem_offset - mem) + pos + offset;
               }

               if(rc == MEM_SIZE){
                       pos += MEM_HALT;        // 8
               }else
                       break;
               
               rewind(fp);
               fseek(fp, offset+pos, SEEK_SET);
               rc = fread(mem, 1, MEM_SIZE, fp);
       }

       fclose(fp);
       return -1;
}

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

