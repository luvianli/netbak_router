#include "huayi_mqtt.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <string.h>
#include "libutils.h"

void huayi_set_wifi(cJSON *response_root)
{	
	char ssid[TEMP_STR_LEN] = {0},oldssid[OPTION_STR_LEN] = {0};
	char key[OPTION_STR_LEN] = {0}, oldkey[TEMP_STR_LEN] = {0};
	int hid = -1,hidden = -1, oldhidden = -1;
	int sta = -1,status = -1, oldstatus = -1;
	int speedLimit = 1,oldspeedLimit = 0,qos_mode = 0,get;
	int flag = 0;
	unsigned long long monthFlow = 0;
	
	if(NULL == response_root) 
	{
		return ;
	}

#if 0
	if(huayi_json_get_int(response_root,"get",&get))
	{
		if(get == 1)
		{
			system("killall huayi_mqtt_pub");
			system("killall huayi_mqtt_content");			
			sleep(2);
			system("huayi_mqtt_content &");
			sleep(5);
			system("huayi_mqtt_pub &");		
		}
	}

	if (huayi_json_get_str(response_root, "ssid", ssid,OPTION_STR_LEN))
	{
		huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid", oldssid);
		if((strlen(ssid) > 0 && strlen(ssid) <= 20) && 0 != strcmp(ssid, oldssid))
		{	
			MQTT_LOG("change ssid");
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid", ssid);
			flag = 1;
		}
	}

	if(huayi_json_get_str(response_root,"password",key,TEMP_STR_LEN)) 
	{	
		huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key", oldkey);
		if((strlen(key) >= 8 && strlen(key) <=25) && (0 != strcmp(key, oldkey))) {
			MQTT_LOG("change key");
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key", key);
			flag = 1;
		}
	}

	if(huayi_json_get_int(response_root,"hidden",&hid)) 
	{	
		if(hid == 0)
			hidden = 1;
		else if(hid == 1)
			hidden = 0;
		huayi_uci_get_int(PKG_WIRELESS,"radio0_0","hidden",&oldhidden);
		if((hidden == 0||hidden == 1) && (hidden != oldhidden)) {
			MQTT_LOG("change hidden %d",hidden);
			huayi_uci_set_int(PKG_WIRELESS,"radio0_0","hidden",hidden);
			flag = 1;
		}
	}

	if(1 == flag ) {
		huayi_uci_commit(PKG_WIRELESS);	
		system("huayi-reset wireless");
	}

	if(huayi_json_get_int(response_root,"speedLimit",&speedLimit)) 
	{	
		huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&oldspeedLimit);
		huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&qos_mode);
		if((speedLimit > 0) && (speedLimit != oldspeedLimit)) {
			MQTT_LOG("change limit");
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 1);
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",speedLimit);
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",speedLimit);
			huayi_uci_commit(PKG_QOS);	
			system("huayi-reset iplimit");
		}else if(speedLimit == 0 && qos_mode == 1) {
			MQTT_LOG("colse limit");
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 0);
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",0);
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",0);
			huayi_uci_commit(PKG_QOS);	
			system("huayi-reset iplimit");
		}
	}	

	if(huayi_json_get_int(response_root,"monthFlow",&monthFlow)) 
	{	
		MQTT_LOG("change monthFlow");
		huayi_uci_set_long_long(PKG_SYSTEM, "network", "monthFlow", monthFlow);
		huayi_uci_commit(PKG_SYSTEM);
	}
#else
	if (huayi_json_get_str(response_root, "ssid", ssid,OPTION_STR_LEN))
	{
		huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid", oldssid);
		if((strlen(ssid) > 0 && strlen(ssid) <= 20) && 0 != strcmp(ssid, oldssid))
		{	
			MQTT_LOG("change ssid");
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid", ssid);
			flag = 1;
		}
	}

	if(huayi_json_get_str(response_root,"password",key,TEMP_STR_LEN)) 
	{	
		huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key", oldkey);
		if((strlen(key) >= 8 && strlen(key) <=25) && (0 != strcmp(key, oldkey))) {
			MQTT_LOG("change key");
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key", key);
			flag = 1;
		}
	}

	if(huayi_json_get_int(response_root,"hidden",&hidden)) 
	{	
		huayi_uci_get_int(PKG_WIRELESS,"radio0_0","hidden",&oldhidden);
		if((hidden == 0||hidden == 1) && (hidden != oldhidden)) {
			MQTT_LOG("change hidden %d",hidden);
			huayi_uci_set_int(PKG_WIRELESS,"radio0_0","hidden",hidden);
			flag = 1;
		}
	}

	if(huayi_json_get_int(response_root,"status",&status)) 
	{	
		huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&oldstatus);
		if((status == 0||status == 1) && (status != oldstatus)) {
			MQTT_LOG("change status %d",status);
			huayi_uci_set_int(PKG_WIRELESS,"radio0_0","disabled",status);
			flag = 1;
		}
	}

	if(1 == flag ) {
		huayi_uci_commit(PKG_WIRELESS);	
		system("huayi-reset wireless");
	}

