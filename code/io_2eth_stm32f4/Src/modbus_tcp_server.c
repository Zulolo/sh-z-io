#include <string.h>
#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"
#include "sh_z_002.h"
#include "mb_tcp_server.h"
#include "di_monitor.h"
#include "ai_monitor.h"

#define PROG                    "FreeModbus"

#define MB_DI_CONF_ADDR			101

extern uint8_t cSN[SH_Z_SN_LEN];

static uint16_t unADCxConvertedValueBuf[SH_Z_002_AI_NUM];

static uint32_t unDI_CNT_FreqValueBuf[SH_Z_002_DI_NUM];
static uint32_t DI_ValuesBuf;
static DI_ConfTypeDef tDI_ConfBuf[SH_Z_002_DI_NUM];

static eMBErrorCode get_DI_value_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	DI_ValuesBuf = DI_get_DI_values();
	return 	MB_ENOERR;
}

static eMBErrorCode get_AI_value_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	AI_get_AI_values(unADCxConvertedValueBuf);
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_conf_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	DI_get_DI_conf(tDI_ConfBuf, usAddress - MB_DI_CONF_ADDR, usNCoils);
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_cnt_freq_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {

	return 	MB_ENOERR;
}

static eMBErrorCode set_DI_conf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	
	return 	MB_ENOERR;
}

const MB_RegAccessTypeDef SH_Z_X_MB_REG[] = {
	{1, sizeof(DI_ValuesBuf), &DI_ValuesBuf, MB_TCP_SVR_FUNC_RD_COLIS_BIT, get_DI_value_buf, NULL, NULL, NULL},	
	{MB_DI_CONF_ADDR, sizeof(tDI_ConfBuf), tDI_ConfBuf, MB_TCP_SVR_FUNC_RD_COLIS_BIT | MB_TCP_SVR_FUNC_WR_COLIS_BIT, get_DI_conf_buf, NULL, NULL, set_DI_conf},	
	{40001, sizeof(unDI_CNT_FreqValueBuf), unDI_CNT_FreqValueBuf , MB_TCP_SVR_FUNC_RD_INPUT_BIT, get_DI_cnt_freq_buf, NULL, NULL, NULL},
	{40101, sizeof(unADCxConvertedValueBuf), unADCxConvertedValueBuf , MB_TCP_SVR_FUNC_RD_INPUT_BIT, get_AI_value_buf, NULL, NULL, NULL},
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
	const MB_RegAccessTypeDef* pRegAccess;
	eMBErrorCode eErrCode;
	int i;
	
	pRegAccess = get_reg_coil_access(usAddress, usNCoils, (eMode == MB_REG_READ) ? (MB_TCP_SVR_FUNC_RD_COLIS_BIT):(MB_TCP_SVR_FUNC_WR_COLIS_BIT));
	if (pRegAccess != NULL) {
		if ((usAddress < pRegAccess->unRegAddr) || ((GET_DI_BYTE_NUM(usAddress - pRegAccess->unRegAddr + usNCoils)) > pRegAccess->unRegByteLen)) {
			return MB_ENOREG;
		}
		if (MB_REG_READ == eMode) {
			if (pRegAccess->preReadF != NULL ) {
				eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			*(uint32_t *)(pRegAccess->pRegValue) = *(uint32_t *)(pRegAccess->pRegValue) >> (usAddress - pRegAccess->unRegAddr);
			memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue), GET_DI_BYTE_NUM(usNCoils));
			
			if (pRegAccess->postReadF != NULL ) {
				eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			return MB_ENOERR;
		} else {
			if (pRegAccess->preWriteF != NULL ) {
				eErrCode = pRegAccess->preWriteF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			for (i = 0; i < usNCoils; i++) {
				if (READ_BIT(*(pucRegBuffer + i/8), (0x01 << (i%8)))) {
					SET_BIT(*((uint8_t *)(pRegAccess->pRegValue) + (i + (usAddress - pRegAccess->unRegAddr))/8), (0x01 << ((i + (usAddress - pRegAccess->unRegAddr))%8)));
				} else {
					CLEAR_BIT(*((uint8_t *)(pRegAccess->pRegValue) + (i + (usAddress - pRegAccess->unRegAddr))/8), (0x01 << ((i + (usAddress - pRegAccess->unRegAddr))%8)));
				}
			}
			
			if (pRegAccess->postWriteF != NULL ) {
				eErrCode = pRegAccess->postWriteF(pucRegBuffer, usAddress - pRegAccess->unRegAddr, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			return	MB_ENOREG;
		}
	} else {
		return	MB_EINVAL;
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

