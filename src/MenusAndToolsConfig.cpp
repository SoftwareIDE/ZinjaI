#include "MenusAndToolsConfig.h"
#include "Language.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "ProjectManager.h"

MenusAndToolsConfig *menu_data;
#if defined(__APPLE__)
	#define _if_not_apple(a,b) b
#else
	#define _if_not_apple(a,b) a
#endif
#if defined(__WIN32__)
	#define _if_win32(a,b) a
#else
	#define _if_win32(a,b) b
#endif

MenusAndToolsConfig::MenusAndToolsConfig (wxMenuBar * _menu_bar) :wx_menu_bar(_menu_bar) {
	tools_custom_item=new wxMenuItem*[10];
	for (int i=0;i<10;i++) tools_custom_item[i] = NULL;
	file_source_history = new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) file_source_history[i]=NULL;
	file_project_history = new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) file_project_history[i]=NULL;
}

void MenusAndToolsConfig::LoadMenuData ( ) {
	
	menues[mnFILE].label = LANG(MENUITEM_FILE,"&Archivo"); {
		AddMenuItem(mnFILE, myMenuItem(wxID_NEW, LANG(MENUITEM_FILE_NEW,"&Nuevo...")).ShortCut("Ctrl+N").Description("Crear un nuevo archivo").Icon("nuevo.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_PROJECT, LANG(MENUITEM_FILE_NEW_PROJECT,"&Nuevo Proyecto...")).ShortCut("Ctrl+Shift+N").Description("Crear un nuevo proyecto").Icon("proyecto.png"));
		AddMenuItem(mnFILE, myMenuItem(wxID_OPEN, LANG(MENUITEM_FILE_OPEN,"&Abrir...")).ShortCut("Ctrl+O").Description("Abrir un archivo o un proyecto existente...").Icon("abrir.png").Map());
		BeginSubMenu(mnFILE,LANG(MENUITEM_FILE_RECENT_SOURCES,"Fuentes Abiertos Recientemente"),"Muestra los ultimos archivos abiertos como programas simples","recentSimple.png",mxID_FILE_SOURCE_RECENT,maMAPPED);
			AddSeparator(mnFILE);
			AddMenuItem(mnFILE, myMenuItem(mxID_FILE_SOURCE_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas...")).Description("Muestra un dialogo con la lista completa de archivos recientes").Icon("recentMore.png"));
		EndSubMenu(mnFILE);
		BeginSubMenu(mnFILE,LANG(MENUITEM_FILE_RECENT_PROJECTS,"Proyectos Abiertos Recientemente"),"Muestra los ultimos proyectos abiertos","recentProject.png",mxID_FILE_PROJECT_RECENT,maMAPPED);
			AddSeparator(mnFILE);
			AddMenuItem(mnFILE, myMenuItem(mxID_FILE_PROJECT_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas...")).Description("Muestra un dialogo con la lista completa de archivos recientes").Icon("recentMore.png"));
		EndSubMenu(mnFILE);
		AddMenuItem(mnFILE, myMenuItem(wxID_SAVE, LANG(MENUITEM_FILE_SAVE,"&Guardar")).ShortCut("Ctrl+S").Description("Guardar el archivo actual").Icon("guardar.png"));
		AddMenuItem(mnFILE, myMenuItem(wxID_SAVEAS, LANG(MENUITEM_FILE_SAVE_AS,"G&uardar Como...")).ShortCut("Ctrl+Shift+S").Description("Guardar el archivo actual con otro nombre...").Icon("guardarComo.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_SAVE_ALL, LANG(MENUITEM_FILE_SAVE_ALL,"Guardar &Todo...")).ShortCut("Ctrl+Alt+Shift+S").Description("Guarda todos los archivos abiertos y el proyecto actual...").Icon("guardarTodo.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_SAVE_PROJECT, LANG(MENUITEM_FILE_SAVE_PROJECT,"Guar&dar Proyecto")).ShortCut("Ctrl+Alt+S").Description("Guardar la configuaricion actual del proyecto").Icon("guardarProyecto.png").Project(true));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_EXPORT_HTML, LANG(MENUITEM_FILE_EXPORT_HTML,"Exportar a HTML...")).Description("Genera un archiv HTML con el codigo fuente").Icon("exportHtml.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_PRINT, LANG(MENUITEM_FILE_PRINT,"&Imprimir...")).Description("Imprime el codigo fuente actual").Icon("imprimir.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_RELOAD, LANG(MENUITEM_FILE_RELOAD,"&Recargar")).ShortCut("Ctrl+Shift+R").Description("Recarga la version en disco del archivo actual.").Icon("recargar.png"));
		AddMenuItem(mnFILE, myMenuItem(wxID_CLOSE, LANG(MENUITEM_FILE_CLOSE,"&Cerrar")).ShortCut("Ctrl+W").Description("Cerrar el archivo actual").Icon("cerrar.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_CLOSE_ALL, LANG(MENUITEM_FILE_CLOSE_ALL,"Cerrar &Todo")).ShortCut("Ctrl+Alt+Shift+W").Description("Cierra todos los archivos abiertos").Icon("cerrarTodo.png"));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_CLOSE_PROJECT, LANG(MENUITEM_FILE_CLOSE_PROJECT,"Cerrar Pro&yecto")).ShortCut("Ctrl+Shift+W").Description("Cierra el proyecto actual").Icon("cerrarProyecto.png").Project(true));
		AddSeparator(mnFILE);
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_PROJECT_CONFIG, LANG(MENUITEM_FILE_PROJECT_CONFIG,"&Configuracion del Proyecto...")).ShortCut("Ctrl+Shift+P").Description("Configurar las propiedades generales de un proyecto...").Icon("projectConfig.png").Project(true));
		AddMenuItem(mnFILE, myMenuItem(mxID_FILE_PREFERENCES, LANG(MENUITEM_FILE_PREFERENCES,"&Preferencias...")).ShortCut("Ctrl+P").Description("Configurar el entorno...").Icon("preferencias.png"));
		AddSeparator(mnFILE);
		AddMenuItem(mnFILE, myMenuItem(wxID_EXIT, LANG(MENUITEM_FILE_EXIT,"&Salir")).ShortCut("Alt+F4").Description("Salir del programa!").Icon("salir.png"));
	}
	
	
	menues[mnEDIT].label = LANG(MENUITEM_EDIT,"&Edicion"); {
		AddMenuItem(mnEDIT, myMenuItem(wxID_UNDO, LANG(MENUITEM_EDIT_UNDO,"&Deshacer")).ShortCut("Ctrl+Z").Description("Deshacer el ultimo cambio").Icon("deshacer.png"));
		AddMenuItem(mnEDIT, myMenuItem(wxID_REDO, LANG(MENUITEM_EDIT_REDO,"&Rehacer")).ShortCut("Ctrl+Shift+Z").Description("Rehacer el ultimo cambio desecho").Icon("rehacer.png"));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem(wxID_CUT, LANG(MENUITEM_EDIT_CUT,"C&ortar")).ShortCut("Ctrl+X").Description("Cortar la seleccion al portapapeles").Icon("cortar.png"));
		AddMenuItem(mnEDIT, myMenuItem(wxID_COPY, LANG(MENUITEM_EDIT_COPY,"&Copiar")).ShortCut("Ctrl+C").Description("Copiar la seleccion al portapapeles").Icon("copiar.png"));
		AddMenuItem(mnEDIT, myMenuItem(wxID_PASTE, LANG(MENUITEM_EDIT_PASTE,"&Pegar")).ShortCut("Ctrl+V").Description("Pegar el contenido del portapapeles").Icon("pegar.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_TOGGLE_LINES_UP, LANG(MENUITEM_EDIT_LINES_UP,"Mover Hacia Arriba")).ShortCut("Ctrl+T").Description("Mueve la o las lineas seleccionadas hacia arriba").Icon("toggleLinesUp.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_TOGGLE_LINES_DOWN, LANG(MENUITEM_EDIT_LINES_DOWN,"Mover Hacia Abajo")).ShortCut("Ctrl+Shift+T").Description("Mueve la o las lineas seleccionadas hacia abajo").Icon("toggleLinesDown.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_DUPLICATE_LINES, LANG(MENUITEM_EDIT_DUPLICATE_LINES,"&Duplicar Linea(s)")).ShortCut("Ctrl+L").Description("Copia la linea actual del cursor, o las lineas seleccionadas, nuevamente a continuacion").Icon("duplicarLineas.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_DELETE_LINES, LANG(MENUITEM_EDIT_DELETE_LINES,"&Eliminar Linea(s)")).ShortCut("Shift+Ctrl+L").Description("Elimina la linea actual del cursor nuevamente, o las lineas seleccionadas").Icon("borrarLineas.png"));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_GOTO_FUNCTION, LANG(MENUITEM_EDIT_GOTO_FUNCTION,"&Ir a Funcion/Clase/Metodo...")).ShortCut("Ctrl+Shift+G").Description("Abrir el fuente con la declaracion de una funcion, clase o metodo...").Icon("irAFuncion.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_GOTO_FILE,"&Ir a Archivo...")).ShortCut("Ctrl+Shift+F").Description("Abrir un archivo en particular buscandolo por parte de su nombre...").Icon("irAArchivo.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_GOTO, LANG(MENUITEM_EDIT_GOTO_LINE,"&Ir a Linea...")).ShortCut("Ctrl+G").Description("Mover el cursor a una linea determinada en el archivo...").Icon("irALinea.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_FIND, LANG(MENUITEM_EDIT_FIND,"&Buscar...")).ShortCut("Ctrl+F").Description("Buscar una cadena en el archivo...").Icon("buscar.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_FIND_PREV, LANG(MENUITEM_EDIT_FIND_PREV,"Buscar &Anterior")).ShortCut("Shift+F3").Description("Repetir la ultima busqueda a partir del cursor hacia atras").Icon("buscarAnterior.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_FIND_NEXT, LANG(MENUITEM_EDIT_FIND_NEXT,"Buscar &Siguiente")).ShortCut("F3").Description("Repetir la ultima busqueda a partir del cursor").Icon("buscarSiguiente.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_REPLACE, LANG(MENUITEM_EDIT_REPLACE,"&Reemplazar...")).ShortCut("Ctrl+R").Description("Reemplazar una cadena con otra en el archivo...").Icon("reemplazar.png"));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_INSERT_HEADER, LANG(MENUITEM_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente")).ShortCut("Ctrl+H").Description("Si es posible, inserta el #include necesario para utilizar la funcion/clase en la cual se encuentra el cursor.").Icon("insertarInclude.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_COMMENT, LANG(MENUITEM_EDIT_COMMENT,"Comentar")).ShortCut("Ctrl+D").Description("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea").Icon("comentar.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_UNCOMMENT, LANG(MENUITEM_EDIT_UNCOMMENT,"Descomentar")).ShortCut("Shift+Ctrl+D").Description("Descomente el texto seleccionado eliminando \"//\" de cada linea").Icon("descomentar.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_INDENT, LANG(MENUITEM_EDIT_INDENT,"Indentar Blo&que")).ShortCut("Ctrl+I").Description("Corrige el indentado de un bloque de codigo agregando o quitando tabs segun corresponda").Icon("indent.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_BRACEMATCH, LANG(MENUITEM_EDIT_BRACEMATCH,"Seleccionar Blo&que")).ShortCut("Ctrl+M").Description("Seleccionar todo el bloque correspondiente a la llave o parentesis sobre el cursor").Icon("mostrarLlave.png"));
		AddMenuItem(mnEDIT, myMenuItem(wxID_SELECTALL, LANG(MENUITEM_EDIT_SELECT_ALL,"&Seleccionar Todo")).ShortCut("Ctrl+A").Description("Seleccionar todo el contenido del archivo").Icon("seleccionarTodo.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_MARK_LINES, LANG(MENUITEM_EDIT_HIGHLIGHT_LINES,"&Resaltar Linea(s)/Quitar Resaltado")).ShortCut("Ctrl+B").Description("Resalta la linea pintandola de otro color").Icon("marcar.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_GOTO_MARK, LANG(MENUITEM_EDIT_FIND_HIGHLIGHTS,"Buscar &Resaltado")).ShortCut("Ctrl+Shift+B").Description("Mueve el cursor a la siguiente linea resaltada").Icon("irAMarca.png"));
		AddMenuItem(mnEDIT, myMenuItem(mxID_EDIT_LIST_MARKS, LANG(MENUITEM_EDIT_LIST_HIGHLIGHTS,"&Listar Lineas Resaltadas")).ShortCut("Ctrl+Alt+B").Description("Muestra una lista de las lineas marcadas en todos los archivos").Icon("listarMarcas.png"));
	}

	
	menues[mnVIEW].label = LANG(MENUITEM_VIEW,_if_not_apple("&Ver","Ver")); {
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_DUPLICATE_TAB, LANG(MENUITEM_VIEW_SPLIT_VIEW,"&Duplicar vista")).Icon("duplicarVista.png"));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_LINE_WRAP, LANG(MENUITEM_VIEW_LINE_WRAP,"&Ajuste de linea")).ShortCut("Alt+F11").Description("Muestra las lineas largas como en varios renglones").Checkeable(false));	
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_WHITE_SPACE, LANG(MENUITEM_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de &fin de linea")).Description("Muestra las lineas largas como en varios renglones").Checkeable(false));	
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_CODE_STYLE, LANG(MENUITEM_VIEW_SYNTAX_HIGHLIGHT,"&Colorear Sintaxis")).ShortCut("Shift+F11").Description("Resalta el codigo con diferentes colores y formatos de fuente.").Checkeable(false));	
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_CODE_COLOURS, LANG(MENUITEM_VIEW_CODE_COLOURS,"Configurar esquema de colores...")).Icon("preferencias.png"));
		BeginSubMenu(mnVIEW,LANG(MENUITEM_VIEW_FOLDING,"Plegado"),"Muestra opciones para plegar y desplegar codigo en distintos niveles","folding.png");
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea")).ShortCut(_if_not_apple("Alt+Up","")).Icon("foldOne.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea")).ShortCut(_if_not_apple("Alt+Down","")).Icon("unfoldOne.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel")).ShortCut(_if_not_apple("Ctrl+1","")).Description("Cierra todos los bolques del primer nivel").Icon("fold1.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel")).ShortCut(_if_not_apple("Ctrl+2","")).Description("Cierra todos los bolques del segundo nivel").Icon("fold2.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel")).ShortCut(_if_not_apple("Ctrl+3","")).Description("Cierra todos los bolques del tercer nivel").Icon("fold3.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel")).ShortCut(_if_not_apple("Ctrl+4","")).Description("Cierra todos los bolques del cuarto nivel").Icon("fold4.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel")).ShortCut(_if_not_apple("Ctrl+5","")).Description("Cierra todos los bolques del quinto nivel").Icon("fold5.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles")).ShortCut(_if_not_apple("Ctrl+0","")).Description("Cierra todos los bolques de todos los niveles").Icon("foldAll.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel")).ShortCut(_if_not_apple("Alt+1","")).Description("Abre todos los bolques del primer nivel").Icon("unfold1.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel")).ShortCut(_if_not_apple("Alt+2","")).Description("Abre todos los bolques del segundo nivel").Icon("unfold2.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel")).ShortCut(_if_not_apple("Alt+3","")).Description("Abre todos los bolques del tercer nivel").Icon("unfold3.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel")).ShortCut(_if_not_apple("Alt+4","")).Description("Abre todos los bolques del cuarto nivel").Icon("unfold4.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel")).ShortCut(_if_not_apple("Alt+5","")).Description("Abre todos los bolques del quinto nivel").Icon("unfold5.png"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles")).ShortCut(_if_not_apple("Alt+0","")).Description("Abre todos los bolques de todos los niveles").Icon("unfoldAll.png"));
		EndSubMenu(mnVIEW);
		AddSeparator(mnVIEW);
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_FULLSCREEN, LANG(MENUITEM_VIEW_FULLSCREEN,"Ver a Pantalla &Completa")).ShortCut("F11").Description("Muestra el editor a pantalla completa, ocultando tambien los demas paneles").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_BEGINNER_PANEL, LANG(MENUITEM_VIEW_BEGINNER_PANEL,"Mostrar Panel de Mini-Plantillas")).Description("Muestra un panel con plantillas y estructuras basicas de c++").Checkeable(false));
		if (config->Init.left_panels) 
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_LEFT_PANELS, LANG(MENUITEM_VIEW_LEFT_PANELS,"&Mostrar Panel de Arboles")).Description("Muestra el panel con los arboles de proyecto, simbolos y explorador de archivos").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_PROJECT_TREE, LANG(MENUITEM_VIEW_PROJECT_TREE,"&Mostrar Arbol de &Proyecto")).Description("Muestra el panel del arbol de proyecto/archivos abiertos").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_EXPLORER_TREE, LANG(MENUITEM_VIEW_EXPLORER_TREE,"Mostrar &Explorardor de Archivos")).ShortCut("Ctrl+E").Description("Muestra el panel explorador de archivos").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_SYMBOLS_TREE, LANG(MENUITEM_VIEW_SYMBOLS_TREE,"Mostrar Arbol de &Simbolos")).Description("Analiza el codigo fuente y construye un arbol con los simbolos declarados en el mismo.").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_COMPILER_TREE, LANG(MENUITEM_VIEW_COMPILER_TREE,"&Mostrar Resultados de la Compilacion")).Description("Muestra un panel con la salida del compilador").Checkeable(false));
		BeginSubMenu(mnVIEW, LANG(MENUITEM_VIEW_TOOLBARS,"Barras de herramientas"));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_FILE, LANG(MENUITEM_VIEW_TOOLBAR_FILE,"&Mostrar Barra de Herramientas Archivo")).Description("Muestra la barra de herramientas para el manejo de archivos").Checkeable(config->Toolbars.positions.file.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_EDIT, LANG(MENUITEM_VIEW_TOOLBAR_EDIT,"&Mostrar Barra de Herramientas Edicion")).Description("Muestra la barra de herramientas para la edicion del fuente").Checkeable(config->Toolbars.positions.edit.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_VIEW, LANG(MENUITEM_VIEW_TOOLBAR_VIEW,"&Mostrar Barra de Herramientas Ver")).Description("Muestra la barra de herramientas para las opciones de visualizacion").Checkeable(config->Toolbars.positions.view.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_FIND, LANG(MENUITEM_VIEW_TOOLBAR_FIND,"&Mostrar Barra de Busqueda Rapida")).Description("Muestra un cuadro de texto en la barra de herramientas que permite buscar rapidamente en un fuente").Checkeable(config->Toolbars.positions.find.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_RUN, LANG(MENUITEM_VIEW_TOOLBAR_RUN,"&Mostrar Barra de Herramientas Ejecucion")).Description("Muestra la barra de herramientas para la compilacion y ejecucion del programa").Checkeable(config->Toolbars.positions.run.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_TOOLS, LANG(MENUITEM_VIEW_TOOLBAR_TOOLS,"&Mostrar Barra de Herramientas Herramientas")).Description("Muestra la barra de herramientas para las herramientas adicionales").Checkeable(config->Toolbars.positions.tools.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_PROJECT, LANG(MENUITEM_VIEW_TOOLBAR_PROJECT,"&Mostrar Barra de Herramientas Proyecto")).Description("Muestra la barra de herramientas para las herramientas personalizables propias del proyecto").Checkeable(config->Toolbars.positions.project.visible).Project(true));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_DEBUG, LANG(MENUITEM_VIEW_TOOLBAR_DEBUG,"&Mostrar Barra de Herramientas Depuracion")).Description("Muestra la barra de herramientas para la depuracion del programa").Checkeable(config->Toolbars.positions.debug.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_MISC, LANG(MENUITEM_VIEW_TOOLBAR_MISC,"&Mostrar Barra de Herramientas Miscelanea")).Description("Muestra la barra de herramientas con commandos miselaneos").Checkeable(config->Toolbars.positions.misc.visible));
			AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBARS_CONFIG, LANG(MENUITEM_VIEW_TOOLBARS_CONFIG,"&Configurar...")).Icon("preferencias.png"));
		EndSubMenu(mnVIEW);
		AddSeparator(mnVIEW);
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior")).ShortCut(_if_win32("Ctrl+>","Ctrl+Shift+<")).Description("Selecciona el error/advertencia anterior de la salida del compilador.").Icon("errorPrev.png"));
		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_NEXT_ERROR, LANG(MENUITEM_VIEW_NEXT_ERROR,"&Ir a siguiente error")).ShortCut("Ctrl+<").Description("Selecciona el proximo error/advertencia de la salida del compilador.").Icon("errorNext.png"));
	}
	
	
	menues[mnRUN].label = LANG(MENUITEM_RUN,"E&jecucion"); {
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_RUN, LANG(MENUITEM_RUN_RUN,"&Ejecutar...")).ShortCut("F9").Description("Guarda y compila si es necesario, luego ejecuta el programa").Icon("ejecutar.png").Map());
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_RUN_OLD, LANG(MENUITEM_RUN_OLD,"Ejecutar (sin recompilar)...")).ShortCut("Ctrl+F9").Description("Ejecuta el binario existente sin recompilar primero").Icon("ejecutar_old.png"));
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_COMPILE, LANG(MENUITEM_RUN_COMPILE,"&Compilar")).ShortCut("Shift+F9").Description("Guarda y compila el fuente actual").Icon("compilar.png").Map());
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_CLEAN, LANG(MENUITEM_RUN_CLEAN,"&Limpiar")).ShortCut("Ctrl+Shift+F9").Description("Elimina los objetos y ejecutables compilados").Icon("limpiar.png").Map());
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_STOP, LANG(MENUITEM_RUN_STOP,"&Detener")).Description("Detiene la ejecucion del programa").Icon("detener.png").Map());
		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_CONFIG, LANG(MENUITEM_RUN_OPTIONS,"&Opciones...")).ShortCut("Alt+F9").Description("Configura la compilacion y ejecucion de los programas").Icon("opciones.png"));
	}
	
	
	menues[mnDEBUG].label = LANG(MENUITEM_DEBUG,"&Depuracion"); {
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_RUN, LANG(MENUITEM_DEBUG_START,"&Iniciar/Continuar")).ShortCut("F5").Icon("depurar.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_PAUSE, LANG(MENUITEM_DEBUG_PAUSE,"Interrum&pir")).Icon("pausar.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_STOP, LANG(MENUITEM_DEBUG_STOP,"&Detener")).ShortCut("Shift+F5").Icon("detener.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_STEP_IN, LANG(MENUITEM_DEBUG_STEP_IN,"Step &In")).ShortCut("F6").Icon("step_in.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_STEP_OVER, LANG(MENUITEM_DEBUG_STEP_OVER,"Step &Over")).ShortCut("F7").Icon("step_over.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_STEP_OUT, LANG(MENUITEM_DEBUG_STEP_OUT,"Step O&ut")).ShortCut("Shift+F6").Icon("step_out.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_RUN_UNTIL, LANG(MENUITEM_DEBUG_RUN_UNTIL,"Ejecutar &Hasta el Cursor")).ShortCut("Shift+F7").Icon("run_until.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_RETURN, LANG(MENUITEM_DEBUG_RETURN,"&Return")).ShortCut("Ctrl+F6").Icon("return.png").Debug(true));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_JUMP, LANG(MENUITEM_DEBUG_JUMP,"Continuar Desde Aqui")).ShortCut("Ctrl+F5").Icon("debug_jump.png").Debug(true));
		AddSeparator(mnDEBUG);
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_TOGGLE_BREAKPOINT, LANG(MENUITEM_DEBUG_TOGGLE_BREAKPOINT,"&Agregar/quitar Breakpoint")).ShortCut("F8").Icon("breakpoint.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_BREAKPOINT_OPTIONS, LANG(MENUITEM_DEBUG_BREAKPOINT_OPTIONS,"&Opciones del Breakpoint...")).ShortCut("Ctrl+F8").Icon("breakpoint_options.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_LIST_BREAKPOINTS, LANG(MENUITEM_DEBUG_LIST_BREAKPOINTS,"&Listar Watch/Break points...")).ShortCut("Shift+F8").Icon("breakpoint_list.png"));
		AddSeparator(mnDEBUG);
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_INSPECT, LANG(MENUITEM_DEBUG_INSPECT,"Panel de In&specciones")).Icon("inspect.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_BACKTRACE, LANG(MENUITEM_DEBUG_BACKTRACE,"&Trazado Inverso")).Icon("backtrace.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_THREADLIST, LANG(MENUITEM_DEBUG_THREADLIST,"&Hilos de Ejecucion")).Icon("threadlist.png"));
		AddMenuItem(mnDEBUG, myMenuItem( mxID_DEBUG_LOG_PANEL, LANG(MENUITEM_DEBUG_SHOW_LOG_PANEL,"&Mostrar mensajes del depurador")).Icon("debug_log_panel.png"));
		AddSeparator(mnDEBUG);
