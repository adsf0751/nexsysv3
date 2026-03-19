#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>			//2019.02.15 added by Hachi
#include <unistd.h>
#include <ctosapi.h>
#include <linux/stddef.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include "../INCLUDES/Define_1.h"
#include "../INCLUDES/Transaction.h"
#include "../DISPLAY/Display.h"
#include "File.h"
#include "Function.h"
#include "Utility.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))

extern	int	ginDebug;
extern	int	ginMachineType;
extern	unsigned long	gulTotalROMSize;
int		ginLogHandle;//2019/3/11 下午 5:53

char            gszCurrentLogFilename[256] = {0};
int             gpLog_fd = -1; // 檔案描述符
unsigned char	guszLogDirCheckBit = VS_FALSE;

/*  Added by Hachi(Nick)
Function        :inBATCH_CloseTraceLog
Date&Time       :2019/3/11 下午 3:51
Describe        :將log檔案開啟
*/
int inUtility_OpenTraceLogFile(void)
{
	char	szTempBuf[100+1] = {0};
    
	strcpy(szTempBuf, _FS_DATA_PATH_);
	strcat(szTempBuf, _EDC_TRACE_LOG_FILE_NAME_);
	ginLogHandle = open(szTempBuf, O_RDWR);
           
	if (ginLogHandle <= 0)
	{
		return (VS_ERROR);
	}
	else
	{
		inFile_Open_File_Cnt_Increase();
		
		return (VS_SUCCESS);
	}
	
	/*Debug Message*/
	if (ginDebug == VS_TRUE)
	{
	    inLogPrintf(AT, "ginLogHandle is %d ,func = %s, Line = %d", ginLogHandle, __FUNCTION__, __LINE__);    
	}
}

/*  Added by Hachi(Nick)
Function        :inBATCH_CloseTraceLog
Date&Time       :2019/3/11 下午 3:51
Describe        :將log檔案關閉
*/
int inUtility_CloseTraceLogFile(void)
{
	int	inHandle = 0;

	inHandle = close(ginLogHandle);

	if (inHandle == -1)
	{
	    ginLogHandle = 0;
	    return (VS_ERROR);
	}
	else
	{
	    ginLogHandle = 0;
	    inFile_Open_File_Cnt_Decrease();
	    
	    return (VS_SUCCESS);
	}

	if (ginDebug == VS_TRUE)
	{       
	    inLogPrintf(AT, "inHandle is %d ,func = %s, Line = %d", inHandle, __FUNCTION__, __LINE__);
	}

}


/*  Added by Hachi(Nick)
Function        :inBATCH_StoreTraceLog
Date&Time       :2019/2/15 上午 11:36
Describe        :將錯誤log資訊記錄到檔案裡
*/

