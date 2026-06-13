
cr.plugins_.catcher = function(runtime) {
    this.runtime = runtime;
};

(function () {
    const pluginProto = cr.plugins_.catcher.prototype;

    pluginProto.Type = function(plugin) {
        this.plugin = plugin;
        this.runtime = plugin.runtime;
    };
    pluginProto.Type.prototype.onCreate = function() {};

    pluginProto.Instance = function(type) {
        this.type = type;
        this.runtime = type.runtime;

        this.windowX = 0;
        this.windowY = 0;
        this.windowW = 0;
        this.windowH = 0;
        this.documentPath = "";
        this.lastFileWriteContent = "";
        this.lastFileWritePath = "";
        this.lastFileReadContent = "";
        this.lastFileReadPath = "";
        this.lastError = "";
        this.isFullscreen = false;

        const self = this;

        // Message listener (Catcher)
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.addEventListener("message", e => {
                let msg = e.data;

                if (typeof msg === "string") {
                    try { msg = JSON.parse(msg); }
                    catch (err) { console.log("Catcher parse fail:", msg); return; }
                }
                if (!msg || typeof msg !== "object") return;

                // Document path (host returns {"exeDir":"file:///..."})
                if (msg && typeof msg.exeDir === "string") {
                    self.documentPath = msg.exeDir;
                    self.runtime.trigger(cr.plugins_.catcher.prototype.cnds.OnDocumentPathReceived, self);
                    return;
                }

                if (typeof msg.x === "number" && typeof msg.y === "number") {
                    self.windowX = (msg.x | 0);
                    self.windowY = (msg.y | 0);
                    self.runtime.trigger(cr.plugins_.catcher.prototype.cnds.OnWindowPositionReceived, self);
                    return;
                }

                if (typeof msg.width === "number" && typeof msg.height === "number") {
                    self.windowW = (msg.width | 0);
                    self.windowH = (msg.height | 0);
                    self.runtime.trigger(cr.plugins_.catcher.prototype.cnds.OnWindowSizeReceived, self);
                    return;
                }

                // Fullscreen reply {"fullscreen":"on"|"off"}
                if (typeof msg.fullscreen === "string") {
                    self.isFullscreen = (msg.fullscreen === "on");
                    return;
                }

                // ReadFile reply {"ok":true,"bytes":N,"path":"...","content":"..."} or error {"ok":false,"error":"...","path":"..."}
                if (typeof msg.ok === "boolean" && typeof msg.path === "string" && (typeof msg.content === "string" || typeof msg.error === "string")) {
                    if (msg.ok && typeof msg.content === "string") {
                        self.lastFileReadPath = msg.path;
                        self.lastFileReadContent = msg.content;
                        self.lastError = "";
                    } else {
                        self.lastFileReadPath = msg.path;
                        self.lastFileReadContent = "";
                        self.lastError = String(msg.error || "ReadFileError");
                    }
                    self.runtime.trigger(cr.plugins_.catcher.prototype.cnds.OnFileRead, self);
                    return;
                }

                // WriteFile confirmation {"ok":true,"bytes":N,"path":"..."} or error {"ok":false,"error":"...","path":"..."}
                if (typeof msg.ok === "boolean" && typeof msg.path === "string" && typeof msg.bytes === "number") {
                    self.lastFileWritePath = msg.path;
                    // Keep lastFileWriteContent as the value set when sending Acts.writeFile
                    self.lastError = msg.ok ? "" : String(msg.error || "WriteFileError");
                    self.runtime.trigger(cr.plugins_.catcher.prototype.cnds.OnFileWritten, self);
                    return;
                }

                // echo for LastFileWriteContent: {"ok":true,"path":"...","content":"..."}
                if (typeof msg.ok === "boolean" && typeof msg.path === "string" && typeof msg.content === "string") {
                    self.lastFileWritePath = msg.path;
                    self.lastFileWriteContent = msg.content;
                    return;
                }

                // replies (title, closing, opened, newWindow, etc.)
                if (typeof msg.title === "string" || typeof msg.closing === "boolean" || typeof msg.opened === "string" || typeof msg.newWindow === "boolean") {
                    if (typeof msg.error === "string") self.lastError = msg.error;
                    return;
                }
            });
        }
    };

    pluginProto.Instance.prototype.onCreate = function() {};

    // Actions
    const Acts = pluginProto.acts = {};

    Acts.setWindowTitle = function(value) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "setWindowTitle", value: String(value || "") });
    };

    Acts.closeWindow = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "closeWindow" });
    };

    Acts.setWindowPosition = function(x, y) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "setWindowPosition", x: x, y: y });
    };

    Acts.setWindowSize = function(w, h) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "setWindowSize", width: w, height: h });
    };

    Acts.toggleFullscreen = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "toggleFullscreen" });
    };

    Acts.readFile = function(path) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "readFile", path: String(path || "") });
    };

    Acts.writeFile = function(path, content) {
        if (window.chrome && window.chrome.webview) {
            this.lastFileWriteContent = String(content || "");
            window.chrome.webview.postMessage({ action: "writeFile", path: String(path || ""), content: String(content || "") });
        }
    };

    Acts.newWindow = function(path) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "newWindow", path: String(path || "") });
    };

    Acts.callBrowser = function(url) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "callBrowser", url: String(url || "") });
    };

    Acts.openFile = function(path) {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "openFile", path: String(path || "") });
    };

    Acts.getWindowPosition = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "getWindowPosition" });
    };

    Acts.getWindowSize = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "getWindowSize" });
    };

    Acts.getDocumentPath = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "get_document_in_pc_direction" });
    };

    // Request last file write file path (uses same host action "LastFileWriteContent" for compatibility)
    Acts.getLastFileWritePath = function() {
        if (window.chrome && window.chrome.webview)
            window.chrome.webview.postMessage({ action: "LastFileWriteContent" });
    };

    // Legacy placeholder (no-op) to preserve ID 14 compatibility
    Acts.legacyPlaceholder = function() {
        // Intentionally empty for compatibility
    };

    // Conditions
    const Cnds = pluginProto.cnds = {};

    Cnds.OnWindowPositionReceived = function() { return true; };
    Cnds.OnWindowSizeReceived = function() { return true; };
    Cnds.OnDocumentPathReceived = function() { return true; };
    Cnds.OnFileRead = function() { return true; };
    Cnds.OnFileWritten = function() { return true; };
    Cnds.IsFullscreenOn = function() { return this.isFullscreen; };
    Cnds.IsFullscreenOff = function() { return !this.isFullscreen; };

    // Expressions
    const Exps = pluginProto.exps = {};

    Exps.LastFileWriteFilePath = function(ret) {
        ret.set_string(this.lastFileWritePath || "");
    };
    Exps.LastFileReadContent = function(ret) {
        ret.set_string(this.lastFileReadContent || "");
    };
    Exps.WindowX = function(ret) {
        ret.set_int(this.windowX | 0);
    };
    Exps.WindowY = function(ret) {
        ret.set_int(this.windowY | 0);
    };
    Exps.WindowWidth = function(ret) {
        ret.set_int(this.windowW | 0);
    };
    Exps.WindowHeight = function(ret) {
        ret.set_int(this.windowH | 0);
    };
    Exps.DocumentPath = function(ret) {
        ret.set_string(this.documentPath || "");
    };

})();