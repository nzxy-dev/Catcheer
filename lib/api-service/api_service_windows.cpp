#include <windows.h>
#include <shellapi.h>
#include <string>

//---------------------------------------------------------------get-exe-dir-full---------------------
std::string GetExeDirShared() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring fullPath(path);
    size_t pos = fullPath.find_last_of(L"\\/");
    std::wstring wDir = fullPath.substr(0, pos);
    int len = WideCharToMultiByte(CP_UTF8, 0, wDir.c_str(), (int)wDir.size(), nullptr, 0, nullptr, nullptr);
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wDir.c_str(), (int)wDir.size(), &out[0], len, nullptr, nullptr);
    return out;
}

//---------------------------------------------------------------open files---------------------
bool OpenSystemFile(const std::string& path) {
    int len = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), (int)path.size(), nullptr, 0);
    std::wstring wPath(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), (int)path.size(), &wPath[0], len);
    HINSTANCE hres = ShellExecuteW(NULL, L"open", wPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)hres) > 32;
}
