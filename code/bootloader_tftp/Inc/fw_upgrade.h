#ifndef FW_UPGRADE_H_
#define FW_UPGRADE_H_

void FWU_run_app(void);
int FWU_check_upgrade_file(char* file_name, unsigned char unBufLen);
void FWU_upgrade(char* file_name);

#endif


