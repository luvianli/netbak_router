/**
 * @file huayi_mac.c
 *
 * Created commands to change huayi config
 *
 * This tool is responsible to change the configuration 
 * of the huayi system.
 * 
 * =========================================================
 * Details about the command mode.
 * huayi_cmd <op>  <cmd> <interface><value>
 * @op: set or get
 * @cmd: suppoted cmd listed in cmd_help.txt
 * @interface: interface listed in cmd_help.txt, as sector/wifi. If empty, means system
 * @value: If op is get, option huayi will not be used. If op is set, the value's type
 * defined in cmd_help.txt(type), supported number of values defined in cmd_help.txt(num)
 *
 * Example:
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "huayi_uci_fun.h"
#include "huayi_config_common.h"

#define LAN_MAC_OFFSET			0x28

enum {	
	BASIC_MAC_MASK=0x1,
	LAN_MAC_MASK=0x2,	
	WAN_MAC_MASK=0x4,	
	W24_MAC_MASK=0x8,	
	W58_MAC_MASK=0x10,
	ALL_MAC_MASK=0x1f,
};



char mtd_factory[OPTION_STR_LEN]={0};

void usage(void)
{
	printf("Usage: huayi_mac <op> [paramter]\n");
	printf("huayi_mac write [ XXXXXXXXXXX | XX:XX:XX:XX:XX:XX] \n");
	printf("huayi_mac read all \n");
	printf("huayi_mac read basic \n");
	printf("huayi_mac read lan \n");
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	printf("huayi_mac read wan \n");
#endif
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	printf("huayi_mac read wlan_2g \n");
	#endif
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	printf("huayi_mac read wlan_5g \n");
	#endif
#endif
	return;
}

int mtd_read(char **value)
{
	int fd = open(mtd_factory, O_RDONLY);
	int i = 0;
	int readmask = 0;
	unsigned char basic_addr[MAC_ROW_LEN]={0};
	unsigned char wlan_addr[MAC_ROW_LEN]={0};
		
	if (fd < 0) 
	{
		printf("Could not open mtd device: %s\n", mtd_factory);
		return -1;
	}	

	if ( !strncmp(*value, "all", 3) )	
	{		
		readmask = ALL_MAC_MASK;	
	}	
	else if ( !strncmp(*value, "basic", 5) )	
	{		
		readmask = BASIC_MAC_MASK;	
	}	
	else if ( !strncmp(*value, "lan", 3) )	
	{		
		readmask = LAN_MAC_MASK;	
	}	
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	else if ( !strncmp(*value, "wan", 3) )	
	{		
		readmask = WAN_MAC_MASK;	
	}
#endif
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	else if ( !strncmp(*value, "wlan_2g", 7) )	
	{		
		readmask = W24_MAC_MASK;	
	}
	#endif
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	else if ( !strncmp(*value, "wlan_5g", 7) )	
	{		
		readmask = W58_MAC_MASK;	
	}
	#endif
#endif
	else	
	{		
		return -1;	
	}

	//read basic mac address
	lseek( fd, HUAYI_FACTORY_FIRST_RF_MAC_OFFSET, SEEK_SET );
	if (read( fd, basic_addr, MAC_ROW_LEN) != MAC_ROW_LEN)
	{
		printf("read() failed\n");
		close(fd);
		return -1;
	}
	
	memcpy(wlan_addr, basic_addr, sizeof(basic_addr));
	
	if ( readmask & BASIC_MAC_MASK )
	{
		//read basic mac address
		printf("basic_mac 		");
		for (i = 0; i < MAC_ROW_LEN; i++)
		{
			printf("%02X", basic_addr[i]);
			if (i < MAC_ROW_LEN-1)
				printf(":");
			else
				printf("\n");
		}
	}

	if ( readmask & LAN_MAC_MASK )
	{
		//read lan mac address
		printf("lan_mac 		");
		basic_addr[5] = basic_addr[5]+2;
		for (i = 0; i < MAC_ROW_LEN; i++)
		{
			printf("%02X", basic_addr[i]);
			if (i < MAC_ROW_LEN-1)
				printf(":");
			else
				printf("\n");
		}
		basic_addr[5] = basic_addr[5]-2;
	}
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	if ( readmask & WAN_MAC_MASK )
	{
		//read wan mac address
		printf("wan_mac 		");
		basic_addr[5] = basic_addr[5]+3;
		for (i = 0; i < MAC_ROW_LEN; i++)
		{
			printf("%02X", basic_addr[i]);
			if (i < MAC_ROW_LEN-1)
				printf(":");
			else
				printf("\n");
		}
		basic_addr[5] = basic_addr[5]-3;
	}
#endif
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	if ( readmask & W24_MAC_MASK )
	{
		//read 2.4G mac address
		printf("wlan_2g_mac 		");
		for (i = 0; i < MAC_ROW_LEN; i++)
		{
			printf("%02X", wlan_addr[i]);
			if (i < MAC_ROW_LEN-1)
				printf(":");
			else
				printf("\n");
		}
	}
	#endif
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if ( readmask & W58_MAC_MASK )
	{
		//read 5.8G mac address
		lseek( fd, HUAYI_FACTORY_SECOND_RF_MAC_OFFSET, SEEK_SET );
		if (read( fd, wlan_addr, MAC_ROW_LEN) != MAC_ROW_LEN)
		{
			printf("read() failed\n");
			close(fd);
			return -1;
		}
		printf("wlan_5g_mac 		");
		for (i = 0; i < MAC_ROW_LEN; i++)
		{
			printf("%02X", wlan_addr[i]);
			if (i < MAC_ROW_LEN-1)
				printf(":");
			else
				printf("\n");
		}
	}
	#endif
#endif
	close(fd);
	return 0;
}



int ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, MAC_ROW_LEN);
	
	for (;;) 
	{
		e[i++] = (unsigned char)strtoul(c, &c, 16);
		if (!*c++ || i == MAC_ROW_LEN)
			break;
	}
	
	return (i == MAC_ROW_LEN);
}

/*=========================================================================*/
/*  函数名称: isMacValid                                                   */
/*  函数功能: 判断MAC的合法性                                              */
/*  输  入  : char * MAC                                                   */
/*  输  出  : int   0:非法，1：合法                                        */
/*  创  建  : huayi                                       */
/*=========================================================================*/
int isMacValid(char *str)
{
	int i = 0; 
	int len = strlen(str);
	
	if(len != 12 && len != 17)
	{
		return 0;
	}

	if (len == 17)
	{
		for(i=0; i<5; i++)
		{
			if((!isxdigit( str[i*3])) || (!isxdigit( str[i*3+1])) || (str[i*3+2] != ':'))
				return 0;
		}
		return (isxdigit(str[15]) && isxdigit(str[16])) ? 1: 0;
	}
	else //len == 12
	{
		for (i=0; i<12; i++)
		{
			if (!isxdigit(str[i]))
				return 0;
		}
		
		return 1;
	}

}


