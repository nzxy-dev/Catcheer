#include <glib.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <unistd.h>
#include <limits.h>
#include <string>

//---------------------------------------------------------------get-exe-dir-full---------------------
std::string GetExeDirShared() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string fullPath = std::string(result, (count > 0) ? count : 0);
    size_t pos = fullPath.find_last_of("/");
    if (pos == std::string::npos) return "./";
    return fullPath.substr(0, pos);
}
//---------------------------------------------------------------open files---------------------
bool OpenSystemFile(const std::string& path) {
    std::string cmd = "xdg-open " + path + " &";
    int r = system(cmd.c_str());
    return (r == 0);
}
