# Catcheer!

Bienvenido al repositorio oficial de catcheer una herramienta programada en c++ de codigo abierto pensada en desarolladores de videojuegos pero tambien util para 
algunas aplicaciones sencillas creadas en html tradicional

## ¿ Que es Catcheer?

Catcheer es un "cargador" de [html](https://es.wikipedia.org/wiki/HTML) minimalista el cual solo proriza la experiencia mas optima y fluida posible tanto como para el usuario final (cliente) como para el desarollador al tener una facilidad de uso increible en el proceso de compilacion y o empaquetado de un producto sea software o programa basado en [html](https://es.wikipedia.org/wiki/HTML) o para el cliente con una optimizacion agresiva por medios de flags agresivos para el GPU y ventana del webkit

## La filosofia de unix

>Haz una cosa y hazla bien.

 catcheer solo llama a el webkit de el sistema operativo (Os) predeterminado tal como lo es en Windows [Webview2](https://developer.microsoft.com/es-es/microsoft-edge/webview2/?form=MA13LH) O en linux con [GTK](https://webkitgtk.org/) 

## ¿ Como usar Catcheer ? (Manual para desarolladores)

 Como se menciona antes catcheer es ridiculamente facil de utilizar sobre todo para programadores su uso es tan facil como:

 **1. Prepare su archivo index.html** con todas sus dependencias dentro de la carpeta **Source**
 >La carpeta a la que se refiere en este repositorio como **Source** va junto al ejecutable de catcheer y debe llamarse "s-folder" si usa la version de Windows o "source" si usa la version de linux,Esto es puramente estetico y puede modificarlo desde el codigo fuente en el modulo del nucleo de Windows: `\lib\core\windows_webview2\webview2.cpp`, Y para el nucle de linux: `lib\core\linux_gtk\gtk.cpp` puede encontrarlo en ambos modulos con el comentario: `//------------------------html file load`

**2. Configure la ventana inicial**
>Para "preparar" la configuracion de la ventana inicial solo debe abrir  un editor de texto plano y escribir el contenido donde establecera propiedades 
>basicas de la ventana como :
> - Tamano de ventana en pixeles
> - Pantalla completa o no
> - Posibilidad de cambiar el tamano 
> - Titulo de la ventana
> - Activacion o desactivacion del marco de la ventana
> Dependiendo de la plataforma el contenido para definir la configuracion inicial es el siguiente:
>
> **Linux Config Template** ;
> ```
> {
>       "title": "catcheer-linux-gtk-debug",
>       "width": 937,
>       "height": 545,
>       "fullscreen": "false",
>       "resizable": "true"
>
>} 
>```
>
>**Windows Config Template** ;
> ```
>[Window]
>title = catcheer-debug-webview2-windows
>size = 800x470
>border = true
>fullscreen = false
>resizable = true
>
> ```
**3. Icono**;
> Para personalizar mas su producto final puede anadir un icono personalizado de manera muy simple .
> Para la plataforma de Windows debera simplemente mover o copiar un archivo `custom.ico` junto al ejecutable 
> Para la plataforma de Linux debera usar un `custom.png` el cual se aplicara solo a la ventana , tambien  puede personalizarlo en el nucleo de linux o windows , puede conseguirlo en ambos nucleos con el comentario: `//------------------set icon|`

## Uso avanzado para programadores (**API**)

Catcheer ofrece una "api" basada en mensajes web sencillos para facilitar operaciones basicas como lectura/escritura de archivos, modificacion de ventanas y mas mostradas a continuacion:

### Metodo de posteo de mensajes especifico :
 ```
window.chrome.webview.postMessage
```
### Ventanas tamanos,posiciones y mas estilos
**Obtener posicion de ventana** 
`{ action: "getWindowPosition" }`
**Obtener tamano de ventana**
`{ action: "getWindowSize" }`
**Establecer posicion de ventana**
`{ action: "setWindowPosition", x, y }`
**EStablecer tamano de ventana**
`{ action: "setWindowSize", width, height }`
**Lanzar a pantalla completa**
`{ action: "toggleFullscreen" }`
**Establecer nuevo titulo de ventana**
`{ action: "setWindowTitle", value: v }`
**Cerrar ventana**
`{ action: "closeWindow" }`

### Archivos locales
**Leer archivo**
`{ action: "readFile", path }`
**Escribir Archivo**
`{ action: "writeFile", path, content: txt }`
**Abrir/Ejecutar Fichero**
`{ action: "openFile", path }`


## Ejecucion de una nueva ventana
>Importante esta funcion no lee archivos dentro de "Source" por defecto sino tomando referencia de la carpeta raiz junto al ejecutable
`{ action: "newWindow", path }`

## Llamar al navegador (Abrir url en el navegador predeterminado)
>En linux esta opcion suele fallar por lo que se  recomienda usar la funcion de abrir archivo 
`{ action: "callBrowser", url }`
## Obtener direccion de ejecucion en el dispositivo
`{ action: "get_document_in_pc_direction" }`