int mtd_write( char **value)
{
	int sz = 0;
	int i = 0;
	int fd = open(mtd_factory, O_RDWR | O_SYNC);
	unsigned char *buf=NULL, *ptr=NULL;
	char add[3]="";
	unsigned char base_mac[6]={0}, mac_24g[6]={0},mac_58g[6]={0};
	
	if (!isMacValid(*value))
	{
		fprintf(stderr, "mac address format error !\n");
		return -1;
	}

	if (fd < 0) 
	{
		fprintf(stderr, "Could not open mtd device: %s\n", mtd_factory);
		return -1;
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if (HUAYI_FACTORY_SECOND_RF_MAC_OFFSET > 4096)
	{
		sz = HUAYI_FACTORY_SECOND_RF_MAC_OFFSET+6;
	}
	else
#endif
	{
		sz = 4*1024;
	}
	
	buf = (unsigned char *)malloc(sz);
	if (NULL == buf)
	{
		printf("Allocate memory for sz failed.\n");
		close(fd);
		return -1;		  
	}
	
	if (read(fd, buf, sz) != sz)
	{
		fprintf(stderr, "read() %s failed\n", mtd_factory);
		goto write_fail;
	}
	
	if (strlen(*value) == 12)
	{
		for (i=0; i<6; i++)
		{
			sscanf(*value, "%2s", add);
			base_mac[i] = strtoul(add,	NULL, 16);
			*value = *value+2;
		}
	}
	else //len == 17
	{
		ether_atoe(*value, base_mac);
	}

	memcpy(mac_24g, base_mac, sizeof(base_mac));		
	mac_24g[5] += 2;	
	if(mac_24g[5] == 0x00)
	{		
		mac_24g[4]+=1;		
		if(mac_24g[4] == 0x00)			
			mac_24g[3]+= 1;	
	}
	
	//lan mac address
	ptr = buf + LAN_MAC_OFFSET;
	for (i=0; i<MAC_ROW_LEN; i++, ptr++)
	{
		*ptr = mac_24g[i];
	}
	
	memset(mac_24g,0,sizeof(mac_24g));

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
	memcpy(mac_24g, base_mac, sizeof(base_mac));	
	memcpy(mac_58g, base_mac, sizeof(base_mac));	
	mac_58g[5] += 1;	
	if(mac_58g[5] == 0x00)
	{		
		mac_58g[4]+=1;		
		if(mac_58g[4] == 0x00)			
			mac_58g[3]+= 1;	
	}
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)

	//write 2.4G mac address
	ptr = buf + HUAYI_FACTORY_FIRST_RF_MAC_OFFSET;
	for (i=0; i<MAC_ROW_LEN; i++, ptr++)
	{
		*ptr = mac_24g[i];
	}
	#endif
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//write 5.8G mac address
	ptr = buf + HUAYI_FACTORY_SECOND_RF_MAC_OFFSET;
	for (i=0; i<MAC_ROW_LEN; i++, ptr++)
	{
		*ptr = mac_58g[i];
	}
	#endif
#endif
	lseek( fd, 0, SEEK_SET );
	if (write( fd, buf, sz ) != sz) 
	{
		fprintf(stderr, "write() %s failed\n", mtd_factory);
		goto write_fail;
	}
	
	close(fd);
	free(buf);
	
	return 0;
	
write_fail:
	close(fd);
	free(buf);
	
	return -1;
}



int main(int argc,char **argv)
{
	char op[OPTION_STR_LEN];

	if (argc < 3)
	{
		goto invald_cmd;
	}

	strcpy(op, argv[1]);
	
	snprintf(mtd_factory,OPTION_STR_LEN,"/dev/mtdblock%d",HUAYI_FACTORY_MTD_NUM);
	
	if (!strcmp(op,"read"))
	{
		if (mtd_read(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}else if (!strcmp(op,"write"))
	{
		if (mtd_write(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}
	
	printf("huayi_mac excution success\n");
	return 0;

invald_cmd:
    usage();
	return 0;
	
cmd_fail:
	printf("huayi_mac excution failed\n");
	return 0;
}

