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

# Advanced operation:

Its operation is simple but practical 3Profiti uses the GDI api to obtain screenshots every so many frames specified by the user in the FPS "edit" 
configuration section and automatically compressed as JPEG format in the .dat file you can use the dat extractor to extract the frames 
from the dat file or using a hexadecimal editor

>To use the dat extractor you need to install Python on your Windows computer.

1. You must open CMD
2. Direct to the installation path in the datextractor folder or simply wherever you have your datextractor, using the "cd" command
3. Execute the code of the function that requires

>Extract Frames
```
python extract_jpgs.py [videoName.dat] [outputfolder] [100"Number of frames to extract"]
```
>Get the maximum number of frames
```
python framecounter.py [videoName.dat]
```
#### without the brackets"[]"
---

# Usage and settings

 Using 3Profiti is quite simple, to start recording the screen just open the application and press the "Rec" button and 
 to end the video just press the "Stop" button. You can change the theme in the "Theme Dark" or "Theme Default" 
 button located just above the edit of the file destination selection.
## Configs

 For more advanced settings just tap the **"Config"** button to open the settings window once there you have multiple options such as: 

---
**(Keep . dat File)** Which allows you to decide whether or not you want to keep the . dat file that contains all the compressed frames in jpg format or if you want 

 to delete it when you finish assembling your MP4 file
 (Assemble MP4vid) This function allows you to decide whether or not you want to generate the final "Video" MP4 file or not 

---

**(Time Lapse Mode)** With this function you can record faster and smoother videos designed for timelapse starts recording without a specific
 frame limit or a completely linear or constant timeline perfect for modest PCs with not much space resources 
 
---

 **(Show cursor)** Do you want the  mouse to be recorded in your video yes or no


---

**(FPS)**
 Edit specifies the amount of FPS you want to record

---

**(Width)** 
the width of the outgoing video / "Resolution"

---

**(Height)** The height of the outgoing video / "Resolution"

---
>*Warning* (When recording in timelapse mode, although fluid, there is a significantly lower number of recorded frames depending on your GPU.
> By increasing the amount of FPS Profiti uses much more GPU so it is more likely to reach even less.
---
## Construct 2

To export to Catcher with Construct 2, simply export your project to HTML5 and copy all the generated content from the main folder containing "index.html" to the "_game" folder in Catcher.

>Catcher plugin 1.2v for construct 2

[Download C2Plugin](https://github.com/nzxy-dev/Catcheer/releases/download/Plugins/Catcher1_2v-Plugin_for_C2.zip)

[PeojectTemplateCapxC2](https://github.com/nzxy-dev/Catcheer/tree/main/extra/Construct2/demo/testC2capx)


