# Catcheer!

Welcome to the official repository of catcheer, an open-source tool programmed in C++ designed for video game developers but also useful for some simple applications created in traditional HTML.

## What is Catcheer?

Catcheer is a minimalist [html](https://es.wikipedia.org/wiki/HTML) "loader" that prioritizes the most optimal and fluid experience possible both for the end user (client) and for the developer. It offers incredible ease of use in the compilation and/or packaging process of a product, whether it is software or a program based on [html](https://es.wikipedia.org/wiki/HTML), and provides the client with aggressive optimization through aggressive flags for the GPU and the webkit window.

## The Unix Philosophy

> Do one thing and do it well.

catcheer only calls the default operating system (OS) webkit, such as [Webview2](https://developer.microsoft.com/es-es/microsoft-edge/webview2/?form=MA13LH) in Windows or [GTK](https://webkitgtk.org/) in Linux.

## How to use Catcheer? (Developer Manual)

As mentioned before, catcheer is ridiculously easy to use, especially for programmers. Its use is as easy as:

**1. Prepare your index.html file** with all its dependencies inside the **Source** folder.
> The folder referred to in this repository as **Source** goes next to the catcheer executable and must be named "s-folder" if you use the Windows version or "source" if you use the Linux version. This is purely aesthetic and you can modify it from the source code in the Windows core module: `\lib\core\windows_webview2\webview2.cpp`, and for the Linux core: `lib\core\linux_gtk\gtk.cpp`. You can find it in both modules with the comment: `//------------------------html file load`

**2. Configure the initial window**
> To "prepare" the configuration of the initial window, you only need to open a plain text editor and write the content where you will establish basic window properties such as:
> - Window size in pixels
> - Fullscreen or not
> - Resizable or not
> - Window title
> - Window frame activation or deactivation
> Depending on the platform, the content to define the initial configuration is as follows:
>
> **Linux Config Template** ;
> ```
> {
>        "title": "catcheer-linux-gtk-debug",
>        "width": 937,
>        "height": 545,
>        "fullscreen": "false",
>        "resizable": "true"
>
> } 
> ```
>
> **Windows Config Template** ;
> ```
> [Window]
> title = catcheer-debug-webview2-windows
> size = 800x470
> border = true
> fullscreen = false
> resizable = true
>
> ```
**3. Icon**;
> To further customize your final product, you can add a custom icon very easily.
> For the Windows platform, you simply need to move or copy a `custom.ico` file next to the executable.
> For the Linux platform, you must use a `custom.png` which will be applied only to the window. You can also customize it in the Linux or Windows core; you can find it in both cores with the comment: `//------------------set icon|`

## Advanced usage for programmers (**API**)

Catcheer offers an "api" based on simple web messages to facilitate basic operations such as reading/writing files, window modification, and more, shown below:

### Specific message posting method:

`window.chrome.webview.postMessage`

### Window sizes, positions, and more styles
**Get window position** `{ action: "getWindowPosition" }`
**Get window size**
`{ action: "getWindowSize" }`
**Set window position**
`{ action: "setWindowPosition", x, y }`
**Set window size**
`{ action: "setWindowSize", width, height }`
**Toggle fullscreen**
`{ action: "toggleFullscreen" }`
**Set new window title**
`{ action: "setWindowTitle", value: v }`
**Close window**
`{ action: "closeWindow" }`

### Local files
**Read file**
`{ action: "readFile", path }`
**Write File**
`{ action: "writeFile", path, content: txt }`
**Open/Execute File**
`{ action: "openFile", path }`


## Execution of a new window
> Important: this function does not read files inside "Source" by default, but instead takes reference from the root folder next to the executable.
`{ action: "newWindow", path }`

## Call browser (Open URL in default browser)
> On Linux this option usually fails, so it is recommended to use the open file function instead.
`{ action: "callBrowser", url }`
## Get execution directory on the device
`{ action: "get_document_in_pc_direction" }`