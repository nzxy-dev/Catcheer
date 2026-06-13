/*
 * ===============|==========================================================|
 *   Catcheer     | Linux core main-module (gtk) v1.04                       |
 * _______________|__________________________________________________________|
 */



//__________________________________________________________________________|
//                                                                          |
//----------------- INCLUDES -----------------------------------------------|
//__________________________________________________________________________|

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <iostream>
#include "../../api-service/api.h"
#include "icon_gtk.h"
#include "performance_linux.h"

//--------------------------------------------------------------------------|


//----------------------------vars|
std::string g_htmlToLoad;

//--------------------------------|


//---------------new window handler-------------------|
void on_new_window_message(const std::string& relPath);



//__________________________________________________________________________|
//                                                                          |
//----------------- API RECEIVER--------------------------------------------|
//__________________________________________________________________________|

// helper
static GtkWindow* GetWindowFromWebView(WebKitWebView* webview) {
    GtkWidget* widget = GTK_WIDGET(webview);
    GtkWidget* top = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    return top ? GTK_WINDOW(top) : NULL;
}

static void on_script_message_received(WebKitUserContentManager* manager,WebKitJavascriptResult* res, gpointer user_data) 
{
    WebKitWebView* webview = WEBKIT_WEB_VIEW(user_data);
    GtkWindow* gtk_window = GetWindowFromWebView(webview);
    if (!gtk_window) return;

    JSCValue* value = webkit_javascript_result_get_js_value(res);
    gchar* str_value = jsc_value_to_string(value);
    std::string message(str_value);
    g_free(str_value);

    std::cout << "[JS -> C++] Message: " << message << std::endl;

    // 1. try module "api-service"
    std::string response = handle_web_message(message);

    if (!response.empty()) {
        std::cout << "[C++ -> JS] API Response: " << response << std::endl;
        std::string js = "if(window.onCatcheerMessage) window.onCatcheerMessage(" + response + ");";
        webkit_web_view_run_javascript(webview, js.c_str(), NULL, NULL, NULL);
    } else {
        // 2. windows actions not handled by api module
        std::string action = ExtractJsonString(message, "action");
        std::cout << "[DEBUG] Action detected: " << action << std::endl;

        // set window title
        if (action == "setWindowTitle") {
            std::string title = ExtractJsonString(message, "value");
            if (!title.empty()) {
                g_idle_add([](gpointer data) -> gboolean {
                    auto pair = static_cast<std::pair<GtkWindow*, std::string>*>(data);
                    gtk_window_set_title(pair->first, pair->second.c_str());
                    delete pair;
                    return FALSE;
                }, new std::pair<GtkWindow*, std::string>(gtk_window, title));
            }
        }

        // close window
	else if (action == "closeWindow") {
		g_idle_add([](gpointer data) -> gboolean {
			GtkWindow* win = static_cast<GtkWindow*>(data);
			gtk_window_close(win);
			return FALSE;
		}, gtk_window);
	}
	
        //fullscreen
        else if (action == "toggleFullscreen") {
            static bool is_full = false;
            g_idle_add([](gpointer data) -> gboolean {
                GtkWindow* win = static_cast<GtkWindow*>(data);
                static bool* flag = &is_full;
                if (!*flag) gtk_window_fullscreen(win);
                else gtk_window_unfullscreen(win);
                *flag = !*flag;
                return FALSE;
            }, gtk_window);
        }

        // set window sz
        else if (action == "setWindowSize") {
            int w = std::stoi(ExtractJsonString(message, "width"));
            int h = std::stoi(ExtractJsonString(message, "height"));
            if (w > 0 && h > 0) {
                g_idle_add([](gpointer data) -> gboolean {
                    auto p = static_cast<std::tuple<GtkWindow*,int,int>*>(data);
                    gtk_window_resize(std::get<0>(*p), std::get<1>(*p), std::get<2>(*p));
                    delete p;
                    return FALSE;
                }, new std::tuple<GtkWindow*,int,int>(gtk_window, w, h));
            }
        }

        //set windows pos
        else if (action == "setWindowPosition") {
            int x = std::stoi(ExtractJsonString(message, "x"));
            int y = std::stoi(ExtractJsonString(message, "y"));
            g_idle_add([](gpointer data) -> gboolean {
                auto p = static_cast<std::tuple<GtkWindow*,int,int>*>(data);
                gtk_window_move(std::get<0>(*p), std::get<1>(*p), std::get<2>(*p));
                delete p;
                return FALSE;
            }, new std::tuple<GtkWindow*,int,int>(gtk_window, x, y));
        }

        //get windows pos
        else if (action == "getWindowPosition") {
            int x, y;
            gtk_window_get_position(gtk_window, &x, &y);
            std::string resp = "{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y) + "}";
            std::string js = "if(window.onCatcheerMessage) window.onCatcheerMessage(" + resp + ");";
            webkit_web_view_run_javascript(webview, js.c_str(), NULL, NULL, NULL);
        }
        
        
         //new window    
	   else if (action == "newWindow") {
       std::string rel = ExtractJsonString(message, "path");
        if (!rel.empty()) {
        on_new_window_message(rel);//handler call
       }
       return;
}



        // get window size
        else if (action == "getWindowSize") {
            int w, h;
            gtk_window_get_size(gtk_window, &w, &h);
            std::string resp = "{\"width\":" + std::to_string(w) + ",\"height\":" + std::to_string(h) + "}";
            std::string js = "if(window.onCatcheerMessage) window.onCatcheerMessage(" + resp + ");";
            webkit_web_view_run_javascript(webview, js.c_str(), NULL, NULL, NULL);
        }
    }
}