#ifndef __WIN32__
		BeginSubMenu(mnDEBUG, LANG(MENUITEM_DEBUG_MORE,"Más..."));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_ATTACH, LANG(MENUITEM_DEBUG_ATTACH,"&Adjuntar...")).Icon("debug_attach.png").Debug(false));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_CORE_DUMP, LANG(MENUITEM_DEBUG_LOAD_CORE_DUMP,"Cargar &Volcado de Memoria...")).Icon("core_dump.png").Debug(false));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_CORE_DUMP,LANG(MENUITEM_SAVE_CORE_DUMP,"Guardar &Volcado de Memoria...")).Icon("core_dump.png").Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_ENABLE_INVERSE_EXEC, LANG(MENUITEM_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras")).Checkeable(false).Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_INVERSE_EXEC, LANG(MENUITEM_DEBUG_INVERSE,"Ejecutar Hacia Atras")).Checkeable(false).Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_SET_SIGNALS, LANG(MENUITEM_DEBUG_SET_SIGNALS,"Configurar comportamiento ante señales...")).Icon("debug_set_signals.png").Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_SEND_SIGNAL, LANG(MENUITEM_DEBUG_SEND_SIGNALS,"Enviar señal...")).Icon("debug_send_signal.png").Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_GDB_COMMAND, LANG(MENUITEM_DEBUG_GDB_COMMAND,"Introducir comandos gdb...")).Icon("gdb_command.png").Debug(true));
			AddMenuItem(mnDEBUG, myMenuItem(mxID_DEBUG_PATCH, LANG(MENUITEM_DEBUG_PATCH,"Actualizar ejecutable (experimental)...")).Icon("debug_patch.png").Debug(true));
		EndSubMenu(mnDEBUG);
#endif
	}
	
	
	menues[mnTOOLS].label = LANG(MENUITEM_TOOLS,"&Herramientas"); {
		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DRAW_FLOW, LANG(MENUITEM_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de &Flujo...")).Description("Genera un diagrama de flujo a partir del bloque de codigo actual").Icon("flujo.png"));
		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DRAW_CLASSES, LANG(MENUITEM_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar &Jerarquia de Clases...")).Icon("clases.png"));
		
		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CODE_COPY_FROM_H, LANG(MENUITEM_TOOLS_CODE_COPY_FROM_H,"Implementar Métodos/Funciones faltantes...")).ShortCut("Ctrl+Shift+H").Icon("copy_code_from_h.png"));
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_COMMENTS,"Coment&arios"),"Permite alinear o quitar los comentarios del codigo","comments.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_ALIGN_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_ALIGN_COMMENTS,"&Alinear Comentarios...")).Description("Mueve todos los comentarios hacia una determinada columna").Icon("align_comments.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_EDIT_COMMENT, LANG(MENUITEM_TOOLS_COMMENTS_COMMENT,"&Comentar")).Description("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea").Icon("comentar.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_EDIT_UNCOMMENT, LANG(MENUITEM_TOOLS_COMMENTS_UNCOMMENT,"&Descomentar")).Description("Descomente el texto seleccionado eliminando \"//\" de cada linea").Icon("descomentar.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_REMOVE_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_DELETE_COMMENTS,"&Eliminar Comentarios")).Description("Quita todos los comentarios del codigo fuente o de la seleccion").Icon("remove_comments.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_PREPROC,"Preprocesador"),"Muestra información generada por el preprocesador de C++","preproc.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_PREPROC_EXPAND_MACROS, LANG(MENUITEM_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros")).ShortCut("Ctrl+Shift+M").Icon("preproc_expand_macros.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_PREPROC_MARK_VALID, LANG(MENUITEM_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas")).ShortCut("Ctrl+Alt+M").Icon("preproc_mark_valid.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_PREPROC_UNMARK_ALL, LANG(MENUITEM_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas")).ShortCut("Ctrl+Alt+Shift+M").Icon("preproc_unmark_all.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_PREPROC_HELP, LANG(MENUITEM_TOOLS_PREPROC_HELP,"Ayuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		AddMenuItem(mnTOOLS, myMenuItem( mxID_TOOLS_MAKEFILE, LANG(MENUITEM_TOOLS_GENERATE_MAKEFILE,"&Generar Makefile...")).Description("Genera el Makefile a partir de los fuentes y la configuracion seleccionada").Icon("makefile.png").Project(true));
		AddMenuItem(mnTOOLS, myMenuItem( mxID_TOOLS_CONSOLE, LANG(MENUITEM_TOOLS_OPEN_TERMINAL,"Abrir Co&nsola...")).Description("Inicia una terminal para interactuar con el interprete de comandos del sistema operativo").Icon("console.png"));
		AddMenuItem(mnTOOLS, myMenuItem( mxID_TOOLS_EXE_PROPS, LANG(MENUITEM_TOOLS_EXE_INFO,"&Propiedades del Ejecutable...")).Description("Muestra informacion sobre el archivo compilado").Icon("exeinfo.png"));
		AddMenuItem(mnTOOLS, myMenuItem( mxID_TOOLS_PROJECT_STATISTICS, LANG(MENUITEM_TOOLS_PROJECT_STATISTICS,"E&stadisticas del Proyecto...")).Description("Muestra informacion estadistica sobre los fuentes y demas archivos del proyecto").Icon("proystats.png").Project(true));
		AddMenuItem(mnTOOLS, myMenuItem( mxID_TOOLS_DRAW_PROJECT, LANG(MENUITEM_TOOLS_DRAW_PROJECT,"Grafo del Proyecto...")).Icon("draw_project.png").Project(true));
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_SHARE,"Compartir Archivos en la &Red Local"),"Permite enviar o recibir codigos fuentes a traves de una red LAN","share.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_SHARE_OPEN, LANG(MENUITEM_TOOLS_SHARE_OPEN,"&Abrir compartido...")).Description("Abre un archivo compartido por otra PC en la red local.").Icon("abrirs.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_SHARE_SHARE, LANG(MENUITEM_TOOLS_SHARE_SHARE,"&Compartir actual...")).Description("Comparte el archivo en la red local.").Icon("compartir.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_SHARE_LIST, LANG(MENUITEM_TOOLS_SHARE_LIST,"&Ver lista de compartidos propios...")).Description("Comparte el archivo en la red local.").Icon("share_list.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_SHARE_HELP, LANG(MENUITEM_TOOLS_SHARE_HELP,"A&yuda...")).Description("Muestra ayuda acerca de la comparticion de archivos en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_DIFF,"&Comparar Archivos (diff)"),"Muestra opciones para plegar y desplegar codigo en distintos niveles","diff.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_TWO,LANG(MENUITEM_TOOLS_DIFF_TWO,"&Dos fuentes abiertos...")).Description("Compara dos archivos de texto abiertos y los colorea segun sus diferencias").Icon("diff_sources.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_DISK,LANG(MENUITEM_TOOLS_DIFF_DISK,"&Fuente actual contra archivo en disco...")).Description("Compara un archivo abierto contra un archivo en disco y lo colorea segun sus diferencias").Icon("diff_source_file.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_HIMSELF,LANG(MENUITEM_TOOLS_DIFF_HIMSELF,"&Cambios en fuente actual contra su version en disco...")).Description("Compara un archivos abierto y modificado contra su version en disco y lo colorea segun sus diferencias").Icon("diff_source_himself.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_PREV,LANG(MENUITEM_TOOLS_DIFF_PREV,"Ir a Diferencia Anterior")).ShortCut("Shift+Alt+PageUp").Icon("diff_prev.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_NEXT,LANG(MENUITEM_TOOLS_DIFF_NEXT,"Ir a Siguiente Diferencia")).ShortCut("Shift+Alt+PageDown").Icon("diff_next.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_SHOW,LANG(MENUITEM_TOOLS_DIFF_SHOW,"Mostrar Cambio")).ShortCut("Alt+Shift+Ins").Description("Muestra en un globo emergente el cambio a aplicar").Icon("diff_show.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_APPLY,LANG(MENUITEM_TOOLS_DIFF_APPLY,"Apl&icar Cambio")).ShortCut("Alt+Ins").Description("Aplica el cambio marcado en la linea actual").Icon("diff_apply.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_DISCARD,LANG(MENUITEM_TOOLS_DIFF_DISCARD,"De&scartar Cambio")).ShortCut("Alt+Del").Description("Descarta el cambio marcado en la linea actual").Icon("diff_discard.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_CLEAR,LANG(MENUITEM_TOOLS_DIFF_CLEAR,"&Borrar Marcas")).ShortCut("Alt+Shift+Del").Description("Quita los colores y marcas que se agregaron en un fuente producto de una comparacion").Icon("diff_clear.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DIFF_HELP,LANG(MENUITEM_TOOLS_DIFF_HELP,"A&yuda...")).Description("Muestra ayuda acerca de la comparacion de fuentes en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_DOXYGEN,"Generar &Documentación (doxygen)"),"Doxygen permite generar automaticamente documentacion a partir del codigo y sus comentarios","doxy.png",wxID_ANY,maPROJECT);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DOXY_GENERATE,LANG(MENUITEM_TOOLS_DOXYGEN_GENERATE,"&Generar...")).ShortCut("Ctrl+Shift+F1").Description("Ejecuta doxygen para generar la documentacion de forma automatica").Icon("doxy_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DOXY_CONFIG,LANG(MENUITEM_TOOLS_DOXYGEN_CONFIGURE,"&Configurar...")).Description("Permite establecer opciones para el archivo de configuracion de doxygen").Icon("doxy_config.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DOXY_VIEW,LANG(MENUITEM_TOOLS_DOXYGEN_VIEW,"&Ver...")).ShortCut("Ctrl+F1").Description("Abre un explorador y muestra la documentacion generada").Icon("doxy_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_DOXY_HELP,LANG(MENUITEM_TOOLS_DOXYGE_HELP,"A&yuda...")).Description("Muestra una breve ayuda acerca de la integracion de Doxygen en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_WXFB,"Diseñar &Interfases (wxFormBuilder)"),"Diseño visual de interfaces con la biblioteca wxWidgets","wxfb.png",wxID_ANY,maPROJECT);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_CONFIG,LANG(MENUITEM_TOOLS_WXFB_CONFIG,"Configurar &Integracion con wxFormBuilder...")).Description("Añade los pasos necesarios a la compilacion para utilizar wxFormBuilder").Icon("wxfb_activate.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_NEW_RES,LANG(MENUITEM_TOOLS_WXFB_NEW_RESOURCE,"&Adjuntar un Nuevo Proyecto wxFB...")).Description("Crea un nuevo proyecto wxFormBuilder y lo agrega al proyecto en ZinjaI").Icon("wxfb_new_res.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_LOAD_RES,LANG(MENUITEM_TOOLS_WXFB_LOAD_RESOURCE,"&Adjuntar un Proyecto wxFB Existente...")).Description("Agrega un proyecto wxFormBuilder ya existente al proyecto en ZinjaI").Icon("wxfb_load_res.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_REGEN,LANG(MENUITEM_TOOLS_WXFB_REGENERATE,"&Regenerar Proyectos wxFB")).ShortCut("Shift+Alt+F9").Description("Ejecuta wxFormBuilder para regenerar los archivos de recurso o fuentes que correspondan").Icon("wxfb_regen.png").Map());
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(MENUITEM_TOOLS_WXFB_INHERIT,"&Generar Clase Heredada...")).Description("Genera una nueva clase a partir de las definidas por algun proyecto wxfb").Icon("wxfb_inherit.png").Map());
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_UPDATE_INHERIT,LANG(MENUITEM_TOOLS_WXFB_UPDATE_INHERIT,"Act&ualizar Clase Heredada...")).Description("Actualiza los metodos de una clase que hereda de las definidas por algun proyecto wxfb").Icon("wxfb_update_inherit.png").Map());
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_HELP_WX,LANG(MENUITEM_TOOLS_WXFB_REFERENCE,"Referencia &wxWidgets...")).Description("Muestra la ayuda de la biblioteca wxWidgets").Icon("ayuda_wx.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_HELP,LANG(MENUITEM_TOOLS_WXFB_HELP,"A&yuda wxFB...")).Description("Muestra una breve ayuda acerca de la integracion de wxFormBuilder en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_CPPCHECK,"Análisis Estático (cppcheck)"),"","cppcheck.png",wxID_ANY,maPROJECT);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CPPCHECK_RUN, LANG(MENUITEM_TOOLS_CPPCHECK_RUN,"Iniciar...")).Icon("cppcheck_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CPPCHECK_CONFIG, LANG(MENUITEM_TOOLS_CPPCHECK_CONFIG,"Configurar...")).Icon("cppcheck_config.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CPPCHECK_VIEW, LANG(MENUITEM_TOOLS_CPPCHECK_VIEW,"Mostrar Panel de Resultados")).Icon("cppcheck_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CPPCHECK_HELP,LANG(MENUITEM_TOOLS_CPPCHECK_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_GPROF,"Perfil de Ejecución (gprof)"),"Gprof permite analizar las llamadas a funciones y sus tiempos de ejecucion.","gprof.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_SET, LANG(MENUITEM_TOOLS_GPROF_ACTIVATE,"Habilitar/Deshabilitar")).Description("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable.").Icon("comp_for_prof.png"));
			BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_GPROF_LAYOUT,"Algoritmo de Dibujo"),"Permite seleccionar entre dos algoritmos diferentes para dibujar el grafo","dotfdp.png");
				AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_DOT, LANG(MENUITEM_TOOLS_GPROF_DOT,"dot")).Checkeable(config->Init.graphviz_dot));
				AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_FDP, LANG(MENUITEM_TOOLS_GPROF_FDP,"fdp")).Checkeable(config->Init.graphviz_dot));
			EndSubMenu(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_SHOW, LANG(MENUITEM_TOOLS_GPROF_SHOW,"Visualizar Resultados (grafo)...")).Description("Muestra graficamente la informacion de profiling de la ultima ejecucion.").Icon("showgprof.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_LIST, LANG(MENUITEM_TOOLS_GPROF_LIST,"Listar Resultados (texto)")).Description("Muestra la informacion de profiling de la ultima ejecucion sin procesar.").Icon("listgprof.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GPROF_HELP,LANG(MENUITEM_TOOLS_GPROF_HELP,"A&yuda...")).Description("Muestra ayuda acerca de como generar e interpretar la informacion de profiling").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_GCOV,"&Test de Cobertura (experimental, gcov)"),"Gcov permite contabilizar cuantas veces se ejecuta cada linea del código fuente.","gcov.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GCOV_SET, LANG(MENUITEM_TOOLS_GCOV_ACTIVATE,"Habilitar/Deshabilitar")).Description("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable.").Icon("gcov_set.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GCOV_SHOW, LANG(MENUITEM_TOOLS_GCOV_SHOW_BAR,"Mostrar barra de resultados")).Description("Muestra un panel con los conteos por linea en el margen izquierdo de la ventana.").Icon("gcov_show.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GCOV_RESET, LANG(MENUITEM_TOOLS_GCOV_RESET,"Eliminar resultados")).Description("Elimina los archivos de resultados generados por el test de cobertura.").Icon("gcov_reset.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_GCOV_HELP,LANG(MENUITEM_TOOLS_GCOV_HELP,"A&yuda...")).Description("Muestra ayuda acerca de como generar e interpretar la informacion del test de cobertura").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
	#if !defined(_WIN32) && !defined(__WIN32__)
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_VALGRIND,"Análisis Dinámico (valgrind)"),"Valgrind permite analizar el uso de memoria dinamica para detectar perdidas y otros errores","valgrind.png");
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_VALGRIND_RUN, LANG(MENUITEM_TOOLS_VALGRIND_RUN,"Ejecutar...")).Icon("valgrind_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_VALGRIND_VIEW, LANG(MENUITEM_TOOLS_VALGRIND_VIEW,"Mostrar Panel de Resultados")).Icon("valgrind_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_VALGRIND_HELP, LANG(MENUITEM_TOOLS_VALGRIND_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
	#endif
		
		BeginSubMenu(mnTOOLS, LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables"),"","customTools.png",mxID_TOOLS_CUSTOM_TOOLS,maMAPPED);
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CUSTOM_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_CUSTOM_TOOLS_SETTINGS,"&Configurar (generales)...")).Icon("customToolsSettings.png"));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_PROJECT_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_PROJECT_TOOLS_SETTINGS,"&Configurar (de proyecto)...")).Icon("projectToolsSettings.png").Project(true));
			AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CUSTOM_HELP, LANG(MENUITEM_TOOLS_CUSTOM_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_CREATE_TEMPLATE, LANG(MENUITEM_TOOLS_CREATE_TEMPLATE,"Guardar como nueva plantilla...")).Description("Permite guardar el programa simple o proyecto actual como plantilla").Icon("create_template.png"));
		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_INSTALL_COMPLEMENTS, LANG(MENUITEM_TOOLS_INSTALL_COMPLEMENTS,"Instalar Complementos...")).Description("Permite instalar un complemento ya descargado para ZinjaI").Icon("updates.png"));
	}	
	
	
	menues[mnHELP].label = LANG(MENUITEM_HELP,"A&yuda"); {
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_ABOUT, LANG(MENUITEM_HELP_ABOUT,"Acerca de...")).Description("Acerca de...").Icon("acercaDe.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_TUTORIAL, LANG(MENUITEM_HELP_TUTORIALS,"Tutoriales...")).Description("Abre el cuadro de ayuda y muestra el indice de tutoriales disponibles").Icon("tutoriales.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_GUI, LANG(MENUITEM_HELP_ZINJAI,"Ayuda sobre ZinjaI...")).ShortCut("F1").Description("Muestra la ayuda sobre el uso y las caracteristicas de este entorno...").Icon("ayuda.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_CPP, LANG(MENUITEM_HELP_CPP,"Referencia C/C++...")).ShortCut("Alt+F1").Description("Muestra una completa referencia sobre el lenguaje").Icon("referencia.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_TIP, LANG(MENUITEM_HELP_TIPS,"&Mostrar sugerencias de uso...")).Description("Muestra sugerencias sobre el uso del programa...").Icon("tip.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_OPINION, LANG(MENUITEM_HELP_OPINION,"Enviar sugerencia o reportar error...")).Description("Permite acceder a los foros oficiales de ZinjaI para dejar sugerencias, comentarios o reportar errores").Icon("opinion.png"));
		AddMenuItem(mnHELP, myMenuItem(mxID_HELP_UPDATES, LANG(MENUITEM_HELP_UPDATES,"&Buscar actualizaciones...")).Description("Comprueba a traves de Internet si hay versiones mas recientes de ZinjaI disponibles...").Icon("updates.png"));
	}
	
}

