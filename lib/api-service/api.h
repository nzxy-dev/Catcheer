#ifndef CATCHEER_API_H
#define CATCHEER_API_H

#include <string>
#include <map>

// -------------------------------------mainhandlaah!

std::string handle_web_message(const std::string& message);

// --------------------------------------json helpers

std::string JsonEscape(const std::string& s);
std::string ExtractJsonString(const std::string& msg, const std::string& key);

// --------------------------------------config structure
struct AppConfig {
    std::string title = "Catcheer";
    int width = 1024;
    int height = 768;
    bool fullscreen = false;
    bool resizable = true;
    bool border = true;
};

AppConfig LoadAppConfig();
std::string GetExeDirShared();
bool ReadFileSimple(const std::string& path, std::string& outContent, std::string& outError);


#endif // CATCHEER_API_H
