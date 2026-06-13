function GetPluginSettings()
{
    return {
        "name":         "Catcher",
        "id":           "catcher",
        "version":      "1.2",
        "description":  "Catcher plugin",
        "author":       "OLDGAME-Proyect",
        "help url":     "",
        "category":     "System",
        "type":         "object",
        "rotatable":    false,
        "flags":        pf_singleglobal
    };
}

// Actions

// 0 - Set Window Title
AddStringParam("Title", "New window title");
AddAction(0, af_none, "Set Window Title", "Window",
    "Set window title to <i>{0}</i>",
    "Change the window title.", "setWindowTitle");

// 1 - Close Window
AddAction(1, af_none, "Close Window", "Window",
    "Close window",
    "Close the window.", "closeWindow");

// 2 - Set Window Position
AddNumberParam("X", "X position of the window");
AddNumberParam("Y", "Y position of the window");
AddAction(2, af_none, "Set Window Position", "Window",
    "Set window position to X {0}, Y {1}",
    "Move the window.", "setWindowPosition");

// 3 - Set Window Size
AddNumberParam("Width", "Width of the window");
AddNumberParam("Height", "Height of the window");
AddAction(3, af_none, "Set Window Size", "Window",
    "Set window size to W {0}, H {1}",
    "Resize the window.", "setWindowSize");

// 4 - Toggle Fullscreen
AddAction(4, af_none, "Toggle Fullscreen", "Window",
    "Toggle fullscreen",
    "Toggle fullscreen mode.", "toggleFullscreen");

// 5 - Read File
AddStringParam("Path", "Path to the file to read");
AddAction(5, af_none, "Read File", "File",
    "Read file at path <i>{0}</i>",
    "Read a file.", "readFile");

// 6 - Write File
AddStringParam("Path", "Path to the file to write");
AddStringParam("Content", "Content to write");
AddAction(6, af_none, "Write File", "File",
    "Write file at path <i>{0}</i> with content <i>{1}</i>",
    "Write a file.", "writeFile");

// 7 - New Window (HTML game)
AddStringParam("Path", "Relative path to HTML file (e.g. _game/index2.html)");
AddAction(7, af_none, "New Window", "Window",
    "Open new window with HTML <i>{0}</i>",
    "Open a new window.", "newWindow");

// 8 - Call Browser (URL)
AddStringParam("URL", "URL to open in the default browser");
AddAction(8, af_none, "Call Browser", "System",
    "Open URL in browser <i>{0}</i>",
    "Open URL in external browser.", "callBrowser");

// 9 - Open File (Shell Open)
AddStringParam("Path", "Path to the file to open");
AddAction(9, af_none, "Open File", "File",
    "Open file at path <i>{0}</i>",
    "Open file with associated application.", "openFile");

// 10 - Get Window Position (request)
AddAction(10, af_none, "Get Window Position", "Window",
    "Request current window position",
    "Ask Catcher to send current window position.", "getWindowPosition");

// 11 - Get Window Size (request)
AddAction(11, af_none, "Get Window Size", "Window",
    "Request current window size",
    "Ask Catcher to send current window size.", "getWindowSize");

// 12 - Get Document Path (request)
AddAction(12, af_none, "Get Document Path", "System",
    "Request document path (exe directory)",
    "Ask Catcher to send its executable directory.", "getDocumentPath");

// 13 - Get Last File Write File Path (request)
AddAction(13, af_none, "Get Last File Write File Path", "File",
    "Request last file write file path",
    "Ask Catcher to send last written file path (and internal stored content).", "getLastFileWritePath");

// 14 - Legacy placeholder (compatibility)
AddAction(14, af_none, "Legacy placeholder", "Compatibility",
    "Legacy placeholder (no-op)",
    "Kept for compatibility with old projects.", "legacyPlaceholder");

// Conditions
AddCondition(0, cf_trigger, "On Window Position Received", "Window",
    "On window position received",
    "Triggered when window position is received.", "OnWindowPositionReceived");

AddCondition(1, cf_trigger, "On Window Size Received", "Window",
    "On window size received",
    "Triggered when window size is received.", "OnWindowSizeReceived");

AddCondition(2, cf_trigger, "On Document Path Received", "System",
    "On document path received",
    "Triggered when document path is received.", "OnDocumentPathReceived");

AddCondition(3, cf_trigger, "On File Read", "File",
    "On file read",
    "Triggered when file content is read.", "OnFileRead");

AddCondition(4, cf_trigger, "On File Written", "File",
    "On file written",
    "Triggered when file is written.", "OnFileWritten");

AddCondition(5, 0, "Is Fullscreen On", "Window",
    "Is fullscreen on",
    "True if fullscreen is active.", "IsFullscreenOn");

AddCondition(6, 0, "Is Fullscreen Off", "Window",
    "Is fullscreen off",
    "True if fullscreen is not active.", "IsFullscreenOff");

// Expressions
AddExpression(0, ef_return_string, "Get Last File Write File Path", "File",
    "LastFileWriteFilePath", "Return last written file path.");

AddExpression(1, ef_return_string, "Get Last File Read Content", "File",
    "LastFileReadContent", "Return last read file content.");

AddExpression(2, ef_return_number, "Get Window X", "Window",
    "WindowX", "Return window X position.");

AddExpression(3, ef_return_number, "Get Window Y", "Window",
    "WindowY", "Return window Y position.");

AddExpression(4, ef_return_number, "Get Window Width", "Window",
    "WindowWidth", "Return window width.");

AddExpression(5, ef_return_number, "Get Window Height", "Window",
    "WindowHeight", "Return window height.");

AddExpression(6, ef_return_string, "Get Document Path", "System",
    "DocumentPath", "Return document path.");

ACESDone();


var property_list = [];

function CreateIDEObjectType() { return new IDEObjectType(); }
function IDEObjectType() {}
IDEObjectType.prototype.CreateInstance = function(instance) { return new IDEInstance(instance); }

function IDEInstance(instance, type) {
    this.instance = instance;
    this.type = type;
    this.properties = {};
}

IDEInstance.prototype.OnInserted = function() {};
IDEInstance.prototype.OnDoubleClicked = function() {};
IDEInstance.prototype.OnPropertyChanged = function(prop_name) {};
IDEInstance.prototype.OnRendererInit = function(renderer) {};
IDEInstance.prototype.Draw = function(renderer) {};
IDEInstance.prototype.OnRendererReleased = function(renderer) {};