#endif

	return;
}

void huayi_set_qos(cJSON *response_root)
{
	int dwspeed = -1, upspeed = -1, enable = 0, upflag = 0, dwflag = 0;
	int olddwspeed = -1, oldupspeed = -1;
	
	if(NULL == response_root) 
	{
		return ;
	}

	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed", &oldupspeed);
	huayi_uci_get_int(PKG_QOS,"qos","total_downspeed", &olddwspeed);
		
	if(huayi_json_get_int(response_root,"speed_downlink", &dwspeed)) 
	{
		if ((-1 != dwspeed) && (olddwspeed != dwspeed)) 
		{
			MQTT_LOG("change speed_downlink %d",dwspeed);
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",dwspeed);
			upflag = 1;
		}
	}
			
	if(huayi_json_get_int(response_root,"speed_uplink", &upspeed)) 
	{
		if ((-1 != upspeed) && (oldupspeed != upspeed))
		{
			MQTT_LOG("change speed_uplink %d",upspeed);
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",upspeed);
			dwflag = 1;
		}
	}
		
	if(((-1 == upflag) && (-1 == dwflag))||((0 == dwflag) && (0 == upflag))	)
		return;
		
	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&enable);
	if((0 == upspeed) && (0 == dwspeed)) 
	{
		//don't speed limit
		if(1 == enable) 
		{//on --.off
				huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 0);
		}
	} else 
	{
		if(0 == enable) 
		{  //off ---. on
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 1);
		}
	}

	huayi_uci_commit(PKG_QOS);	
	system("huayi-reset iplimit");
	
	return;
}


void huayi_set_system(cJSON *response_root)
{
	int reboot = -1,reSet = -1;
	int switchsim = -1,simNum = 0,at_card = 0;
	int heartbeat = 30,oldheartbeat = 0;
	int upgrade = 0,out_slot_status = 0;
	int olddisconn = 0,disconn = 0;
	char firmware_url[LIST_STR_LEN] = {0};

	if(NULL == response_root) 
	{
		return;
	}

#if 0
	if(huayi_json_get_int(response_root,"disconn",&disconn)) 
	{
		huayi_uci_get_int(PKG_SYSTEM,"network","disconn",&olddisconn);		
		if((disconn == 0 || disconn == 1) && olddisconn != disconn)
		{
			MQTT_LOG("change disconn");
			huayi_uci_set_int(PKG_SYSTEM,"network","disconn",disconn);
			huayi_uci_commit(PKG_SYSTEM);
		}
	}

	if(huayi_json_get_int(response_root,"switch",&switchsim)) 
	{
		switchsim = switchsim -1;
		char cmd[RESULT_STR_LEN] = {0};
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simNum);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);		
		if(switchsim >= 0 && switchsim < simNum && switchsim != at_card)
		{
			MQTT_LOG("switch sim %d", switchsim);
			sprintf(cmd, "switch_sim %d &", switchsim);
			system(cmd);
			sleep(10);
			save_useing_siminfo(switchsim);
		}
	}

	if(huayi_json_get_int(response_root,"heartbeat",&heartbeat)) 
	{
		huayi_uci_get_int(PKG_SYSTEM,"server","heartbeat", &oldheartbeat);
		if(heartbeat >= 30 && heartbeat != oldheartbeat)
		{
			MQTT_LOG("change heartbeat");
			huayi_uci_set_int(PKG_SYSTEM,"server","heartbeat", heartbeat);
			huayi_uci_commit(PKG_SYSTEM);
			system("killall huayi_mqtt_pub");
			system("killall huayi_mqtt_content");			
			sleep(2);
			system("huayi_mqtt_content &");
			sleep(5);
			system("huayi_mqtt_pub &");	
		}
	}

	if(huayi_json_get_int(response_root,"upgrade",&upgrade)) 
	{
		if(upgrade == 1)
		{
			if(!huayi_json_get_str(response_root,"firmware_url", firmware_url, LIST_STR_LEN)) {
				MQTT_LOG("firmware_url param is not respon !\n");
				return;
			} else {
				if(0 == strlen(firmware_url)) 
				{
           		 	MQTT_LOG("firmware_url len is %d !\n", strlen(firmware_url));
					return;
				}
			}
			MQTT_LOG("firmware_url is %s !",firmware_url);

			int ret = process_downloadimage(firmware_url);
			if ( 0 != ret) {
				remove(IMAGE_PATH_STOR);
			}

		}
	}
