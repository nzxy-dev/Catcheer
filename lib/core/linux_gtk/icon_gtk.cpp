#include "icon_gtk.h"
#include <iostream>

void SetWindowIcon(GtkWindow* window, const std::string& iconPath) {
    GError* error = NULL;
    
    // load file 
    if (!gtk_window_set_icon_from_file(window, iconPath.c_str(), &error)) {
        std::cerr << "[GTK] Failed set_icon_from_file: " << (error ? error->message : "Unknown") << std::endl;
        if (error) g_error_free(error);
        error = NULL;
    }
}
