#include "api.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
// ini parser helper
std::string GetIniValue(const std::string& content, const std::string& key) {
    size_t pos = content.find(key + " =");
    if (pos == std::string::npos) pos = content.find(key + "=");
    if (pos == std::string::npos) return "";

    size_t start = content.find("=", pos) + 1;
    size_t end = content.find_first_of("\r\n", start);
    
    std::string val = content.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
    // trim
    val.erase(0, val.find_first_not_of(" \t"));
    val.erase(val.find_last_not_of(" \t") + 1);
    return val;
}
#endif

AppConfig LoadAppConfig() {
    AppConfig config;
    std::string exeDir = GetExeDirShared();
    std::string content, err;

#ifdef _WIN32
    // config.ini for Windows
    std::string configPath = exeDir + "/config.ini";
    
    if (ReadFileSimple(configPath, content, err)) {
        std::cout << "[Config] Loading from config.ini" << std::endl;
        std::string t = GetIniValue(content, "title");
        if (!t.empty()) config.title = t;

        std::string s = GetIniValue(content, "size");
        if (!s.empty()) {
            size_t x = s.find('x');
            if (x != std::string::npos) {
                config.width = std::stoi(s.substr(0, x));
                config.height = std::stoi(s.substr(x + 1));
            }
        }

        std::string f = GetIniValue(content, "Forcefullscreen");
        if (f.empty()) f = GetIniValue(content, "fullscreen");
        if (!f.empty()) config.fullscreen = (f == "true" || f == "1");

        std::string r = GetIniValue(content, "resizable");
        if (!r.empty()) config.resizable = (r == "true" || r == "1");

        std::string b = GetIniValue(content, "border");
        if (!b.empty()) config.border = (b == "true" || b == "1");
    }
#else
    // config.json for Linux
    std::string configPath = exeDir + "/config.json";
    if (ReadFileSimple(configPath, content, err)) {
        std::cout << "[Config] Loading from config.json" << std::endl;
        std::string t = ExtractJsonString(content, "title");
        if (!t.empty()) config.title = t;
        
        std::string w = ExtractJsonString(content, "width");
        if (!w.empty()) config.width = std::stoi(w);
        
        std::string h = ExtractJsonString(content, "height");
        if (!h.empty()) config.height = std::stoi(h);

        std::string f = ExtractJsonString(content, "fullscreen");
        config.fullscreen = (f == "true");

        std::string r = ExtractJsonString(content, "resizable");
        if (!r.empty()) config.resizable = (r == "true");
        
        std::string b = ExtractJsonString(content, "border");
        if (!b.empty()) config.border = (b == "true");
    }
#endif

    return config;
}