int inUtility_StoreTraceLog(char *szMsg)
{
        int		inWriteSizes = 0, i = 0;
        long		lnOldSize = 0;
        char		szTempBuf[100+1] = {0}; 
        char		szTempWrite[3000+1] = "";
        char		szTempOut[3000+1] = "";
        unsigned long	ulHandle;
        RTC_NEXSYS	get_time;
	
        strcpy(szTempBuf, _FS_DATA_PATH_);
        strcat(szTempBuf, _EDC_TRACE_LOG_FILE_NAME_);
	
        lnOldSize = lnFILE_GetSize(&ulHandle, ( unsigned char *)_EDC_TRACE_LOG_FILE_NAME_);
        
        /*檢查寫檔前的檔案大小*/
        if (ginDebug == VS_TRUE)
        {
            inLogPrintf(AT, "lnOldsize is %ld,line is %d", lnOldSize, __LINE__);
        }
        
        inFunc_GetSystemDateAndTime(&get_time);
        inWriteSizes = snprintf(szTempWrite,sizeof(szTempWrite),"%02u%02u%02u%02u%02u %s\n"
                                             ,get_time.uszMonth
                                             ,get_time.uszDay
                                             ,get_time.uszHour
                                             ,get_time.uszMinute
                                             ,get_time.uszSecond
                                             ,szMsg);                                           
        
        if (!memcmp(&szTempWrite[10], " : EDC_BOOT_Time" , 16)) /* 開機時間永遠寫再第一格 */
        {
                lseek(ginLogHandle, 0L, SEEK_SET);
                if (ginDebug == VS_TRUE)
		{
                    inLogPrintf(AT, "fun=%s ,line =%d", __FUNCTION__, __LINE__);
                }
        }
        else if(lnOldSize + inWriteSizes <= 3000)
        {
                lseek(ginLogHandle, 0L, SEEK_END);
                
                if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "lnOldSize +inWriteSizes<=3000");
                    inLogPrintf(AT, "lnOldSize is : %ld,inWriteSizes is: %d", lnOldSize, inWriteSizes);
                }
        }
        else
        {
                lseek(ginLogHandle, 0L, SEEK_SET);
                read(ginLogHandle, szTempOut, (int)lnOldSize);
                
                if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "lnOldSize +inWriteSizes>3000");
                    inLogPrintf(AT, "lnOldSize is : %ld,inWriteSizes is: %d", lnOldSize, inWriteSizes);
                }
            
                for (i = 27; i < (int)lnOldSize; i++)
                {
                        if(szTempOut[i] == 0x0A)
                        {
                                if((i - 27 + 1) >= inWriteSizes)
                                {
                                        lseek(ginLogHandle, 0L, SEEK_SET);
                                        memset(szTempOut, 0x00, sizeof(szTempOut));
                                        read(ginLogHandle, szTempOut, 27);
                                        lseek(ginLogHandle, (long)(i + 1), SEEK_SET);
                                        read(ginLogHandle, szTempOut+27, (int)lnOldSize-(i+1));
                                        
                                        if(ginDebug==VS_TRUE)
                                        {
                                            inLogPrintf(AT, "function = %s ,line = %d,index i = %d", __FUNCTION__, __LINE__, i);
//                                            inLogPrintf(AT, "szTempOut buf is: \n%s\n", szTempOut);
                                        }
                                        break;
                                }
                        }       
                }
               
                truncate(szTempBuf, 0);/*LOG檔案清空*/
                
                if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "lnOldsize is %ld,line is %d", lnOldSize, __LINE__);
                }    
                
                lseek(ginLogHandle, 0L, SEEK_SET); //2019/3/12 上午 10:33
                write(ginLogHandle, szTempOut, ((int)lnOldSize - (i + 1)) + 27); 
                
                if (ginDebug == VS_TRUE)
                {
//                    inLogPrintf(AT, "szTempOut buf is:\n%s\n",szTempOut);
                    inLogPrintf(AT, "function = %s ,line = %d,index i = %d", __FUNCTION__, __LINE__, i);
                }
        }
        write(ginLogHandle,szTempWrite,inWriteSizes);

        /*檢查寫完的檔案內容*/
        /*
        lnOldSize = lnFILE_GetSize(&ulHandle,( unsigned char *)_EDC_TRACE_LOG_FILE_NAME_);
        printf("lnOldsize is %ld,line is %d\n",lnOldSize,__LINE__);
        lseek(ginLogHandle, 0L,SEEK_SET); 
        memset(szTempOut,0x00,sizeof(szTempOut));
        read(ginLogHandle,szTempOut,(int)lnOldSize);
        printf("String BUFF is :\n%s\n",szTempOut);
        */
        
        return (VS_SUCCESS);
}

/*  Added by Hachi(Nick)
Function        :inBATCH_StoreTraceLog
Date&Time       :2019/2/15 上午 11:36
Describe        :將檔案內容清除，並保留開機時間
*/
int inUtility_ClearFile(void)
{
        int		inRetVal;
        long		lnOldSize;
        unsigned long	ulHandle;
        unsigned char	uzPathBuf[100+1] = "";
        unsigned char	uzTempBuf[27+1] = "";
        
        strcpy((char *)uzPathBuf, _FS_DATA_PATH_);
        strcat((char *)uzPathBuf, _EDC_TRACE_LOG_FILE_NAME_);
        
        inRetVal = open((char *)uzPathBuf, O_RDONLY);
	if (inRetVal > 0)
	{
		inFile_Open_File_Cnt_Increase();
	}
        read(inRetVal, uzTempBuf, 27);
        inRetVal = close(inRetVal);
	if (inRetVal == 0)
	{
		inFile_Open_File_Cnt_Decrease();
	}
        
        if(ginDebug==VS_TRUE)
        {
            inLogPrintf(AT, "the Readstring is: %s",uzTempBuf); 
        }
       
        inRetVal = open((char *)uzPathBuf, O_TRUNC|O_RDWR);
	if (inRetVal > 0)
	{
		inFile_Open_File_Cnt_Increase();
	}
        write(inRetVal, uzTempBuf, 27);
        inRetVal = close(inRetVal);
	if (inRetVal == 0)
	{
		inFile_Open_File_Cnt_Decrease();
	}
        
        if (ginDebug == VS_TRUE)
        {
		lnOldSize = lnFILE_GetSize(&ulHandle,(unsigned char *)_EDC_TRACE_LOG_FILE_NAME_);
		inLogPrintf(AT, "the Writestring is: %s",uzTempBuf);
		inLogPrintf(AT, "Oldsize is %ld,line is %d", lnOldSize, __LINE__);
        }
                
        /* 刪除檔案用*/
        //ret=remove((char *)uzPathBuf);
        //printf("ret is %d\n",ret);
        
        return(VS_SUCCESS);
}

