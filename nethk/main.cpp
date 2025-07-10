/*
#include <psapi.h>
#pragma comment(lib,"psapi.lib")
*/
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>

#include "ChkVerVS.h"
#include "hkApiFunc.h"

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#include "DebugZone.h"
#ifndef __RELEASE
C_DebugZone g_DebugMsg;
#define DBG_INIT	g_DebugMsg.Init
#define DEBUGMSG	g_DebugMsg.DebugMsg
#else
#define DBG_INIT
#define DEBUGMSG
#endif

// ZwProtectVirtualMemory

extern oZwProtectVirtualMemory fnZwProtectVirtualMemory;
extern oSend pSend;
extern oRecv pRecv;
extern oWSASend pWSASend;
extern oWSARecv pWSARecv;
extern oDeleteFileA pDeleteFileA;
extern oDeleteFileW pDeleteFileW;


void tKeyChk();

char fPath[1024] = { 0 };
char DllPath[1024] = { 0 };
char fName[64] = { 0 };
HANDLE hProcess = NULL;
SOCKET dgsock = NULL;
SOCKET diesock = NULL;

BOOL bDungeonStart = FALSE;
UINT uTimeCount = 0;

bool WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, void* lpReserved) {
	lpReserved;
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	{	// 로그 초기화
		GetModuleFileName(hModule, fPath, sizeof(fPath));
		_tcscpy_s(fName, sizeof(fName), strrchr(fPath, '\\') + 1);
		_tcsncpy_s(DllPath, sizeof(DllPath), fPath, strlen(fPath) - strlen(fName));
		DBG_INIT(DllPath);
		/*
		// 권한얻기
		BYTE WasEn;
		typedef LONG (WINAPI *TRAP)(DWORD, bool, DWORD, BYTE*);
		TRAP RAP;
		RAP = (TRAP)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlAdjustPrivilege");
		if( RAP ) {
			if ( RAP( 20, false, 0, &WasEn ) >= 0 ) {
				RAP( 20, true, 0, &WasEn );
			}
		}
		*/
		hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ | PROCESS_VM_WRITE, false, GetCurrentProcessId());

		HMODULE hWs2_32 = GetModuleHandle("ws2_32.dll");
		if (!hWs2_32) {
			hWs2_32 = LoadLibrary("ws2_32.dll");
		}
		if (hWs2_32) {
			pSend = (oSend)DetourFunc((BYTE*)GetProcAddress(hWs2_32, "send"), (BYTE*)MySend, 5);
			pRecv = (oRecv)DetourFunc((BYTE*)GetProcAddress(hWs2_32, "recv"), (BYTE*)MyRecv, 5);
		}
		if (hWs2_32) {
			pWSASend = (oWSASend)DetourFunc((BYTE*)GetProcAddress(hWs2_32, "WSASend"), (BYTE*)MyWSASend, 5);
			pWSARecv = (oWSARecv)DetourFunc((BYTE*)GetProcAddress(hWs2_32, "WSARecv"), (BYTE*)MyWSARecv, 5);
		}

		/*
		HMODULE hNtdll = GetModuleHandle("ntdll.dll");
		if ( !hNtdll ) {
			hNtdll = LoadLibrary("ntdll.dll");
		}
		if ( hNtdll ) {
			BYTE *dwDbgUiRemoteBreakin = (BYTE *)GetProcAddress(hNtdll, "DbgUiRemoteBreakin");
			DWORD dwProtect = 0;
			fnVirtualProtect(hProcess, dwDbgUiRemoteBreakin, 2, 0, &dwProtect);
			*(WORD*)(dwDbgUiRemoteBreakin) = 0x32EB;
			fnVirtualProtect(hProcess, dwDbgUiRemoteBreakin, 2, dwProtect, 0);
		}
		*/
		HMODULE hKernel32 = GetModuleHandle("kernel32.dll");
		if (hKernel32) {
			pDeleteFileA = (oDeleteFileA)DetourFunc2((BYTE*)GetProcAddress(hKernel32, "DeleteFileA"), (BYTE*)MyDeleteFileA, 5);
			//pDeleteFileW = (oDeleteFileW)DetourFunc((BYTE *)GetProcAddress(hKernel32, "DeleteFileW"), (BYTE *)MyDeleteFileW, 5);
		}
		//CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)tKeyChk,0,0,0);		// 키 스레드 생성
		DisableThreadLibraryCalls(hModule);
	}
	break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}

void tKeyChk()
{
	BOOL KeyPress[64] = { 0 };

	DWORD dwCount4 = 0;
	do {
		static DWORD loop = 0;
		static DWORD start = GetTickCount();
		DWORD rTime = GetTickCount() - start;
		if (rTime > 100) {	// 이 안은 0.1초에 한번씩 들어옴
			dwCount4 = ++loop;
			start = GetTickCount();
		}
		if (KEY_DOWN(VK_DELETE) && !KeyPress[0]) {	// 이건 키 입력 이런식으로 추가하면됨
			KeyPress[0] = TRUE;
		}
		if (KEY_UP(VK_DELETE) && KeyPress[0]) {
			KeyPress[0] = FALSE;
		}
		if (KEY_DOWN(VK_END) && !KeyPress[1]) {	// 
			KeyPress[1] = TRUE;
		}
		if (KEY_UP(VK_END) && KeyPress[1]) {
			KeyPress[1] = FALSE;
		}
		Sleep(1);
	} while (true);
}
