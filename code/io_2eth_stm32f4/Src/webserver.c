#include <string.h>
#include "cmsis_os.h"
#include "lwip.h"
#include "sh_z_002.h"
#include "lwip/apps/fs.h"
#include "lwip/apps/tftp_server.h"
#include "httpd.h"
#include "spiffs.h"

extern spiffs SPI_FFS_fs;
extern osMutexId WebServerFileMutexHandle;

extern void http_server_socket_thread(void *arg);

static spiffs_file gFileDesc;

int fs_open_custom(struct fs_file *file, const char *name) {
//	spiffs_stat FileState;
//	int res;
//  
//	osMutexWait(WebServerFileMutexHandle, osWaitForever);
//	gFileDesc = SPIFFS_open(&SPI_FFS_fs, name, SPIFFS_RDONLY, 0);
//	if (gFileDesc > 0) {	
//		res = SPIFFS_fstat(&SPI_FFS_fs, gFileDesc, &FileState);
//		if (res < 0 ) {
//			return 0;
//		}
//		file->index = 0;
//		file->len = FileState.size;
//		return gFileDesc;
//	} else {
//		return 0;
//	}	
	file->index = 0;
	file->len = 100;
	printf("webserver_file_open\n");
	return 0;
}

void fs_close_custom(struct fs_file *file) {
//	SPIFFS_close(&SPI_FFS_fs, gFileDesc);
//	osMutexRelease(WebServerFileMutexHandle);
	printf("webserver_file_close\n");
}

int fs_read_custom(struct fs_file *file, char *buffer, int count) {
//	int res;
//	res = SPIFFS_read(&SPI_FFS_fs, gFileDesc, (u8_t *)buffer, count);
//	if (res >= 0) {
//		file->index += res;
//	} 
//	return res;
	
	file->index = 100;
	return 100;
}

//void httpd_cgi_handler(const char* uri, int iNumParams, char **pcParam, char **pcValue) {
//	
//}

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
	return ERR_OK;						   
}
					   
err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
	
	
	return ERR_OK;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
	

}

void* tftp_file_open(const char* fname, const char* mode, u8_t write) {
	printf("tftp_file_open\n");
	return (void*)5;
}

void tftp_file_close(void* handle) {
	printf("tftp_file_close\n");
}

int tftp_file_read(void* handle, void* buf, int bytes) {
	printf("tftp_file_read\n");
	return 0;
}

int tftp_file_write(void* handle, struct pbuf* p) {
	printf("tftp_file_write\n");
	return 0;
}

const struct tftp_context TFTP_Ctx = {.open = tftp_file_open, .close = tftp_file_close, .read = tftp_file_read, .write = tftp_file_write};

void start_webserver(void const * argument) {
	
//	httpd_init();
	http_server_socket_thread(NULL);
}

void start_tftp(void const * argument) {
	tftp_init(&TFTP_Ctx);
	while(1) {
		osDelay(500);
	}
}
