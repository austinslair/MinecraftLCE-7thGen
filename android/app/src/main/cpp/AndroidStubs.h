#pragma once
#include "AndroidTypes.h"
#include <pthread.h>
#include <unistd.h>

#define _ASSERT(x) ((void)0)

struct AndroidCriticalSection {
    pthread_mutex_t mutex;
};

typedef struct AndroidCriticalSection CRITICAL_SECTION;
typedef CRITICAL_SECTION* PCRITICAL_SECTION;
typedef CRITICAL_SECTION* LPCRITICAL_SECTION;

void InitializeCriticalSection(CRITICAL_SECTION* cs);
void DeleteCriticalSection(CRITICAL_SECTION* cs);
void EnterCriticalSection(CRITICAL_SECTION* cs);
void LeaveCriticalSection(CRITICAL_SECTION* cs);
BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs);

DWORD TlsAlloc(void);
BOOL TlsFree(DWORD dwTlsIndex);
LPVOID TlsGetValue(DWORD dwTlsIndex);
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

void Sleep(DWORD dwMilliseconds);
BOOL CloseHandle(HANDLE hObject);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
void MemSect(int);
