
#include "huayi_uhello.h"



int main(void)
{
	printf("this package is a simple for xplatform!!!!\n");
	
	printf("line=%d,%d!!!!\n",__LINE__,CONFIG_HUAYI_PLATFORM_SUPPORT);
	
	printf("line=%d,%d!!!!\n",__LINE__,HUAYI_ETH_PORT_NUM);
	
	//printf("line=%d,%d!!!!\n",__LINE__,CONFIG_PACKAGE_HUAYI_DDNS);

	//printf("line=%d,%d!!!!\n",__LINE__,CONFIG_PACKAGE_HUAYI_DHCP_DETECT);

	//printf("line=%d,%d!!!!\n",__LINE__,CONFIG_PACKAGE_HUAYI_PROTO_DETECT);

	//printf("line=%d,%d!!!!\n",__LINE__,CONFIG_PACKAGE_HUAYI_PROTO_DETECT);
	
	return 0;
}

