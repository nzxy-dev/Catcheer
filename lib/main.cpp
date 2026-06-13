#include <windows.h>
#include "core/windows_webview2/webview2.h"

// el punto de entrada mas absurdo de la historia
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    return RunWindowsEngine(hInstance, nCmdShow);
}
