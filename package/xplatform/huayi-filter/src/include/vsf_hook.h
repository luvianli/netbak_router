/*
**  File:    vsf_hook.h
**  Author:  rex.tian
**  Date:    23/2/2018
**
**  Purpose:
**    	Behaviour manager module.
**
*/

#ifndef VSF_HOOK_H
#define VSF_HOOK_H
#include "qos_def.h"
#include "vsf_config.h"
//#include "flow_analyze.h"


#define CLASS_NAME_LEN		32

struct vsf_class_ops {
	char    name[CLASS_NAME_LEN];						
	void	*data;
};


#endif /* VSF_HOOK_H */
