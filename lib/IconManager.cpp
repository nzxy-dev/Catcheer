#include "IconManager.h"

void SetDynamicIcon(HWND hwnd, const wchar_t* iconPath) {
    HICON hIcon = (HICON)LoadImage(
        nullptr,
        iconPath,
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE
    );

    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)hIcon);
        SetClassLongPtr(hwnd, GCLP_HICONSM, (LONG_PTR)hIcon);
    }
}