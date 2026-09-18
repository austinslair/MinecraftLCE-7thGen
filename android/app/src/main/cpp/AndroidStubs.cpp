#include "stdafx.h"
#include "AndroidStubs.h"
#include "HitResult.h"
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

static std::vector<pthread_key_t> g_tlsKeys;
static std::mutex g_tlsMutex;

DWORD TlsAlloc(void) {
    std::lock_guard<std::mutex> lock(g_tlsMutex);
    pthread_key_t key;
    if (pthread_key_create(&key, nullptr) != 0) {
        return (DWORD)-1;
    }
    g_tlsKeys.push_back(key);
    return (DWORD)(g_tlsKeys.size() - 1);
}

BOOL TlsFree(DWORD dwTlsIndex) {
    std::lock_guard<std::mutex> lock(g_tlsMutex);
    if (dwTlsIndex < g_tlsKeys.size()) {
        pthread_key_delete(g_tlsKeys[dwTlsIndex]);
        return TRUE;
    }
    return FALSE;
}

LPVOID TlsGetValue(DWORD dwTlsIndex) {
    if (dwTlsIndex < g_tlsKeys.size()) {
        return pthread_getspecific(g_tlsKeys[dwTlsIndex]);
    }
    return nullptr;
}

BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue) {
    if (dwTlsIndex < g_tlsKeys.size()) {
        return pthread_setspecific(g_tlsKeys[dwTlsIndex], lpTlsValue) == 0 ? TRUE : FALSE;
    }
    return FALSE;
}

void InitializeCriticalSection(CRITICAL_SECTION* cs) {
    if (!cs) return;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cs->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

void DeleteCriticalSection(CRITICAL_SECTION* cs) {
    if (!cs) return;
    pthread_mutex_destroy(&cs->mutex);
}

void EnterCriticalSection(CRITICAL_SECTION* cs) {
    if (!cs) return;
    pthread_mutex_lock(&cs->mutex);
}

void LeaveCriticalSection(CRITICAL_SECTION* cs) {
    if (!cs) return;
    pthread_mutex_unlock(&cs->mutex);
}

BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs) {
    if (!cs) return FALSE;
    return pthread_mutex_trylock(&cs->mutex) == 0 ? TRUE : FALSE;
}

void Sleep(DWORD dwMilliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(dwMilliseconds));
}

BOOL CloseHandle(HANDLE hObject) {
    return TRUE;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    return 0;
}

HitResult::HitResult(int x, int y, int z, int f, Vec3 *pos) {
    this->type = TILE;
    this->x = x;
    this->y = y;
    this->z = z;
    this->f = f;
    this->pos = Vec3::newTemp(pos->x, pos->y, pos->z);
    this->entity = nullptr;
}

HitResult::HitResult(shared_ptr<Entity> entity) {
    this->type = ENTITY;
    this->entity = entity;
    this->pos = nullptr;
    this->x = this->y = this->z = this->f = 0;
}

double HitResult::distanceTo(shared_ptr<Entity> e) {
    return 0.0;
}
