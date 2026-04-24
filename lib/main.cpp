// main.cpp
//----------------------------------------------------------------------------------------------------
// Aplicación Win32 con WebView2 que carga HTML embebido desde recursos.
// Compatible con C++17. Incluye comentarios detallados en cada sección.
//-----------------------------------------------------------------------------------------------------
// Requisitos previos: 
// - Paquete Microsoft.Web.WebView2 instalado por NuGet.
// - El recurso 'IDR_HTML1' de tipo RT_HTML configurado en resources.h
//----------------------------------------------------------------------------------------------------
// Para compilar en Visual Studio: asegúrese de usar como estándar de lenguaje C++17 (o posterior).

#include <windows.h>
#include <string>
#include <vector>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "resource.h" 
#include <WebView2EnvironmentOptions.h>
#include "wil/win32_helpers.h"
#include "IconManager.h"
#include "PerformanceBooster.h"








// Nombre y título de la clase de ventana principal
const wchar_t CLASS_NAME[] = L"WebView2SampleWindow";
const wchar_t WINDOW_TITLE[] = L"Catcher";

HINSTANCE g_hInstance = nullptr;

// Punteros inteligentes globales para el controlador y la instancia WebView2.
wil::com_ptr<ICoreWebView2Controller> g_webViewController;
wil::com_ptr<ICoreWebView2>           g_webView;
// Estado del último archivo escrito
static std::wstring g_lastWrittenContent;
static std::wstring g_lastWrittenPath;
// Estado de fullscreen y ventana previa
static bool g_isFullscreen = false;
static RECT g_windowedRect = { 0 };
static DWORD g_windowedStyle = 0;
static std::wstring g_startHtmlRelPath;


// Prototipos de funciones principales
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeWebView2(HWND hwnd);
void ResizeWebView2(HWND hwnd);
std::wstring LoadHtmlResource(int resourceId, HINSTANCE hInstance);

// Escapar texto para JSON (\, ", saltos de línea, etc.)
static std::wstring JsonEscape(const std::wstring& s) {
    std::wstring o;
    o.reserve(s.size() * 2);
    for (wchar_t c : s) {
        switch (c) {
        case L'\\': o += L"\\\\"; break;
        case L'"':  o += L"\\\""; break;
        case L'\n': o += L"\\n";  break;
        case L'\r': o += L"\\r";  break;
        case L'\t': o += L"\\t";  break;
        default:    o.push_back(c); break;
        }
    }
    return o;
}

// Extraer cadena de un JSON simple: "key":"value"
static std::wstring ExtractJsonString(const std::wstring& msg, const std::wstring& key) {
    std::wstring pattern = L"\"" + key + L"\"";
    size_t kp = msg.find(pattern);
    if (kp == std::wstring::npos) return L"";
    size_t colon = msg.find(L":", kp);
    if (colon == std::wstring::npos) return L"";
    size_t first = msg.find(L"\"", colon + 1);
    if (first == std::wstring::npos) return L"";
    size_t second = msg.find(L"\"", first + 1);
    if (second == std::wstring::npos) return L"";
    return msg.substr(first + 1, second - first - 1);
}

// Convertir wstring a UTF-8 bytes
static std::string ToUtf8(const std::wstring& w) {
    if (w.empty()) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], len, nullptr, nullptr);
    return out;
}

// Leer archivo completo (binario) y decodificar a UTF-8 o ANSI a wstring
static bool ReadFileWide(const std::wstring& path, std::wstring& outContent, DWORD& outBytes, std::wstring& outError) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) { outError = L"OpenFailed"; return false; }
    LARGE_INTEGER sz;
    if (!GetFileSizeEx(h, &sz)) { outError = L"SizeFailed"; CloseHandle(h); return false; }
    if (sz.QuadPart > (size_t)INT_MAX) { outError = L"TooLarge"; CloseHandle(h); return false; }
    std::vector<char> buf((size_t)sz.QuadPart);
    DWORD read = 0;
    if (!ReadFile(h, buf.data(), (DWORD)buf.size(), &read, nullptr)) {
        outError = L"ReadFailed";
        CloseHandle(h);
        return false;
    }
    CloseHandle(h);
    outBytes = read;



    // Intentar UTF-8, si no, ANSI
    int wlen = MultiByteToWideChar(CP_UTF8, 0, buf.data(), (int)read, nullptr, 0);
    UINT cp = CP_UTF8;
    if (wlen <= 0) {
        wlen = MultiByteToWideChar(CP_ACP, 0, buf.data(), (int)read, nullptr, 0);
        cp = CP_ACP;
    }
    if (wlen <= 0) {
        outError = L"DecodeFailed";
        return false;
    }
    outContent.resize(wlen);
    MultiByteToWideChar(cp, 0, buf.data(), (int)read, &outContent[0], wlen);
    return true;
}


