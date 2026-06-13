#include "performance_linux.h"
#include <stdlib.h>
#include <sys/resource.h>
#include <iostream>

void LinuxBooster::Boost() {
    // process priority (HIGH_PRIORITY_CLASS)
    // nice in linuxx is from -20 to lower like 19 
    // -10 is a healthy and stable priority!!!!
    setpriority(PRIO_PROCESS, 0, -10);

    // flags by gpu , THE OG
    setenv("WEBKIT_FORCE_COMPOSITING_MODE", "1", 1);
    setenv("WEBKIT_DISABLE_COMPOSITING_MODE", "0", 1);
    
    // canva 2d and webgl acceleration
    setenv("WEBKIT_USE_GLX", "1", 1);
    
    // rendering zro copy (render optimization)
    setenv("WEBKIT_DISABLE_GLES2", "0", 1);
    
    // audio latency
    setenv("PULSE_LATENCY_MSEC", "20", 1);


    std::cout << "[BOOSTER] Optimization for Linux set (GPU Force + Process Priority)" << std::endl;
}