/*
Function        :inBATCH_StoreTraceLog_OneStep
Date&Time       :2019/4/8 下午 5:18
Describe        :
*/
int inUtility_StoreTraceLog_OneStep(char *szMsg, ...)
{
	va_list		list;
	char		szDebugMsg[5000 + 1] = {0};
	
	inUtility_OpenTraceLogFile();
	
	va_start(list, szMsg);
	vsprintf(szDebugMsg, szMsg, list);
	va_end(list);
	inUtility_StoreTraceLog(szDebugMsg);
	
	inUtility_CloseTraceLogFile();
	
	return (VS_SUCCESS);
}

/*
 Function        :vdUtility_SYSFIN_OpenLogFile
 Date&Time       :2024/10/9 上午 11:15
 Describe        :開啟 log 檔案
*/
void vdUtility_SYSFIN_OpenLogFile()
{
	char szLogFilename[256] = {0};

	// 取得 log 檔名
	vdUtility_SYSFIN_get_log_filename(szLogFilename, sizeof(szLogFilename));

	// 開啟 log 檔案，若不存在則創建
	gpLog_fd = open(szLogFilename, O_CREAT | O_WRONLY | O_APPEND, 0644);
	memset(gszCurrentLogFilename, 0x00, sizeof(gszCurrentLogFilename));
	sprintf(gszCurrentLogFilename, "%s", szLogFilename);
	
	if (gpLog_fd == -1)
	{
		if (ginDebug == VS_TRUE)
		{
			inLogPrintf(AT, "Error opening log file %s", gszCurrentLogFilename);
			inLogPrintf(AT," [FS] open failed: %s", strerror(errno));
		}
	}
}

/*
 Function        :vdUtility_SYSFIN_CloseLogFile
 Date&Time       :2024/10/9 上午 11:16
 Describe        :關閉 log 檔案
*/
void vdUtility_SYSFIN_CloseLogFile()
{
	if (gpLog_fd != -1)
	{
		close(gpLog_fd);
		gpLog_fd = -1;
		memset(gszCurrentLogFilename, 0x00, sizeof(gszCurrentLogFilename));
	}
}

/*
 Function        :vdUtility_SYSFIN_LogMessage
 Date&Time       :2024/10/9 上午 11:16
 Describe        :寫入 log 並立即使用 fsync 同步到磁碟
*/
void vdUtility_SYSFIN_LogMessage(char* szLocation, const char *format, ...)
{
	int	inWriteRetVal = 0;
	
	/* 總空間小於200M的機器不存log */
	if (gulTotalROMSize < 200)
	{
		return ;
	}
	
	/* 清除最舊log */
	vdUtility_SYSFIN_ManageLogFileSize();
	
        if (gpLog_fd == -1)
        {
			vdUtility_SYSFIN_OpenLogFile();
			if (gpLog_fd == -1)
			{
				if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "vdUtility_SYSFIN_LogMessage Open File Error");
                }
				return;
			}
		lseek(gpLog_fd, 0L, SEEK_END);
        }
    
        char szLogMessage[8 * 1024] = {0};
	char szFinalLogMessage[10 * 1024] = {0};

        va_list args;
        va_start(args, format);

        // 取得目前時間，格式為 24-10-08 17:22:10.210
        RTC_NEXSYS      srRTC;
        unsigned long	ulMileSecond = 0;
        char            szTimeStamp[24];
        struct timeval  tTimeVal;
        
        inFunc_GetSystemDateAndTime(&srRTC);
        gettimeofday(&tTimeVal, NULL);
	ulMileSecond = (tTimeVal.tv_usec / 1000);
        snprintf(szTimeStamp, sizeof(szTimeStamp), "%02u-%02u-%02u %02u:%02u:%02u.%03lu",
                 srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay,
                 srRTC.uszHour, srRTC.uszMinute, srRTC.uszSecond, ulMileSecond); // 固定毫秒

        vsnprintf(szLogMessage, sizeof(szLogMessage), format, args);

        // 寫入 log 訊息，包含時間戳
        
        snprintf(szFinalLogMessage, sizeof(szFinalLogMessage), "[%s][%s] %s\n", szTimeStamp, szLocation, szLogMessage);
        inWriteRetVal = write(gpLog_fd, szFinalLogMessage, strlen(szFinalLogMessage));

        // 使用 fsync 將資料同步到磁碟
        if (fsync(gpLog_fd) != 0)
        {
                if (ginDebug == VS_TRUE)
                {
                    inLogPrintf(AT, "fsync failed");
                }
        }
	
        va_end(args);
	
	if (inWriteRetVal < 0)
	{
		vdUtility_SYSFIN_CloseLogFile();
	}
}

/*
 Function        :vdUtility_SYSFIN_ManageLogFileSize
 Date&Time       :2024/10/9 上午 10:56
 Describe        :檢查並管理 log 檔案大小
 */