// Escribir archivo con contenido en UTF-8
static bool WriteFileWide(const std::wstring& path, const std::wstring& content, DWORD& outBytes, std::wstring& outError) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) { outError = L"CreateFailed"; return false; }
    std::string utf8 = ToUtf8(content);
    DWORD written = 0;
    if (!WriteFile(h, utf8.data(), (DWORD)utf8.size(), &written, nullptr)) { outError = L"WriteFailed"; CloseHandle(h); return false; }
    CloseHandle(h);
    outBytes = written;
    return true;
}



// Función principal de la aplicación.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    // Inicialización de COM en el hilo principal (necesario para el WebView2).
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"No se pudo inicializar COM.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return -1;
    }
    PerformanceBooster::Boost();

    g_hInstance = hInstance;

    // Definir y registrar la clase de ventana.
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"Error al registrar la clase de ventana.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    // Crear la ventana principal centrada.
    // BLOQUE: Configuración desde archivo INI
    // Lee parámetros desde "config.ini" ubicado junto al .exe
    std::wstring GetExeDir();
    std::wstring exeDir = GetExeDir();
    std::wstring iniPath = exeDir + L"\\config.ini";

    wchar_t title[256] = L"";
    GetPrivateProfileStringW(L"Window", L"title", WINDOW_TITLE, title, 256, iniPath.c_str());

    wchar_t size[64] = L"";
    GetPrivateProfileStringW(L"Window", L"size", L"1024x768", size, 64, iniPath.c_str());

    wchar_t border[16] = L"";
    GetPrivateProfileStringW(L"Window", L"border", L"true", border, 16, iniPath.c_str());

    wchar_t fullscreen[16] = L"";
    GetPrivateProfileStringW(L"Window", L"fullscreen", L"false", fullscreen, 16, iniPath.c_str());

    wchar_t resizable[16] = L"";
    GetPrivateProfileStringW(L"Window", L"resizable", L"true", resizable, 16, iniPath.c_str());

    // Parsear tamaño
    int width = 1024, height = 768;
    swscanf_s(size, L"%dx%d", &width, &height);

    // Determinar estilo de ventana
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (wcscmp(border, L"false") == 0) style = WS_POPUP;
    if (wcscmp(resizable, L"false") == 0) style &= ~WS_SIZEBOX;
    if (wcscmp(resizable, L"false") == 0) {
        style &= ~WS_SIZEBOX;         // Bloquea resize manual
        style &= ~WS_MAXIMIZEBOX;     // Desactiva botón de maximizar
    }

    


    // Crear ventana
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        title,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr,
        hInstance,
        nullptr
    );
    SetDynamicIcon(hwnd, L"custom.ico");

    // Aplicar fullscreen si corresponde
    if (wcscmp(fullscreen, L"true") == 0) {
        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(hwnd, HWND_TOP, 0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }





    if (!hwnd) {
        MessageBox(nullptr, L"No se pudo crear la ventana principal.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Bucle principal de mensajes
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Liberar recursos de COM.
    CoUninitialize();

    return 0;
}

// Procedimiento principal de ventana.
// (Gestiona la creación, redimensionado y destrucción de la ventana)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        InitializeWebView2(hwnd);
        return 0;
    case WM_SIZE:
        ResizeWebView2(hwnd);
        return 0;
    case WM_CLOSE:
        // Liberar instancias antes de destruir la ventana.
        g_webView = nullptr;
        g_webViewController = nullptr;
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}




// Inicialización de WebView2 y carga del HTML desde el recurso
// + Devuelve la carpeta donde está el .exe
std::wstring GetExeDir()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring fullPath(path);
    size_t pos = fullPath.find_last_of(L"\\/");
    return fullPath.substr(0, pos);
}






void InitializeWebView2(HWND hwnd)

