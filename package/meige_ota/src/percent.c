#include "std_c.h"

extern uint32_t chartoint(char* date)
{
    uint32_t temp=0;
    temp=(date[0] & 0xff)|((date[1] & 0xff)<<8)|((date[2] & 0xff)<<16)|((date[3] & 0xff)<<24);
    return temp;
}

extern uint16_t chartoshort(char* date)
{
    uint16_t temp=0;
    temp=(date[0] & 0xff)|((date[1] & 0xff)<<8);
    return temp;
}

extern void inttochar(char* str,uint32_t iddr)
{
    str[0] =(char)(iddr & 0xff);
    str[1] =(char)((iddr >> 8)  & 0xff);
    str[2] =(char)((iddr >> 16) & 0xff);
    str[3] =(char)((iddr >> 24) & 0xff);
}

extern void shorttochar(char* str,uint16_t iddr)
{
    str[0] =(char)(iddr & 0xff);
    str[1] =(char)((iddr >> 8)  & 0xff);
}

