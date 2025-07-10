#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

#if !defined(__RELEASE)

#define DEBUG_MSG

#ifdef DEBUG_MSG
#define DEBUGZONE(x)	x
#else
#define DEBUGZONE(x)
#endif

#define TRACE_BUFFER_SIZE 4096


class C_DebugZone
{
private:
	TCHAR strBuf[512];
	TCHAR Timebuf[24];
	HFILE hFileMSG;
	TCHAR Path[1024];
public:
	BOOL Init(TCHAR *fPath = NULL) {
		if ( fPath ) {
			_tcscpy_s(Path, sizeof(Path), fPath);
			_tcscat_s(Path, "Debug.txt");
			OutputDebugString(Path);
		}
		else {
			GetCurrentDirectory(sizeof(Path),Path);
			_tcscat_s(Path, "\\Debug.txt");
			OutputDebugString(Path);
		}
		DEBUGZONE (	// 肺弊颇老 积己
			//_strtime(Timebuf);
			_strtime_s(Timebuf);
			OutputDebugString(Path);
		
			DEBUGZONE(hFileMSG = _lcreat(Path, 0));
			sprintf_s(strBuf, "[%s] DebugZone Start Done.\r\n", Timebuf);
			OutputDebugString(strBuf);
			_lwrite(hFileMSG, strBuf, _tcslen(strBuf));
		);
		
		return true;
	}
	void DebugMsg(TCHAR *lpszFormat, ...)
	{
		TCHAR szBuffer[TRACE_BUFFER_SIZE] = {0};
		if ( lpszFormat ) {
			DEBUGZONE (	// 肺弊颇老 积己
				va_list fmtList;
				va_start(fmtList, lpszFormat);
				_vsntprintf_s(szBuffer, TRACE_BUFFER_SIZE - 1, TRACE_BUFFER_SIZE - 1, lpszFormat, fmtList);
				//_vsnprintf_s(szBuffer, TRACE_BUFFER_SIZE - 1, TRACE_BUFFER_SIZE - 1, (char *)lpszFormat, fmtList);
				va_end(fmtList);
				szBuffer[TRACE_BUFFER_SIZE - 1] = 0;
				OutputDebugString(strBuf);
			
				_strtime_s(Timebuf);
				sprintf_s(strBuf, "[%s] %s\r\n", Timebuf, szBuffer);
				_lwrite(hFileMSG, strBuf, _tcslen(strBuf));
				//ZeroMemory(&lpszFormat, sizeof(lpszFormat));
				memset(strBuf, 0, sizeof(strBuf));
				memset(szBuffer, 0, sizeof(szBuffer));
			);
		}
	}
	
};

#else
#define DEBUGZONE
#endif