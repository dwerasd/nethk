#pragma once

#include <WinSock2.h>
#include <windows.h>


typedef DWORD (__stdcall *oZwProtectVirtualMemory) (IN HANDLE, IN OUT PVOID, IN OUT PULONG, IN ULONG , OUT PULONG);   
typedef int ( __stdcall *oSend ) ( SOCKET s, const char *buf, int len, int flags );
typedef int ( __stdcall *oRecv ) ( SOCKET s, char *buf, int len, int flags );

typedef int ( __stdcall* oWSASend ) (IN SOCKET s, IN LPWSABUF lpBuffers, IN DWORD dwBufferCount, OUT LPDWORD lpNumberOfBytesSent, IN DWORD dwFlags, IN LPWSAOVERLAPPED lpOverlapped, IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int ( __stdcall* oWSARecv ) (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine );
typedef BOOL ( __stdcall* oDeleteFileA ) (IN LPCSTR lpFileName);
typedef BOOL ( __stdcall* oDeleteFileW ) (IN LPCWSTR lpFileName);

int __stdcall MySend ( DWORD CallFrom, SOCKET s, const char *buf, int len, int flags );
int __stdcall MyRecv ( DWORD CallFrom, SOCKET s, char *buf, int len, int flags );

int __stdcall MyWSASend(DWORD CallFrom, IN SOCKET s, IN LPWSABUF lpBuffers, IN DWORD dwBufferCount, OUT LPDWORD lpNumberOfBytesSent, IN DWORD dwFlags, IN LPWSAOVERLAPPED lpOverlapped, IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
int __stdcall MyWSARecv(DWORD CallFrom, SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine );
void fnSendPacket(SOCKET sock, PBYTE Packet, UINT uSize);

BOOL MyDeleteFileA(DWORD CallFrom, LPCSTR lpFileName);
BOOL MyDeleteFileW(DWORD CallFrom, LPCWSTR lpFileName);

DWORD DetourFunc(BYTE *src, const BYTE *dst, const int len);
DWORD DetourFunc2(BYTE *src, const BYTE *dst, const int len);
BOOL fnVirtualProtect(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
