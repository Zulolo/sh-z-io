#ifndef _DI_MONITOR_H
#define _DI_MONITOR_H

#include "cmsis_os.h"
#include "sh_z_002.h"

typedef struct {
	uint8_t bDI_Value[SH_Z_002_DI_NUM];
} DI_Reg_ValueTypeDef;

#endif