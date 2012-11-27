 Español
=========

Este proyecto está configurado para utilizar OpenGL con GLUT. En la configuración 
para Windows se utiliza freeglut y se enlaza estáticamente (no requiere ningún 
dll). 

Para realizar un enlace dinámico hay que cambiar el nombre de la librería 
a enlazar freeglut_static por freeglut y quitar la definición de la macro
FREEGLUT_STATIC (ejecucion->limpiar antes de volver a compilar). Los archivos de 
estas librerías (cabeceras, objetos, y dlls) se encuentran en la carpeta OpenGL 
dentro de MinGW. Si va a utilizar enlazado dinámico recuerde copiar freeglut.dll 
al directorio de su programa o a windows\system32. Todas estas opciones del 
proyecto se configuran desde "Opciones..." en el menú "Ejecución".

La biblioteca glew se incluye en su versión para Windows, pero el proyecto no 
está configurado para utilizarla. Si así lo requiere, debe añadir glew32s a la lista 
de bibliotecas a enlazar y GLEW_BUILD y GLEW_STATIC a la lista de macros a definir 
en las opciones de compilación y enlazado del proyecto.


 English
=========

This project is set to use OpenGL with GLUT. In the Windows' configuration
freeglut is used and linked statically (so it won't require any dll file).

To perform a dynamic link must change the name of the library to link from
freeglut to freeglut_static, an delete the macro FREEGLUT_STATIC definition.
(use Run->Clean before recompiling). Files for these libraries (headers, objects,
and DLLs) are located in an OpenGL folder within MinGW folder. If you are using 
dynamic linking remember copying freeglut.dll to your project's working directory 
(or to windows\system32 folder to make it available system wide). All these project 
options are set from "Options ..." in the "Run" menu.

Glew library is included in the Windows' version, but the project is not configured 
to use. If you need it, you must add "glew32s" to the list libraries to link and 
GLEW_BUILD and GLEW_STATIC to the list of macros to define in the compile and 
link options for the project.