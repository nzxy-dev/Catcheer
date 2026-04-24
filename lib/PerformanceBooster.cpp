#include "PerformanceBooster.h"
#include <psapi.h>

void PerformanceBooster::Boost() {
    // Elevar prioridad del proceso
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // Aumentar prioridad del hilo principal
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Reservar mas memoria virtual si es necesario (opcional)
    SIZE_T minWorkingSet = 128 * 1024 * 1024; // 128 MB
    SIZE_T maxWorkingSet = 512 * 1024 * 1024; // 512 MB
    SetProcessWorkingSetSize(GetCurrentProcess(), minWorkingSet, maxWorkingSet);

    // Activar modo de ejecucion continua (evita throttling en chromebooks)
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
}