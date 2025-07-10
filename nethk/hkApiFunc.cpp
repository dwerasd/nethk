#include "hkApiFunc.h"
#include "Pattern.h"

#include "ChkVerVS.h"

#include "DebugZone.h"
#ifndef __RELEASE
extern C_DebugZone g_DebugMsg;
#define DBG_INIT	g_DebugMsg.Init
#define DEBUGMSG	g_DebugMsg.DebugMsg
#else
#define DBG_INIT
#define DEBUGMSG
#endif

oZwProtectVirtualMemory fnZwProtectVirtualMemory = NULL;
oSend pSend = NULL;
oRecv pRecv = NULL;

oWSASend pWSASend = NULL;
oWSARecv pWSARecv = NULL;
oDeleteFileW pDeleteFileW = NULL;
oDeleteFileA pDeleteFileA = NULL;

extern HANDLE hProcess;

DWORD DetourFunc(BYTE* src, const BYTE* dst, const int len) {
	DEBUGMSG("HookAddr : %x", src);
	BYTE* ppSrc = (BYTE*)malloc(len + 5);
	if (!ppSrc) { return(0); }
	memcpy(ppSrc, src, len);		// 원래 코드를 보관하고
	ppSrc[len] = 0xE9;				// ppSrc+5바이트 한 곳으로 점프하는 코드를 생성
	*(DWORD*)(ppSrc + len + 1) = (DWORD)((BYTE*)src - ppSrc) - 5;

	BYTE* jmp = (BYTE*)malloc(5 + 6);
	if (!jmp) { free(ppSrc); return(0); }
	jmp[0] = 0x58;	// pop eax
	jmp[1] = 0x83;	// sub
	jmp[2] = 0xEC;	// esp,
	jmp[3] = 0x04;	// 0x04
	jmp[4] = 0x50;	// push eax			리턴 주소를 넣으면 인자로 함수에서 받을 수 있음.
	jmp[5] = 0xE9;	// jmp
	*(DWORD*)(jmp + 6) = (DWORD)((BYTE*)dst - jmp) - 10;	// MySend
	DWORD dwProtect = 0;
	fnVirtualProtect(hProcess, (LPVOID)src, 5, PAGE_READWRITE, &dwProtect);
	src[0] = 0xE9;									// jmp
	*(DWORD*)(src + 1) = (DWORD)((jmp - len) - src);	// ppSrc
	fnVirtualProtect(hProcess, (LPVOID)src, 5, dwProtect, NULL);
	return (DWORD)ppSrc;
}

DWORD DetourFunc2(BYTE* src, const BYTE* dst, const int len) {
	BYTE* ppSrc = (BYTE*)malloc(len + 5);
	if (!ppSrc) { return(0); }
	memcpy(ppSrc, src, len);		// 원래 코드를 보관하고
	ppSrc[len] = 0xE9;				// ppSrc+5바이트 한 곳으로 점프하는 코드를 생성
	*(DWORD*)(ppSrc + len + 1) = (DWORD)((BYTE*)src - ppSrc) - 5;

	BYTE* jmp = (BYTE*)malloc(5);
	if (!jmp) { free(ppSrc); return(0); }
	jmp[0] = 0xE9;	// jmp
	*(DWORD*)(jmp + 1) = (DWORD)((BYTE*)dst - jmp) - 5;	// MySend
	DWORD dwProtect = 0;
	fnVirtualProtect(hProcess, (LPVOID)src, 5, PAGE_READWRITE, &dwProtect);
	src[0] = 0xE9;									// jmp
	*(DWORD*)(src + 1) = (DWORD)((jmp - len) - src);	// ppSrc
	fnVirtualProtect(hProcess, (LPVOID)src, 5, dwProtect, NULL);
	return (DWORD)ppSrc;
}

BOOL fnVirtualProtect(HANDLE _hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect) {
	if (!fnZwProtectVirtualMemory) {
		BYTE* pZwProtectVirtualMemory = (BYTE*)GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwProtectVirtualMemory");
		BYTE* NewZwProtectVirtualMemory = (BYTE*)malloc(10);
		if (!NewZwProtectVirtualMemory || !pZwProtectVirtualMemory) {
			DEBUGMSG("ZwProtectVirtualMemory not found");
			return FALSE;
		}
		memcpy(NewZwProtectVirtualMemory, pZwProtectVirtualMemory, 5);
		*(NewZwProtectVirtualMemory + 5) = 0xE9;
		*(DWORD*)(NewZwProtectVirtualMemory + 6) = (DWORD)((BYTE*)pZwProtectVirtualMemory - NewZwProtectVirtualMemory) - 5;
		fnZwProtectVirtualMemory = (oZwProtectVirtualMemory)NewZwProtectVirtualMemory;
	}
	if (!fnZwProtectVirtualMemory) {
		DEBUGMSG("fnZwProtectVirtualMemory is NULL");
		return FALSE;
	}
	if (!_hProcess) {
		_hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
	}
	if (!_hProcess) {
		DEBUGMSG("OpenProcess failed");
		return FALSE;
	}
	PVOID pAddr = lpAddress;
	ULONG ulSize = dwSize;
	return fnZwProtectVirtualMemory(_hProcess, &pAddr, &ulSize, flNewProtect, lpflOldProtect) ? FALSE : TRUE;;
}

