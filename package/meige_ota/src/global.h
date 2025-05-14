#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef EXTERN
#define EXTERN extern
#endif

typedef enum MODE_TYPE
{
    MODE_NULL,
    MODE_8910,
    MODE_8850,
    MODE_8310
} e_modeType;


typedef struct DOWN_INFO
{
    char        debug;
    char        erase_all;
    char        backupNV;
    e_modeType  modeType;
} t_downInfo;


EXTERN t_downInfo g_downInfo;


#endif

