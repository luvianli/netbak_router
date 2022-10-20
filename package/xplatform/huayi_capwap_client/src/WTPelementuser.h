#ifndef _WTPELEMENTUSER_HEADER_
#define _WTPELEMENTUSER_HEADER_

#include "WTPcommon.h"
#include "CWprotocol.h"

WTPBool CWWTPGetBoardData(CWWTPVendorInfos *valPtr, struct VcapwapAgent* Agent);
WTPBool CWWTPGetVendorInfos(CWWTPVendorInfos *valPtr, struct VcapwapAgent* Agent) ;

unsigned char CWWTPGetMaxRadios(void);
unsigned char CWWTPGetRadiosInUseNum(void);
int CWWTPGetEncCapabilities(void); 





#endif
