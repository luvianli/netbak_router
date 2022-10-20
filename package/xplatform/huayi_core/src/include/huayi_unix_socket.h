#ifndef __HUAYI_UNIX_SOCKET_HEADER__
#define __HUAYI_UNIX_SOCKET_HEADER__

#include "huayi_core_base.h"
#include "huayi_handle_reload.h"

VTBool huayi_core_ipc_init(struct VcoreAgent *Agent);
void huayi_core_receive_ipc_msg(struct VcoreAgent *Agent);
VTBool huayi_core_ipc_destory(struct VcoreAgent *Agent);


#endif

