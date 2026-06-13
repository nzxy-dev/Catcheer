/*
 * ===============|==========================================================|
 *   Catcheer     | Windows core main-module (Webview2) v1.09                |
 * _______________|__________________________________________________________|
 */

//__________________________________________________________________________|
//                                                                          |
//----------------- INCLUDES -----------------------------------------------|
//__________________________________________________________________________|

#include "webview2.h"
#include <string>
#include <vector>
#include <wrl.h>
#include <wil/com.h>
#include <objbase.h>
#include "WebView2.h" 
#include <WebView2EnvironmentOptions.h>
#include "wil/win32_helpers.h"
#include "icon_webview2.h"
#include "performance_windows.h"
#include "../api-service/api.h"

//------------------------------ convert helpers --------------------------------------|

static std::string WSToS(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], len, nullptr, nullptr);
    return out;
}

static std::wstring SToWS(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], len);
    return out;
}

//--------------------------------------------------------------------------|



//------------------------------ global vars --------------------------------------|

const wchar_t CLASS_NAME[] = L"CatcheerWindow";
HINSTANCE g_hInstance = nullptr;
wil::com_ptr<ICoreWebView2Controller> g_webViewController;
wil::com_ptr<ICoreWebView2>           g_webView;

static bool g_isFullscreen = false;
static RECT g_windowedRect = { 0 };
static DWORD g_windowedStyle = 0;
static std::wstring g_startHtmlRelPath;

//--------------------------------------------------------------------------|



//----------------------internal structs---------------------------------------|

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeWebView2(HWND hwnd);
void ResizeWebView2(HWND hwnd);

//engine_start

