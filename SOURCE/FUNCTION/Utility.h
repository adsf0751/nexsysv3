/* 
 * File:   Utility.h
 * Author: RussellBai
 *
 * Created on 2019年4月23日, 下午 4:40
 */

#define MAX_TOTAL_LOG_SIZE (1024 * 1024 * 10)  // 10 MB
#define _AP_PUB_LOG_PATH_ "/home/ap/pub/sysfinlog/"
#define _AP_PUB_LOG_DIR_NAME_	"sysfinlog"

int inUtility_OpenTraceLogFile(void);		//2019/3/12 下午 4:42
int inUtility_CloseTraceLogFile(void);		//2019/3/12 下午 4:42
int inUtility_StoreTraceLog(char *szMsg); 	//2019.02.11 added by Hachi
int inUtility_ClearFile (void);			//2019/2/26 上午 9:03
int inUtility_StoreTraceLog_OneStep(char *szMsg, ...);	/* 2019/4/8 下午 5:20 added by Russell */
void vdUtility_SYSFIN_OpenLogFile(void);
void vdUtility_SYSFIN_CloseLogFile(void);
void vdUtility_SYSFIN_LogMessage(char* szLocation, const char *format, ...);
void vdUtility_SYSFIN_ManageLogFileSize(void);
int inUtility_SYSFIN_GetTotalLogSize(void);
void vdUtility_SYSFIN_DeleteOldestLogFile(void);
void vdUtility_SYSFIN_get_log_filename(char *szFilename, size_t size);
void vdUtility_SYSFIN_check_log_date(void);