#else
	if(huayi_json_get_int(response_root,"simSwitch",&switchsim)) 
	{
		//switchsim = switchsim -1;
		char cmd[RESULT_STR_LEN] = {0};
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simNum);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);	
		if(switchsim == 3)
			switchsim = simNum -1;	
		if(switchsim >= 0 && switchsim < simNum && switchsim != at_card)
		{
			MQTT_LOG("switch sim %d", switchsim);
			sprintf(cmd, "switch_sim %d &", switchsim);
			system(cmd);
			sleep(10);
			huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", switchsim);
			huayi_uci_commit(PKG_M_SYSTEM);
			save_useing_siminfo(switchsim);
		}
	}

	if(huayi_json_get_int(response_root,"nextRptTime",&heartbeat)) 
	{
		huayi_uci_get_int(PKG_SYSTEM,"mqtt","interval", &oldheartbeat);
		if(heartbeat >= 30 && heartbeat != oldheartbeat)
		{
			MQTT_LOG("change nextRptTime %d",heartbeat);
			huayi_uci_set_int(PKG_SYSTEM,"mqtt","interval", heartbeat);
			huayi_uci_commit(PKG_SYSTEM);
		}
	}

	if(huayi_json_get_int(response_root,"out_slot_status",&out_slot_status)) 
	{
		huayi_uci_set_int(PKG_M_SYSTEM, "system", "external_card_enable", out_slot_status);
		huayi_uci_commit(PKG_M_SYSTEM);
		MQTT_LOG("set out_slot_status %d", out_slot_status);
	}
	
#endif
	if(huayi_json_get_int(response_root,"reboot",&reboot)) 
	{
		if(reboot == 1)
			system("reboot");
	}

	if(huayi_json_get_int(response_root,"reset",&reSet)) 
	{
		if(reSet == 1)
			system("huayi-reset reset");
	}

	if(huayi_json_get_str(response_root,"firmware_url", firmware_url, LIST_STR_LEN))
	{
		if(0 == strlen(firmware_url)) 
		{
			MQTT_LOG("firmware_url len is %d !\n", strlen(firmware_url));
			return;
		}
		
		MQTT_LOG("firmware_url is %s !",firmware_url);
		
		int ret = process_downloadimage(firmware_url);
		if ( 0 != ret) {
			remove(IMAGE_PATH_STOR);
		}
	}

	return;
}

int process_downloadimage(char *fwurl)
{
    int iState = -1, len = 0;
    char *fileMd5 = NULL;
    char szCmd[CMD_STR_LEN];
	
    if ((strlen(fwurl) == 0))
		return -1;
	sprintf(szCmd, "curl \"%s\" -s -L -o %s --connect-timeout 60 -m 43200 --speed-limit 5000 --speed-time 300", fwurl,IMAGE_PATH_STOR);
    iState = system(szCmd);
    
    if ((WIFEXITED(iState)) && (WEXITSTATUS(iState) == 0) && (iState != -1))
    {
		len = get_file_size(IMAGE_PATH_STOR);
		if(!huayi_check_clound_firmware(IMAGE_PATH_STOR, 0, len))
		{
			system("huayi_cmd set system upgrade_keep_setting 1");
			MQTT_LOG("%s check firmware ok, do upgrade !\n",__FUNCTION__);
			system("huayi-reset upgrade");
			return 0;
		}
    }            
   
	MQTT_LOG("[firmwire] download image file fail, download:%d\n",iState);
	
    return iState;    
}

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

int main(int argc,char *argv[])
{
    FILE *file = fopen("/tmp/log/mqtt_sub_cont", "r");
    if (file == NULL) {
        return -1;
    }

	fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
	fclose(file);
	system("rm -rf /tmp/log/mqtt_sub_cont");

	MQTT_LOG("data:%s",data);

	cJSON *json = cJSON_Parse(data);
	if(json == NULL)
	{
		MQTT_LOG("json error");
		return;
	}

	huayi_set_wifi(json);
	huayi_set_qos(json);
	huayi_set_system(json);
	
	{//infochange reload
		system("killall huayi_mqtt_pub");
		system("killall huayi_mqtt_content");			
		sleep(2);
		system("huayi_mqtt_content &");
		sleep(5);
		system("huayi_mqtt_pub &"); 
	}
	
	return 0;
}
