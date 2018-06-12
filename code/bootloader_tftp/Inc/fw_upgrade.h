#ifndef FW_UPGRADE_H_
#define FW_UPGRADE_H_

typedef enum {
	NO_FW_INTERNAL_NO_FW_FS = 0,
	VALID_FW_INTERNAL_NO_FW_FS,
	FW_FS_NEWER,
	FW_INTERNAL_FS_MATCH_LATEST
} fw_status;

void FWU_run_app(void);
fw_status FWU_check_upgrade_file(char* pFileName, unsigned char unBufLen);
void FWU_upgrade(char* pFileName);

#endif