int RunWindowsEngine(HINSTANCE hInstance, int nCmdShow) {
    // com init
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    PerformanceBooster::Boost();
    g_hInstance = hInstance;

    AppConfig config = LoadAppConfig();

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassEx(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!config.border) style = WS_POPUP;
    if (!config.resizable) {
        style &= ~WS_SIZEBOX;
        style &= ~WS_MAXIMIZEBOX;
    }

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, SToWS(config.title).c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, config.width, config.height,
        nullptr, nullptr, hInstance, nullptr);
    
    //------------------set icon|
    SetDynamicIcon(hwnd, L"custom.ico");

    if (config.fullscreen) {
        g_windowedStyle = GetWindowLong(hwnd, GWL_STYLE);
        GetWindowRect(hwnd, &g_windowedRect);
        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        g_isFullscreen = true;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: InitializeWebView2(hwnd); return 0;
    case WM_SIZE:   ResizeWebView2(hwnd);     return 0;
    case WM_CLOSE:  g_webView = nullptr; g_webViewController = nullptr; DestroyWindow(hwnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//--------------------------------------------------------------------------|


//------------------------- Init_webview2 ----------------------------------|
void InitializeWebView2(HWND hwnd) {
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AdditionalBrowserArguments(L"--allow-file-access-from-files --enable-gpu --enable-webgl --autoplay-policy=no-user-gesture-required");

    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    MessageBox(hwnd, L"Error intializating web view2", L"Catcheer Error", MB_OK | MB_ICONERROR);
                    return result;
                }
                env->CreateCoreWebView2Controller(hwnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result) || !controller) {
                                MessageBox(hwnd, L"Error intializating web view2 controller", L"Catcheer Error", MB_OK | MB_ICONERROR);
                                return result;
                            }
                            g_webViewController = controller;
                            RECT bounds; GetClientRect(hwnd, &bounds);
                            g_webViewController->put_Bounds(bounds);
                            g_webViewController->put_IsVisible(TRUE); 
                            ResizeWebView2(hwnd); 

                            controller->get_CoreWebView2(&g_webView);

                            wil::com_ptr<ICoreWebView2Settings> settings;
                            g_webView->get_Settings(&settings);
                            settings->put_IsScriptEnabled(TRUE);
                            settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            settings->put_IsWebMessageEnabled(TRUE);
                            
                            //---------------contextual menu (left-click) block------|
                            settings->put_AreDefaultContextMenusEnabled(FALSE);//    |
                            //-------------------------------------------------------|

                            std::wstring cmd = GetCommandLineW();
                            size_t p = cmd.find(L"--html");
                            if (p != std::wstring::npos) {
                                size_t q1 = cmd.find(L"\"", p);
                                size_t q2 = (q1 != std::wstring::npos) ? cmd.find(L"\"", q1 + 1) : std::wstring::npos;
                                if (q1 != std::wstring::npos && q2 != std::wstring::npos) 
                                    g_startHtmlRelPath = cmd.substr(q1 + 1, q2 - q1 - 1);
                            }
                            
                            
                            //------------------------html file load
                            std::wstring path = SToWS(GetExeDirShared()) + L"\\" + (g_startHtmlRelPath.empty() ? L"S-folder\\index.html" : g_startHtmlRelPath);
                            std::wstring url = L"file:///" + path;
                            for (auto& ch : url) if (ch == L'\\') ch = L'/';
                            g_webView->Navigate(url.c_str());
                            
 
                            //---------------------------web message handlers
                            g_webView->add_WebMessageReceived(
                                Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [hwnd](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        wil::unique_cotaskmem_string raw;
                                        args->get_WebMessageAsJson(&raw);
                                        std::string message = WSToS(raw.get());

                                        std::string apiResponse = handle_web_message(message);
                                        if (!apiResponse.empty()) {
                                            sender->PostWebMessageAsString(SToWS(apiResponse).c_str());
                                        } else {
                                            if (message.find("setWindowTitle") != std::string::npos) {
                                                std::string title = ExtractJsonString(message, "value");
                                                if (!title.empty()) SetWindowText(hwnd, SToWS(title).c_str());
                                            }
                                            else if (message.find("closeWindow") != std::string::npos) {
                                                PostMessage(hwnd, WM_CLOSE, 0, 0);
                                            }
                                                                                    
                                            else if (message.find("getWindowSize") != std::string::npos) {
                                                RECT rc;
                                                GetClientRect(hwnd, &rc);
                                                int w = rc.right - rc.left;
                                                int h = rc.bottom - rc.top;
                                                std::wstring resp =
                                                    L"{\"width\":" + std::to_wstring(w) +
                                                    L",\"height\":" + std::to_wstring(h) + L"}";
                                                sender->PostWebMessageAsString(resp.c_str());
                                            }
                                            
                                            else if (message.find("getWindowPosition") != std::string::npos) {
                                                RECT rc;
                                                GetWindowRect(hwnd, &rc);
                                                std::wstring resp =
                                                    L"{\"x\":" + std::to_wstring(rc.left) +
                                                    L",\"y\":" + std::to_wstring(rc.top) + L"}";
                                                sender->PostWebMessageAsString(resp.c_str());
                                            }
                                            
                                            else if (message.find("setWindowPosition") != std::string::npos) {
                                                int x = 0, y = 0; 
                                                sscanf_s(message.c_str(), "{\"action\":\"setWindowPosition\",\"x\":%d,\"y\":%d}", &x, &y);

                                                SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                                                std::wstring resp = L"{\"ok\":true,\"x\":" + std::to_wstring(x) + L",\"y\":" + std::to_wstring(y) + L"}";
                                                sender->PostWebMessageAsString(resp.c_str());
                                            }
                                            
                                            else if (message.find("setWindowSize") != std::string::npos) {
                                                int w = 0, h = 0;
                                                sscanf_s(message.c_str(), "{\"action\":\"setWindowSize\",\"width\":%d,\"height\":%d}", &w, &h);

                                                SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
                                                std::wstring resp = L"{\"ok\":true,\"width\":" + std::to_wstring(w) + L",\"height\":" + std::to_wstring(h) + L"}";
                                                sender->PostWebMessageAsString(resp.c_str());
                                            }
                                            
                                            else if (message.find("newWindow") != std::string::npos) {
                                                wchar_t exePath[MAX_PATH];
                                                GetModuleFileNameW(NULL, exePath, MAX_PATH);

                                                std::string relPath = ExtractJsonString(message, "path");
                                                std::wstring params = L"";
                                                if (!relPath.empty()) {
                                                    params = L"--html \"" + SToWS(relPath) + L"\"";
                                                }
                                                
                                                ShellExecuteW(NULL, L"open", exePath, params.c_str(), NULL, SW_SHOWNORMAL);
                                                sender->PostWebMessageAsString(L"{\"newWindow\":\"ok\"}");
                                            }

                                            else if (message.find("toggleFullscreen") != std::string::npos) {
                                                if (!g_isFullscreen) {
                                                    g_windowedStyle = GetWindowLong(hwnd, GWL_STYLE);
                                                    GetWindowRect(hwnd, &g_windowedRect);
                                                    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
                                                    SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_SHOWWINDOW);
                                                } else {
                                                    SetWindowLong(hwnd, GWL_STYLE, g_windowedStyle);
                                                    SetWindowPos(hwnd, HWND_TOP, g_windowedRect.left, g_windowedRect.top, g_windowedRect.right - g_windowedRect.left, g_windowedRect.bottom - g_windowedRect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
                                                }
                                                g_isFullscreen = !g_isFullscreen;
                                            }
                                        }
                                        return S_OK;
                                    }
                                ).Get(), nullptr);
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}


// Set webview 2 to client viewport
void ResizeWebView2(HWND hwnd) {
    if (g_webViewController) {
        RECT bounds; GetClientRect(hwnd, &bounds);
        g_webViewController->put_Bounds(bounds);
    }
    
}
