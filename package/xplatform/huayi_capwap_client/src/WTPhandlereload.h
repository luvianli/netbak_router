#ifndef _WTPHANDLE_RELOAD_HEADER_
#define _WTPHANDLE_RELOAD_HEADER_

#include "WTPcommon.h"

#define FIRMWARE_NAME "/tmp/firmware.out"
#define FIRMWARE_NAME_BIN "/tmp/firmware.bin"


void start_firmware_update(struct VcapwapAgent* Agent);
void CWExecuteReload(void * arg);
void WTPExecuteNeedReloadWireless(void);
void WTPDoReset(struct VcapwapAgent* Agent);
void WTPDoReboot(struct VcapwapAgent* Agent);
void WTPDoLedOn(struct VcapwapAgent* Agent);
void WTPDoLedOff(struct VcapwapAgent* Agent);


#endif
