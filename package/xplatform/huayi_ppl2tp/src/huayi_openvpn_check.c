#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>

int main(int argc,char *argv[])
{
	int enable = 0;
	while(1)
	{
		sleep(10);
		int enable = 0;
		huayi_uci_get_int(PKG_PPL2TP,"openvpn","enable",&enable);

		if(!enable)
		{
			continue;
		}
		if (internet_do_ping_detect(INTERNET_DOMAIN, NULL) == 1)
		{
			system("echo nameserver 8.8.8.8 > /tmp/resolv.conf.auto");
			system("echo nameserver 8.8.4.4 >> /tmp/resolv.conf.auto");
		}else
		{
			system("echo nameserver 8.8.8.8 > /tmp/resolv.conf.auto");
		}
	}

	return 0;

}