BOOL CheckPattern(BYTE* src, BYTE* dst, int len) {
	BOOL bResult = TRUE;
	for (BYTE i = 0; i < len; ++i) {
		if (*(BYTE*)(src + i) != *(BYTE*)(dst + i)) {
			//DEBUGMSG("*(BYTE *)(src+%d) : %x, *(BYTE *)(dst+%d) : %x", i , *(BYTE *)(src+i),  i , *(BYTE *)(dst+i));
			bResult = FALSE;	// 다른 부분이 있다면 FALSE를 반환함.
			break;
		}
	}
	return bResult;
}

int __stdcall MyWSASend(DWORD CallFrom, IN SOCKET s, IN LPWSABUF lpBuffers, IN DWORD dwBufferCount, OUT LPDWORD lpNumberOfBytesSent, IN DWORD dwFlags, IN LPWSAOVERLAPPED lpOverlapped, IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	DEBUGMSG("MyWSASend: %x", CallFrom);
	return pWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}
int __stdcall MyWSARecv(DWORD CallFrom, SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	DEBUGMSG("MyWSARecv: %x", CallFrom);
	return pWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
}

void fnSendPacket(SOCKET sock, PBYTE Packet, UINT uSize) {
	WSAOVERLAPPED SendOverlapped = { 0 };
	SecureZeroMemory((PVOID)&SendOverlapped, sizeof(WSAOVERLAPPED));
	SendOverlapped.hEvent = WSACreateEvent();
	WSABUF SendPacket;
	SendPacket.buf = (char*)Packet;
	SendPacket.len = uSize;

	DWORD dwResult = 0;
	MyWSASend(0, sock, &SendPacket, 1, &dwResult, 0, &SendOverlapped, 0);	// sock는 정해둔 패킷이 send되었을때 얻었다가 사용함.
	//WSAResetEvent(SendOverlapped.hEvent);
}
char SendBuf[1024] = { 0 };
int __stdcall MySend(DWORD CallFrom, SOCKET s, const char* buf, int len, int flags) {
	DEBUGMSG("SendFrom: %x, Sockid : %d, PacketSize : %d", CallFrom, s, len);
	
	for (int i = 0; i < len; ++i) {
		char a[4] = { 0 };
		sprintf_s(a, "%02X ", *(BYTE*)(buf + i));
		strcat_s(SendBuf, a);
		if ((i % 64) == 0 && i) {
			DEBUGMSG("%s", SendBuf);
			memset(SendBuf, 0, 1024);
		}
	}
	DEBUGMSG("%s", SendBuf);

	return pSend(s, buf, len, flags);	// 원래 코드를 호출해줌.
}
char RecvBuf[1024] = { 0 };
int __stdcall MyRecv(DWORD CallFrom, SOCKET s, char* buf, int len, int flags) {
	DEBUGMSG("RecvFrom: %x, Sockid: %d, PacketSize: %d", CallFrom, s, len);
	int nRecv = pRecv(s, buf, len, flags);
	
	for (int i = 0; i < len + 1; ++i)
	{
		char a[4] = { 0 };
		sprintf_s(a, "%02X ", *(BYTE*)(buf + i));
		strcat_s(RecvBuf, a);
		if ((i % 64) == 0 && i) {
			DEBUGMSG("%s", RecvBuf);
			memset(RecvBuf, 0, 1024);
		}
	}
	DEBUGMSG("%s", RecvBuf);
	return nRecv;
}
/*
BOOL MyDeleteFileA(DWORD CallFrom, LPCSTR lpFileName) {
	DEBUGMSG("[DeleteFileA] CallFrom %X, %s", CallFrom, lpFileName);
	CHAR MyDirectory[1024] = "c:\\lin\\";
	CHAR DelFName[64] = {0};
	_tcscpy_s(DelFName, sizeof(DelFName), strrchr(lpFileName,'\\')+1);
	_tcscat_s(MyDirectory, DelFName);
	CopyFile(lpFileName, MyDirectory, FALSE);
	return pDeleteFileA(lpFileName);
}
*/

BOOL MyDeleteFileA(DWORD CallFrom, LPCSTR lpFileName) {
	DEBUGMSG("[DeleteFileA] %x %s", lpFileName);
	CHAR MyDirectory[1024] = "c:\\lin\\";
	CHAR DelFName[64] = { 0 };
	_tcscpy_s(DelFName, sizeof(DelFName), strrchr(lpFileName, '\\') + 1);
	if (_tcslen(DelFName) > 4) {
		_tcscat_s(MyDirectory, DelFName);
		CopyFileA(lpFileName, MyDirectory, FALSE);
	}
	return pDeleteFileA(lpFileName);
}

BOOL MyDeleteFileW(DWORD CallFrom, LPCWSTR lpFileName) {
	//DEBUGMSG("[DeleteFileA] %s", lpFileName);
	OutputDebugStringW(lpFileName);
	WCHAR MyDirectory[1024] = L"c:\\lin\\";
	WCHAR DelFName[64] = { 0 };
	wcscpy_s(DelFName, wcsrchr(lpFileName, L'\\') + 1);
	OutputDebugStringW(DelFName);
	wcscat_s(MyDirectory, DelFName);
	CopyFileW(lpFileName, MyDirectory, FALSE);
	return pDeleteFileW(lpFileName);
}
