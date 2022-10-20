
#include "huayi_netlink.h"

VTBool huayi_port_event_init(struct VcoreAgent *Agent)
{
	struct sockaddr_nl  local;
	
	Agent->huayi_port_fd = socket(AF_NETLINK, SOCK_RAW, HUAYI_PORT_NETLINK_MSG_TYPE);
    if(Agent->huayi_port_fd < 0)
    {
        printf("huayi_port_event_init:create socket error...\n");
        return VT_FALSE;
    }

	memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = HUAYI_PORT_NETLINK_PORT_ID; 
    local.nl_groups = 0;
    if(bind(Agent->huayi_port_fd, (struct sockaddr *)&local, sizeof(local)) != 0)
    {
        printf("huayi_port_event_init:bind error\n");
        close(Agent->huayi_port_fd);
        return VT_FALSE;
    }

	return VT_TRUE;
}


void huayi_port_event_receive(struct VcoreAgent *Agent)
{
	struct huayi_port_netlink_rec info;
	struct sockaddr_nl	dest_addr;
	int byteReceived = 0;
	int struct_len = sizeof(struct sockaddr_nl);
	struct huayi_port_event  *event;

	memset(&info, 0, sizeof(info)); 
	byteReceived = recvfrom(Agent->huayi_port_fd, &info,sizeof(struct huayi_port_netlink_rec), 0, (struct sockaddr*)&dest_addr, (socklen_t *)&struct_len);
    if(byteReceived < 0)
    {
	   printf("huayi_port_event_receive Receive Error\n");
	   return;
    }
	
	event = (struct huayi_port_event *)info.buff;

	huayi_port_event_process(Agent,event);

	return;
	
}


VTBool huayi_port_event_destory(struct VcoreAgent *Agent)
{
	close(Agent->huayi_port_fd);

	return VT_TRUE;
}

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
VTBool hook_gpio_init(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	#if defined(CONFIG_HUAYI_BUTTON_RESET_SUPPORT)
	snprintf(cmd, sizeof(cmd), "btnd hook %d", CONFIG_HUAYI_BUTTON_RESET_GPIO_NUM);
	system(cmd);
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_WPS_SUPPORT)
	if(CONFIG_HUAYI_BUTTON_RESET_GPIO_NUM != CONFIG_HUAYI_BUTTON_WPS_GPIO_NUM)
	{
		snprintf(cmd, sizeof(cmd), "btnd hook %d", CONFIG_HUAYI_BUTTON_WPS_GPIO_NUM);
		system(cmd);
	}
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
	snprintf(cmd, sizeof(cmd), "btnd hook %d", CONFIG_HUAYI_BUTTON_WIFI_GPIO_NUM);
	system(cmd);
	#endif
	
	return VT_TRUE;
}

VTBool huayi_gpio_event_init(struct VcoreAgent *Agent)
{
	struct sockaddr_nl  local;
	
	Agent->huayi_gpio_fd = socket(AF_NETLINK, SOCK_RAW, HUAYI_GPIO_NETLINK_MSG_TYPE);
    if(Agent->huayi_gpio_fd < 0)
    {
        printf("huayi_gpio_event_init:create socket error...\n");
        return VT_FALSE;
    }

	memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = HUAYI_GPIO_NETLINK_PORT_ID; 
    local.nl_groups = 0;
    if(bind(Agent->huayi_gpio_fd, (struct sockaddr *)&local, sizeof(local)) != 0)
    {
        printf("huayi_gpio_event_init:bind error\n");
        close(Agent->huayi_gpio_fd);
        return VT_FALSE;
    }

	return VT_TRUE;
}


void huayi_gpio_event_receive(struct VcoreAgent *Agent)
{
	struct huayi_gpio_netlink_rec info;
	struct sockaddr_nl	dest_addr;
	int byteReceived = 0;
	int struct_len = sizeof(struct sockaddr_nl);
	struct huayi_gpio_event  *event;

	memset(&info, 0, sizeof(info)); 
	byteReceived = recvfrom(Agent->huayi_gpio_fd, &info,sizeof(struct huayi_gpio_netlink_rec), 0, (struct sockaddr*)&dest_addr, (socklen_t *)&struct_len);
    if(byteReceived < 0)
    {
	   printf("huayi_gpio_event_receive Receive Error\n");
	   return;
    }
	
	event = (struct huayi_gpio_event *)info.buff;

	huayi_gpio_event_process(Agent,event);

	return;
	
}


VTBool huayi_gpio_event_destory(struct VcoreAgent *Agent)
{
	close(Agent->huayi_gpio_fd);

	return VT_TRUE;
}
#endif


