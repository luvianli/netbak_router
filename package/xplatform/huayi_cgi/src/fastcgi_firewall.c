
#include "fastcgi_common.h"


#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
int get_remote_manage_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int remote_enable = 0;
	int port = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"remote","remote_enable",&remote_enable);
	
	huayi_add_switch_to_json(result_root,"remote_admin",remote_enable);

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"remote","port",&port);

	cJSON_AddNumberToObject(result_root,"port",port);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_remote_manage_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int remote_enable = 0;
	int port = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"remote_admin",&remote_enable)||
		!huayi_json_get_int(request_root,"port",&port))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"remote","remote_enable",remote_enable);

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"remote","port",port);

	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset remotemanage");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_mac_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *macfilter_array = NULL;
	cJSON *macfilter_obj = NULL;
	int  mac_filter = 0;
	char section[OPTION_STR_LEN] = {0};
	char mac_addr[MAC_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};
	int i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","mac_filter",&mac_filter);
	
	huayi_add_switch_to_json(result_root,"mac_filter",mac_filter);
	
	macfilter_array = cJSON_CreateArray();
	
	while(1)
	{
		memset(desc, 0, sizeof(desc));
		snprintf(section,sizeof(section),"@macfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"mac_addr",mac_addr))
		{
			break;
		}
		huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		macfilter_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(macfilter_obj,"mac_addr",mac_addr);
		
		cJSON_AddStringToObject(macfilter_obj,"com",desc);

		cJSON_AddItemToArray(macfilter_array,macfilter_obj);
		
		i++;
	}

	cJSON_AddItemToObject(result_root,"macfilter_arr",macfilter_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_mac_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*macfilter_array = NULL;
	cJSON 	*macfilter_obj = NULL;
	int 	size = 0;
	int 	i = 0;
	int		k = 0;
	int		max_id=0;
	char section[OPTION_STR_LEN] = {0};
	int  mac_filter = 0;
	char mac_addr[MAC_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};

	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"mac_filter",&mac_filter))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"global","mac_filter",mac_filter);
	
	if(!(macfilter_array= cJSON_GetObjectItem(request_root,"macfilter_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(macfilter_array);

	snprintf(section,sizeof(section),"@macfilter[-1]");
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"id",&max_id);	
	
	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@macfilter[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_HUAYI_FIREWALL,section);
		}
	}
	

	for(k=0;k<size;k++)
	{
		macfilter_obj = cJSON_GetArrayItem(macfilter_array,k);
		huayi_json_get_str(macfilter_obj,"mac_addr",mac_addr,MAC_STR_LEN);
		huayi_json_get_str(macfilter_obj,"com",desc,OPTION_STR_LEN);
		
		huayi_uci_add_section(PKG_HUAYI_FIREWALL,"macfilter");
		
		snprintf(section,sizeof(section),"@macfilter[%d]",k);
		
		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"mac_addr",mac_addr);
		
		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"id",k+1);
			
	}
	
	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset macfilter");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_url_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *urlfilter_array = NULL;
	cJSON *urlfilter_obj = NULL;
	int  url_filter = 0;
	char section[OPTION_STR_LEN] = {0};
	char url[OPTION_STR_LEN] = {0};
	int i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","url_filter",&url_filter);
	
	huayi_add_switch_to_json(result_root,"url_filter",url_filter);
	
	urlfilter_array = cJSON_CreateArray();
	
	while(1)
	{
		snprintf(section,sizeof(section),"@urlfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"url",url))
		{
			break;
		}
		
		urlfilter_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(urlfilter_obj,"urlfilter_url",url);
		
		
		cJSON_AddItemToArray(urlfilter_array,urlfilter_obj);
		
		i++;
	}

	cJSON_AddItemToObject(result_root,"urlfilter_arr",urlfilter_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_url_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*urlfilter_array = NULL;
	cJSON 	*urlfilter_obj = NULL;
	int 	size = 0;
	int 	i = 0;
	int		k = 0;
	int		max_id=0;
	char section[OPTION_STR_LEN] = {0};
	int  url_filter = 0;
	char url[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"url_filter",&url_filter))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"global","url_filter",url_filter);
	
	if(!(urlfilter_array= cJSON_GetObjectItem(request_root,"urlfilter_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(urlfilter_array);

	snprintf(section,sizeof(section),"@urlfilter[-1]");
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"id",&max_id);	
	
	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@urlfilter[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_HUAYI_FIREWALL,section);
		}
	}
	

	for(k=0;k<size;k++)
	{
		urlfilter_obj = cJSON_GetArrayItem(urlfilter_array,k);
		
		huayi_json_get_str(urlfilter_obj,"urlfilter_url",url,OPTION_STR_LEN);
		if(strlen(url) > 0)
		{
			huayi_uci_add_section(PKG_HUAYI_FIREWALL,"urlfilter");
		
			snprintf(section,sizeof(section),"@urlfilter[%d]",k);
		
			huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"url",url);
		}
		
		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"id",k+1);
			
	}
	
	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset urlfilter");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_ip_port_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *ipportfilter_array = NULL;
	cJSON *ipportfilter_obj = NULL;
	int  ip_port_filter = 0;
	char section[OPTION_STR_LEN] = {0};
	char src_ip[IPV4_STR_LEN] = {0};
	int	 proto				= 0;
	int  port_start	= 0;
	int  port_end	= 0;
	char desc[OPTION_STR_LEN] = {0};
	int i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","ip_port_filter",&ip_port_filter);
	
	cJSON_AddNumberToObject(result_root,"ip_port_filter",ip_port_filter);
	
	ipportfilter_array = cJSON_CreateArray();
	
	while(1)
	{
		memset(desc, 0, sizeof(desc));
		
		snprintf(section,sizeof(section),"@ipportfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"src_ip",src_ip))
		{
			break;
		}

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"proto",&proto);

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"port_start",&port_start);

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"port_end",&port_end);

		huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		ipportfilter_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(ipportfilter_obj,"src_addr",src_ip);

		cJSON_AddNumberToObject(ipportfilter_obj,"proto",proto);

		cJSON_AddNumberToObject(ipportfilter_obj,"start_port",port_start);

		cJSON_AddNumberToObject(ipportfilter_obj,"end_port",port_end);
		
		cJSON_AddStringToObject(ipportfilter_obj,"com",desc);
		
		cJSON_AddItemToArray(ipportfilter_array,ipportfilter_obj);
		
		i++;
	}

	cJSON_AddItemToObject(result_root,"ipportfilter_arr",ipportfilter_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_ip_port_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *ipportfilter_array = NULL;
	cJSON *ipportfilter_obj = NULL;
	int 	size = 0;
	int 	i = 0;
	int		k = 0;
	int		max_id=0;
	char section[OPTION_STR_LEN] = {0};
	int  ip_port_filter = 0;
	char src_ip[IPV4_STR_LEN] = {0};
	int	 proto				= 0;
	int  port_start	= 0;
	int  port_end	= 0;
	char desc[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"ip_port_filter",&ip_port_filter))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"global","ip_port_filter",ip_port_filter);
	
	if(!(ipportfilter_array= cJSON_GetObjectItem(request_root,"ipportfilter_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(ipportfilter_array);

	snprintf(section,sizeof(section),"@ipportfilter[-1]");
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"id",&max_id);	
	
	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@ipportfilter[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_HUAYI_FIREWALL,section);
		}
	}
	

	for(k=0;k<size;k++)
	{
		ipportfilter_obj = cJSON_GetArrayItem(ipportfilter_array,k);

		huayi_json_get_str(ipportfilter_obj,"src_addr",src_ip,IPV4_STR_LEN);

		huayi_json_get_int(ipportfilter_obj,"proto",&proto);

		huayi_json_get_int(ipportfilter_obj,"start_port",&port_start);

		huayi_json_get_int(ipportfilter_obj,"end_port",&port_end);

		huayi_json_get_str(ipportfilter_obj,"com",desc,OPTION_STR_LEN);

		huayi_uci_add_section(PKG_HUAYI_FIREWALL,"ipportfilter");
		
		snprintf(section,sizeof(section),"@ipportfilter[%d]",k);
		
		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"src_ip",src_ip);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"proto",proto);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"port_start",port_start);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"port_end",port_end);

		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"id",k+1);
			
	}
	
	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset ipportfilter");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_dmz_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int dmz_enable = 0;
	char dest_ip[IPV4_STR_LEN] = {0};
	
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"dmz","dmz_enable",&dmz_enable);
	
	huayi_add_switch_to_json(result_root,"dmz_admin",dmz_enable);

	huayi_uci_get_str(PKG_HUAYI_FIREWALL,"dmz","dest_ip",dest_ip);

	cJSON_AddStringToObject(result_root,"dest_addr",dest_ip);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_dmz_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int dmz_enable = 0;
	char dest_ip[IPV4_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"dmz_admin",&dmz_enable)||
		!huayi_json_get_str(request_root,"dest_addr",dest_ip,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"dmz","dmz_enable",dmz_enable);

	huayi_uci_set_str(PKG_HUAYI_FIREWALL,"dmz","dest_ip",dest_ip);

	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset dmz");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_port_forward_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *port_forward_array = NULL;
	cJSON *port_forward_obj = NULL;
	int  port_forward = 0;
	char section[OPTION_STR_LEN] = {0};
	char ip_addr[IPV4_STR_LEN] = {0};
	int	 proto = 0;
	int  internal_port=0;
	int  external_port=0;
	char desc[OPTION_STR_LEN] = {0};
	int i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","port_forward",&port_forward);
	
	huayi_add_switch_to_json(result_root,"port_admin",port_forward);
	
	port_forward_array = cJSON_CreateArray();
	
	while(1)
	{
		memset(desc, 0, sizeof(desc));
		
		snprintf(section,sizeof(section),"@port_forward[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"ip_addr",ip_addr))
		{
			break;
		}

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"proto",&proto);

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"internal_port",&internal_port);

		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"external_port",&external_port);

		huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		port_forward_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(port_forward_obj,"ip_addr",ip_addr);

		cJSON_AddNumberToObject(port_forward_obj,"proto",proto);

		cJSON_AddNumberToObject(port_forward_obj,"in_port",internal_port);

		cJSON_AddNumberToObject(port_forward_obj,"ex_port",external_port);
		
		cJSON_AddStringToObject(port_forward_obj,"com",desc);
		
		cJSON_AddItemToArray(port_forward_array,port_forward_obj);
		
		i++;
	}

	cJSON_AddItemToObject(result_root,"port_forward_arr",port_forward_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_port_forward_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *port_forward_array = NULL;
	cJSON *port_forward_obj = NULL;
	int 	size = 0;
	int 	i = 0;
	int		k = 0;
	int		max_id=0;
	char section[OPTION_STR_LEN] = {0};
	char ip_addr[IPV4_STR_LEN] = {0};
	int	 proto = 0;
	int  internal_port=0;
	int  external_port=0;
	char desc[OPTION_STR_LEN] = {0};
	int  port_forward = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"port_admin",&port_forward))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"global","port_forward",port_forward);
	
	if(!(port_forward_array= cJSON_GetObjectItem(request_root,"port_forward_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(port_forward_array);

	snprintf(section,sizeof(section),"@port_forward[-1]");
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"id",&max_id);	
	
	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@port_forward[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_HUAYI_FIREWALL,section);
		}
	}
	

	for(k=0;k<size;k++)
	{
		port_forward_obj = cJSON_GetArrayItem(port_forward_array,k);

		huayi_json_get_str(port_forward_obj,"ip_addr",ip_addr,IPV4_STR_LEN);
		
		huayi_json_get_int(port_forward_obj,"proto",&proto);

		huayi_json_get_int(port_forward_obj,"in_port",&internal_port);

		huayi_json_get_int(port_forward_obj,"ex_port",&external_port);

		huayi_json_get_str(port_forward_obj,"com",desc,OPTION_STR_LEN);

		huayi_uci_add_section(PKG_HUAYI_FIREWALL,"port_forward");
		
		snprintf(section,sizeof(section),"@port_forward[%d]",k);
		
		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"ip_addr",ip_addr);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"proto",proto);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"internal_port",internal_port);

		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"external_port",external_port);

		huayi_uci_set_str(PKG_HUAYI_FIREWALL,section,"desc",desc);
		
		huayi_uci_set_int(PKG_HUAYI_FIREWALL,section,"id",k+1);
			
	}
	
	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset portforward");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_upnpd_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *upnp_array = NULL;
	cJSON *upnp_obj = NULL;
	int  enable_upnp = 0;
	FILE *fp;
	char buf[TEMP_STR_LEN] = {0};
	char *p = NULL;
	char proto[SHORT_STR_LEN]={0}, cliIp[IPV4_STR_LEN] = { 0 }, name[TEMP_STR_LEN] = {0};
	int eport, iport, j = 1;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_UPNPD,"config","enable_upnp",&enable_upnp);
	
	huayi_add_switch_to_json(result_root,"upnp_admin",enable_upnp);
	
	upnp_array = cJSON_CreateArray();

	fp = fopen("/tmp/upnp.leases", "r");
	
	if(fp == NULL) 
	{
		goto complete;
	}
	
	while(fgets(buf, sizeof(buf), fp)) 
	{		
		p = NULL;		
		p = strtok(buf, ":");		
		if (p) 
		{			
			strcpy(proto, p);			
			while ((p = strtok(NULL,":")) != NULL) 
			{				
				switch (j) 
				{					
					case 1:						
						eport = atoi(p);						
						j += 1;					
						break;					
					case 2:						
						strcpy(cliIp, p);						
						j += 1;					
						break;					
					case 3:						
						iport = atoi(p);		
						j += 1;			
						break;				
					case 4:					
						//unused = atoi(p);			
						j += 1;					
						break;		
					case 5:					
						strcpy(name, p);	
						j += 1;					
						break;					
					default :						
						break;				
				}		
			}		
		}
		j = 1;
		
		upnp_obj = cJSON_CreateObject();
		
		cJSON_AddStringToObject(upnp_obj, "proto", proto);
		cJSON_AddStringToObject(upnp_obj, "name", name);
		cJSON_AddStringToObject(upnp_obj, "ip_addr", cliIp);	
		cJSON_AddNumberToObject(upnp_obj, "in_port",iport);
		cJSON_AddNumberToObject(upnp_obj, "ex_port", eport);
		cJSON_AddItemToArray(upnp_array, upnp_obj);		
	}	

	fclose(fp);

