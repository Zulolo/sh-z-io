#ifndef _SH_Z_002_H
#define _SH_Z_002_H

#include "sh_z.h"

#define SH_Z_002_SLAVE_ID					2
#define SH_Z_002_DI_NUM						4
#define SH_Z_002_AI_NUM						4
#define SH_Z_002_DI_PIN_OFFSET		8

#define SH_Z_002_VERSION					((uint16_t)0x0100)

#define SH_Z_002_INFO_FILE_NAME		"info.json"
#define SH_Z_002_CONF_FILE_NAME		"conf.json"

#define CONF_STATIC_IP_JSON_TAG		"static_ip"
#define CONF_IP_ADDR_JSON_TAG			"ip_addr"
#define CONF_NETMASK_JSON_TAG			"netmask"
#define CONF_GW_ADDR_JSON_TAG			"gw_addr"
#define CONF_MAC_ADDR_JSON_TAG		"mac_addr"

typedef struct {
  unsigned short bStaticIP : 1;
	unsigned short bReserved : 15;
	unsigned char uIP_Addr[4];
	unsigned char uNetmask[4];
	unsigned char uGateway[4];
	unsigned char uMAC_Addr[6];
} ETH_Conf_t;


#endif
