#include "api.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

// ---------------- JSON HELPERS ----------------

std::string JsonEscape(const std::string& s) {
    std::string o;
    o.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
        case '\\': o += "\\\\"; break;
        case '"':  o += "\\\""; break;
        case '\n': o += "\\n";  break;
        case '\r': o += "\\r";  break;
        case '\t': o += "\\t";  break;
        default:    o.push_back(c); break;
        }
    }
    return o;
}

std::string ExtractJsonString(const std::string& msg, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = msg.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = msg.find(":", keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    size_t start = msg.find_first_not_of(" \t\n\r", colonPos + 1);
    if (start == std::string::npos) return "";

    if (msg[start] == '"') {
        size_t end = msg.find("\"", start + 1);
        if (end == std::string::npos) return "";
        return msg.substr(start + 1, end - start - 1);
    } else {
        size_t end = msg.find_first_of(",} \t\n\r", start);
        if (end == std::string::npos) return msg.substr(start);
        return msg.substr(start, end - start);
    }
}

// ---------------- FILE HELPERS ----------------

bool ReadFileSimple(const std::string& path, std::string& outContent, std::string& outError) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) { outError = "OpenFailed"; return false; }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        outContent.assign(buffer.data(), size);
        return true;
    }
    outError = "ReadFailed";
    return false;
}

bool WriteFileSimple(const std::string& path, const std::string& content, std::string& outError) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) { outError = "CreateFailed"; return false; }
    file << content;
    return true;
}

// ---------------- MESSAGE HANDLER ----------------

extern std::string GetExeDirShared();
extern bool OpenSystemFile(const std::string& path);

std::string handle_web_message(const std::string& message) {
    std::string action = ExtractJsonString(message, "action");
    if (action.empty()) return "";

    if (action == "get_document_in_pc_direction") {
        std::string dir = GetExeDirShared();
        for (auto& ch : dir) if (ch == '\\') ch = '/';
        return "{\"exeDir\":\"file:///" + dir + "\"}";
    }

    else if (action == "readFile") {
        std::string path = ExtractJsonString(message, "path");
        std::string content, err;
        if (ReadFileSimple(path, content, err)) {
            return "{\"ok\":true,\"content\":\"" + JsonEscape(content) + "\",\"path\":\"" + JsonEscape(path) + "\"}";
        } else {
            return "{\"ok\":false,\"error\":\"" + err + "\",\"path\":\"" + JsonEscape(path) + "\"}";
        }
    }

    else if (action == "writeFile") {
        std::string path = ExtractJsonString(message, "path");
        std::string content = ExtractJsonString(message, "content");
        std::string err;
        if (WriteFileSimple(path, content, err)) {
            return "{\"ok\":true,\"path\":\"" + JsonEscape(path) + "\"}";
        } else {
            return "{\"ok\":false,\"error\":\"" + err + "\",\"path\":\"" + JsonEscape(path) + "\"}";
        }
    }

    else if (action == "callBrowser" || action == "openFile") {
        std::string urlOrPath = ExtractJsonString(message, "url");
        if (urlOrPath.empty()) urlOrPath = ExtractJsonString(message, "path");

        if (urlOrPath.empty()) return "{\"ok\":false,\"error\":\"MissingTarget\"}";

        if (OpenSystemFile(urlOrPath)) {
            return "{\"ok\":true,\"opened\":\"" + JsonEscape(urlOrPath) + "\"}";
        } else {
            return "{\"ok\":false,\"error\":\"OpenFailed\",\"target\":\"" + JsonEscape(urlOrPath) + "\"}";
        }
    }

    return "";
}
