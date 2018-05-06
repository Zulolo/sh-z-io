
#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"
#include "sh_z_002.h"
#include "mb_tcp_server.h"
#include "di_monitor.h"

#define PROG                    "FreeModbus"

extern uint8_t cSN[SH_Z_SN_LEN];

static DI_Reg_ValueTypeDef DI_ValuesBuf;

static eMBErrorCode get_DI_value( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs );

const MB_RegAccessTypeDef SH_Z_X_MB_REG[] = {
	{100, sizeof(DI_Reg_ValueTypeDef), &DI_ValuesBuf, get_DI_value, NULL, NULL, NULL},
};

static eMBErrorCode get_DI_value( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete ) {
		
	return 	MB_ENOERR;
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

