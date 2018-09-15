#ifndef FW_UPGRADE_H_
#define FW_UPGRADE_H_

#define BOOTLOADER_FLAG				0x5AA50524
#define SH_Z_002_MAGIC_NUM		0x5AA55A4B
#define UPDATE_FILE_NAME			"sh_z_002_update.bin"

typedef enum {
	NO_FW_INTERNAL_NO_FW_FS = 0,
	VALID_FW_INTERNAL_NO_FW_FS,
	FW_FS_EXIST,
	FW_FS_NEWER,
	FW_INTERNAL_FS_MATCH_LATEST
} fw_status;

int FWU_run_app(void);
void FWU_backup_fw(void);
fw_status FWU_check_upgrade_file(char* pFileName, unsigned char unBufLen);
int FWU_upgrade(char* pFileName);
int FWU_get_fw_version_internal(void);
int FWU_get_fw_version_FS(const char* pFileName);

#endif