void vdUtility_SYSFIN_ManageLogFileSize()
{
        long total_size = inUtility_SYSFIN_GetTotalLogSize();
        
        while (total_size > MAX_TOTAL_LOG_SIZE)
        {
            vdUtility_SYSFIN_DeleteOldestLogFile();
            total_size = inUtility_SYSFIN_GetTotalLogSize();
        }
}


/*
 Function        :inUtility_SYSFIN_GetTotalLogSize
 Date&Time       :2024/10/8 下午 6:18
 Describe        :計算所有 log 檔案的總大小
 */
int inUtility_SYSFIN_GetTotalLogSize()
{
        int inFileSize = 0;
        long total_size = 0;
        struct dirent *entry;
        DIR *dir = opendir(_AP_PUB_LOG_PATH_);

        if (dir == NULL)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Unable to open log directory");
                }
                
                return -1;
        }

        // 遍歷目錄並累加檔案大小
        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_type == DT_REG) // 僅計算檔案
                {
                    char filepath[512] = {0};
                    snprintf(filepath, sizeof(filepath), "%s%s", _AP_PUB_LOG_PATH_, entry->d_name);
                    inFileSize = 0;
                    inFile_Linux_Get_FileSize_By_Stat((char *)filepath, &inFileSize);
                    total_size += inFileSize;
                }
        }

        closedir(dir);
        
        return total_size;
}

/*
 Function        :vdUtility_SYSFIN_DeleteOldestLogFile
 Date&Time       :2024/10/8 下午 6:06
 Describe        :刪除最舊的 log 檔案
 */
void vdUtility_SYSFIN_DeleteOldestLogFile()
{
        struct dirent *entry;
        DIR *dir = opendir(_AP_PUB_LOG_PATH_);
        char szOldest_file[512] = {0};
        time_t oldest_time = time(NULL);

        if (dir == NULL)
        {
                if (ginDebug == VS_TRUE)
                {
                        inLogPrintf(AT, "Unable to open log directory");
                }
        }

        // 找出最舊的檔案
        while ((entry = readdir(dir)) != NULL)
        {
                if (entry->d_type == DT_REG) // 僅檔案
                {
                        char filepath[512] = {0};
                        snprintf(filepath, sizeof(filepath), "%s%s", _AP_PUB_LOG_PATH_, entry->d_name);

                        struct stat st;
                        if (stat(filepath, &st) == 0)
                        {
                                if (st.st_mtime < oldest_time)
                                {
                                    oldest_time = st.st_mtime;
                                    strncpy(szOldest_file, filepath, sizeof(szOldest_file) - 1);
                                }
                        }
                }
        }

        closedir(dir);

        // 刪除最舊的檔案
        if (strlen(szOldest_file) > 0)
        {
                if (remove(szOldest_file) == 0)
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "Deleted oldest log file: %s\n", szOldest_file);
                        }
                }
                else
                {
                        if (ginDebug == VS_TRUE)
                        {
                                inLogPrintf(AT, "Error deleting log file");
                        }
                }
        }
}

/*
 Function        :vdUtility_SYSFIN_get_log_filename
 Date&Time       :2024/10/9 上午 11:17
 Describe        :取得 log 檔名
*/
void vdUtility_SYSFIN_get_log_filename(char *szFilename, size_t size)
{
        RTC_NEXSYS srRTC = {0};
	
	if (guszLogDirCheckBit != VS_TRUE)
	{	
		struct stat st;
		if (stat(_AP_PUB_PATH_ _AP_PUB_LOG_DIR_NAME_, &st) == 0 && (st.st_mode & S_IFDIR))
		{

		}
		else
		{
			inFunc_Dir_Make(_AP_PUB_LOG_DIR_NAME_, _AP_PUB_PATH_);
		}
		
		guszLogDirCheckBit = VS_TRUE;
	}
        
        inFunc_GetSystemDateAndTime(&srRTC);
        snprintf(szFilename, size, "%s%02u%02u%02u_nexsys.log", _AP_PUB_LOG_PATH_, srRTC.uszYear, srRTC.uszMonth, srRTC.uszDay);
}

/*
 Function        :vdUtility_SYSFIN_check_log_date
 Date&Time       :2025/1/15 下午 1:59
 Describe        :若vdUtility_SYSFIN_get_log_filename取得的檔名
 */
void vdUtility_SYSFIN_check_log_date()
{
        char szLogFilename[256] = {0};

        // 取得 log 檔名
        vdUtility_SYSFIN_get_log_filename(szLogFilename, sizeof(szLogFilename));
        
        if (memcmp(gszCurrentLogFilename, szLogFilename, max(strlen(gszCurrentLogFilename), strlen(szLogFilename))))
        {
                vdUtility_SYSFIN_CloseLogFile();
        }
        
}