{
    

    // (Crear opciones con el flag para permitir acceso a archivos locales)
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    // Habilita aceleración por GPU para WebView2
    options->put_AdditionalBrowserArguments(
        L"--allow-file-access-from-files "
        L"--enable-gpu "
        L"--enable-webgl "
        L"--ignore-gpu-blocklist "
        L"--use-angle=gl "
        L"--enable-zero-copy "
        L"--enable-gpu-rasterization "
        L"--enable-native-gpu-memory-buffers "
        L"--autoplay-policy=no-user-gesture-required "
        L"--enable-features=Fullscreen"
    );

    // Usar options.Get en vez de nullptr
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, options.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (FAILED(result) || !env) {
                    MessageBox(hwnd, L"Error al crear el entorno WebView2.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                    return result;
                }



                env->CreateCoreWebView2Controller(
                    hwnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (FAILED(result) || !controller) {
                                wchar_t buf[128];
                                swprintf_s(buf, L"CreateCoreWebView2Controller HR=0x%08X", result);
                                OutputDebugString(buf);
                                MessageBox(hwnd, L"Error al crear el controlador WebView2.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                                return result;
                            }



                            g_webViewController = controller;
                                                     

                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            g_webViewController->put_Bounds(bounds);

                            wil::com_ptr<ICoreWebView2> webview;
                            controller->get_CoreWebView2(&webview);
                            g_webView = webview;

                            // Settings del WebView: habilitar APIs necesarias para Construct 2 Browser
                            wil::com_ptr<ICoreWebView2Settings> settings;
                            g_webView->get_Settings(&settings);

                            settings->put_IsScriptEnabled(TRUE);                 // ExecJS / eval
                            settings->put_AreDefaultScriptDialogsEnabled(TRUE);  // alert/confirm/prompt
                            settings->put_IsWebMessageEnabled(TRUE);             // comunicación nativa opcional

                            // Opcionales (utiles en desarrollo)
                            settings->put_AreDevToolsEnabled(TRUE);
                            settings->put_IsZoomControlEnabled(TRUE);

                            // Leer argumento html "ruta_relativa" si existe
                            std::wstring cmd = GetCommandLineW();
                            size_t p = cmd.find(L"--html");
                            if (p != std::wstring::npos) {
                                // Buscar el primer par de comillas tras --html
                                size_t q1 = cmd.find(L"\"", p);
                                size_t q2 = (q1 != std::wstring::npos) ? cmd.find(L"\"", q1 + 1) : std::wstring::npos;
                                if (q1 != std::wstring::npos && q2 != std::wstring::npos && q2 > q1) {
                                    g_startHtmlRelPath = cmd.substr(q1 + 1, q2 - q1 - 1);
                                }
                            }

                            g_webView->add_NewWindowRequested(
                                Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
                                        wil::unique_cotaskmem_string uri;
                                        args->get_Uri(&uri);
                                        args->put_Handled(TRUE);
                                        sender->Navigate(uri ? uri.get() : L"about:blank");
                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );


                            // Construir ruta local al HTML inicial
                            std::wstring exeDir = GetExeDir();
                            std::wstring ruta = exeDir + L"\\" + (g_startHtmlRelPath.empty() ? L"_game\\index.html" : g_startHtmlRelPath);

                            // Convertir a formato file:///
                            std::wstring url = L"file:///" + ruta;
                            for (auto& ch : url) if (ch == L'\\') ch = L'/';

                            if (g_webView) {
                                g_webView->Navigate(url.c_str());
                            }
                            else {
                                MessageBox(hwnd, L"No se pudo inicializar WebView2.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                            }

                            // Evento de depuración + bloqueo de menú contextual
                            g_webView->add_NavigationCompleted(
                                Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        BOOL isSuccess = FALSE;
                                        HRESULT hr = args->get_IsSuccess(&isSuccess);
                                        if (SUCCEEDED(hr) && isSuccess) {
                                            OutputDebugString(L"Navegación completada correctamente.\n");
                                        }
                                        else {
                                            OutputDebugString(L"Error en la navegación.\n");
                                        }
                                        sender->ExecuteScript(
                                            LR"(document.addEventListener('contextmenu', event => event.preventDefault());)",
                                            nullptr
                                        );
                                       
                          

                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );

                            // Mensajes Web
                            g_webView->add_WebMessageReceived(
                                Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [hwnd](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        // 1. Obtener el JSON del mensaje
                                        wil::unique_cotaskmem_string raw;
                                        args->get_WebMessageAsJson(&raw);
                                        std::wstring message = raw.get();

                                        // 2. Accion: getWindowPosition
                                        if (message.find(L"getWindowPosition") != std::wstring::npos) {
                                            RECT rc;
                                            GetWindowRect(hwnd, &rc);
                                            std::wstring resp =
                                                L"{\"x\":" + std::to_wstring(rc.left) +
                                                L",\"y\":" + std::to_wstring(rc.top) + L"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        }
                                        // 3. Accion: getWindowSize
                                        else if (message.find(L"getWindowSize") != std::wstring::npos) {
                                            RECT rc;
                                            GetClientRect(hwnd, &rc);
                                            int w = rc.right - rc.left;
                                            int h = rc.bottom - rc.top;
                                            std::wstring resp =
                                                L"{\"width\":" + std::to_wstring(w) +
                                                L",\"height\":" + std::to_wstring(h) + L"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        }
                                        else if (message.find(L"get_document_in_pc_direction") != std::wstring::npos) {
                                            std::wstring dir = GetExeDir();
                                            for (auto& ch : dir) if (ch == L'\\') ch = L'/';
                                            std::wstring resp = L"{\"exeDir\":\"file:///" + dir + L"\"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        
                                        }

                                        // 5. Accion: setWindowTitle
                                        else if (message.find(L"setWindowTitle") != std::wstring::npos) {
                                            // Extraer value del JSON de forma simple
                                            // Espera un mensaje como: {"action":"setWindowTitle","value":"eureka"}
                                            size_t keyPos = message.find(L"\"value\"");
                                            if (keyPos != std::wstring::npos) {
                                                size_t colon = message.find(L":", keyPos);
                                                size_t firstQuote = message.find(L"\"", colon + 1);
                                                size_t secondQuote = (firstQuote != std::wstring::npos)
                                                    ? message.find(L"\"", firstQuote + 1)
                                                    : std::wstring::npos;
                                                if (firstQuote != std::wstring::npos && secondQuote != std::wstring::npos && secondQuote > firstQuote) {
                                                    std::wstring title = message.substr(firstQuote + 1, secondQuote - (firstQuote + 1));
                                                    SetWindowText(hwnd, title.c_str());
                                                    std::wstring resp = L"{\"ok\":true,\"title\":\"" + title + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                                else {
                                                    sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"No title value\"}");
                                                }
                                            }
                                            else {
                                                sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"Missing value key\"}");
                                            }
                                        }

                                        // 6. Accion: closeWindow
                                        else if (message.find(L"closeWindow") != std::wstring::npos) {
                                            // Enviar respuesta antes de cerrar
                                            sender->PostWebMessageAsString(L"{\"closing\":true}");
                                            PostMessage(hwnd, WM_CLOSE, 0, 0);
                                        }

                                        // Accion: setWindowPosition EX : {"action":"setWindowPosition","x":100,"y":200}
                                        else if (message.find(L"setWindowPosition") != std::wstring::npos) {
                                            int x = 0, y = 0;
                                            // Formato esperado 
                                            swscanf_s(message.c_str(), L"{\"action\":\"setWindowPosition\",\"x\":%d,\"y\":%d}", &x, &y);

                                            SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                                            std::wstring resp = L"{\"ok\":true,\"x\":" + std::to_wstring(x) + L",\"y\":" + std::to_wstring(y) + L"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        }

                                        // Accion: setWindowSize EX: {"action":"setWindowSize","width":800,"height":600}
                                        else if (message.find(L"setWindowSize") != std::wstring::npos) {
                                            int w = 0, h = 0;
                                            swscanf_s(message.c_str(), L"{\"action\":\"setWindowSize\",\"width\":%d,\"height\":%d}", &w, &h);

                                            SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
                                            std::wstring resp = L"{\"ok\":true,\"width\":" + std::to_wstring(w) + L",\"height\":" + std::to_wstring(h) + L"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        }

                                        // Acción: toggleFullscreen {"action":"toggleFullscreen"}
                                        // Devuelve {"fullscreen":"on"} o {"fullscreen":"off"} y restaura estado previo al salir
                                        else if (message.find(L"toggleFullscreen") != std::wstring::npos) {
                                            if (!g_isFullscreen) {
                                                // Guardar estado actual
                                                g_windowedStyle = GetWindowLong(hwnd, GWL_STYLE);
                                                GetWindowRect(hwnd, &g_windowedRect);

                                                // Entrar a fullscreen (WS_POPUP a tamaño de pantalla)
                                                SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
                                                SetWindowPos(hwnd, HWND_TOP, 0, 0,
                                                    GetSystemMetrics(SM_CXSCREEN),
                                                    GetSystemMetrics(SM_CYSCREEN),
                                                    SWP_FRAMECHANGED | SWP_SHOWWINDOW);

                                                g_isFullscreen = true;
                                                sender->PostWebMessageAsString(L"{\"fullscreen\":\"on\"}");
                                            }
                                            else {
                                                // Salir de fullscreen restaurando estilo y rect previos
                                                SetWindowLong(hwnd, GWL_STYLE, g_windowedStyle);
                                                SetWindowPos(hwnd, HWND_TOP,
                                                    g_windowedRect.left, g_windowedRect.top,
                                                    g_windowedRect.right - g_windowedRect.left,
                                                    g_windowedRect.bottom - g_windowedRect.top,
                                                    SWP_FRAMECHANGED | SWP_SHOWWINDOW);

                                                g_isFullscreen = false;
                                                sender->PostWebMessageAsString(L"{\"fullscreen\":\"off\"}");
                                            }
                                        }

                                        // 7. Accion: readFile EX:  {"action":"readFile","path":"C:/Users/.../file.txt"}
                                        else if (message.find(L"readFile") != std::wstring::npos) {
                                            std::wstring path = ExtractJsonString(message, L"path");
                                            if (path.empty()) {
                                                sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"MissingPath\"}");
                                            }
                                            else {
                                                // Permitir barras invertidas en json enviando escapadas
                                                DWORD bytes = 0; std::wstring content, err;
                                                if (ReadFileWide(path, content, bytes, err)) {
                                                    std::wstring resp = L"{\"ok\":true,\"bytes\":" + std::to_wstring(bytes) +
                                                        L",\"path\":\"" + JsonEscape(path) + L"\",\"content\":\"" + JsonEscape(content) + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                                else {
                                                    std::wstring resp = L"{\"ok\":false,\"error\":\"" + err + L"\",\"path\":\"" + JsonEscape(path) + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                            }
                                        }

                                        // 8. Accion: writeFile  EX: {"action":"writeFile","path":"C:/.../file.txt","content":"..."}
                                        else if (message.find(L"writeFile") != std::wstring::npos) {
                                            std::wstring path = ExtractJsonString(message, L"path");
                                            std::wstring content = ExtractJsonString(message, L"content");
                                            if (path.empty()) {
                                                sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"MissingPath\"}");
                                            }
                                            else {
                                                DWORD bytes = 0; std::wstring err;
                                                if (WriteFileWide(path, content, bytes, err)) {
                                                    g_lastWrittenPath = path;
                                                    g_lastWrittenContent = content;
                                                    std::wstring resp = L"{\"ok\":true,\"bytes\":" + std::to_wstring(bytes) +
                                                        L",\"path\":\"" + JsonEscape(path) + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                                else {
                                                    std::wstring resp = L"{\"ok\":false,\"error\":\"" + err + L"\",\"path\":\"" + JsonEscape(path) + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                            }
                                        }

                                        // 9. Accion: LastFileWriteContent  EX:  {"action":"LastFileWriteContent"}
                                        else if (message.find(L"LastFileWriteContent") != std::wstring::npos) {
                                            std::wstring resp = L"{\"ok\":true,\"path\":\"" + JsonEscape(g_lastWrittenPath) +
                                                L"\",\"content\":\"" + JsonEscape(g_lastWrittenContent) + L"\"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                        }

                                        // Accion: newWindow EX: {"action":"newWindow","path":"_game\\index2.html"}
                                        // Lanza nueva instancia de catcher, pasando --html "ruta" para que abra ese HTML.
                                        else if (message.find(L"newWindow") != std::wstring::npos) {
                                            // Ejecutable actual
                                            wchar_t exePath[MAX_PATH];
                                            GetModuleFileNameW(NULL, exePath, MAX_PATH);

                                            // Extraer ruta relativa (opcional)
                                            std::wstring relPath = ExtractJsonString(message, L"path");

                                            // Construir parámetros (--html "ruta")
                                            std::wstring params = L"";
                                            if (!relPath.empty()) {
                                                params = L"--html \"" + relPath + L"\"";
                                            }

                                            HINSTANCE hres = ShellExecuteW(NULL, L"open", exePath,
                                                params.empty() ? NULL : params.c_str(),
                                                NULL, SW_SHOWNORMAL);

                                            bool ok = ((INT_PTR)hres) > 32;
                                            std::wstring resp = ok
                                                ? L"{\"ok\":true,\"newWindow\":true}"
                                                : L"{\"ok\":false,\"error\":\"LaunchFailed\"}";
                                            sender->PostWebMessageAsString(resp.c_str());
                                            }

                                            // Accion: callBrowser {"action":"callBrowser","url":"https://loquesea.tld/path"}
                                            // Abre la urll exacta que llega en el mensaje con el navegador por defecto.
                                        else if (message.find(L"callBrowser") != std::wstring::npos) {
                                                std::wstring url = ExtractJsonString(message, L"url");
                                                if (url.empty()) {
                                                    sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"MissingUrl\"}");
                                                }
                                                else {
                                                    HINSTANCE hres = ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
                                                    bool ok = ((INT_PTR)hres) > 32;
                                                    std::wstring resp = ok
                                                        ? L"{\"ok\":true,\"opened\":\"" + JsonEscape(url) + L"\"}"
                                                        : L"{\"ok\":false,\"error\":\"OpenUrlFailed\",\"url\":\"" + JsonEscape(url) + L"\"}";
                                                    sender->PostWebMessageAsString(resp.c_str());
                                                }
                                                }

                                                // Accion: openFile {"action":"openFile","path":"C:\\Users\\jhondoe\\Desktop\\hi.ink"}
                                                // (Abre el archivo con su aplicacion asociada del sistema)
                                        else if (message.find(L"openFile") != std::wstring::npos) {
                                                    std::wstring path = ExtractJsonString(message, L"path");
                                                    if (path.empty()) {
                                                        sender->PostWebMessageAsString(L"{\"ok\":false,\"error\":\"MissingPath\"}");
                                                    }
                                                    else {
                                                        HINSTANCE hres = ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                                                        bool ok = ((INT_PTR)hres) > 32;
                                                        std::wstring resp = ok
                                                            ? L"{\"ok\":true,\"opened\":\"" + JsonEscape(path) + L"\"}"
                                                            : L"{\"ok\":false,\"error\":\"OpenFileFailed\",\"path\":\"" + JsonEscape(path) + L"\"}";
                                                        sender->PostWebMessageAsString(resp.c_str());
                                                    }
                                                    }





                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );





                            return S_OK;
                        }
                    ).Get()
                );

                return S_OK;
            }
        ).Get()
    );


    if (FAILED(hr)) {
        MessageBox(hwnd, L"Fallo en la llamada a CreateCoreWebView2EnvironmentWithOptions.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
    }
}

// Ajuste del area visible de WebView2 al area cliente de la ventana.
void ResizeWebView2(HWND hwnd)
{
    if (g_webViewController) {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        g_webViewController->put_Bounds(bounds);
    }
}

// Funcion para cargar un recurso HTML embebido y devolverlo como wstring.
// (Asume que el HTML esta embebido en UTF-8)
std::wstring LoadHtmlResource(int resourceId, HINSTANCE hInstance)
{
    HRSRC hResInfo = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_HTML);
    if (!hResInfo) return L"";

    HGLOBAL hResData = LoadResource(hInstance, hResInfo);
    if (!hResData) return L"";

    DWORD resSize = SizeofResource(hInstance, hResInfo);
    const void* pResData = LockResource(hResData);

    if (!pResData || resSize == 0) return L"";

    // Convertir de UTF-8 a wstring UTF-16
    std::string htmlAnsi((const char*)pResData, resSize);

    int wlen = MultiByteToWideChar(CP_UTF8, 0,
        htmlAnsi.c_str(), (int)htmlAnsi.size(),
        nullptr, 0);

    if (wlen <= 0)
        return L""; // No se pudo decodificar

    std::wstring htmlWide(wlen, 0);

    // Usamos &htmlWide[0] para evitar problemas con const-correctness
    MultiByteToWideChar(CP_UTF8, 0,
        htmlAnsi.c_str(), (int)htmlAnsi.size(),
        &htmlWide[0], wlen);

    return htmlWide;
}

// El codigo original de este software ademas de su diseno y concepcion es de : Sebastian.G.Romanelli
// Este software posee una licencia MIT lo que permite su libre modifcacion y distribucion
// Copyright (c) 2026 NZXY-Project