complete:	
	cJSON_AddItemToObject(result_root,"upnp_arr",upnp_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_upnpd_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int  enable_upnp = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"upnp_admin",&enable_upnp))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_UPNPD,"config","enable_upnp",enable_upnp);
	

	huayi_uci_commit(PKG_UPNPD);

	system("huayi-reset upnpd");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wan_ping_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int wanping_en = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"wanping","wanping_en",&wanping_en);
	
	huayi_add_switch_to_json(result_root,"wanping_admin",wanping_en);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}
	
	
int set_wan_ping_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int wanping_en = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wanping_admin",&wanping_en))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"wanping","wanping_en",wanping_en);

	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset wanping");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	cJSON *acl_array =NULL;
	cJSON *acl_obj = 0;
	int acl_count = 0;
	char acl_list[LONG_BUFF_LEN] = {0};
	char mac_address[MAC_STR_LEN] = {0};
	
	int  i = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);

	acl_array = cJSON_CreateArray();

	if(acl_count > 0)
	{
		huayi_uci_get_str(PKG_HUAYI_FIREWALL,"acl","acl",acl_list);
		while(huayi_get_nth_value(i++, acl_list, WEB_SPACE_DELIMIT, mac_address, sizeof(mac_address)))
		{
			acl_obj =  cJSON_CreateObject();
			cJSON_AddStringToObject(acl_obj,"acl_addr",mac_address);
			cJSON_AddItemToArray(acl_array,acl_obj);
		}
	}
	
	cJSON_AddItemToObject(result_root,"acl_arr",acl_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int add_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int acl_count = 0;
	char mac_address[MAC_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"acl_addr",mac_address,MAC_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);

	if(acl_count > 50)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(acl_count > 0)
	{
		if(is_mac_in_acl(mac_address))
		{
			return FCGI_RESP_CODE_DULICATE_DATA;
		}
	}

	huayi_uci_add_list(PKG_HUAYI_FIREWALL,"acl","acl",mac_address);

	acl_count++;
	
	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"acl","acl_count",acl_count);

	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset acl");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int delete_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int acl_count = 0;
	char mac_address[MAC_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"acl_addr",mac_address,MAC_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);

	if(acl_count > 50) 
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(acl_count <= 0) 
	{
		return FCGI_RESP_CODE_DATA_CONTENT_ERROR;
	}

	if(!is_mac_in_acl(mac_address)) 
	{
		return FCGI_RESP_CODE_DATA_CONTENT_ERROR;
	}
	
	huayi_uci_del_list(PKG_HUAYI_FIREWALL,"acl","acl",mac_address);

	acl_count--;
	
	huayi_uci_set_int(PKG_HUAYI_FIREWALL,"acl","acl_count",acl_count);

	huayi_uci_commit(PKG_HUAYI_FIREWALL);

	system("huayi-reset acl");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_qos_iplimit_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *qos_iplimit_array = NULL;
	cJSON *qos_iplimit_obj = NULL;
	char section[OPTION_STR_LEN] = {0};
	char start_ip[IPV4_STR_LEN] = {0};	
	char end_ip[IPV4_STR_LEN] = {0};	
	char desc[OPTION_STR_LEN] = {0};
	int enable = 0;
	int up_bw = 0,down_bw = 0;
	int i=0;
	int upSpeed=0,dwSpeed=0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&enable);
	
	cJSON_AddNumberToObject(result_root,"qos_admin",enable);

	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&upSpeed);
	huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&dwSpeed);

	upSpeed = upSpeed / 1024;
	dwSpeed = dwSpeed / 1024;

	cJSON_AddNumberToObject(result_root,"totalUpBw",upSpeed);
	cJSON_AddNumberToObject(result_root,"totalDwBw",dwSpeed);
	
	qos_iplimit_array = cJSON_CreateArray();
	
	while(1)
	{
		snprintf(section,sizeof(section),"@iplimit[%d]",i);
		if(!huayi_uci_get_str(PKG_QOS,section,"start_ip",start_ip))
		{
			break;
		}

		huayi_uci_get_str(PKG_QOS,section,"end_ip",end_ip);

		huayi_uci_get_int(PKG_QOS,section,"up_bw",&up_bw);

		huayi_uci_get_int(PKG_QOS,section,"down_bw",&down_bw);

		memset(desc,0,sizeof(desc));
		huayi_uci_get_str(PKG_QOS,section,"desc",desc);
		
		qos_iplimit_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(qos_iplimit_obj,"ip_start",start_ip);

		cJSON_AddStringToObject(qos_iplimit_obj,"ip_end",end_ip);

		up_bw = up_bw / 1024;
		down_bw = down_bw / 1024;
		
		cJSON_AddNumberToObject(qos_iplimit_obj,"upBandwidth",up_bw);

		cJSON_AddNumberToObject(qos_iplimit_obj,"dwBandwidth",down_bw);
		
		cJSON_AddStringToObject(qos_iplimit_obj,"com",desc);
		
		cJSON_AddItemToArray(qos_iplimit_array,qos_iplimit_obj);
		
		i++;
	}

	cJSON_AddItemToObject(result_root,"iplimit_arr",qos_iplimit_array);

	if(HUAYI_TRUE == huayi_uci_get_int(PKG_HWNAT,"global","qos_admin",&enable))
	{
		cJSON_AddNumberToObject(result_root,"can_hwnat",1);
	}
	else
	{
		cJSON_AddNumberToObject(result_root,"can_hwnat",0);
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}	

