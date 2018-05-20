#include <string.h>
#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"
#include "sh_z_002.h"
#include "mb_tcp_server.h"
#include "di_monitor.h"

#define PROG                    "FreeModbus"

extern uint8_t cSN[SH_Z_SN_LEN];
extern struct netif gnetif;
extern __IO uint16_t unADCxConvertedValue[4];
extern osTimerId GARP_TimerHandle;

static uint32_t DI_ValuesBuf;

void send_GARP(void const * argument) {
	etharp_gratuitous(&gnetif);
}

static eMBErrorCode get_DI_value( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_ValuesBuf = DI_get_DI_values();
	return 	MB_ENOERR;
}
const MB_RegAccessTypeDef SH_Z_X_MB_REG[] = {
	{100, sizeof(DI_ValuesBuf), &DI_ValuesBuf, MB_TCP_SVR_FUNC_RD_COLIS_BIT, get_DI_value, NULL, NULL, NULL},
	{200, sizeof(unADCxConvertedValue), unADCxConvertedValue , MB_TCP_SVR_FUNC_RD_INPUT_BIT, NULL, NULL, NULL, NULL},
	{0, 0, NULL, 0, NULL, NULL, NULL, NULL}
};

static const MB_RegAccessTypeDef* get_reg_coil_access(USHORT usAddress, USHORT usNCoils, MB_TCP_ServerFuncBit eFunc) {
	const MB_RegAccessTypeDef* pRegAccess = SH_Z_X_MB_REG;
	USHORT usNBytes = GET_DI_BYTE_NUM(usNCoils);
	while (pRegAccess->unRegByteLen != 0) {
		if ((usAddress >= pRegAccess->unRegAddr) && 
			(usAddress < (pRegAccess->unRegAddr + pRegAccess->unRegByteLen)) && 
			((usAddress + usNBytes) <= (pRegAccess->unRegAddr + pRegAccess->unRegByteLen))) {
			if ((pRegAccess->unMB_RegFuncEnBits & eFunc) != 0) {
				return pRegAccess;
			} else {
				return NULL;
			}		
		}
		pRegAccess++;
	}
	return NULL;
}

static const MB_RegAccessTypeDef* get_reg_access(USHORT usAddress, USHORT usNRegs, MB_TCP_ServerFuncBit eFunc) {
	const MB_RegAccessTypeDef* pRegAccess = SH_Z_X_MB_REG;
	while (pRegAccess->unRegByteLen != 0) {
		if ((usAddress >= pRegAccess->unRegAddr) && 
			(usAddress < (pRegAccess->unRegAddr + pRegAccess->unRegByteLen)) && 
			((usAddress + usNRegs * 2) <= (pRegAccess->unRegAddr + pRegAccess->unRegByteLen))) {
			if ((pRegAccess->unMB_RegFuncEnBits & eFunc) != 0) {
				return pRegAccess;
			} else {
				return NULL;
			}		
		}
		pRegAccess++;
	}
	return NULL;
}


eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	const MB_RegAccessTypeDef* pRegAccess = get_reg_access(usAddress, usNRegs, MB_TCP_SVR_FUNC_RD_INPUT_BIT);
	eMBErrorCode eErrCode;
	
	if (pRegAccess != NULL) {
		if (pRegAccess->preReadF != NULL ) {
			eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, usNRegs * 2);
		
		if (pRegAccess->postReadF != NULL ) {
			eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		return MB_ENOERR;
	} else {
		return MB_ENOREG;
	}
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode ) {
	const MB_RegAccessTypeDef* pRegAccess = get_reg_coil_access(usAddress, usNCoils, MB_TCP_SVR_FUNC_RD_COLIS_BIT);
	eMBErrorCode eErrCode;
	if (pRegAccess != NULL) {
		if (pRegAccess->preReadF != NULL ) {
			eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, usNCoils);
		
		if (pRegAccess->postReadF != NULL ) {
			eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		return MB_ENOERR;
	} else {
		return	MB_ENOREG;
	}
}

eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode ) {
	const MB_RegAccessTypeDef* pRegAccess = 
		get_reg_access(usAddress, usNRegs, (eMode == MB_REG_READ) ? (MB_TCP_SVR_FUNC_RD_HOLDING_BIT):(MB_TCP_SVR_FUNC_WR_HOLDING_BIT));
	eMBErrorCode eErrCode;
	
	if (pRegAccess != NULL) {
		if (MB_REG_READ == eMode) {
			if (pRegAccess->preReadF != NULL ) {
				eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, usNRegs * 2);
			
			if (pRegAccess->postReadF != NULL ) {
				eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}					
		} else {
			if (pRegAccess->preWriteF != NULL ) {
				eErrCode = pRegAccess->preWriteF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			memcpy((uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, pucRegBuffer, usNRegs * 2);
			
			if (pRegAccess->postWriteF != NULL ) {
				eErrCode = pRegAccess->postWriteF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}			
		}
		return MB_ENOERR;
	} else {
		return MB_ENOREG;
	}
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete ) {
		
	return 	MB_EILLSTATE;
}			   
							   
void start_modbus_tcp_server(void const * argument) {
	eMBErrorCode    xStatus;
	eMBSetSlaveID(SH_Z_002_SLAVE_ID, TRUE, cSN, SH_Z_SN_LEN);

	osTimerStart(GARP_TimerHandle, 5000);
	
	if( eMBTCPInit( MB_TCP_PORT_USE_DEFAULT ) != MB_ENOERR ) {
		printf( "%s: can't initialize modbus stack!\r\n", PROG );
	} else if( eMBEnable(  ) != MB_ENOERR ) {
		printf( "%s: can't enable modbus stack!\r\n", PROG );
	} else {
		do {
			xStatus = eMBPoll(  );
		}
		while( xStatus == MB_ENOERR );
	}
	/* An error occured. Maybe we can restart. */
	( void )eMBDisable(  );
	( void )eMBClose(  );			

}