void MenusAndToolsConfig::LoadToolbarData ( ) {
	
	toolbars[tbFILE].label = LANG(CAPTION_TOOLBAR_FILE,"Archivo"); {
		AddToolbarItem(tbFILE,myToolbarItem("file.new_file",menues[tbFILE],wxID_NEW)); // ,LANG(TOOLBAR_CAPTION_FILE_NEW,"Nuevo..."),ipre+_T("nuevo.png"),LANG(TOOLBAR_DESC_FILE_NEW,"Archivo -> Nuevo..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.new_project",menues[tbFILE],mxID_FILE_PROJECT)); // ,LANG(TOOLBAR_CAPTION_FILE_NEW_PROJECT,"Nuevo Proyecto..."),ipre+_T("proyecto.png"),LANG(TOOLBAR_DESC_FILE_NEW_PROJECT,"Archivo -> Nuevo Proyecto..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.open",menues[tbFILE],wxID_OPEN)); // ,LANG(TOOLBAR_CAPTION_FILE_OPEN,"Abrir..."),ipre+_T("abrir.png"),LANG(TOOLBAR_DESC_FILE_OPEN,"Archivo -> Abrir..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.recent_simple",menues[tbFILE],mxID_FILE_SOURCE_HISTORY_MORE)); // ,LANG(TOOLBAR_CAPTION_FILE_RECENT_SOURCES,"Fuentes Recientes..."),ipre+_T("recentSimple.png"),LANG(TOOLBAR_DESC_FILE_RECENT_SOURCES,"Archivo -> Fuentes Abiertos Recientemente..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.recent_project",menues[tbFILE],mxID_FILE_PROJECT_HISTORY_MORE)); // ,LANG(TOOLBAR_CAPTION_FILE_RECENT_PROJECTS,"Proyectos Recientes..."),ipre+_T("recentProject.png"),LANG(TOOLBAR_DESC_FILE_RECENT_PROJECTS,"Archivo -> Proyectos Abiertos Recientemete..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.open_header",menues[tbFILE],mxID_FILE_OPEN_H)); // ,LANG(TOOLBAR_CAPTION_FILE_OPEN_H,"Abrir h/cpp Complementario"),ipre+_T("abrirp.png"),LANG(TOOLBAR_DESC_FILE_OPEN_H,"Abrir h/cpp Complementario"));
		AddToolbarItem(tbFILE,myToolbarItem("file.open_selected",menues[tbFILE],mxID_FILE_OPEN_SELECTED)); // ,LANG(TOOLBAR_CAPTION_FILE_OPEN_SELECTED,"Abrir Seleccionado"),ipre+_T("abrirh.png"),LANG(TOOLBAR_DESC_FILE_OPEN_SELECTED,"Archivo -> Abrir Seleccionado"));
		AddToolbarItem(tbFILE,myToolbarItem("file.save",menues[tbFILE],wxID_SAVE)); // ,LANG(TOOLBAR_CAPTION_FILE_SAVE,"Guardar"),ipre+_T("guardar.png"),LANG(TOOLBAR_DESC_FILE_SABE,"Archivo -> Guardar"));
		AddToolbarItem(tbFILE,myToolbarItem("file.save_as",menues[tbFILE],wxID_SAVEAS)); // ,LANG(TOOLBAR_CAPTION_FILE_SAVE_AS,"Guardar Como..."),ipre+_T("guardarComo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_AS,"Archivo -> Guardar Como..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.save_all",menues[tbFILE],mxID_FILE_SAVE_ALL)); // ,LANG(TOOLBAR_CAPTION_FILE_SAVE_ALL,"Guardar Todo"),ipre+_T("guardarTodo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_ALL,"Archivo -> Guardar Todo"));
		AddToolbarItem(tbFILE,myToolbarItem("file.save_project",menues[tbFILE],mxID_FILE_SAVE_PROJECT)); // ,LANG(TOOLBAR_CAPTION_FILE_SAVE_PROJECT,"Guardar Proyecto"),ipre+_T("guardarProyecto.png"),LANG(TOOLBAR_DESC_FILE_SAVE_PROJECT,"Archivo -> Guardar Proyecto"));
		AddToolbarItem(tbFILE,myToolbarItem("file.export_html",menues[tbFILE],mxID_FILE_EXPORT_HTML)); // ,LANG(TOOLBAR_CAPTION_FILE_EXPORT_HTML,"Exportar a HTML..."),ipre+_T("exportHtml.png"),LANG(TOOLBAR_DESC_FILE_EXPORT_HTML,"Archivo -> Exportar a HTML..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.print",menues[tbFILE],mxID_FILE_PRINT)); // ,LANG(TOOLBAR_CAPTION_FILE_PRINT,"Imprimir..."),ipre+_T("imprimir.png"),LANG(TOOLBAR_DESC_FILE_PRINT,"Archivo -> Imprimir..."));
		AddToolbarItem(tbFILE,myToolbarItem("file.reload",menues[tbFILE],mxID_FILE_RELOAD)); // ,LANG(TOOLBAR_CAPTION_FILE_RELOAD,"Recargar"),ipre+_T("recargar.png"),LANG(TOOLBAR_DESC_FILE_RELOAD,"Archivo -> Recargar"));
		AddToolbarItem(tbFILE,myToolbarItem("file.close",menues[tbFILE],wxID_CLOSE)); // ,LANG(TOOLBAR_CAPTION_FILE_CLOSE,"Cerrar"),ipre+_T("cerrar.png"),LANG(TOOLBAR_DESC_FILE_CLOSE,"Archivo -> Cerrar"));
		AddToolbarItem(tbFILE,myToolbarItem("file.close_all",menues[tbFILE],mxID_FILE_CLOSE_ALL)); // ,LANG(TOOLBAR_CAPTION_FILE_CLOSE_ALL,"Cerrar Todo"),ipre+_T("cerrarTodo.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_ALL,"Archivo -> Cerrar Todo"));
		AddToolbarItem(tbFILE,myToolbarItem("file.close_project",menues[tbFILE],mxID_FILE_CLOSE_PROJECT)); // ,LANG(TOOLBAR_CAPTION_FILE_CLOSE_PROJECT,"Cerrar Proyecto"),ipre+_T("cerrarProyecto.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_PROJECT,"Archivo -> Cerrar Proyecto"));
		AddToolbarItem(tbFILE,myToolbarItem("file.project_config",menues[tbFILE],mxID_FILE_PROJECT_CONFIG)); // ,LANG(TOOLBAR_CAPTION_FILE_PROJECT_CONFIG,"Configuracion de proyecto..."),ipre+_T("projectConfig.png"),LANG(TOOLBAR_DESC_FILE_PROJECT_CONFIG,"Archivo -> Configuracion del Proyecto..."));
	}
	
	
	toolbars[tbEDIT].label = LANG(CAPTION_TOOLBAR_EDIT,"Edición"); {
		AddToolbarItem(tbEDIT,myToolbarItem("edit.undo",menues[tbEDIT],wxID_UNDO)); // ,LANG(TOOLBAR_CAPTION_EDIT_UNDO,"Deshacer"),ipre+_T("deshacer.png"),LANG(TOOLBAR_DESC_EDIT_UNDO,"Editar -> Deshacer"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.redo",menues[tbEDIT],wxID_REDO)); // ,LANG(TOOLBAR_CAPTION_EDIT_REDO,"Rehacer"),ipre+_T("rehacer.png"),LANG(TOOLBAR_DESC_EDIT_REDO,"Editar -> Rehacer"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.copy",menues[tbEDIT],wxID_COPY)); // ,LANG(TOOLBAR_CAPTION_EDIT_COPY,"Copiar"),ipre+_T("copiar.png"),LANG(TOOLBAR_DESC_EDIT_COPY,"Editar -> Copiar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.cut",menues[tbEDIT],wxID_CUT)); // ,LANG(TOOLBAR_CAPTION_EDIT_CUT,"Cortar"),ipre+_T("cortar.png"),LANG(TOOLBAR_DESC_EDIT_CUT,"Editar -> Cortar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.paste",menues[tbEDIT],wxID_PASTE)); // ,LANG(TOOLBAR_CAPTION_EDIT_PASTE,"Pegar"),ipre+_T("pegar.png"),LANG(TOOLBAR_DESC_EDIT_PASTE,"Editar -> Pegar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.move_up",menues[tbEDIT],mxID_EDIT_TOGGLE_LINES_UP)); // ,LANG(TOOLBAR_CAPTION_EDIT_LINES_UP,"Mover Hacia Arriba"),ipre+_T("toggleLinesUp.png"),LANG(TOOLBAR_DESC_EDIT_LINES_UP,"Editar -> Mover Hacia Arriba"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.move_down",menues[tbEDIT],mxID_EDIT_TOGGLE_LINES_DOWN)); // ,LANG(TOOLBAR_CAPTION_EDIT_LINES_DOWN,"Mover Hacia Abajo"),ipre+_T("toggleLinesDown.png"),LANG(TOOLBAR_DESC_EDIT_LINES_DOWN,"Editar -> Mover Hacia Abajo"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.duplicate",menues[tbEDIT],mxID_EDIT_DUPLICATE_LINES)); // ,LANG(TOOLBAR_CAPTION_EDIT_DUPLICATE_LINES,"Duplicar Linea(s)"),ipre+_T("duplicarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DUPLICATE_LINES,"Editar -> Duplicar Linea(s)"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.delete_lines",menues[tbEDIT],mxID_EDIT_DELETE_LINES)); // ,LANG(TOOLBAR_CAPTION_EDIT_DELETE_LINES,"Eliminar Linea(s)"),ipre+_T("borrarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DELETE_LINES,"Editar -> Eliminar Linea(s)"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.goto_line",menues[tbEDIT],mxID_EDIT_GOTO)); // ,LANG(TOOLBAR_CAPTION_EDIT_GOTO_LINE,"Ir a Linea..."),ipre+_T("irALinea.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_LINE,"Editar -> Ir a linea..."));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.goto_class",menues[tbEDIT],mxID_EDIT_GOTO_FUNCTION)); // ,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FUNCTION,"Ir a Clase/Metodo/Funcion..."),ipre+_T("irAFuncion.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FUNCTION,"Editar -> Ir a Clase/Metodo/Funcion..."));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.goto_file",menues[tbEDIT],mxID_EDIT_GOTO_FILE)); // ,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FILE,"Ir a Archivo..."),ipre+_T("irAArchivo.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FILE,"Editar -> Ir a Archivo..."));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.find",menues[tbEDIT],mxID_EDIT_FIND)); // ,LANG(TOOLBAR_CAPTION_EDIT_FIND,"Buscar..."),ipre+_T("buscar.png"),LANG(TOOLBAR_DESC_EDIT_FIND,"Editar -> Buscar..."));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.find_prev",menues[tbEDIT],mxID_EDIT_FIND_PREV)); // ,LANG(TOOLBAR_CAPTION_EDIT_FIND_PREV,"Buscar anterior"),ipre+_T("buscarAnterior.png"),LANG(TOOLBAR_DESC_EDIT_FIND_PREV,"Editar -> Buscar Anterior"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.find_next",menues[tbEDIT],mxID_EDIT_FIND_NEXT)); // ,LANG(TOOLBAR_CAPTION_EDIT_FIND_NEXT,"Buscar siguiente"),ipre+_T("buscarSiguiente.png"),LANG(TOOLBAR_DESC_EDIT_FIND_NEXT,"Editar -> Buscar Siguiente"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.replace",menues[tbEDIT],mxID_EDIT_REPLACE)); // ,LANG(TOOLBAR_CAPTION_EDIT_REPLACE,"Reemplazar..."),ipre+_T("reemplazar.png"),LANG(TOOLBAR_DESC_EDIT_REPLACE,"Editar -> Reemplazar..."));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.inser_header",menues[tbEDIT],mxID_EDIT_INSERT_HEADER)); // ,LANG(TOOLBAR_CAPTION_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),ipre+_T("insertarInclude.png"),LANG(TOOLBAR_DESC_EDIT_INSERT_INCLUDE,"Editar -> Insertar #include Correspondiente"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.comment",menues[tbEDIT],mxID_EDIT_COMMENT)); // ,LANG(TOOLBAR_CAPTION_EDIT_COMMENT,"Comentar"),ipre+_T("comentar.png"),LANG(TOOLBAR_DESC_EDIT_COMMNENT,"Editar -> Comentar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.uncomment",menues[tbEDIT],mxID_EDIT_UNCOMMENT)); // ,LANG(TOOLBAR_CAPTION_EDIT_UNCOMMENT,"Descomentar"),ipre+_T("descomentar.png"),LANG(TOOLBAR_DESC_EDIT_UNCOMMENT,"Editar -> Descomentar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.indent",menues[tbEDIT],mxID_EDIT_INDENT)); // ,LANG(TOOLBAR_CAPTION_EDIT_INDENT,"Indentar"),ipre+_T("indent.png"),LANG(TOOLBAR_DESC_EDIT_INDENT,"Editar -> Indentar"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.select_block",menues[tbEDIT],mxID_EDIT_BRACEMATCH)); // ,LANG(TOOLBAR_CAPTION_EDIT_BRACEMATCH,"Seleccionar Bloque"),ipre+_T("mostrarLlave.png"),LANG(TOOLBAR_DESC_EDIT_BRACEMATCH,"Editar -> Seleccionar Bloque"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.select_all",menues[tbEDIT],wxID_SELECTALL)); // ,LANG(TOOLBAR_CAPTION_EDIT_SELECT_ALL,"Seleccionar Todo"),ipre+_T("seleccionarTodo.png"),LANG(TOOLBAR_DESC_EDIT_SELECT_ALL,"Editar -> Seleccionar Todo"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.toggle_user_mark",menues[tbEDIT],mxID_EDIT_MARK_LINES)); // ,LANG(TOOLBAR_CAPTION_EDIT_HIGHLIGHT_LINES,"Resaltar Linea(s)/Quitar Resaltado"),ipre+_T("marcar.png"),LANG(TOOLBAR_DESC_EDIT_HIGHLIGHT_LINES,"Editar -> Resaltar Linea(s)/Quitar Resaltado"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.find_user_mark",menues[tbEDIT],mxID_EDIT_GOTO_MARK)); // ,LANG(TOOLBAR_CAPTION_EDIT_FIND_HIGHLIGHTS,"Buscar Resaltado"),ipre+_T("irAMarca.png"),LANG(TOOLBAR_DESC_EDIT_FIND_HIGHLIGHTS,"Editar -> Buscar Resaltado"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.list_user_marks",menues[tbEDIT],mxID_EDIT_LIST_MARKS)); // ,LANG(TOOLBAR_CAPTION_EDIT_LIST_HIGHLIGHTS,"Listar Lineas Resaltadas"),ipre+_T("listarMarcas.png"),LANG(TOOLBAR_DESC_EDIT_LIST_HIGHLIGHTS,"Editar -> Listar Lineas Resaltadas"));
		AddToolbarItem(tbEDIT,myToolbarItem("edit.autocomplete",menues[tbEDIT],mxID_EDIT_FORCE_AUTOCOMPLETE)); // ,LANG(TOOLBAR_CAPTION_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),ipre+_T("autocompletar.png"),LANG(TOOLBAR_DESC_EDIT_FORCE_AUTOCOMPLETE,"Editar -> Autocompletar"));
	}
	
	
	toolbars[tbVIEW].label = LANG(CAPTION_TOOLBAR_VIEW,"Ver"); {
		AddToolbarItem(tbVIEW,myToolbarItem("view.split_view",menues[tbVIEW],mxID_VIEW_DUPLICATE_TAB)); // ,LANG(TOOLBAR_CAPTION_VIEW_SPLIT_VIEW,"Duplicar Vista"),ipre+_T("duplicarVista.png"),LANG(TOOLBAR_DESC_VIEW_SPLIT_VIEW,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.line_wrap",menues[tbVIEW],mxID_VIEW_LINE_WRAP)); // ,LANG(TOOLBAR_CAPTION_VIEW_LINE_WRAP,"Ajuste de Linea"),ipre+_T("lineWrap.png"),LANG(TOOLBAR_DESC_VIEW_LINE_WRAP,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.white_space",menues[tbVIEW],mxID_VIEW_WHITE_SPACE)); // ,LANG(TOOLBAR_CAPTION_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de de fin de linea"),ipre+_T("whiteSpace.png"),LANG(TOOLBAR_DESC_VIEW_WHITE_SPACES,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.sintax_colour",menues[tbVIEW],mxID_VIEW_CODE_STYLE)); // ,LANG(TOOLBAR_CAPTION_VIEW_SYNTAX_HIGHLIGHT,"Colorear Sintaxis"),ipre+_T("syntaxColour.png"),LANG(TOOLBAR_DESC_VIEW_SYNTAX_HIGHLIGHT,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.update_symbols",menues[tbVIEW],mxID_VIEW_UPDATE_SYMBOLS)); // ,LANG(TOOLBAR_CAPTION_VIEW_SYMBOLS_TREE,"Arbol de simbolos"),ipre+_T("symbolsTree.png"),LANG(TOOLBAR_DESC_VIEW_SYMBOLS_TREE,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.explorer_tree",menues[tbVIEW],mxID_VIEW_EXPLORER_TREE)); // ,LANG(TOOLBAR_CAPTION_VIEW_EXPLORER_TREE,"Explorador de Archivos"),ipre+_T("explorerTree.png"),LANG(TOOLBAR_DESC_VIEW_EXPLORER_TREE,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.project_tree",menues[tbVIEW],mxID_VIEW_PROJECT_TREE)); // ,LANG(TOOLBAR_CAPTION_VIEW_PROJECT_TREE,"Arbol de Proyecto"),ipre+_T("projectTree.png"),LANG(TOOLBAR_DESC_VIEW_PROJECT_TREE,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.compiler_tree",menues[tbVIEW],mxID_VIEW_COMPILER_TREE)); // ,LANG(TOOLBAR_CAPTION_VIEW_COMPILER_TREE,"Resultados de La Compilacion"),ipre+_T("compilerTree.png"),LANG(TOOLBAR_DESC_VIEW_COMPILER_TREE,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.full_screen",menues[tbVIEW],mxID_VIEW_FULLSCREEN)); // ,LANG(TOOLBAR_CAPTION_VIEW_FULLSCREEN,"Pantalla Completa"),ipre+_T("fullScreen.png"),LANG(TOOLBAR_DESC_VIEW_FULLSCREEN,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.beginner_panel",menues[tbVIEW],mxID_VIEW_BEGINNER_PANEL)); // ,LANG(TOOLBAR_CAPTION_VIEW_BEGINNER_PANEL,"Panel de Asistencias"),ipre+_T("beginer_panel.png"),LANG(TOOLBAR_DESC_VIEW_BEGINER_PANEL,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.prev_error",menues[tbVIEW],mxID_VIEW_PREV_ERROR)); // ,LANG(TOOLBAR_CAPTION_VIEW_PREV_ERROR,"Ir a Error Anterior"),ipre+_T("errorPrev.png"),LANG(TOOLBAR_DESC_VIEW_PREV_ERROR,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.next_error",menues[tbVIEW],mxID_VIEW_NEXT_ERROR)); // ,LANG(TOOLBAR_CAPTION_VIEW_NEXT_ERROR,"Ir a siguiente error"),ipre+_T("errorNext.png"),LANG(TOOLBAR_DESC_VIEW_NEXT_ERROR,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_all",menues[tbVIEW],mxID_FOLD_HIDE_ALL)); // ,LANG(TOOLBAR_CAPTION_FOLD_ALL_LEVELS,"Plegar Todos los Niveles"),ipre+_T("foldAll.png"),LANG(TOOLBAR_DESC_FOLD_ALL_LEVELS,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_1",menues[tbVIEW],mxID_FOLD_HIDE_1)); // 1,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_1,"Plegar el Primer Nivel"),ipre+_T("fold1.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_1,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_2",menues[tbVIEW],mxID_FOLD_HIDE_2)); // 2,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_2,"Plegar el Segundo Nivel"),ipre+_T("fold2.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_2,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_3",menues[tbVIEW],mxID_FOLD_HIDE_3)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_4",menues[tbVIEW],mxID_FOLD_HIDE_4)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.fold_5",menues[tbVIEW],mxID_FOLD_HIDE_5)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_all",menues[tbVIEW],mxID_FOLD_SHOW_ALL)); // ,LANG(TOOLBAR_CAPTION_UNFOLD_ALL_LEVELS,"Desplegar Todos los Niveles"),ipre+_T("unfoldAll.png"),LANG(TOOLBAR_DESC_UNFOLD_ALL_LEVELS,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_1",menues[tbVIEW],mxID_FOLD_SHOW_1)); // 1,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_1,"Desplegar el Primer Nivel"),ipre+_T("unfold1.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_1,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_2",menues[tbVIEW],mxID_FOLD_SHOW_2)); // 2,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_2,"Desplegar el Segundo Nivel"),ipre+_T("unfold2.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_2,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_3",menues[tbVIEW],mxID_FOLD_SHOW_3)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_4",menues[tbVIEW],mxID_FOLD_SHOW_4)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem("view.unfold_5",menues[tbVIEW],mxID_FOLD_SHOW_5)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
	}
	
	toolbars[tbRUN].label = LANG(CAPTION_TOOLBAR_RUN,"Ejecución"); {
		AddToolbarItem(tbRUN,myToolbarItem("run.compile",menues[tbRUN],mxID_RUN_COMPILE)); // ,LANG(TOOLBAR_CAPTION_RUN_COMPILE,"Compilar"),ipre+_T("compilar.png"),LANG(TOOLBAR_DESC_RUN_COMPILE,"Ejecucion -> Compilar"));
		AddToolbarItem(tbRUN,myToolbarItem("run.run",menues[tbRUN],mxID_RUN_RUN)); // ,LANG(TOOLBAR_CAPTION_RUN_RUN,"Guardar, Compilar y Ejecutar..."),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_RUN_RUN,"Ejecucion -> Guardar, Compilar y Ejecutar"));
		AddToolbarItem(tbRUN,myToolbarItem("run.run_old",menues[tbRUN],mxID_RUN_RUN_OLD)); // ,LANG(TOOLBAR_CAPTION_RUN_OLD,"Ejecutar Sin Recompilar..."),ipre+_T("ejecutar_old.png"),LANG(TOOLBAR_DESC_RUN_OLD,"Ejecucion -> Ejecutar Sin Recompilar"));
		AddToolbarItem(tbRUN,myToolbarItem("run.stop",menues[tbRUN],mxID_RUN_STOP)); // ,LANG(TOOLBAR_CAPTION_RUN_STOP,"Detener"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_RUN_STOP,"Ejecucion -> Detener"));
		AddToolbarItem(tbRUN,myToolbarItem("run.clean",menues[tbRUN],mxID_RUN_CLEAN)); // ,LANG(TOOLBAR_CAPTION_RUN_CLEAN,"Limpiar"),ipre+_T("limpiar.png"),LANG(TOOLBAR_DESC_RUN_CLEAN,"Ejecucion -> Limpiar"));
		AddToolbarItem(tbRUN,myToolbarItem("run.options",menues[tbRUN],mxID_RUN_CONFIG)); // ,LANG(TOOLBAR_CAPTION_RUN_OPTIONS,"Opciones de compilacion y ejecucion..."),ipre+_T("opciones.png"),LANG(TOOLBAR_DESC_RUN_OPTIONS,"Ejecucion -> Opciones..."));
		AddToolbarItem(tbRUN,myToolbarItem("run.debug",menues[tbRUN],mxID_DEBUG_RUN)); // ,LANG(TOOLBAR_CAPTION_DEBUG_RUN,"Depurar"),ipre+_T("depurar.png"),LANG(TOOLBAR_DESC_DEBUG_RUN,"Depurar -> Iniciar"));
#ifndef __WIN32__
		AddToolbarItem(tbRUN,myToolbarItem("run.debug_attach",menues[tbRUN],mxID_DEBUG_ATTACH)); // ,LANG(TOOLBAR_CAPTION_DEBUG_ATTACH,"Attachear Depurador"),ipre+_T("debug_attach.png"),LANG(TOOLBAR_DESC_DEBUG_ATTACH,"Depurar -> Adjuntar"));
		AddToolbarItem(tbRUN,myToolbarItem("run.load_core_dump",menues[tbRUN],mxID_DEBUG_CORE_DUMP)); // ,LANG(TOOLBAR_CAPTION_DEBUG_CORE_DUMP,"Cargar Volcado de Memoria"),ipre+_T("core_dump.png"),LANG(TOOLBAR_DESC_DEBUG_CORE_DUMP,"Depurar -> Cargar Volcado de Memoria"));
#endif
		AddToolbarItem(tbRUN,myToolbarItem("run.break_toggle",menues[tbRUN],mxID_DEBUG_TOGGLE_BREAKPOINT)); // ,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		AddToolbarItem(tbRUN,myToolbarItem("run.break_options",menues[tbRUN],mxID_DEBUG_BREAKPOINT_OPTIONS)); // ,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		AddToolbarItem(tbRUN,myToolbarItem("run.break_list",menues[tbRUN],mxID_DEBUG_LIST_BREAKPOINTS)); // ,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		AddToolbarItem(tbRUN,myToolbarItem("run.inspections",menues[tbRUN],mxID_DEBUG_INSPECT)); // ,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
	}
	
	
	toolbars[tbDEBUG].label = LANG(CAPTION_TOOLBAR_DEBUG,"Depuración"); {
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.start",menues[tbDEBUG],mxID_DEBUG_RUN)); // ,LANG(TOOLBAR_CAPTION_DEBUG_START,"Comenzar/Continuar Depuracion"),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_DEBUG_START,"Depurar -> Comenzar/Continuar"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.pause",menues[tbDEBUG],mxID_DEBUG_PAUSE)); // ,LANG(TOOLBAR_CAPTION_DEBUG_PAUSE,"Interrumpir Ejecucion en Depuracion"),ipre+_T("pausar.png"),LANG(TOOLBAR_DESC_DEBUG_PAUSE,"Depurar -> Interrumpir"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.stop",menues[tbDEBUG],mxID_DEBUG_STOP)); // ,LANG(TOOLBAR_CAPTION_DEBUG_STOP,"Detener Depuracion"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_DEBUG_STOP,"Depurar -> Detener"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.step_in",menues[tbDEBUG],mxID_DEBUG_STEP_IN)); // ,LANG(TOOLBAR_CAPTION_DEBUG_STEP_IP,"Step In"),ipre+_T("step_in.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_IN,"Depurar -> Step In"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.step_over",menues[tbDEBUG],mxID_DEBUG_STEP_OVER)); // ,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OVER,"Step Over"),ipre+_T("step_over.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OVER,"Depurar -> Step Over"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.step_out",menues[tbDEBUG],mxID_DEBUG_STEP_OUT)); // ,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OUT,"Step Out"),ipre+_T("step_out.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OUT,"Depurar -> Step Out"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.run_until",menues[tbDEBUG],mxID_DEBUG_RUN_UNTIL)); // ,LANG(TOOLBAR_CAPTION_DEBUG_RUN_UNTIL,"Ejecutar Hasta El Cursor"),ipre+_T("run_until.png"),LANG(TOOLBAR_DESC_DEBUG_RUN_UNTIL,"Depurar -> Ejecutar Hasta el Cursos"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.function_return",menues[tbDEBUG],mxID_DEBUG_RETURN)); // ,LANG(TOOLBAR_CAPTION_DEBUG_RETURN,"Return"),ipre+_T("return.png"),LANG(TOOLBAR_DESC_DEBUG_RETURN,"Depurar -> Return"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.jump",menues[tbDEBUG],mxID_DEBUG_JUMP)); // ,LANG(TOOLBAR_CAPTION_DEBUG_JUMP,"Continuar desde aqui"),ipre+_T("debug_jump.png"),LANG(TOOLBAR_DESC_DEBUG_JUMP,"Depurar -> Continuar Desde Aqui"));
#ifndef __WIN32__
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.enable_inverse_exec",menues[tbDEBUG],mxID_DEBUG_ENABLE_INVERSE_EXEC)); // ,LANG(TOOLBAR_CAPTION_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras"),ipre+_T("reverse_enable.png"),LANG(TOOLBAR_DESC_DEBUG_ENABLE_INVERSE,"Depurar -> Habilitar Ejecucion Hacia Atras"),wxITEM_CHECK);
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.inverse_exec",menues[tbDEBUG],mxID_DEBUG_INVERSE_EXEC)); // ,LANG(TOOLBAR_CAPTION_DEBUG_INVERSE,"Ejecutar Hacia Atras"),ipre+_T("reverse_toggle.png"),LANG(TOOLBAR_DESC_DEBUG_INVERSE,"Depurar -> Ejecutar Hacia Atras..."),wxITEM_CHECK);
#endif
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.inspections",menues[tbDEBUG],mxID_DEBUG_INSPECT)); // ,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.backtrace",menues[tbDEBUG],mxID_DEBUG_BACKTRACE)); // ,LANG(TOOLBAR_CAPTION_BACKTRACE,"Trazado Inverso"),ipre+_T("backtrace.png"),LANG(TOOLBAR_DESC_BACKTRACE,"Depurar -> Trazado Inverso"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.threadlist",menues[tbDEBUG],mxID_DEBUG_THREADLIST)); // ,LANG(TOOLBAR_CAPTION_THREADLIST,"Hilos de Ejecucion"),ipre+_T("threadlist.png"),LANG(TOOLBAR_DESC_THREADLIST,"Depurar -> Hilos de Ejecucion"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.break_toggle",menues[tbDEBUG],mxID_DEBUG_TOGGLE_BREAKPOINT)); // ,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.break_options",menues[tbDEBUG],mxID_DEBUG_BREAKPOINT_OPTIONS)); // ,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.break_list",menues[tbDEBUG],mxID_DEBUG_LIST_BREAKPOINTS)); // ,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		AddToolbarItem(tbDEBUG,myToolbarItem("debug.log_panel",menues[tbDEBUG],mxID_DEBUG_LOG_PANEL)); // ,LANG(TOOLBAR_CAPTION_DEBUG_SHOW_LOG_PANEL,"Mostrar Mensajes del Depurador"),ipre+_T("debug_log_panel.png"),LANG(TOOLBAR_DESC_DEBUG_SHOW_LOG_PANEL,"Depurar -> Mostrar Mensajes del Depurador..."));
	}
	
	
	toolbars[tbTOOLS].label = LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"); {
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.draw_flow",menues[tbTOOLS],mxID_TOOLS_DRAW_FLOW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de Flujo..."),ipre+_T("flujo.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_FLOWCHART,"Herramientas -> Dibujar Diagrama de Flujo..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.draw_classes",menues[tbTOOLS],mxID_TOOLS_DRAW_CLASSES)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar Jerarquia de Clases..."),ipre+_T("clases.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_CLASS_HIERARCHY,"Herramientas -> Dibujar Jerarquia de Clases..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.copy_code_from_h",menues[tbTOOLS],mxID_TOOLS_CODE_COPY_FROM_H)); // ,LANG(TOOLBAR_CAPTION_TOOLS_COPY_CODE_FROM_H,"Implementar Metodos/Funciones Faltantes..."),ipre+_T("copy_code_from_h.png"),LANG(TOOLBAR_DESC_TOOLS_COPY_CODE_FROM_H,"Herramientas -> Implementar Metodos/Funciones Faltantes..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.align_comments",menues[tbTOOLS],mxID_TOOLS_ALIGN_COMMENTS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_ALIGN_COMMENTS,"Alinear Comentarios..."),ipre+_T("align_comments.png"),LANG(TOOLBAR_TOOLS_DESC_COMMENTS_ALIGN_COMMENTS,"Herramientas -> Comentarios -> Alinear Comentarios..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.remove_comments",menues[tbTOOLS],mxID_TOOLS_REMOVE_COMMENTS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_DELETE_COMMENTS,"Eliminar Comentarios"),ipre+_T("remove_comments.png"),LANG(TOOLBAR_DESC_TOOLS_COMMENTS_DELETE_COMMENTS,"Herramientas -> Comtearios -> Eliminar Comentarios"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.preproc_mark_valid",menues[tbTOOLS],mxID_TOOLS_PREPROC_MARK_VALID)); // ,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas"),ipre+_T("preproc_mark_valid.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_MARK_VALID,"Herramientas -> Preprocesador -> Marcar Lineas No Compiladas"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.preproc_unmark_all",menues[tbTOOLS],mxID_TOOLS_PREPROC_UNMARK_ALL)); // ,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas"),ipre+_T("preproc_unmark_all.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_UNMARK_ALL,"Herramientas -> Preprocesador -> Borrar Marcas"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.preproc_expand_macros",menues[tbTOOLS],mxID_TOOLS_PREPROC_EXPAND_MACROS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros"),ipre+_T("preproc_expand_macros.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_EXPAND_MACROS,"Herramientas -> Preprocesador -> Expandir Macros"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.generate_makefile",menues[tbTOOLS],mxID_TOOLS_MAKEFILE)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GENERATE_MAKEFILE,"Generar Makefile..."),ipre+_T("makefile.png"),LANG(TOOLBAR_DESC_TOOLS_GENERATE_MAKEFILE,"Herramientas -> Generar Makefile..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.open_terminal",menues[tbTOOLS],mxID_TOOLS_CONSOLE)); // ,LANG(TOOLBAR_CAPTION_TOOLS_OPEN_TERMINAL,"Abrir terminal..."),ipre+_T("console.png"),LANG(TOOLBAR_DESC_OPEN_TERMINAL,"Herramientas -> Abrir consola..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.exe_info",menues[tbTOOLS],mxID_TOOLS_EXE_PROPS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_EXE_INFO,"Propiedades del Ejecutable..."),ipre+_T("exeinfo.png"),LANG(TOOLBAR_DESC_EXE_INFO,"Herramientas -> Propiedades del Ejecutable..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.proy_stats",menues[tbTOOLS],mxID_TOOLS_PROJECT_STATISTICS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_PROY_STATS,"Estadisticas del Proyecto..."),ipre+_T("proystats.png"),LANG(TOOLBAR_DESC_PROY_STATS,"Herramientas -> Estadisticas del Proyecto..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.draw_classes",menues[tbTOOLS],mxID_TOOLS_DRAW_CLASSES)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),ipre+_T("draw_project.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_PROJECT,"Herramientas -> Grafo del Proyecto..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.open_shared",menues[tbTOOLS],mxID_TOOLS_SHARE_OPEN)); // ,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_OPEN,"Abrir Archivo Compartido..."),ipre+_T("abrirs.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_OPEN,"Herramientas -> Compartir Archivos en la Red Local -> Abrir Compartido..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.share_source",menues[tbTOOLS],mxID_TOOLS_SHARE_SHARE)); // ,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_SHARE,"Compartir Fuente..."),ipre+_T("compartir.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_SHARE,"Herramientas -> Compartir Archivos en la Red Local -> Compartir Actual..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.share_list",menues[tbTOOLS],mxID_TOOLS_SHARE_LIST)); // ,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_LIST,"Listar Compartidos Propios..."),ipre+_T("share_list.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_LIST,"Herramientas -> Compartir Archivos en la Red Local -> Listar Compartidos Propios..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_two_sources",menues[tbTOOLS],mxID_TOOLS_DIFF_TWO)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_TWO,"Compara Dos Fuentes Abiertos..."),ipre+_T("diff_sources.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_TWO,"Herramientas -> Comparar Archivos -> Dos Fuentes Abiertos"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_other_file",menues[tbTOOLS],mxID_TOOLS_DIFF_DISK)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISK,"Comparar Fuente con Archivo en Disco..."),ipre+_T("diff_source_file.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISK,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Archivo en Disco"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_himself",menues[tbTOOLS],mxID_TOOLS_DIFF_HIMSELF)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_HIMSELF,"Comparar Cambios en el Fuente con su Version en Disco..."),ipre+_T("diff_source_himself.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_HIMSELF,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Version en Disco"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_show",menues[tbTOOLS],mxID_TOOLS_DIFF_SHOW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_apply",menues[tbTOOLS],mxID_TOOLS_DIFF_APPLY)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_discard",menues[tbTOOLS],mxID_TOOLS_DIFF_DISCARD)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.diff_clear",menues[tbTOOLS],mxID_TOOLS_DIFF_CLEAR)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.doxy_generate",menues[tbTOOLS],mxID_TOOLS_DOXY_GENERATE)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_GENERATE,"Generar Documentacion Doxygen..."),ipre+_T("doxy_run.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_GENERATE,"Herramientas -> Generar Documentacion -> Generar..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.doxy_config",menues[tbTOOLS],mxID_TOOLS_DOXY_CONFIG)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_CONFIGURE,"Configurar Documentacion Doxygen..."),ipre+_T("doxy_config.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_CONFIGURE,"Herramientas -> Generar Documentacion -> Configurar..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.doxy_view",menues[tbTOOLS],mxID_TOOLS_DOXY_VIEW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_VIEW,"Ver Documentacion Doxygen..."),ipre+_T("doxy_view.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_VIEW,"Herramientas -> Generar Documentacion -> Ver..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_config",menues[tbTOOLS],mxID_TOOLS_WXFB_CONFIG)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_CONFIG,"Activar Integracion wxFormBuilder"),ipre+_T("wxfb_activate.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_ACTIVATE,"Herramientas -> wxFormBuilder -> Configurar Integracion wxFormBuilder"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_new_res",menues[tbTOOLS],mxID_TOOLS_WXFB_NEW_RES)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_NEW_RESOURCE,"Adjuntar un Nuevo Proyecto wxFB"),ipre+_T("wxfb_new_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_NEW_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Nuevo Proyecto wxFB"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_load_res",menues[tbTOOLS],mxID_TOOLS_WXFB_LOAD_RES)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_LOAD_RESOURCE,"Adjuntar un Proyecto wxFB Existente"),ipre+_T("wxfb_load_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_LOAD_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Proyecto wxFB Existente"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_regen",menues[tbTOOLS],mxID_TOOLS_WXFB_REGEN)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REGENERATE,"Regenerar Proyectos wxFB"),ipre+_T("wxfb_regen.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REGENERATE,"Herramientas -> wxFormBuilder -> Regenerar Proyectos wxFB"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_inherit",menues[tbTOOLS],mxID_TOOLS_WXFB_INHERIT_CLASS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),ipre+_T("wxfb_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_INHERIT,"Herramientas -> wxFormBuilder -> Generar Clase Heredada..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_update_inherit",menues[tbTOOLS],mxID_TOOLS_WXFB_UPDATE_INHERIT)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_UPDATE_INHERIT,"Actualizar Clase Heredada..."),ipre+_T("wxfb_update_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_UPDATE_INHERIT,"Herramientas -> wxFormBuilder -> Actualizar Clase Heredada..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.wxfb_help_wx",menues[tbTOOLS],mxID_TOOLS_WXFB_HELP_WX)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),ipre+_T("ayuda_wx.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REFERENCE,"Herramientas -> wxFormBuilder -> Referencia wxWidgets..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.cppcheck_run",menues[tbTOOLS],mxID_TOOLS_CPPCHECK_RUN)); // ,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_RUN,"Comenzar Analisis Estatico..."),ipre+_T("cppcheck_run.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_RUN,"Herramientas -> Analisis Estatico -> Iniciar..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.cppcheck_config",menues[tbTOOLS],mxID_TOOLS_CPPCHECK_CONFIG)); // ,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_CONFIG,"Configurar Analisis Estatico..."),ipre+_T("cppcheck_config.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_CONFIG,"Herramientas -> Analisis Estatico-> Configurar..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.cppcheck_view",menues[tbTOOLS],mxID_TOOLS_CPPCHECK_VIEW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_VIEW,"Mostrar Resultados del Analisis Estatico"),ipre+_T("cppcheck_view.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_VIEW,"Herramientas -> Analisis Estatico -> Mostrar Panel de Resultados"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gprof_activate",menues[tbTOOLS],mxID_TOOLS_GPROF_SET)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_ACTIVATE,"Activar Perfilado de Ejecucion"),ipre+_T("comp_for_prof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_ACTIVATE,"Herramientas -> Perfil de Ejecucion -> Activar"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gprof_show_graph",menues[tbTOOLS],mxID_TOOLS_GPROF_SHOW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_SHOW,"Graficar Resultados del Perfilado de Ejecucion"),ipre+_T("showgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_SHOW,"Herramientas -> Perfil de Ejecucion -> Visualizar Resultados (grafo)..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gprof_list_output",menues[tbTOOLS],mxID_TOOLS_GPROF_LIST)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_LIST,"Listar Resultados del Perfilado de Ejecucion"),ipre+_T("listgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_LIST,"Herramientas -> Perfil de Ejecucion -> Listar Resultados (texto)..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gcov_activate",menues[tbTOOLS],mxID_TOOLS_GCOV_SET)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_ACTIVATE,"Activar Test de Cobertura"),ipre+_T("gcov_set.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_ACTIVATE,"Herramientas -> Test de Cobertura -> Habilitar/Deshabilitar"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gcov_show_bar",menues[tbTOOLS],mxID_TOOLS_GCOV_SHOW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_RESET,"Mostrar barra de resultados de Test de Cobertura"),ipre+_T("gcov_show.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_SHOW_BAR,"Herramientas -> Test de Cobertura -> Mostrar Barra de Resultados"));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.gcov_reset",menues[tbTOOLS],mxID_TOOLS_GCOV_RESET)); // ,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_SHOW_BAR,"Eliminar resultados de Test de Cobertura"),ipre+_T("gcov_reset.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_RESET,"Herramientas -> Test de Cobertura -> Eliminar Resultados"));
#ifndef __WIN32__
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.valgrind_run",menues[tbTOOLS],mxID_TOOLS_VALGRIND_RUN)); // ,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_RUN,"Ejecutar Para Analisis Dinamico"),ipre+_T("valgrind_run.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_RUN,"Herramientas -> Analisis Dinámico -> Ejecutar..."));
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.valgrind_view",menues[tbTOOLS],mxID_TOOLS_VALGRIND_VIEW)); // ,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_VIEW,"Mostrar Resultados del Analisis Dinamico"),ipre+_T("valgrind_view.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_VIEW,"Herramientas -> Analisis Dinámico -> Mostrar Panel de Resultados"));
#endif
		for (int i=0;i<MAX_CUSTOM_TOOLS;i++) {
			if (config->CustomTools[i].on_toolbar) {
				wxString str(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada ")); str<<i<<" ("<<config->CustomTools[i].name<<")";
				AddToolbarItem(tbTOOLS,myToolbarItem("",mxID_CUSTOM_TOOL_0+i,wxString("customTool")<<i<<".png",str).Description(str));
			}
		}
		AddToolbarItem(tbTOOLS,myToolbarItem("tools.custom_settings",menues[tbTOOLS],mxID_TOOLS_CUSTOM_TOOLS_SETTINGS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),ipre+_T("customToolsSettings.png"),LANG(TOOLBAR_DESC_TOOLS_CUSTOM_TOOLS_SETTINGS,"Herramientas -> Herramientas Personalizadas -> Configurar (generales)..."));
	}
	
	
	toolbars[tbMISC].label = LANG(CAPTION_TOOLBAR_MISC,"Miscelánea"); {
		AddToolbarItem(tbMISC,myToolbarItem("misc.preferences",menues[tbMISC],mxID_FILE_PREFERENCES)); // ,LANG(TOOLBAR_CAPTION_FILE_PREFERENCES,"Preferencias..."),ipre+_T("preferencias.png"),LANG(TOOLBAR_DESC_FILE_PREFERENCES,"Archivo -> Preferencias..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.tutorials",menues[tbMISC],mxID_HELP_TUTORIAL)); // ,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Tutoriales..."),ipre+_T("tutoriales.png"),LANG(TOOLBAR_DESC_HELP_TUTORIALS,"Ayuda -> Tutoriales..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.help_ide",menues[tbMISC],mxID_HELP_GUI)); // ,LANG(TOOLBAR_CAPTION_HELP_ZINJAI,"Ayuda Sobre ZinjaI..."),ipre+_T("ayuda.png"),LANG(TOOLBAR_DESC_HELP_ZINJAI,"Ayuda -> Ayuda sobre ZinjaI..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.help_cpp",menues[tbMISC],mxID_HELP_CPP)); // ,LANG(TOOLBAR_CAPTION_HELP_CPP,"Ayuda Sobre C/C++..."),ipre+_T("referencia.png"),LANG(TOOLBAR_DESC_HELP_CPP,"Ayuda -> Ayuda sobre C++..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.show_tips",menues[tbMISC],mxID_HELP_TIP)); // ,LANG(TOOLBAR_CAPTION_HELP_TIPS,"Sugerencias..."),ipre+_T("tip.png"),LANG(TOOLBAR_DESC_HELP_TIPS,"Ayuda -> Mostrar Sugerencias..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.about",menues[tbMISC],mxID_HELP_ABOUT)); // ,LANG(TOOLBAR_CAPTION_HELP_ABOUT,"Acerca de..."),ipre+_T("acercaDe.png"),LANG(TOOLBAR_DESC_HELP_ABOUT,"Ayuda -> Acerca de..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.opinion",menues[tbMISC],mxID_HELP_OPINION)); // ,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Enviar sugerencia o reportar error..."),ipre+_T("opinion.png"),LANG(TOOLBAR_DESC_HELP_OPINION,"Ayuda -> Opina sobre ZinjaI..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.find_updates",menues[tbMISC],mxID_HELP_UPDATES)); // ,LANG(TOOLBAR_CAPTION_HELP_UPDATES,"Buscar Actualizaciones..."),ipre+_T("updates.png"),LANG(TOOLBAR_DESC_HELP_UPDATES,"Ayuda -> Buscar Actualizaciones..."));
		AddToolbarItem(tbMISC,myToolbarItem("misc.exit",menues[tbMISC],wxID_EXIT)); // ,LANG(TOOLBAR_CAPTION_FILE_EXIT,"Salir"),ipre+_T("salir.png"),LANG(TOOLBAR_DESC_FILE_EXIT,"Archivo -> Salir"));
	}
	
	
	toolbars[tbPROJECT].label = LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"); {
		if (project->GetWxfbActivated()) {
			AddToolbarItem(tbPROJECT,myToolbarItem("",menues[tbPROJECT],mxID_TOOLS_WXFB_INHERIT_CLASS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),ipre+_T("wxfb_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_INHERIT,"Herramientas -> wxFormBuilder -> Generar Clase Heredada..."));
			AddToolbarItem(tbPROJECT,myToolbarItem("",menues[tbPROJECT],mxID_TOOLS_WXFB_HELP_WX)); // ,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),ipre+_T("ayuda_wx.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REFERENCE,"Herramientas -> wxFormBuilder -> Referencia wxWidgets..."));
		}
		bool have_tool=false;
		for (int i=0;i<MAX_PROJECT_CUSTOM_TOOLS;i++) {
			if (project->custom_tools[i].on_toolbar) {
				have_tool=true;
				wxString str(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada ")); str<<i<<" ("<<project->custom_tools[i].name<<")";;
				AddToolbarItem(tbPROJECT,myToolbarItem("",mxID_CUSTOM_PROJECT_TOOL_0+i,wxString("projectTool")<<i<<".png",str).Description(str));
			}
		}
		if (!have_tool) AddToolbarItem(tbPROJECT,myToolbarItem("",menues[tbPROJECT],mxID_TOOLS_PROJECT_TOOLS_SETTINGS)); // ,LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),ipre+_T("projectToolsSettings.png"),LANG(TOOLBAR_DESC_TOOLS_PROJECT_TOOLS_SETTINGS,"Herramientas -> Herramientas Personalizadas -> Configurar (de proyecto)..."));
	}
	
}

void MenusAndToolsConfig::CreateMenues () {
	
	vector<wxMenu*> menu_stack;
	wxMenu *current_menu;
	wxString menu_icon_prefix = DIR_PLUS_FILE("16","");
	
	for(unsigned int menu_id=0;menu_id<mnHIDDEN;menu_id++) { 
		menu_stack.clear();
		current_menu = menues[menu_id].wx_menu = new wxMenu;
		wx_menu_bar->Append(current_menu,menues[menu_id].label);
		unsigned int items_size = menues[menu_id].items.size();
		for(unsigned int j=0;j<items_size;j++) { 
			myMenuItem &mi=menues[menu_id].items[j];
			int props=mi.properties;
			wxMenuItem *wx_item=NULL;
			if (props&maSEPARATOR) {
				current_menu->AppendSeparator();
			} else if (props&maBEGIN_SUBMENU) {
				wxMenu *new_menu = new wxMenu;
				wx_item = utils->AddSubMenuToMenu(current_menu,new_menu,mi.label,mi.description,menu_icon_prefix+mi.icon);
				if (props&maMAPPED) {
//					mapped_items.push_back(MappedItem(mi.wx_id,wx_item));
					mapped_menues.push_back(MappedSomething<wxMenu*>(mi.wx_id,new_menu));
				}
				menu_stack.push_back(current_menu);
				current_menu=new_menu;
			} else if (props&maEND_SUBMENU) {
				current_menu=menu_stack.back(); menu_stack.pop_back();
			} else {
				if (props&(maCHECKED|maCHECKEABLE))
					wx_item = utils->AddCheckToMenu(current_menu,mi.wx_id,mi.label,mi.shortcut,mi.description,props&maCHECKED);
				else 
					wx_item = utils->AddItemToMenu(current_menu,mi.wx_id,mi.label,mi.shortcut,mi.description,menu_icon_prefix+mi.icon);
				if (props&maMAPPED) mapped_items.push_back(MappedSomething<wxMenuItem*>(mi.wx_id,wx_item));
			}
			if (props&maDEBUG) { items_debug.push_back(AutoenabligItem(wx_item,true)); }
			else if (props&maNODEBUG) { items_debug.push_back(AutoenabligItem(wx_item,false)); }
			if (props&maPROJECT) { items_project.push_back(AutoenabligItem(wx_item,true)); }
			else if (props&maNOPROJECT) { items_project.push_back(AutoenabligItem(wx_item,false)); }
		}
	}
	
	SetProjectMode(false);
	SetDebugMode(false);
}

void MenusAndToolsConfig::CreateToolbars ( ) {
	
}

void MenusAndToolsConfig::ParseMenuConfigLine (const wxString & key, const wxString & value) {
	
}

void MenusAndToolsConfig::ParseToolbarConfigLine (const wxString & key, const wxString & value) {
	
}

void MenusAndToolsConfig::SetDebugMode (bool mode) {
	for(unsigned int i=0;i<items_debug.size();i++) 
		items_debug[i].Enable(mode);
}

void MenusAndToolsConfig::SetProjectMode (bool mode) {
	for(unsigned int i=0;i<items_project.size();i++) 
		items_project[i].Enable(mode);
}
