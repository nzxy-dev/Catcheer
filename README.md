> Catcher is a simple application focused on video games that allows you to reproduce/distribute your HTML/WEB games for Windows with greater ease, 
>compatibility and control over the system and the aesthetics of the experience that the developer is looking for. Catcher works with Webview2

Virustotal Results (2/66)
[1.2v 24/4/26 14:08](https://www.virustotal.com/gui/file/2078824339291e4d3cbc7c9f956e1759656f2f4a697df1ea9cc5698a5af9ab76?nocache=1)

## First release (1.2v)
---
### Features:

Management and modification of basic window appearance:
(Size, position, title)

-Full screen support

-WebGL support

-Initial state configuration with a "config.ini" file:
(Force full screen, initial window size, initial window position, initial window title)

-Loading custom icons for the taskbar and window with a "custom.ico" file
---

 >Catcher uses a minimum of 70 MB of memory when running.

 ##### catcher_1.2build_32_bits


 ---
 Some catcher functions have been fixed and made easier with simple messages. You can find them and their functions below or check the default index.html that comes with catcher in the "_game" folder.     
 ---
 ```
window.chrome.webview.postMessage
```
 // Windows state
 ```
{ action: "getWindowPosition" }
{ action: "getWindowSize" }
{ action: "setWindowPosition", x, y }
{ action: "setWindowSize", width, height }
{ action: "toggleFullscreen" }
{ action: "setWindowTitle", value: v }
{ action: "closeWindow" }
```

// Files
```
{ action: "readFile", path }
{ action: "writeFile", path, content: txt }
{ action: "LastFileWriteContent" }
{ action: "openFile", path }
```

// Windows
```
{ action: "newWindow" }
{ action: "newWindow", path }  //"_game/index2.html" 
```

// Browser Call
```
{ action: "callBrowser", url }
```

// Others
```
{ action: "get_document_in_pc_direction" }
```
---
## Construct 2

To export to Catcher with Construct 2, simply export your project to HTML5 and copy all the generated content from the main folder containing "index.html" to the "_game" folder in Catcher.

>Catcher plugin 1.2v for construct 2

[Download C2Plugin](https://github.com/nzxy-dev/Catcheer/releases/download/Plugins/Catcher1_2v-Plugin_for_C2.zip)

[PeojectTemplateCapxC2](https://github.com/nzxy-dev/Catcheer/tree/main/extra/Construct2/demo/testC2capx)