//-----------------------------------------------------------------------------------------------------------------|



// Load error webkir debug
static gboolean on_load_failed(WebKitWebView* webview, WebKitLoadEvent load_event, gchar* failing_uri, GError* error, gpointer user_data) {
    std::cerr << "!!! Error in load: " << failing_uri << " -> " << error->message << std::endl;
    return FALSE;
}


GtkWidget* CreateCatcheerWindow(const std::string& htmlPath) {

    AppConfig config = LoadAppConfig();

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), config.width, config.height);
    gtk_window_set_title(GTK_WINDOW(window), config.title.c_str());

    //------------------set icon|
    SetWindowIcon(GTK_WINDOW(window), GetExeDirShared() + "/custom.png");

    if (config.fullscreen) gtk_window_fullscreen(GTK_WINDOW(window));
    if (!config.resizable) gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    if (!config.border) gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    //===========================|webkit settings|==================================|
    WebKitSettings* settings = webkit_settings_new();//                             |
    webkit_settings_set_allow_file_access_from_file_urls(settings, TRUE);//         |//file acces
    webkit_settings_set_allow_universal_access_from_file_urls(settings, TRUE);//    |
    webkit_settings_set_enable_webgl(settings, TRUE);//                             |//webgl
    webkit_settings_set_enable_javascript(settings, TRUE);//                        |//javascript
    webkit_settings_set_enable_developer_extras(settings, TRUE);//                  |//developer console
    webkit_settings_set_enable_write_console_messages_to_stdout(settings, TRUE);//  |    
    // audio flags------------------------------------------------------------------|
    webkit_settings_set_enable_media_stream(settings, TRUE);//                      |//enable audio
    webkit_settings_set_enable_mediasource(settings, TRUE);//                       |
    //==============================================================================|

    WebKitUserContentManager* manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(manager, "catcheer");

    const char* shim =
        "if(!window.chrome) window.chrome = {};"
        "if(!window.chrome.webview) window.chrome.webview = {};"
        "window.chrome.webview.postMessage = function(m) {"
        "  var s = (typeof m === 'string') ? m : JSON.stringify(m);"
        "  window.webkit.messageHandlers.catcheer.postMessage(s);"
        "};"
        "window.chrome.webview._listeners = [];"
        "window.chrome.webview.addEventListener = function(t, l) { if(t==='message') this._listeners.push(l); };"
        "window.onCatcheerMessage = function(data) {"
        "  var evt = { data: data };"
        "  if(window.chrome.webview.onmessage) window.chrome.webview.onmessage(evt);"
        "  window.chrome.webview._listeners.forEach(l => l(evt));"
        "};"
        "console.log('Catcheer Shim Ready');";

    WebKitUserScript* script = webkit_user_script_new(
        shim,
        WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END,
        NULL,
        NULL
    );
    webkit_user_content_manager_add_script(manager, script);

    GtkWidget* webview = webkit_web_view_new_with_user_content_manager(manager);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), settings);

    g_signal_connect(manager, "script-message-received::catcheer",
                     G_CALLBACK(on_script_message_received), webview);

    g_signal_connect(webview, "load-failed",
                     G_CALLBACK(on_load_failed), NULL);

    g_signal_connect(webview, "context-menu",
                     G_CALLBACK(gtk_true), NULL);

    gtk_container_add(GTK_CONTAINER(window), webview);

    std::string full = "file://" + GetExeDirShared() + "/" + htmlPath;
    for (char& c : full) if (c == '\\') c = '/';

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), full.c_str());

    gtk_widget_show_all(window);

    return window;
}

//new window message function
void on_new_window_message(const std::string& relPath) {
    std::string clean = relPath;
    if (!clean.empty() && clean[0] == '/')
        clean = clean.substr(1);

    g_idle_add([](gpointer data) -> gboolean {
        std::string* p = static_cast<std::string*>(data);
        CreateCatcheerWindow(*p);
        delete p;
        return FALSE;
    }, new std::string(clean));
}


//__________________________________________________________________________|
//                                                                          |
//----------------- MAIN ---------------------------------------------------|
//__________________________________________________________________________|

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    LinuxBooster::Boost();
    //------------------------html file load
    CreateCatcheerWindow("source/index.html");
    gtk_main();
    return 0;
}