int set_qos_iplimit_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *iplimit_array = NULL;
	cJSON *iplimit_obj = NULL;
	char section[OPTION_STR_LEN] = {0};
	char start_ip[IPV4_STR_LEN] = {0};	
	char end_ip[IPV4_STR_LEN] = {0};	
	char desc[OPTION_STR_LEN] = {0};
	int up_bw = 0,down_bw = 0;
	int size = 0,i = 0,enable = 0, hw_enabled = 0,pre_hw =0;
	int max_id=0;
	int	k = 0;
	int upSpeed = 0, dwSpeed = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"qos_admin",&enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"totalUpBw",&upSpeed))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"totalDwBw",&dwSpeed))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	upSpeed = upSpeed * 1024;
	dwSpeed = dwSpeed * 1024;

	huayi_uci_set_int(PKG_QOS,"qos","qos_mode",enable);
	huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",upSpeed);
	huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",dwSpeed);
	
	if(!(iplimit_array= cJSON_GetObjectItem(request_root,"iplimit_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	size = cJSON_GetArraySize(iplimit_array);

	snprintf(section,sizeof(section),"@iplimit[-1]");
	huayi_uci_get_int(PKG_QOS,section,"id",&max_id);
	
	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@iplimit[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_QOS,section);
		}
	}

	for(k=0;k<size;k++)
	{
		iplimit_obj = cJSON_GetArrayItem(iplimit_array,k);

		huayi_json_get_str(iplimit_obj,"ip_start",start_ip,IPV4_STR_LEN);

		huayi_json_get_str(iplimit_obj,"ip_end",end_ip,IPV4_STR_LEN);

		huayi_json_get_int(iplimit_obj,"upBandwidth",&up_bw);

		huayi_json_get_int(iplimit_obj,"dwBandwidth",&down_bw);

		huayi_json_get_str(iplimit_obj,"com",desc,OPTION_STR_LEN);

		huayi_uci_add_section(PKG_QOS,"iplimit");
		
		snprintf(section,sizeof(section),"@iplimit[%d]",k);
		
		huayi_uci_set_str(PKG_QOS,section,"start_ip",start_ip);

		huayi_uci_set_str(PKG_QOS,section,"end_ip",end_ip);

		up_bw = up_bw * 1024;
		
		down_bw = down_bw * 1024;
		
		huayi_uci_set_int(PKG_QOS,section,"up_bw",up_bw);

		huayi_uci_set_int(PKG_QOS,section,"down_bw",down_bw);

		huayi_uci_set_str(PKG_QOS,section,"desc",desc);

		huayi_uci_set_int(PKG_QOS,section,"id",k+1);
	}

	huayi_uci_commit(PKG_QOS);
	
	system("huayi-reset iplimit");

	if(HUAYI_TRUE == huayi_uci_get_int(PKG_HWNAT,"global","enabled",&hw_enabled))
	{
		if(1 == enable)
		{
			huayi_uci_set_int(PKG_HWNAT,"global","pre_hw",hw_enabled);
			if(1 == hw_enabled)
			{
				huayi_uci_set_int(PKG_HWNAT,"global","enabled",0);
				huayi_uci_commit(PKG_HWNAT);
				system("huayi-reset hwnat");
			}
		}else
		{
			huayi_uci_get_int(PKG_HWNAT,"global","pre_hw",&pre_hw);
			huayi_uci_set_int(PKG_HWNAT,"global","enabled",pre_hw);
			huayi_uci_commit(PKG_HWNAT);
			system("huayi-reset hwnat");
		}
	}	
	
	return FCGI_RESP_CODE_SUCCESS;
}

#endif

