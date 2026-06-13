#ifndef ICON_GTK_H
#define ICON_GTK_H

#include <gtk/gtk.h>
#include <string>

// load icon
void SetWindowIcon(GtkWindow* window, const std::string& iconPath);

#endif // ICON_GTK_H
