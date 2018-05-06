#ifndef _MB_TCP_SERVER_H
#define _MB_TCP_SERVER_H

#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"

#define SH_Z_002_SLAVE_DSCRPT		""

typedef struct {
	uint16_t unRegAddr;
	uint16_t unRegLen;
	void* pRegValue;
	eMBErrorCode (*preReadF) (UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs);
	eMBErrorCode (*postReadF) (UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs);
	eMBErrorCode (*preWriteF) (UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs);
	eMBErrorCode (*postWriteF) (UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs);
} MB_RegAccessTypeDef;

#endif
