#include "performance_windows.h"
#include <psapi.h>

void PerformanceBooster::Boost() {
    // hight proprity to process
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // give hight proprity to main thread
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // reserve more RAM (optional)
    SIZE_T minWorkingSet = 128 * 1024 * 1024; // Default 128 MB
    SIZE_T maxWorkingSet = 512 * 1024 * 1024; // Default 512 MB
    SetProcessWorkingSetSize(GetCurrentProcess(), minWorkingSet, maxWorkingSet);

    // enable continuous execution mode (void throttling in low end laptops)
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
}
