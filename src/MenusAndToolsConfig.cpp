#include "MenusAndToolsConfig.h"
#include "Language.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"
#include "DebugManager.h"

MenusAndToolsConfig *menu_data;
#ifdef __APPLE__
	#define _if_not_apple(a,b) b
#else
	#define _if_not_apple(a,b) a
#endif
#ifdef __WIN32__
	#define _if_win32(a,b) a
#else
	#define _if_win32(a,b) b
#endif

MenusAndToolsConfig::MenusAndToolsConfig () {
	wx_menu_bar = nullptr;
//	tools_custom_item=new wxMenuItem*[10];
//	for (int i=0;i<10;i++) tools_custom_item[i] = nullptr;
	file_source_history = new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) file_source_history[i]=nullptr;
	file_project_history = new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) file_project_history[i]=nullptr;
	
	LoadMenuData();
	LoadToolbarsData();
}

void MenusAndToolsConfig::LoadMenuData ( ) {
	
	menues[mnFILE].Init("file",LANG(MENUITEM_FILE,"&Archivo")); {
		AddMenuItem(mnFILE, myMenuItem("new_file",mxID_FILE_NEW, LANG(MENUITEM_FILE_NEW,"&Nuevo...")).ShortCut("Ctrl+N").Description("Crear un nuevo archivo").Icon("nuevo.png"));
		AddMenuItem(mnFILE, myMenuItem("new_project",mxID_FILE_PROJECT, LANG(MENUITEM_FILE_NEW_PROJECT,"&Nuevo Proyecto...")).ShortCut("Ctrl+Shift+N").Description("Crear un nuevo proyecto").Icon("proyecto.png"));
		AddMenuItem(mnFILE, myMenuItem("open",mxID_FILE_OPEN, LANG(MENUITEM_FILE_OPEN,"&Abrir...")).ShortCut("Ctrl+O").Description("Abrir un archivo o un proyecto existente...").Icon("abrir.png").Map());
		BeginSubMenu(mnFILE,LANG(MENUITEM_FILE_RECENT_SOURCES,"Fuentes Abiertos Recientemente"),"Muestra los ultimos archivos abiertos como programas simples","recentSimple.png",mxID_FILE_SOURCE_RECENT,maMAPPED);
			AddSeparator(mnFILE);
			AddMenuItem(mnFILE, myMenuItem("recent_simple",mxID_FILE_SOURCE_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas...")).Description("Muestra un dialogo con la lista completa de archivos recientes").Icon("recentMore.png"));
		EndSubMenu(mnFILE);
		BeginSubMenu(mnFILE,LANG(MENUITEM_FILE_RECENT_PROJECTS,"Proyectos Abiertos Recientemente"),"Muestra los ultimos proyectos abiertos","recentProject.png",mxID_FILE_PROJECT_RECENT,maMAPPED);
			AddSeparator(mnFILE);
			AddMenuItem(mnFILE, myMenuItem("recent_project",mxID_FILE_PROJECT_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas...")).Description("Muestra un dialogo con la lista completa de archivos recientes").Icon("recentMore.png"));
		EndSubMenu(mnFILE);
		AddMenuItem(mnFILE, myMenuItem("save",mxID_FILE_SAVE, LANG(MENUITEM_FILE_SAVE,"&Guardar")).ShortCut("Ctrl+S").Description("Guardar el archivo actual").Icon("guardar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("save_as",mxID_FILE_SAVE_AS, LANG(MENUITEM_FILE_SAVE_AS,"G&uardar Como...")).ShortCut("Ctrl+Shift+S").Description("Guardar el archivo actual con otro nombre...").Icon("guardarComo.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("save_all",mxID_FILE_SAVE_ALL, LANG(MENUITEM_FILE_SAVE_ALL,"Guardar &Todo...")).ShortCut("Ctrl+Alt+Shift+S").Description("Guarda todos los archivos abiertos y el proyecto actual...").Icon("guardarTodo.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("save_project",mxID_FILE_SAVE_PROJECT, LANG(MENUITEM_FILE_SAVE_PROJECT,"Guar&dar Proyecto")).ShortCut("Ctrl+Alt+S").Description("Guardar la configuarición actual del proyecto").Icon("guardarProyecto.png").EnableIf(ecPROJECT));
		AddMenuItem(mnFILE, myMenuItem("export_html",mxID_FILE_EXPORT_HTML, LANG(MENUITEM_FILE_EXPORT_HTML,"Exportar a HTML...")).Description("Genera un archiv HTML con el codigo fuente").Icon("exportHtml.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("print",mxID_FILE_PRINT, LANG(MENUITEM_FILE_PRINT,"&Imprimir...")).Description("Imprime el codigo fuente actual").Icon("imprimir.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("reload",mxID_FILE_RELOAD, LANG(MENUITEM_FILE_RELOAD,"&Recargar")).ShortCut("Ctrl+Shift+R").Description("Recarga la version en disco del archivo actual.").Icon("recargar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("close",mxID_FILE_CLOSE, LANG(MENUITEM_FILE_CLOSE,"&Cerrar")).ShortCut("Ctrl+W").Description("Cerrar el archivo actual").Icon("cerrar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("close_all",mxID_FILE_CLOSE_ALL, LANG(MENUITEM_FILE_CLOSE_ALL,"Cerrar &Todo")).ShortCut("Ctrl+Alt+Shift+W").Description("Cierra todos los archivos abiertos").Icon("cerrarTodo.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("close_project",mxID_FILE_CLOSE_PROJECT, LANG(MENUITEM_FILE_CLOSE_PROJECT,"Cerrar Pro&yecto")).ShortCut("Ctrl+Shift+W").Description("Cierra el proyecto actual").Icon("cerrarProyecto.png").EnableIf(ecPROJECT));
		AddSeparator(mnFILE);
		AddMenuItem(mnFILE, myMenuItem("project_config",mxID_FILE_PROJECT_CONFIG, LANG(MENUITEM_FILE_PROJECT_CONFIG,"&Configuración del Proyecto...")).ShortCut("Ctrl+Shift+P").Description("Configurar las propiedades generales de un proyecto...").Icon("projectConfig.png").EnableIf(ecSOURCE));
		AddMenuItem(mnFILE, myMenuItem("preferences",mxID_FILE_PREFERENCES, LANG(MENUITEM_FILE_PREFERENCES,"&Preferencias...")).ShortCut("Ctrl+P").Description("Configurar el entorno...").Icon("preferencias.png"));
		AddSeparator(mnFILE);
		AddMenuItem(mnFILE, myMenuItem("exit",mxID_FILE_EXIT, LANG(MENUITEM_FILE_EXIT,"&Salir")).ShortCut("Alt+F4").Description("Salir del programa!").Icon("salir.png"));
	}
	
	
	menues[mnEDIT].Init("edit",LANG(MENUITEM_EDIT,"&Edición")); {
		AddMenuItem(mnEDIT, myMenuItem("edit",mxID_EDIT_UNDO, LANG(MENUITEM_EDIT_UNDO,"&Deshacer")).ShortCut("Ctrl+Z").Description("Deshacer el ultimo cambio").Icon("deshacer.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("redo",mxID_EDIT_REDO, LANG(MENUITEM_EDIT_REDO,"&Rehacer")).ShortCut("Ctrl+Shift+Z").Description("Rehacer el ultimo cambio desecho").Icon("rehacer.png").EnableIf(ecSOURCE));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem("cut",mxID_EDIT_CUT, LANG(MENUITEM_EDIT_CUT,"C&ortar")).ShortCut("Ctrl+X").Description("Cortar la selección al portapapeles").Icon("cortar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("copy",mxID_EDIT_COPY, LANG(MENUITEM_EDIT_COPY,"&Copiar")).ShortCut("Ctrl+C").Description("Copiar la selección al portapapeles").Icon("copiar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("paste",mxID_EDIT_PASTE, LANG(MENUITEM_EDIT_PASTE,"&Pegar")).ShortCut("Ctrl+V").Description("Pegar el contenido del portapapeles").Icon("pegar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("move_up",mxID_EDIT_TOGGLE_LINES_UP, LANG(MENUITEM_EDIT_LINES_UP,"Mover Hacia Arriba")).ShortCut("Ctrl+T").Description("Mueve la o las lineas seleccionadas hacia arriba").Icon("toggleLinesUp.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("move_down",mxID_EDIT_TOGGLE_LINES_DOWN, LANG(MENUITEM_EDIT_LINES_DOWN,"Mover Hacia Abajo")).ShortCut("Ctrl+Shift+T").Description("Mueve la o las lineas seleccionadas hacia abajo").Icon("toggleLinesDown.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("duplicate",mxID_EDIT_DUPLICATE_LINES, LANG(MENUITEM_EDIT_DUPLICATE_LINES,"&Duplicar Linea(s)")).ShortCut("Ctrl+L").Description("Copia la linea actual del cursor, o las lineas seleccionadas, nuevamente a continuación").Icon("duplicarLineas.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("delete_lines",mxID_EDIT_DELETE_LINES, LANG(MENUITEM_EDIT_DELETE_LINES,"&Eliminar Linea(s)")).ShortCut("Shift+Ctrl+L").Description("Elimina la linea actual del cursor nuevamente, o las lineas seleccionadas").Icon("borrarLineas.png").EnableIf(ecSOURCE));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem("goto_class",mxID_EDIT_GOTO_FUNCTION, LANG(MENUITEM_EDIT_GOTO_FUNCTION,"&Ir a Funcion/Clase/Metodo...")).ShortCut("Ctrl+Shift+G").Description("Abrir el fuente con la declaración de una funcion, clase o metodo...").Icon("irAFuncion.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnEDIT, myMenuItem("goto_file",mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_GOTO_FILE,"&Ir a Archivo...")).ShortCut("Ctrl+Shift+F").Description("Abrir un archivo en particular buscandolo por parte de su nombre...").Icon("irAArchivo.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnEDIT, myMenuItem("goto_line",mxID_EDIT_GOTO, LANG(MENUITEM_EDIT_GOTO_LINE,"&Ir a Linea...")).ShortCut("Ctrl+G").Description("Mover el cursor a una linea determinada en el archivo...").Icon("irALinea.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("find",mxID_EDIT_FIND, LANG(MENUITEM_EDIT_FIND,"&Buscar...")).ShortCut("Ctrl+F").Description("Buscar una cadena en el archivo...").Icon("buscar.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnEDIT, myMenuItem("find_prev",mxID_EDIT_FIND_PREV, LANG(MENUITEM_EDIT_FIND_PREV,"Buscar &Anterior")).ShortCut("Shift+F3").Description("Repetir la ultima busqueda a partir del cursor hacia atras").Icon("buscarAnterior.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("find_next",mxID_EDIT_FIND_NEXT, LANG(MENUITEM_EDIT_FIND_NEXT,"Buscar &Siguiente")).ShortCut("F3").Description("Repetir la ultima busqueda a partir del cursor").Icon("buscarSiguiente.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("replace",mxID_EDIT_REPLACE, LANG(MENUITEM_EDIT_REPLACE,"&Reemplazar...")).ShortCut("Ctrl+R").Description("Reemplazar una cadena con otra en el archivo...").Icon("reemplazar.png").EnableIf(ecSOURCE));
		AddSeparator(mnEDIT);
		AddMenuItem(mnEDIT, myMenuItem("inser_header",mxID_EDIT_INSERT_HEADER, LANG(MENUITEM_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente")).ShortCut("Ctrl+H").Description("Si es posible, inserta el #include necesario para utilizar la funcion/clase en la cual se encuentra el cursor.").Icon("insertarInclude.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("comment",mxID_EDIT_COMMENT, LANG(MENUITEM_EDIT_COMMENT,"Comentar")).ShortCut("Ctrl+D").Description("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea").Icon("comentar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("uncomment",mxID_EDIT_UNCOMMENT, LANG(MENUITEM_EDIT_UNCOMMENT,"Descomentar")).ShortCut("Shift+Ctrl+D").Description("Descomente el texto seleccionado eliminando \"//\" de cada linea").Icon("descomentar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("indent",mxID_EDIT_INDENT, LANG(MENUITEM_EDIT_INDENT,"Indentar Blo&que")).ShortCut("Ctrl+I").Description("Corrige el indentado de un bloque de codigo agregando o quitando tabs segun corresponda").Icon("indent.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("select_block",mxID_EDIT_BRACEMATCH, LANG(MENUITEM_EDIT_BRACEMATCH,"Seleccionar Blo&que")).ShortCut("Ctrl+M").Description("Seleccionar todo el bloque correspondiente a la llave o parentesis sobre el cursor").Icon("mostrarLlave.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("select_all",mxID_EDIT_SELECT_ALL, LANG(MENUITEM_EDIT_SELECT_ALL,"&Seleccionar Todo")).ShortCut("Ctrl+A").Description("Seleccionar todo el contenido del archivo").Icon("seleccionarTodo.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("toggle_user_mark",mxID_EDIT_MARK_LINES, LANG(MENUITEM_EDIT_HIGHLIGHT_LINES,"&Resaltar Linea(s)/Quitar Resaltado")).ShortCut("Ctrl+B").Description("Resalta la linea pintandola de otro color").Icon("marcar.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("find_user_mark",mxID_EDIT_GOTO_MARK, LANG(MENUITEM_EDIT_FIND_HIGHLIGHTS,"Buscar &Resaltado")).ShortCut("Ctrl+Shift+B").Description("Mueve el cursor a la siguiente linea resaltada").Icon("irAMarca.png").EnableIf(ecSOURCE));
		AddMenuItem(mnEDIT, myMenuItem("list_user_marks",mxID_EDIT_LIST_MARKS, LANG(MENUITEM_EDIT_LIST_HIGHLIGHTS,"&Listar Lineas Resaltadas")).ShortCut("Ctrl+Alt+B").Description("Muestra una lista de las lineas marcadas en todos los archivos").Icon("listarMarcas.png").EnableIf(ecPROJECT_OR_SOURCE));
	}

	
	menues[mnVIEW].Init("view",LANG(MENUITEM_VIEW,_if_not_apple("&Ver","Ver"))); {
		AddMenuItem(mnVIEW, myMenuItem("split_view",mxID_VIEW_DUPLICATE_TAB, LANG(MENUITEM_VIEW_SPLIT_VIEW,"&Duplicar vista")).Icon("duplicarVista.png").EnableIf(ecSOURCE));
		AddMenuItem(mnVIEW, myMenuItem("line_wrap",mxID_VIEW_LINE_WRAP, LANG(MENUITEM_VIEW_LINE_WRAP,"&Ajuste de linea")).ShortCut("Alt+F11").Description("Muestra las lineas largas como en varios renglones").Icon("lineWrap.png").Checkeable(false).EnableIf(ecSOURCE));	
		AddMenuItem(mnVIEW, myMenuItem("white_space",mxID_VIEW_WHITE_SPACE, LANG(MENUITEM_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de &fin de linea")).Description("Muestra las lineas largas como en varios renglones").Icon("whiteSpace.png").Checkeable(false).EnableIf(ecSOURCE));	
		AddMenuItem(mnVIEW, myMenuItem("sintax_colour",mxID_VIEW_CODE_STYLE, LANG(MENUITEM_VIEW_SYNTAX_HIGHLIGHT,"&Colorear Sintaxis")).ShortCut("Shift+F11").Description("Resalta el codigo con diferentes colores y formatos de fuente.").Icon("syntaxColour.png").Checkeable(false).EnableIf(ecSOURCE));	
		AddMenuItem(mnVIEW, myMenuItem("config_colours",mxID_VIEW_CODE_COLOURS, LANG(MENUITEM_VIEW_CODE_COLOURS,"Configurar esquema de colores...")).Icon("preferencias.png"));
		BeginSubMenu(mnVIEW,myMenuItem("",wxID_ANY,LANG(MENUITEM_VIEW_FOLDING,"Plegado")).Description("Muestra opciones para plegar y desplegar codigo en distintos niveles").Icon("folding.png").EnableIf(ecSOURCE));
			AddMenuItem(mnVIEW, myMenuItem("fold_current",mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea")).ShortCut(_if_not_apple("Alt+Up","")).Icon("foldOne.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_current",mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea")).ShortCut(_if_not_apple("Alt+Down","")).Icon("unfoldOne.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_1",mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel")).ShortCut(_if_not_apple("Ctrl+1","")).Description("Cierra todos los bolques del primer nivel").Icon("fold1.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_2",mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel")).ShortCut(_if_not_apple("Ctrl+2","")).Description("Cierra todos los bolques del segundo nivel").Icon("fold2.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_3",mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel")).ShortCut(_if_not_apple("Ctrl+3","")).Description("Cierra todos los bolques del tercer nivel").Icon("fold3.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_4",mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel")).ShortCut(_if_not_apple("Ctrl+4","")).Description("Cierra todos los bolques del cuarto nivel").Icon("fold4.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_5",mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel")).ShortCut(_if_not_apple("Ctrl+5","")).Description("Cierra todos los bolques del quinto nivel").Icon("fold5.png"));
			AddMenuItem(mnVIEW, myMenuItem("fold_all",mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles")).ShortCut(_if_not_apple("Ctrl+0","")).Description("Cierra todos los bolques de todos los niveles").Icon("foldAll.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_1",mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel")).ShortCut(_if_not_apple("Alt+1","")).Description("Abre todos los bolques del primer nivel").Icon("unfold1.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_2",mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel")).ShortCut(_if_not_apple("Alt+2","")).Description("Abre todos los bolques del segundo nivel").Icon("unfold2.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_3",mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel")).ShortCut(_if_not_apple("Alt+3","")).Description("Abre todos los bolques del tercer nivel").Icon("unfold3.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_4",mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel")).ShortCut(_if_not_apple("Alt+4","")).Description("Abre todos los bolques del cuarto nivel").Icon("unfold4.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_5",mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel")).ShortCut(_if_not_apple("Alt+5","")).Description("Abre todos los bolques del quinto nivel").Icon("unfold5.png"));
			AddMenuItem(mnVIEW, myMenuItem("unfold_all",mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles")).ShortCut(_if_not_apple("Alt+0","")).Description("Abre todos los bolques de todos los niveles").Icon("unfoldAll.png"));
		EndSubMenu(mnVIEW);
		AddSeparator(mnVIEW);
		AddMenuItem(mnVIEW, myMenuItem("full_screen",mxID_VIEW_FULLSCREEN, LANG(MENUITEM_VIEW_FULLSCREEN,"Ver a Pantalla &Completa")).ShortCut("F11").Description("Muestra el editor a pantalla completa, ocultando tambien los demas paneles").Icon("fullScreen.png").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("beginner_panel",mxID_VIEW_BEGINNER_PANEL, LANG(MENUITEM_VIEW_BEGINNER_PANEL,"Mostrar Panel de Mini-Plantillas")).Description("Muestra un panel con plantillas y estructuras basicas de c++").Icon("beginer_panel.png").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("left_panels",mxID_VIEW_LEFT_PANELS, LANG(MENUITEM_VIEW_LEFT_PANELS,"&Mostrar Panel de Arboles")).Description("Muestra el panel con los arboles de proyecto, simbolos y explorador de archivos").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("project_tree",mxID_VIEW_PROJECT_TREE, LANG(MENUITEM_VIEW_PROJECT_TREE,"&Mostrar Arbol de &Proyecto")).Description("Muestra el panel del arbol de proyecto/archivos abiertos").Icon("projectTree.png").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("explorer_tree",mxID_VIEW_EXPLORER_TREE, LANG(MENUITEM_VIEW_EXPLORER_TREE,"Mostrar &Explorardor de Archivos")).ShortCut("Ctrl+E").Description("Muestra el panel explorador de archivos").Icon("explorerTree.png").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("symbols_tree",mxID_VIEW_SYMBOLS_TREE, LANG(MENUITEM_VIEW_SYMBOLS_TREE,"Mostrar Arbol de &Simbolos")).Description("Analiza el codigo fuente y construye un arbol con los simbolos declarados en el mismo.").Icon("symbolsTree.png").Checkeable(false));
		AddMenuItem(mnVIEW, myMenuItem("compiler_tree",mxID_VIEW_COMPILER_TREE, LANG(MENUITEM_VIEW_COMPILER_TREE,"&Mostrar Resultados de la Compilación")).Description("Muestra un panel con la salida del compilador").Icon("compilerTree.png").Checkeable(false));
		BeginSubMenu(mnVIEW, LANG(MENUITEM_VIEW_TOOLBARS,"Barras de herramientas"));
			// los atributos de Checkeable se setean más tarde porque a esta altura todavía no se leyó la configuracion
			AddMenuItem(mnVIEW, myMenuItem("toolbar_file",mxID_VIEW_TOOLBAR_FILE, LANG(MENUITEM_VIEW_TOOLBAR_FILE,"&Mostrar Barra de Herramientas Archivo")).Description("Muestra la barra de herramientas para el manejo de archivos")/*.Checkeable(_toolbar_visible(tbFILE))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_edit",mxID_VIEW_TOOLBAR_EDIT, LANG(MENUITEM_VIEW_TOOLBAR_EDIT,"&Mostrar Barra de Herramientas Edición")).Description("Muestra la barra de herramientas para la edición del fuente")/*.Checkeable(_toolbar_visible(tbEDIT))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_view",mxID_VIEW_TOOLBAR_VIEW, LANG(MENUITEM_VIEW_TOOLBAR_VIEW,"&Mostrar Barra de Herramientas Ver")).Description("Muestra la barra de herramientas para las opciones de visualización")/*.Checkeable(_toolbar_visible(tbVIEW))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_find",mxID_VIEW_TOOLBAR_FIND, LANG(MENUITEM_VIEW_TOOLBAR_FIND,"&Mostrar Barra de Busqueda Rapida")).Description("Muestra un cuadro de texto en la barra de herramientas que permite buscar rapidamente en un fuente")/*.Checkeable(_toolbar_visible(tbFIND))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_run",mxID_VIEW_TOOLBAR_RUN, LANG(MENUITEM_VIEW_TOOLBAR_RUN,"&Mostrar Barra de Herramientas Ejecución")).Description("Muestra la barra de herramientas para la compilación y ejecución del programa")/*.Checkeable(_toolbar_visible(tbRUN))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_tools",mxID_VIEW_TOOLBAR_TOOLS, LANG(MENUITEM_VIEW_TOOLBAR_TOOLS,"&Mostrar Barra de Herramientas Herramientas")).Description("Muestra la barra de herramientas para las herramientas adicionales")/*.Checkeable(_toolbar_visible(tbTOOLS))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_projecs",mxID_VIEW_TOOLBAR_PROJECT, LANG(MENUITEM_VIEW_TOOLBAR_PROJECT,"&Mostrar Barra de Herramientas Proyecto")).Description("Muestra la barra de herramientas para las herramientas personalizables propias del proyecto")/*.Checkeable(_toolbar_visible(tbPROJECT))*/.EnableIf(ecPROJECT));
			AddMenuItem(mnVIEW, myMenuItem("toolbar_debug",mxID_VIEW_TOOLBAR_DEBUG, LANG(MENUITEM_VIEW_TOOLBAR_DEBUG,"&Mostrar Barra de Herramientas Depuración")).Description("Muestra la barra de herramientas para la depuración del programa")/*.Checkeable(_toolbar_visible(tbDEBUG))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_misc",mxID_VIEW_TOOLBAR_MISC, LANG(MENUITEM_VIEW_TOOLBAR_MISC,"&Mostrar Barra de Herramientas Miscelanea")).Description("Muestra la barra de herramientas con commandos miselaneos")/*.Checkeable(_toolbar_visible(tbMISC))*/);
			AddMenuItem(mnVIEW, myMenuItem("toolbar_config",mxID_VIEW_TOOLBARS_CONFIG, LANG(MENUITEM_VIEW_TOOLBARS_CONFIG,"&Configurar...")).Icon("preferencias.png"));
		EndSubMenu(mnVIEW);
		AddSeparator(mnVIEW);
		AddMenuItem(mnVIEW, myMenuItem("prev_error",mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior")).ShortCut(_if_win32("Ctrl+Shift+<","Ctrl+>")).Description("Selecciona el error/advertencia anterior de la salida del compilador.").Icon("errorPrev.png").EnableIf(ecSOURCE));
		AddMenuItem(mnVIEW, myMenuItem("next_error",mxID_VIEW_NEXT_ERROR, LANG(MENUITEM_VIEW_NEXT_ERROR,"&Ir a siguiente error")).ShortCut("Ctrl+<").Description("Selecciona el proximo error/advertencia de la salida del compilador.").Icon("errorNext.png").EnableIf(ecSOURCE));
	}
	
	
	menues[mnRUN].Init("run",LANG(MENUITEM_RUN,"E&jecución")); {
		AddMenuItem(mnRUN, myMenuItem("run",mxID_RUN_RUN, LANG(MENUITEM_RUN_RUN,"&Ejecutar...")).ShortCut("F9").Description("Guarda y compila si es necesario, luego ejecuta el programa").Icon("ejecutar.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnRUN, myMenuItem("run_old",mxID_RUN_RUN_OLD, LANG(MENUITEM_RUN_OLD,"Ejecutar (sin recompilar)...")).ShortCut("Ctrl+F9").Description("Ejecuta el binario existente sin recompilar primero").Icon("ejecutar_old.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnRUN, myMenuItem("compile",mxID_RUN_COMPILE, LANG(MENUITEM_RUN_COMPILE,"&Compilar")).ShortCut("Shift+F9").Description("Guarda y compila el fuente actual").Icon("compilar.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnRUN, myMenuItem("clean",mxID_RUN_CLEAN, LANG(MENUITEM_RUN_CLEAN,"&Limpiar")).ShortCut("Ctrl+Shift+F9").Description("Elimina los objetos y ejecutables compilados").Icon("limpiar.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnRUN, myMenuItem("stop",mxID_RUN_STOP, LANG(MENUITEM_RUN_STOP,"&Detener")).Description("Detiene la ejecución del programa").Icon("detener.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnRUN, myMenuItem("options",mxID_RUN_CONFIG, LANG(MENUITEM_RUN_OPTIONS,"&Opciones...")).ShortCut("Alt+F9").Description("Configura la compilación y ejecución de los programas").Icon("opciones.png").EnableIf(ecPROJECT_OR_SOURCE));
	}
	
	
	menues[mnDEBUG].Init("debug",LANG(MENUITEM_DEBUG,"&Depuración")); {
		AddMenuItem(mnDEBUG, myMenuItem("debug", mxID_DEBUG_RUN, LANG(MENUITEM_DEBUG_START,"&Iniciar/Continuar")).ShortCut("F5").Icon("depurar.png"));
		AddMenuItem(mnDEBUG, myMenuItem("pause", mxID_DEBUG_PAUSE, LANG(MENUITEM_DEBUG_PAUSE,"Interrum&pir")).Icon("pausar.png").EnableIf(ecDEBUG_NOT_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("stop", mxID_DEBUG_STOP, LANG(MENUITEM_DEBUG_STOP,"&Detener")).ShortCut("Shift+F5").Icon("detener.png").EnableIf(ecDEBUG));
		AddMenuItem(mnDEBUG, myMenuItem("step_in", mxID_DEBUG_STEP_IN, LANG(MENUITEM_DEBUG_STEP_IN,"Step &In")).ShortCut("F6").Icon("step_in.png").EnableIf(ecDEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("step_over", mxID_DEBUG_STEP_OVER, LANG(MENUITEM_DEBUG_STEP_OVER,"Step &Over")).ShortCut("F7").Icon("step_over.png").EnableIf(ecDEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("step_out", mxID_DEBUG_STEP_OUT, LANG(MENUITEM_DEBUG_STEP_OUT,"Step O&ut")).ShortCut("Shift+F6").Icon("step_out.png").EnableIf(ecDEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("run_until", mxID_DEBUG_RUN_UNTIL, LANG(MENUITEM_DEBUG_RUN_UNTIL,"Ejecutar &Hasta el Cursor")).ShortCut("Shift+F7").Icon("run_until.png").EnableIf(ecDEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("function_return", mxID_DEBUG_RETURN, LANG(MENUITEM_DEBUG_RETURN,"&Return")).ShortCut("Ctrl+F6").Icon("return.png").EnableIf(ecDEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("jump", mxID_DEBUG_JUMP, LANG(MENUITEM_DEBUG_JUMP,"Continuar Desde Aqui")).ShortCut("Ctrl+F5").Icon("debug_jump.png").EnableIf(ecDEBUG_PAUSED));
		AddSeparator(mnDEBUG);
		AddMenuItem(mnDEBUG, myMenuItem("break_toggle",mxID_DEBUG_TOGGLE_BREAKPOINT, LANG(MENUITEM_DEBUG_TOGGLE_BREAKPOINT,"&Agregar/quitar Breakpoint")).ShortCut("F8").Icon("breakpoint.png").EnableIf(ecSOURCE));
		AddMenuItem(mnDEBUG, myMenuItem("break_options",mxID_DEBUG_BREAKPOINT_OPTIONS, LANG(MENUITEM_DEBUG_BREAKPOINT_OPTIONS,"&Opciones del Breakpoint...")).ShortCut("Ctrl+F8").Icon("breakpoint_options.png").EnableIf(ecNOT_DEBUG_OR_DEBUG_PAUSED));
		AddMenuItem(mnDEBUG, myMenuItem("break_list",mxID_DEBUG_LIST_BREAKPOINTS, LANG(MENUITEM_DEBUG_LIST_BREAKPOINTS,"&Listar Watch/Break points...")).ShortCut("Shift+F8").Icon("breakpoint_list.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddSeparator(mnDEBUG);
		AddMenuItem(mnDEBUG, myMenuItem("inspections",mxID_DEBUG_INSPECT, LANG(MENUITEM_DEBUG_INSPECT,"Panel de In&specciones")).Icon("inspect.png").ShortCut("Shift+F2"));
		AddMenuItem(mnDEBUG, myMenuItem("backtrace", mxID_DEBUG_BACKTRACE, LANG(MENUITEM_DEBUG_BACKTRACE,"&Trazado Inverso")).Icon("backtrace.png"));
		AddMenuItem(mnDEBUG, myMenuItem("threadlist", mxID_DEBUG_THREADLIST, LANG(MENUITEM_DEBUG_THREADLIST,"&Hilos de Ejecución")).Icon("threadlist.png"));
		AddMenuItem(mnDEBUG, myMenuItem("log_panel", mxID_DEBUG_LOG_PANEL, LANG(MENUITEM_DEBUG_SHOW_LOG_PANEL,"&Mostrar mensajes del depurador")).Icon("debug_log_panel.png"));
		AddSeparator(mnDEBUG);
		BeginSubMenu(mnDEBUG, LANG(MENUITEM_DEBUG_MORE,"Más..."));
			AddMenuItem(mnDEBUG, myMenuItem("debug_attach",mxID_DEBUG_ATTACH, LANG(MENUITEM_DEBUG_ATTACH,"&Adjuntar...")).Icon("debug_attach.png").EnableIf(ecNOT_DEBUG));
			AddMenuItem(mnDEBUG, myMenuItem("debug_target",mxID_DEBUG_TARGET, LANG(MENUITEM_DEBUG_TARGET,"&Conectar...")).Icon("debug_target.png").EnableIf(ecNOT_DEBUG));
#ifndef __WIN32__
			AddMenuItem(mnDEBUG, myMenuItem("core_dump",mxID_DEBUG_LOAD_CORE_DUMP, LANG(MENUITEM_DEBUG_LOAD_CORE_DUMP,"Cargar &Volcado de Memoria...")).Icon("core_dump.png").EnableIf(ecNOT_DEBUG));
			AddMenuItem(mnDEBUG, myMenuItem("save_core_dump",mxID_DEBUG_SAVE_CORE_DUMP,LANG(MENUITEM_SAVE_CORE_DUMP,"Guardar &Volcado de Memoria...")).Icon("core_dump.png").EnableIf(ecDEBUG_PAUSED));
			AddMenuItem(mnDEBUG, myMenuItem("enable_inverse_exec",mxID_DEBUG_ENABLE_INVERSE_EXEC, LANG(MENUITEM_DEBUG_ENABLE_INVERSE,"Habilitar Ejecución Hacia Atras")).Icon("reverse_enable.png").Checkeable(false).EnableIf(ecDEBUG_PAUSED));
			AddMenuItem(mnDEBUG, myMenuItem("inverse_exec",mxID_DEBUG_INVERSE_EXEC, LANG(MENUITEM_DEBUG_INVERSE,"Ejecutar Hacia Atras")).Icon("reverse_toggle.png").Checkeable(false).EnableIf(ecDEBUG_PAUSED));
			AddMenuItem(mnDEBUG, myMenuItem("set_signals",mxID_DEBUG_SET_SIGNALS, LANG(MENUITEM_DEBUG_SET_SIGNALS,"Configurar comportamiento ante señales...")).Icon("debug_set_signals.png").EnableIf(ecNOT_DEBUG_OR_DEBUG_PAUSED));
			AddMenuItem(mnDEBUG, myMenuItem("send_signal",mxID_DEBUG_SEND_SIGNAL, LANG(MENUITEM_DEBUG_SEND_SIGNALS,"Enviar señal...")).Icon("debug_send_signal.png").EnableIf(ecDEBUG_PAUSED));
#endif
			AddMenuItem(mnDEBUG, myMenuItem("gdb_command",mxID_DEBUG_GDB_COMMAND, LANG(MENUITEM_DEBUG_GDB_COMMAND,"Introducir comandos gdb...")).Icon("gdb_command.png").EnableIf(ecDEBUG_PAUSED));
#ifndef __WIN32__
			AddMenuItem(mnDEBUG, myMenuItem("gdb_patch",mxID_DEBUG_PATCH, LANG(MENUITEM_DEBUG_PATCH,"Actualizar ejecutable (experimental)...")).Icon("debug_patch.png").EnableIf(ecDEBUG_PAUSED));
#endif
		EndSubMenu(mnDEBUG);
	}
	
	
	menues[mnTOOLS].Init("tools",LANG(MENUITEM_TOOLS,"&Herramientas")); {
		AddMenuItem(mnTOOLS, myMenuItem("draw_flow",mxID_TOOLS_DRAW_FLOW, LANG(MENUITEM_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de &Flujo...")).Description("Genera un diagrama de flujo a partir del bloque de codigo actual").Icon("flujo.png").EnableIf(ecSOURCE));
		AddMenuItem(mnTOOLS, myMenuItem("draw_classes",mxID_TOOLS_DRAW_CLASSES, LANG(MENUITEM_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar &Jerarquía de Clases...")).Icon("clases.png").EnableIf(ecPROJECT_OR_SOURCE));
		
		AddMenuItem(mnTOOLS, myMenuItem("copy_code_from_h",mxID_TOOLS_CODE_COPY_FROM_H, LANG(MENUITEM_TOOLS_CODE_COPY_FROM_H,"Implementar Métodos/Funciones faltantes...")).ShortCut("Ctrl+Shift+H").Icon("copy_code_from_h.png").EnableIf(ecSOURCE));
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_COMMENTS,"Coment&arios")).Description("Permite alinear o quitar los comentarios del codigo").Icon("comments.png").EnableIf(ecSOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("align_comments",mxID_TOOLS_ALIGN_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_ALIGN_COMMENTS,"&Alinear Comentarios...")).Description("Mueve todos los comentarios hacia una determinada columna").Icon("align_comments.png"));
			AddMenuItem(mnTOOLS, myMenuItem("comment",mxID_EDIT_COMMENT, LANG(MENUITEM_TOOLS_COMMENTS_COMMENT,"&Comentar")).Description("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea").Icon("comentar.png"));
			AddMenuItem(mnTOOLS, myMenuItem("uncomment",mxID_EDIT_UNCOMMENT, LANG(MENUITEM_TOOLS_COMMENTS_UNCOMMENT,"&Descomentar")).Description("Descomente el texto seleccionado eliminando \"//\" de cada linea").Icon("descomentar.png"));
			AddMenuItem(mnTOOLS, myMenuItem("remove_comments",mxID_TOOLS_REMOVE_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_DELETE_COMMENTS,"&Eliminar Comentarios")).Description("Quita todos los comentarios del codigo fuente o de la selección").Icon("remove_comments.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_PREPROC,"Preprocesador")).Description("Muestra información generada por el preprocesador de C++").Icon("preproc.png").EnableIf(ecSOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("preproc_expand_macros",mxID_TOOLS_PREPROC_EXPAND_MACROS, LANG(MENUITEM_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros")).ShortCut("Ctrl+Shift+M").Icon("preproc_expand_macros.png"));
			AddMenuItem(mnTOOLS, myMenuItem("preproc_mark_valid",mxID_TOOLS_PREPROC_MARK_VALID, LANG(MENUITEM_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas")).ShortCut("Ctrl+Alt+M").Icon("preproc_mark_valid.png"));
			AddMenuItem(mnTOOLS, myMenuItem("preproc_unmark_all",mxID_TOOLS_PREPROC_UNMARK_ALL, LANG(MENUITEM_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas")).ShortCut("Ctrl+Alt+Shift+M").Icon("preproc_unmark_all.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_PREPROC_HELP, LANG(MENUITEM_TOOLS_PREPROC_HELP,"Ayuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		AddMenuItem(mnTOOLS, myMenuItem("generate_makefile", mxID_TOOLS_MAKEFILE, LANG(MENUITEM_TOOLS_GENERATE_MAKEFILE,"&Generar Makefile...")).Description("Genera el Makefile a partir de los fuentes y la configuración seleccionada").Icon("makefile.png").EnableIf(ecPROJECT));
		AddMenuItem(mnTOOLS, myMenuItem("open_terminal", mxID_TOOLS_CONSOLE, LANG(MENUITEM_TOOLS_OPEN_TERMINAL,"Abrir Co&nsola...")).Description("Inicia una terminal para interactuar con el interprete de comandos del sistema operativo").Icon("console.png"));
		AddMenuItem(mnTOOLS, myMenuItem("exe_info", mxID_TOOLS_EXE_PROPS, LANG(MENUITEM_TOOLS_EXE_INFO,"&Propiedades del Ejecutable...")).Description("Muestra información sobre el archivo compilado").Icon("exeinfo.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnTOOLS, myMenuItem("proy_stats", mxID_TOOLS_PROJECT_STATISTICS, LANG(MENUITEM_TOOLS_PROJECT_STATISTICS,"E&stadisticas del Proyecto...")).Description("Muestra información estadistica sobre los fuentes y demas archivos del proyecto").Icon("proystats.png").EnableIf(ecPROJECT));
		AddMenuItem(mnTOOLS, myMenuItem("draw_project", mxID_TOOLS_DRAW_PROJECT, LANG(MENUITEM_TOOLS_DRAW_PROJECT,"Grafo del Proyecto...")).Icon("draw_project.png").EnableIf(ecPROJECT));
		
		BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_SHARE,"Compartir Archivos en la &Red Local"),"Permite enviar o recibir codigos fuentes a traves de una red LAN","share.png");
			AddMenuItem(mnTOOLS, myMenuItem("open_shared",mxID_TOOLS_SHARE_OPEN, LANG(MENUITEM_TOOLS_SHARE_OPEN,"&Abrir compartido...")).Description("Abre un archivo compartido por otra PC en la red local.").Icon("abrirs.png"));
			AddMenuItem(mnTOOLS, myMenuItem("share_source",mxID_TOOLS_SHARE_SHARE, LANG(MENUITEM_TOOLS_SHARE_SHARE,"&Compartir actual...")).Description("Comparte el archivo en la red local.").Icon("compartir.png").EnableIf(ecSOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("share_list",mxID_TOOLS_SHARE_LIST, LANG(MENUITEM_TOOLS_SHARE_LIST,"&Ver lista de compartidos propios...")).Description("Comparte el archivo en la red local.").Icon("share_list.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_SHARE_HELP, LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra ayuda acerca de la compartición de archivos en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_DIFF,"&Comparar Archivos (diff)")).Description("Muestra opciones para plegar y desplegar codigo en distintos niveles").Icon("diff.png").EnableIf(ecSOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("diff_two_sources",mxID_TOOLS_DIFF_TWO,LANG(MENUITEM_TOOLS_DIFF_TWO,"&Dos fuentes abiertos...")).Description("Compara dos archivos de texto abiertos y los colorea segun sus diferencias").Icon("diff_sources.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_other_file",mxID_TOOLS_DIFF_DISK,LANG(MENUITEM_TOOLS_DIFF_DISK,"&Fuente actual contra archivo en disco...")).Description("Compara un archivo abierto contra un archivo en disco y lo colorea segun sus diferencias").Icon("diff_source_file.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_himself",mxID_TOOLS_DIFF_HIMSELF,LANG(MENUITEM_TOOLS_DIFF_HIMSELF,"&Cambios en fuente actual contra su version en disco...")).Description("Compara un archivos abierto y modificado contra su version en disco y lo colorea segun sus diferencias").Icon("diff_source_himself.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("diff_prev",mxID_TOOLS_DIFF_PREV,LANG(MENUITEM_TOOLS_DIFF_PREV,"Ir a Diferencia Anterior")).ShortCut("Shift+Alt+PageUp").Icon("diff_prev.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_next",mxID_TOOLS_DIFF_NEXT,LANG(MENUITEM_TOOLS_DIFF_NEXT,"Ir a Siguiente Diferencia")).ShortCut("Shift+Alt+PageDown").Icon("diff_next.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_show",mxID_TOOLS_DIFF_SHOW,LANG(MENUITEM_TOOLS_DIFF_SHOW,"Mostrar Cambio")).ShortCut("Alt+Shift+Ins").Description("Muestra en un globo emergente el cambio a aplicar").Icon("diff_show.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_apply",mxID_TOOLS_DIFF_APPLY,LANG(MENUITEM_TOOLS_DIFF_APPLY,"Apl&icar Cambio")).ShortCut("Alt+Ins").Description("Aplica el cambio marcado en la linea actual").Icon("diff_apply.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_discard",mxID_TOOLS_DIFF_DISCARD,LANG(MENUITEM_TOOLS_DIFF_DISCARD,"De&scartar Cambio")).ShortCut("Alt+Del").Description("Descarta el cambio marcado en la linea actual").Icon("diff_discard.png"));
			AddMenuItem(mnTOOLS, myMenuItem("diff_clear",mxID_TOOLS_DIFF_CLEAR,LANG(MENUITEM_TOOLS_DIFF_CLEAR,"&Borrar Marcas")).ShortCut("Alt+Shift+Del").Description("Quita los colores y marcas que se agregaron en un fuente producto de una comparación").Icon("diff_clear.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_DIFF_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra ayuda acerca de la comparación de fuentes en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_LIZARD,"Medir complejidad (lizard)")).Description("Permite analizar la complejidad ciclomática por función.").Icon("doxy.png").EnableIf(ecPROJECT));
			AddMenuItem(mnTOOLS, myMenuItem("doxy_generate",mxID_TOOLS_LIZARD_RUN,LANG(MENUITEM_TOOLS_LIZARD_RUN,"&Analizar ahora...")).ShortCut("").Icon("lizard_run.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_DOXY_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra una breve ayuda acerca de la integración de Doxygen en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_DOXYGEN,"Generar &Documentación (doxygen)")).Description("Doxygen permite generar automaticamente documentación a partir del codigo y sus comentarios").Icon("doxy.png").EnableIf(ecPROJECT));
			AddMenuItem(mnTOOLS, myMenuItem("doxy_generate",mxID_TOOLS_DOXY_GENERATE,LANG(MENUITEM_TOOLS_DOXYGEN_GENERATE,"&Generar...")).ShortCut("Ctrl+Shift+F1").Description("Ejecuta doxygen para generar la documentación de forma automatica").Icon("doxy_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem("doxy_config",mxID_TOOLS_DOXY_CONFIG,LANG(MENUITEM_TOOLS_DOXYGEN_CONFIGURE,"&Configurar...")).Description("Permite establecer opciones para el archivo de configuración de doxygen").Icon("doxy_config.png"));
			AddMenuItem(mnTOOLS, myMenuItem("doxy_view",mxID_TOOLS_DOXY_VIEW,LANG(MENUITEM_TOOLS_DOXYGEN_VIEW,"&Ver...")).ShortCut("Ctrl+F1").Description("Abre un explorador y muestra la documentación generada").Icon("doxy_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_DOXY_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra una breve ayuda acerca de la integración de Doxygen en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_WXFB,"Diseñar &Interfases (wxFormBuilder)")).Description("Diseño visual de interfaces con la biblioteca wxWidgets").Icon("wxfb.png").EnableIf(ecPROJECT));
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_config",mxID_TOOLS_WXFB_CONFIG,LANG(MENUITEM_TOOLS_WXFB_CONFIG,"Configurar &Integración con wxFormBuilder...")).Description("Configura características adicionales que facilitan el uso de wxFormBuilder").Icon("wxfb_activate.png"));
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_new_res",mxID_TOOLS_WXFB_NEW_RES,LANG(MENUITEM_TOOLS_WXFB_NEW_RESOURCE,"&Adjuntar un Nuevo Proyecto wxFB...")).Description("Crea un nuevo proyecto wxFormBuilder y lo agrega al proyecto en ZinjaI").Icon("wxfb_new_res.png"));
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_load_res",mxID_TOOLS_WXFB_LOAD_RES,LANG(MENUITEM_TOOLS_WXFB_LOAD_RESOURCE,"&Adjuntar un Proyecto wxFB Existente...")).Description("Agrega un proyecto wxFormBuilder ya existente al proyecto en ZinjaI").Icon("wxfb_load_res.png"));
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_regen",mxID_TOOLS_WXFB_REGEN,LANG(MENUITEM_TOOLS_WXFB_REGENERATE,"&Regenerar Proyectos wxFB")).ShortCut("Shift+Alt+F9").Description("Ejecuta wxFormBuilder para regenerar los archivos de recurso o fuentes que correspondan").Icon("wxfb_regen.png").Map());
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_inherit",mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(MENUITEM_TOOLS_WXFB_INHERIT,"&Generar Clase Heredada...")).Description("Genera una nueva clase a partir de las definidas por algun proyecto wxfb").Icon("wxfb_inherit.png").Map());
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_update_inherit",mxID_TOOLS_WXFB_UPDATE_INHERIT,LANG(MENUITEM_TOOLS_WXFB_UPDATE_INHERIT,"Act&ualizar Clase Heredada...")).Description("Actualiza los metodos de una clase que hereda de las definidas por algun proyecto wxfb").Icon("wxfb_update_inherit.png").Map());
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("wxfb_help_wx",mxID_TOOLS_WXFB_HELP_WX,LANG(MENUITEM_TOOLS_WXFB_REFERENCE,"Referencia &wxWidgets...")).Description("Muestra la ayuda de la biblioteca wxWidgets").Icon("ayuda_wx.png"));
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_WXFB_HELP,LANG(MENUITEM_TOOLS_WXFB_HELP,"A&yuda wxFB...")).Description("Muestra una breve ayuda acerca de la integración de wxFormBuilder en ZinjaI").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_CPPCHECK,"Análisis Estático (cppcheck)")).Icon("cppcheck.png").EnableIf(ecPROJECT));
			AddMenuItem(mnTOOLS, myMenuItem("cppcheck_run",mxID_TOOLS_CPPCHECK_RUN, LANG(MENUITEM_TOOLS_CPPCHECK_RUN,"Iniciar...")).Icon("cppcheck_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem("cppcheck_config",mxID_TOOLS_CPPCHECK_CONFIG, LANG(MENUITEM_TOOLS_CPPCHECK_CONFIG,"Configurar...")).Icon("cppcheck_config.png"));
			AddMenuItem(mnTOOLS, myMenuItem("cppcheck_view",mxID_TOOLS_CPPCHECK_VIEW, LANG(MENUITEM_TOOLS_CPPCHECK_VIEW,"Mostrar Panel de Resultados")).Icon("cppcheck_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_CPPCHECK_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_GPROF,"Perfil de Ejecución (gprof)")).Description("Gprof permite analizar las llamadas a funciones y sus tiempos de ejecución.").Icon("gprof.png").EnableIf(ecPROJECT_OR_SOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("gprof_activate",mxID_TOOLS_GPROF_SET, LANG(MENUITEM_TOOLS_GPROF_ACTIVATE,"Habilitar/Deshabilitar")).Description("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable.").Icon("comp_for_prof.png"));
			BeginSubMenu(mnTOOLS,LANG(MENUITEM_TOOLS_GPROF_LAYOUT,"Algoritmo de Dibujo"),"Permite seleccionar entre dos algoritmos diferentes para dibujar el grafo","dotfdp.png");
				AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_GPROF_DOT, LANG(MENUITEM_TOOLS_GPROF_DOT,"dot"))/*.Checkeable(config->Init.graphviz_dot)*/);
				AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_GPROF_FDP, LANG(MENUITEM_TOOLS_GPROF_FDP,"fdp"))/*.Checkeable(config->Init.graphviz_dot)*/);
			EndSubMenu(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("gprof_show_graph",mxID_TOOLS_GPROF_SHOW, LANG(MENUITEM_TOOLS_GPROF_SHOW,"Visualizar Resultados (grafo)...")).Description("Muestra graficamente la información de profiling de la ultima ejecución.").Icon("showgprof.png"));
			AddMenuItem(mnTOOLS, myMenuItem("gprof_list_output",mxID_TOOLS_GPROF_LIST, LANG(MENUITEM_TOOLS_GPROF_LIST,"Listar Resultados (texto)")).Description("Muestra la información de profiling de la ultima ejecución sin procesar.").Icon("listgprof.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_GPROF_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra ayuda acerca de como generar e interpretar la información de profiling").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_GCOV,"&Test de Cobertura (experimental, gcov)")).Description("Gcov permite contabilizar cuantas veces se ejecuta cada linea del código fuente.").Icon("gcov.png").EnableIf(ecPROJECT_OR_SOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("gcov_activate",mxID_TOOLS_GCOV_SET, LANG(MENUITEM_TOOLS_GCOV_ACTIVATE,"Habilitar/Deshabilitar")).Description("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable.").Icon("gcov_set.png"));
			AddMenuItem(mnTOOLS, myMenuItem("gcov_show_bar",mxID_TOOLS_GCOV_SHOW, LANG(MENUITEM_TOOLS_GCOV_SHOW_BAR,"Mostrar barra de resultados")).Description("Muestra un panel con los conteos por linea en el margen izquierdo de la ventana.").Icon("gcov_show.png"));
			AddMenuItem(mnTOOLS, myMenuItem("gcov_reset",mxID_TOOLS_GCOV_RESET, LANG(MENUITEM_TOOLS_GCOV_RESET,"Eliminar resultados")).Description("Elimina los archivos de resultados generados por el test de cobertura.").Icon("gcov_reset.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_GCOV_HELP,LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Description("Muestra ayuda acerca de como generar e interpretar la información del test de cobertura").Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
	#ifndef __WIN32__
		BeginSubMenu(mnTOOLS,myMenuItem("",wxID_ANY,LANG(MENUITEM_TOOLS_VALGRIND,"Análisis Dinámico (valgrind)")).Description("Valgrind permite analizar el uso de memoria dinamica para detectar perdidas y otros errores").Icon("valgrind.png").EnableIf(ecPROJECT_OR_SOURCE));
			AddMenuItem(mnTOOLS, myMenuItem("valgrind_run",mxID_TOOLS_VALGRIND_RUN, LANG(MENUITEM_TOOLS_VALGRIND_RUN,"Ejecutar...")).Icon("valgrind_run.png"));
			AddMenuItem(mnTOOLS, myMenuItem("valgrind_debug",mxID_TOOLS_VALGRIND_DEBUG, LANG(MENUITEM_TOOLS_VALGRIND_RUN,"Depurar...")).Icon("valgrind_debug.png"));
			AddMenuItem(mnTOOLS, myMenuItem("valgrind_view",mxID_TOOLS_VALGRIND_VIEW, LANG(MENUITEM_TOOLS_VALGRIND_VIEW,"Mostrar Panel de Resultados")).Icon("valgrind_view.png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_VALGRIND_HELP, LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
	#endif
		
		BeginSubMenu(mnTOOLS, LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables"),"","customTools.png",mxID_TOOLS_CUSTOM_TOOLS,maMAPPED);
			for (int i=0;i<MAX_CUSTOM_TOOLS;i++)
				AddMenuItem(mnTOOLS, myMenuItem(wxString("custom_tool_")<<i,mxID_CUSTOM_TOOL_0+i, wxString()<<i<<": <NULL>").Description(wxString(LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables"))<<" -> "<<i<<": <NULL>").Icon(wxString("customTool")<<i<<".png"));
			AddSeparator(mnTOOLS);
			AddMenuItem(mnTOOLS, myMenuItem("custom_settings",mxID_TOOLS_CUSTOM_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_CUSTOM_TOOLS_SETTINGS,"&Configurar (generales)...")).Icon("customToolsSettings.png"));
			AddMenuItem(mnTOOLS, myMenuItem("project_tools_settings",mxID_TOOLS_PROJECT_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_PROJECT_TOOLS_SETTINGS,"&Configurar (de proyecto)...")).Icon("projectToolsSettings.png").EnableIf(ecPROJECT));
			AddMenuItem(mnTOOLS, myMenuItem("",mxID_TOOLS_CUSTOM_HELP, LANG(MENUITEM_TOOLS_COMMON_HELP,"A&yuda...")).Icon("ayuda.png"));
		EndSubMenu(mnTOOLS);
		
		AddSeparator(mnTOOLS);
		
		AddMenuItem(mnTOOLS, myMenuItem("save_as_template",mxID_TOOLS_CREATE_TEMPLATE, LANG(MENUITEM_TOOLS_CREATE_TEMPLATE,"Guardar como nueva plantilla...")).Description("Permite guardar el programa simple o proyecto actual como plantilla").Icon("create_template.png").EnableIf(ecPROJECT_OR_SOURCE));
		AddMenuItem(mnTOOLS, myMenuItem("install_complement",mxID_TOOLS_INSTALL_COMPLEMENTS, LANG(MENUITEM_TOOLS_INSTALL_COMPLEMENTS,"Instalar Complementos...")).Description("Permite instalar un complemento ya descargado para ZinjaI").Icon("updates.png"));
	}	
	
	
	menues[mnHELP].Init("help",LANG(MENUITEM_HELP,"A&yuda")); {
		AddMenuItem(mnHELP, myMenuItem("help_cpp",mxID_HELP_CPP, LANG(MENUITEM_HELP_CPP,"Referencia C/C++...")).ShortCut("Alt+F1").Description("Muestra una completa referencia sobre el lenguaje").Icon("referencia.png"));
		AddMenuItem(mnHELP, myMenuItem("help_ide",mxID_HELP_GUI, LANG(MENUITEM_HELP_ZINJAI,"Ayuda sobre ZinjaI...")).ShortCut("F1").Description("Muestra la ayuda sobre el uso y las caracteristicas de este entorno...").Icon("ayuda.png"));
		AddMenuItem(mnHELP, myMenuItem("tutorials",mxID_HELP_TUTORIAL, LANG(MENUITEM_HELP_TUTORIALS,"Tutoriales...")).Description("Abre el cuadro de ayuda y muestra el indice de tutoriales disponibles").Icon("tutoriales.png"));
		AddMenuItem(mnHELP, myMenuItem("shortcuts",mxID_HELP_SHORTCUTS, LANG(MENUITEM_HELP_SHORTCUTS,"Atajos de teclado...")).Description("Muestra en la ventana de ayuda la lista de atajos de teclado disponibles en ZinjaI").Icon("shortcuts.png"));
		AddMenuItem(mnHELP, myMenuItem("show_tips",mxID_HELP_TIP, LANG(MENUITEM_HELP_TIPS,"&Mostrar sugerencias de uso...")).Description("Muestra sugerencias sobre el uso del programa...").Icon("tip.png"));
		AddSeparator(mnHELP);
		AddMenuItem(mnHELP, myMenuItem("opinion",mxID_HELP_OPINION, LANG(MENUITEM_HELP_OPINION,"Enviar sugerencia o reportar error...")).Description("Permite acceder a los foros oficiales de ZinjaI para dejar sugerencias, comentarios o reportar errores").Icon("opinion.png"));
		AddMenuItem(mnHELP, myMenuItem("find_updates",mxID_HELP_UPDATES, LANG(MENUITEM_HELP_UPDATES,"&Buscar actualizaciones...")).Description("Comprueba a traves de Internet si hay versiones mas recientes de ZinjaI disponibles...").Icon("updates.png"));
		AddMenuItem(mnHELP, myMenuItem("about",mxID_HELP_ABOUT, LANG(MENUITEM_HELP_ABOUT,"Acerca de...")).Description("Acerca de...").Icon("acercaDe.png"));
	}
	
	menues[mnHIDDEN].Init("hidden",LANG(MENUITEM_GLOBAL_SHORTCUTS,"Otros atajos")); {
		AddMenuItem(mnHIDDEN, myMenuItem("highlight_keyword",mxID_EDIT_HIGHLIGHT_WORD,LANG(MENUITEM_HIDDEN_HIGHLIGHT_KEYWORD,"Resaltar identificador")));
		AddMenuItem(mnHIDDEN, myMenuItem("lowercase",mxID_EDIT_MAKE_LOWERCASE,LANG(MENUITEM_HIDDEN_MAKE_LOWERCASE,"Pasar a minúsuculas")).ShortCut("Ctrl+U"));
		AddMenuItem(mnHIDDEN, myMenuItem("uppercase",mxID_EDIT_MAKE_UPPERCASE,LANG(MENUITEM_HIDDEN_MAKE_UPPERCASE,"Pasar a mayúsuculas")).ShortCut("Ctrl+Shift+U"));
		AddMenuItem(mnHIDDEN, myMenuItem("update_symbols",mxID_VIEW_UPDATE_SYMBOLS,LANG(MENUITEM_HIDDEN_UPDATE_SYMBOLS,"Actualizar arbol de simbolos")).ShortCut("F2"));
		AddMenuItem(mnHIDDEN, myMenuItem("autocomplete",mxID_EDIT_FORCE_AUTOCOMPLETE,LANG(MENUITEM_HIDDEN_FORCE_AUTOCOMPLETE,"Autocompletar")).ShortCut("Ctrl+Space"));
		AddMenuItem(mnHIDDEN, myMenuItem("open_selected",mxID_FILE_OPEN_SELECTED,LANG(MENUITEM_HIDDEN_OPEN_SELECTED,"Abrir archivo seleccionado en el código")).ShortCut("Ctrl+Return"));
		AddMenuItem(mnHIDDEN, myMenuItem("rect_edit",mxID_EDIT_RECTANGULAR_EDITION,LANG(MENUITEM_HIDDEN_RECTANGULAR_EDITION,"Activar edición rectangular")).ShortCut("Alt+Return"));
		AddMenuItem(mnHIDDEN, myMenuItem("prev_tab_1",mxID_VIEW_NOTEBOOK_PREV,LANG(MENUITEM_HIDDEN_PREV_TAB,"Pestaña de código anterior")).ShortCut("Ctrl+Shift+Tab"));
		AddMenuItem(mnHIDDEN, myMenuItem("next_tab_1",mxID_VIEW_NOTEBOOK_NEXT,LANG(MENUITEM_HIDDEN_NEXT_TAB,"Pestaña de código siguiente")).ShortCut("Ctrl+Tab"));
		AddMenuItem(mnHIDDEN, myMenuItem("prev_tab_2",mxID_VIEW_NOTEBOOK_PREV,LANG(MENUITEM_HIDDEN_PREV_TAB,"Pestaña de código anterior")).ShortCut("Ctrl+PageUp"));
		AddMenuItem(mnHIDDEN, myMenuItem("next_tab_2",mxID_VIEW_NOTEBOOK_NEXT,LANG(MENUITEM_HIDDEN_NEXT_TAB,"Pestaña de código siguiente")).ShortCut("Ctrl+PageDown"));
		AddMenuItem(mnHIDDEN, myMenuItem("",mxID_DEBUG_DO_THAT,"").ShortCut("Ctrl+Shift+F5"));
		AddMenuItem(mnHIDDEN, myMenuItem("open_header",mxID_FILE_OPEN_H,LANG(MENUITEM_HIDDEN_OPEN_H,"Alternar entre .h y .cpp")).ShortCut("F12"));
		AddMenuItem(mnHIDDEN, myMenuItem("compiling_options",mxID_RUN_CONFIG,LANG(MENUITEM_HIDDEN_RUN_CONFIG,"Opciones de compilación y ejecución")).ShortCut("Ctrl+Alt+P"));
		AddMenuItem(mnHIDDEN, myMenuItem("",mxID_INTERNAL_INFO,"").ShortCut("Ctrl+Shift+F6"));
		AddMenuItem(mnHIDDEN, myMenuItem("where_am_i",mxID_WHERE_AM_I,LANG(MENUITEM_HIDDEN_WHERE_I_AM,"Mostrar ubicación y contexto en el código")).ShortCut("Ctrl+Alt+Space"));
		AddMenuItem(mnHIDDEN, myMenuItem("autocode_autocomplete",mxID_EDIT_AUTOCODE_AUTOCOMPLETE,LANG(MENUITEM_HIDDEN_AUTOCODE_AUTOCOMPLETE,"Autocompletado de autocódigos")).ShortCut("Ctrl+Shift+Space"));
		AddMenuItem(mnHIDDEN, myMenuItem("close_all_but",mxID_FILE_CLOSE_ALL_BUT_ONE,LANG(MENUITEM_HIDDEN_CLOSE_ALL_BUT_ONE,"Cerrar todos los fuentes excepto el actual")).ShortCut("Ctrl+Alt+W"));	
		AddMenuItem(mnHIDDEN, myMenuItem("toolbar_find",mxID_EDIT_FIND_FROM_TOOLBAR,LANG(MENUITEM_HIDDEN_FIND_FROM_TOOLBAR,"Ir a la barra de herramienta de búsqueda")).ShortCut("Ctrl+Alt+F"));	
		AddMenuItem(mnHIDDEN, myMenuItem("macro_record",mxID_MACRO_RECORD,LANG(MENUITEM_HIDDEN_MACRO_RECORD,"Iniciar/Detener grabación de macro de teclado")).ShortCut("Ctrl+Shift+Q"));
		AddMenuItem(mnHIDDEN, myMenuItem("macro_replay",mxID_MACRO_REPLAY,LANG(MENUITEM_HIDDEN_MACRO_REPLAY,"Reproducir macro de teclado")).ShortCut("Ctrl+Q"));	
		AddMenuItem(mnHIDDEN, myMenuItem("help_code",mxID_HELP_CODE,LANG(MENUITEM_HIDDEN_HELP_CODE,"Ayuda rápida")).ShortCut("Shift+F1"));
		AddMenuItem(mnHIDDEN, myMenuItem("history_prev",mxID_NAVIGATION_HISTORY_PREV,LANG(MENUITEM_HIDDEN_NAVIGATION_HISTORY_PREV,"Historial de navegación -> ubicación anterior")).ShortCut("Alt+Left"));
		AddMenuItem(mnHIDDEN, myMenuItem("history_next",mxID_NAVIGATION_HISTORY_NEXT,LANG(MENUITEM_HIDDEN_NAVIGATION_HISTORY_NEXT,"Historial de navegación -> ubicación siguiente")).ShortCut("Alt+Right"));
		AddMenuItem(mnHIDDEN, myMenuItem("break_enable",mxID_DEBUG_ENABLE_DISABLE_BREAKPOINT,LANG(MENUITEM_HIDDEN_ENABLE_BREAKPOINT,"Habilitar/deshabilitar punto de interrupción")).ShortCut("").Checkeable(true));
		AddMenuItem(mnHIDDEN, myMenuItem("update_inspections",mxID_DEBUG_UPDATE_INSPECTIONS, LANG(MENUITEM_DEBUG_UPDATE_INSPECTIONS,"Actualizar inspecciones")).Icon("inspect_update.png"));
		for(int i=0;i<MAX_PROJECT_CUSTOM_TOOLS;i++) AddMenuItem(mnHIDDEN, myMenuItem(wxString("project_tool_")<<i,mxID_CUSTOM_PROJECT_TOOL_0+i,LANG1(MENUITEM_HIDDEN_PROJECT_CUSTOM_TOOL,"Herramienta personalizada de project <{1}>",wxString()<<i)).ShortCut(""));
		AddMenuItem(mnHIDDEN, myMenuItem("change_shortcuts",mxID_CHANGE_SHORTCUTS, LANG(MENUITEM_HELP_SHORTCUTS,"Atajos de teclado...")).ShortCut("Ctrl+Alt+Z"));
		AddMenuItem(mnHIDDEN, myMenuItem("toolbar_settings",mxID_TOOLBAR_SETTINGS, LANG(MENUITEM_TOOLBAR_SETTINGS,"Configurar barras de herramientas...")).ShortCut(""));
	}
	
}

void MenusAndToolsConfig::LoadToolbarsData ( ) {
	
	icon_size=24;
	
	toolbars[tbFILE].Init("file",LANG(CAPTION_TOOLBAR_FILE,"Archivo"),"T1"); {
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_NEW).Visible());
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_PROJECT));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_OPEN).Visible());
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_SOURCE_HISTORY_MORE));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_PROJECT_HISTORY_MORE));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnHIDDEN],mxID_FILE_OPEN_H));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnHIDDEN],mxID_FILE_OPEN_SELECTED));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_SAVE).Visible());
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_SAVE_AS).Visible());
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_SAVE_ALL));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_SAVE_PROJECT));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_EXPORT_HTML));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_PRINT));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_RELOAD));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_CLOSE));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_CLOSE_ALL));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_CLOSE_PROJECT));
		AddToolbarItem(tbFILE,myToolbarItem(menues[mnFILE],mxID_FILE_PROJECT_CONFIG));
	}
	
	
	toolbars[tbEDIT].Init("edit",LANG(CAPTION_TOOLBAR_EDIT,"Edición"),"T1"); {
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_UNDO).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_REDO).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_COPY).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_CUT).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_PASTE).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_TOGGLE_LINES_UP));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_TOGGLE_LINES_DOWN));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_DUPLICATE_LINES));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_DELETE_LINES));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_GOTO).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_GOTO_FUNCTION).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_GOTO_FILE).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_FIND).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_FIND_PREV));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_FIND_NEXT).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_REPLACE).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_INSERT_HEADER).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_COMMENT).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_UNCOMMENT).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_INDENT).Visible());
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_BRACEMATCH));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_SELECT_ALL));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_MARK_LINES));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_GOTO_MARK));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnEDIT],mxID_EDIT_LIST_MARKS));
		AddToolbarItem(tbEDIT,myToolbarItem(menues[mnHIDDEN],mxID_EDIT_FORCE_AUTOCOMPLETE));
	}
	
	
	toolbars[tbVIEW].Init("view",LANG(CAPTION_TOOLBAR_VIEW,"Ver"),"t1"); {
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_DUPLICATE_TAB).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_LINE_WRAP).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_WHITE_SPACE).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_CODE_STYLE).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnHIDDEN],mxID_VIEW_UPDATE_SYMBOLS));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_EXPLORER_TREE));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_PROJECT_TREE));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_COMPILER_TREE));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_FULLSCREEN).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_BEGINNER_PANEL));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_PREV_ERROR).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_VIEW_NEXT_ERROR).Visible());
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_ALL));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_1)); // 1,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_1,"Plegar el Primer Nivel"),ipre+_T("fold1.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_1,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_2)); // 2,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_2,"Plegar el Segundo Nivel"),ipre+_T("fold2.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_2,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_3)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_4)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_HIDE_5)); // 3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_ALL));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_1)); // 1,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_1,"Desplegar el Primer Nivel"),ipre+_T("unfold1.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_1,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_2)); // 2,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_2,"Desplegar el Segundo Nivel"),ipre+_T("unfold2.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_2,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_3)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_4)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		AddToolbarItem(tbVIEW,myToolbarItem(menues[mnVIEW],mxID_FOLD_SHOW_5)); // 3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
	}
	
	toolbars[tbRUN].Init("run",LANG(CAPTION_TOOLBAR_RUN,"Ejecución"),"T1"); {
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_COMPILE).Visible());
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_RUN).Label(LANG(TOOLBAR_CAPTION_RUN_RUN,"Guardar, compilar y ejecutar...")).Visible());
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_RUN_OLD));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_STOP).Visible());
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_CLEAN));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnRUN],mxID_RUN_CONFIG).Visible());
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_RUN).Visible());
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_ATTACH));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_TARGET));
#ifndef __WIN32__
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_LOAD_CORE_DUMP));
#endif
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_TOGGLE_BREAKPOINT));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_BREAKPOINT_OPTIONS));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_LIST_BREAKPOINTS));
		AddToolbarItem(tbRUN,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_INSPECT));
	}
	
	
	toolbars[tbDEBUG].Init("debug",LANG(CAPTION_TOOLBAR_DEBUG,"Depuración"),"t3"); {
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_RUN).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_PAUSE).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_STOP).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_STEP_IN).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_STEP_OVER).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_STEP_OUT).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_RUN_UNTIL).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_RETURN).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_JUMP).Visible());
#ifndef __WIN32__
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_ENABLE_INVERSE_EXEC).Checkeable());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_INVERSE_EXEC).Checkeable());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_SET_SIGNALS));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_SEND_SIGNAL));
#endif
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_INSPECT));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnHIDDEN],mxID_DEBUG_UPDATE_INSPECTIONS));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_BACKTRACE));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_THREADLIST));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_TOGGLE_BREAKPOINT));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_BREAKPOINT_OPTIONS));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnHIDDEN],mxID_DEBUG_ENABLE_DISABLE_BREAKPOINT));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_LIST_BREAKPOINTS).Visible());
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_LOG_PANEL));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_GDB_COMMAND));
#ifndef __WIN32__
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_SAVE_CORE_DUMP));
		AddToolbarItem(tbDEBUG,myToolbarItem(menues[mnDEBUG],mxID_DEBUG_PATCH));
#endif
	}
	
	
	toolbars[tbTOOLS].Init("tools",LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"),"t1"); {
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DRAW_FLOW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DRAW_CLASSES));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CODE_COPY_FROM_H).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_ALIGN_COMMENTS));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_REMOVE_COMMENTS));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_PREPROC_MARK_VALID));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_PREPROC_UNMARK_ALL));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_PREPROC_EXPAND_MACROS).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_MAKEFILE));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CONSOLE).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_EXE_PROPS));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_PROJECT_STATISTICS));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DRAW_PROJECT));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_SHARE_OPEN).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_SHARE_SHARE));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_SHARE_LIST));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_TWO).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_DISK));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_HIMSELF));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_SHOW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_APPLY));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_DISCARD));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DIFF_CLEAR));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DOXY_GENERATE).Visible());
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DOXY_CONFIG));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_DOXY_VIEW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_CONFIG));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_NEW_RES));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_LOAD_RES));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_REGEN));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_INHERIT_CLASS));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_UPDATE_INHERIT));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_WXFB_HELP_WX));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CPPCHECK_RUN));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CPPCHECK_CONFIG));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CPPCHECK_VIEW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GPROF_SET));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GPROF_SHOW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GPROF_LIST));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GCOV_SET));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GCOV_SHOW));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_GCOV_RESET));
#ifndef __WIN32__
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_VALGRIND_RUN));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_VALGRIND_DEBUG));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_VALGRIND_VIEW));
#endif
		for (int i=0;i<MAX_CUSTOM_TOOLS;i++)
			AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_CUSTOM_TOOL_0+i));
		AddToolbarItem(tbTOOLS,myToolbarItem(menues[mnTOOLS],mxID_TOOLS_CUSTOM_TOOLS_SETTINGS));
	}
	
	
	toolbars[tbMISC].Init("misc",LANG(CAPTION_TOOLBAR_MISC,"Miscelánea"),"T1"); {
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnFILE],mxID_FILE_PREFERENCES).Visible());
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_CPP).Visible());
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_GUI).Visible());
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_TUTORIAL));
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_SHORTCUTS));
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_TIP));
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_ABOUT));
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_OPINION).Visible());
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnHELP],mxID_HELP_UPDATES));
		AddToolbarItem(tbMISC,myToolbarItem(menues[mnFILE],mxID_FILE_EXIT));
	}
	
	/// @todo: agregar la de proyecto para que sea mas facil abajo crearla???
	toolbars[tbFIND].Init("find",LANG(CAPTION_TOOLBAR_FIND,"Búsqueda"),"T1");
	toolbars[tbPROJECT].Init("project",LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"),"T1");
	toolbars[tbSTATUS].Init("status",LANG(CAPTION_TOOLBAR_STATUS,"Estado de la depuración"),"t3");
	toolbars[tbDIFF].Init("diff",LANG(CAPTION_TOOLBAR_DIFF,"Diff"),"f");
	
}

	
void MenusAndToolsConfig::PopulateMenu(int menu_id) {
	vector<wxMenu*> menu_stack;
	wxMenu *current_menu = menues[menu_id].wx_menu = new wxMenu;
	wx_menu_bar->Append(current_menu,menues[menu_id].label);
	unsigned int items_size = menues[menu_id].items.size();
	for(unsigned int j=0;j<items_size;j++) { 
		myMenuItem &mi=menues[menu_id].items[j];
		int props=mi.properties;
		wxMenuItem *wx_item = nullptr;
		if (props&maSEPARATOR) {
			current_menu->AppendSeparator();
		} else if (props&maBEGIN_SUBMENU) {
			wxMenu *new_menu = new wxMenu;
			/*mi.*/wx_item = mxUT::AddSubMenuToMenu(current_menu,new_menu,mi.label,mi.description,mi.icon);
			if (props&maMAPPED) {
//					mapped_items.push_back(MappedItem(mi.wx_id,wx_item));
				mapped_menues.push_back(MappedSomething<wxMenu*>(mi.wx_id,new_menu));
			}
			menu_stack.push_back(current_menu);
			current_menu=new_menu;
		} else if (props&maEND_SUBMENU) {
			current_menu=menu_stack.back(); menu_stack.pop_back();
		} else if (!(props&maHIDDEN)) {
			wx_item = mxUT::AddItemToMenu(current_menu,&mi);
			if (props&maMAPPED) mapped_items.push_back(MappedSomething<wxMenuItem*>(mi.wx_id,/*mi.*/wx_item));
		}
		if (props&maDEBUG) { items_debug.push_back(AutoenabligItem(wx_item,true)); }
		else if (props&maNODEBUG) { items_debug.push_back(AutoenabligItem(wx_item,false)); }
		if (props&maPROJECT) { items_project.push_back(AutoenabligItem(wx_item,true)); }
//		else if (props&maNOPROJECT) { items_project.push_back(AutoenabligItem(wx_item,false)); }
	}
}


int getKeyCode(const wxString &key) {
	if (key.Len()==1) return key[0];
	if (key=="BACK") return WXK_BACK;
	if (key=="TAB") return WXK_TAB;
	if (key=="RETURN") return WXK_RETURN;
	if (key=="ESCAPE") return WXK_ESCAPE;
	if (key=="SPACE") return WXK_SPACE;
	if (key=="DELETE") return WXK_DELETE;
	if (key=="START") return WXK_START;
	if (key=="LBUTTON") return WXK_LBUTTON;
	if (key=="RBUTTON") return WXK_RBUTTON;
	if (key=="CANCEL") return WXK_CANCEL;
	if (key=="MBUTTON") return WXK_MBUTTON;
	if (key=="CLEAR") return WXK_CLEAR;
	if (key=="SHIFT") return WXK_SHIFT;
	if (key=="ALT") return WXK_ALT;
	if (key=="CONTROL") return WXK_CONTROL;
	if (key=="MENU") return WXK_MENU;
	if (key=="PAUSE") return WXK_PAUSE;
	if (key=="CAPITAL") return WXK_CAPITAL;
	if (key=="END") return WXK_END;
	if (key=="HOME") return WXK_HOME;
	if (key=="LEFT") return WXK_LEFT;
	if (key=="UP") return WXK_UP;
	if (key=="RIGHT") return WXK_RIGHT;
	if (key=="DOWN") return WXK_DOWN;
	if (key=="SELECT") return WXK_SELECT;
	if (key=="PRINT") return WXK_PRINT;
	if (key=="EXECUTE") return WXK_EXECUTE;
	if (key=="SNAPSHOT") return WXK_SNAPSHOT;
	if (key=="INSERT") return WXK_INSERT;
	if (key=="HELP") return WXK_HELP;
	if (key=="NUMPAD0") return WXK_NUMPAD0;
	if (key=="NUMPAD1") return WXK_NUMPAD1;
	if (key=="NUMPAD2") return WXK_NUMPAD2;
	if (key=="NUMPAD3") return WXK_NUMPAD3;
	if (key=="NUMPAD4") return WXK_NUMPAD4;
	if (key=="NUMPAD5") return WXK_NUMPAD5;
	if (key=="NUMPAD6") return WXK_NUMPAD6;
	if (key=="NUMPAD7") return WXK_NUMPAD7;
	if (key=="NUMPAD8") return WXK_NUMPAD8;
	if (key=="NUMPAD9") return WXK_NUMPAD9;
	if (key=="MULTIPLY") return WXK_MULTIPLY;
	if (key=="ADD") return WXK_ADD;
	if (key=="SEPARATOR") return WXK_SEPARATOR;
	if (key=="SUBTRACT") return WXK_SUBTRACT;
	if (key=="DECIMAL") return WXK_DECIMAL;
	if (key=="DIVIDE") return WXK_DIVIDE;
	if (key=="F1") return WXK_F1;
	if (key=="F2") return WXK_F2;
	if (key=="F3") return WXK_F3;
	if (key=="F4") return WXK_F4;
	if (key=="F5") return WXK_F5;
	if (key=="F6") return WXK_F6;
	if (key=="F7") return WXK_F7;
	if (key=="F8") return WXK_F8;
	if (key=="F9") return WXK_F9;
	if (key=="F10") return WXK_F10;
	if (key=="F11") return WXK_F11;
	if (key=="F12") return WXK_F12;
	if (key=="F13") return WXK_F13;
	if (key=="F14") return WXK_F14;
	if (key=="F15") return WXK_F15;
	if (key=="F16") return WXK_F16;
	if (key=="F17") return WXK_F17;
	if (key=="F18") return WXK_F18;
	if (key=="F19") return WXK_F19;
	if (key=="F20") return WXK_F20;
	if (key=="F21") return WXK_F21;
	if (key=="F22") return WXK_F22;
	if (key=="F23") return WXK_F23;
	if (key=="F24") return WXK_F24;
	if (key=="NUMLOCK") return WXK_NUMLOCK;
	if (key=="SCROLL") return WXK_SCROLL;
	if (key=="PAGEUP") return WXK_PAGEUP;
	if (key=="PAGEDOWN") return WXK_PAGEDOWN;
	if (key=="NUMPAD_SPACE") return WXK_NUMPAD_SPACE;
	if (key=="NUMPAD_TAB") return WXK_NUMPAD_TAB;
	if (key=="NUMPAD_ENTER") return WXK_NUMPAD_ENTER;
	if (key=="NUMPAD_F1") return WXK_NUMPAD_F1;
	if (key=="NUMPAD_F2") return WXK_NUMPAD_F2;
	if (key=="NUMPAD_F3") return WXK_NUMPAD_F3;
	if (key=="NUMPAD_F4") return WXK_NUMPAD_F4;
	if (key=="NUMPAD_HOME") return WXK_NUMPAD_HOME;
	if (key=="NUMPAD_LEFT") return WXK_NUMPAD_LEFT;
	if (key=="NUMPAD_UP") return WXK_NUMPAD_UP;
	if (key=="NUMPAD_RIGHT") return WXK_NUMPAD_RIGHT;
	if (key=="NUMPAD_DOWN") return WXK_NUMPAD_DOWN;
	if (key=="NUMPAD_PAGEUP") return WXK_NUMPAD_PAGEUP;
	if (key=="NUMPAD_PAGEDOWN") return WXK_NUMPAD_PAGEDOWN;
	if (key=="NUMPAD_END") return WXK_NUMPAD_END;
	if (key=="NUMPAD_BEGIN") return WXK_NUMPAD_BEGIN;
	if (key=="NUMPAD_INSERT") return WXK_NUMPAD_INSERT;
	if (key=="NUMPAD_DELETE") return WXK_NUMPAD_DELETE;
	if (key=="NUMPAD_EQUAL") return WXK_NUMPAD_EQUAL;
	if (key=="NUMPAD_MULTIPLY") return WXK_NUMPAD_MULTIPLY;
	if (key=="NUMPAD_ADD") return WXK_NUMPAD_ADD;
	if (key=="NUMPAD_SEPARATOR") return WXK_NUMPAD_SEPARATOR;
	if (key=="NUMPAD_SUBTRACT") return WXK_NUMPAD_SUBTRACT;
	if (key=="NUMPAD_DECIMAL") return WXK_NUMPAD_DECIMAL;
	if (key=="NUMPAD_DIVIDE") return WXK_NUMPAD_DIVIDE;
	// the following key codes are only generated under Windows currently
	if (key=="WINDOWS_LEFT") return WXK_WINDOWS_LEFT;
	if (key=="WINDOWS_RIGHT") return WXK_WINDOWS_RIGHT;
	if (key=="WINDOWS_MENU") return WXK_WINDOWS_MENU;
	if (key=="COMMAND") return WXK_COMMAND;
	return 0;
}
/**
* @brief Crea los menúes de la ventana principal y los configura en el modo inicial (no debug, no project)
**/
void MenusAndToolsConfig::CreateMenues () {
	
	if (!wx_menu_bar) {
		wx_menu_bar = new wxMenuBar;
		main_window->SetMenuBar(wx_menu_bar);
	} else {
		while (wx_menu_bar->GetMenuCount()) wx_menu_bar->Remove(0);
	}
		
	// create regular menus
	for(unsigned int menu_id=0;menu_id<mnHIDDEN;menu_id++) PopulateMenu(menu_id);
	
	// create some special submenues
//	main_window->UpdateCustomTools(false);
	wxString ipre=DIR_PLUS_FILE("16","recent");
	for(int k=0;k<2;k++) { 
		wxString *cfglast = k==0?config->Files.last_project:config->Files.last_source;
		wxMenuItem **mnihistory = k==0?menu_data->file_project_history:menu_data->file_source_history;
		wxMenu *mnurecent = menu_data->GetMenu(k==0?mxID_FILE_PROJECT_RECENT:mxID_FILE_SOURCE_RECENT);
		int history_id = k==0?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0;
		for (int i=0;i<config->Init.history_len;i++)
			if (!cfglast[i].IsEmpty()) 
				mnihistory[i] = mxUT::AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],wxString(ipre)<<i<<(".png"),i);
	}
	
	SetAccelerators();
	
	// set items state // ahora gestionado en el evento menu_open
	SetProjectMode(project);
	SetDebugMode(debug?debug->IsDebugging():false);
}

void MenusAndToolsConfig::SetAccelerators() {
	// set accelerators for commands that are not in any menu
	wxAcceleratorEntry *aentries = new wxAcceleratorEntry[menues[mnHIDDEN].items.size()]; int ac=0;
	for(unsigned int i=0;i<menues[mnHIDDEN].items.size();i++) {
		int id=menues[mnHIDDEN].items[i].wx_id;
		wxString str=menues[mnHIDDEN].items[i].shortcut;
		str.MakeUpper(); str.Replace(" ","");
		if (!str.Len()) continue;
		int flags=0;
		if (str.Contains("CTRL+")) { flags|=wxACCEL_CTRL; str.Replace("CTRL+",""); }
		if (str.Contains("SHIFT+")) { flags|=wxACCEL_SHIFT; str.Replace("SHIFT+",""); }
		if (str.Contains("ALT+")) { flags|=wxACCEL_ALT; str.Replace("ALT+",""); }
		int keycode=getKeyCode(str);
		if (!keycode) continue;
		aentries[ac++].Set(flags,keycode,id);
	}
	wxAcceleratorTable accel(ac,aentries);
	main_window->SetAcceleratorTable(accel);
	delete []aentries;
}

void MenusAndToolsConfig::CreateWxToolbar(int tb_id) {
	toolbars[tb_id].wx_toolbar = new wxToolBar(main_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER | (toolbars[tb_id].position.left||toolbars[tb_id].position.right?wxTB_VERTICAL:wxTB_HORIZONTAL));
	toolbars[tb_id].wx_toolbar->SetToolBitmapSize(wxSize(icon_size,icon_size)); 
	main_window->aui_manager.AddPane(toolbars[tb_id].wx_toolbar, wxAuiPaneInfo().Name(wxString("toolbar_")+toolbars[tb_id].key).Caption(toolbars[tb_id].label).ToolbarPane().Hide());
}

void MenusAndToolsConfig::UpdateToolbar(int tb_id, bool only_items) {
	if (!only_items) {
		// destroy the previous one
		main_window->aui_manager.DetachPane(toolbars[tb_id].wx_toolbar);
		toolbars[tb_id].wx_toolbar->Destroy();
		toolbars[tb_id].wx_toolbar=nullptr;
		CreateWxToolbar(tb_id);
	} else {
		toolbars[tb_id].wx_toolbar->ClearTools();
	}
	PopulateToolbar(tb_id);
	AdjustToolbarSize(tb_id);
}
	
void MenusAndToolsConfig::CreateToolbars() {
	
	wxString ipre=DIR_PLUS_FILE(wxString()<<icon_size,"");
	
	if (!wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,wxString()<<icon_size))) {
		wxString icsz = wxString()<<icon_size<<"x"<<icon_size;
		mxMessageDialog(nullptr,
			wxString()<<LANG1(MAIN_WINDOW_NO_ICON_SIZE,""
			"El tema de iconos seleccionado no tiene iconos del tamaño elegido (<{1}>)\n"
			"Se utilizaran los iconos del tamaño predeterminado (16x16).\n"
			"Para modificarlo utilice el cuadro de Preferencias (menu Archivo).",wxString()<<icsz),
			LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
		icon_size=16;
		ipre=DIR_PLUS_FILE("16","");
	}
	
	// create empty wxToolBars 
	for(unsigned int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) CreateWxToolbar(tb_id);
	
	// barras estandar: FILE, EDIT, VIEW, RUN, DEBUG, TOOLS, MISC, PROJECT
	for(int tb_id=0;tb_id<tbPROJECT;tb_id++) PopulateToolbar(tb_id); 
	
	// barras especiales: busqueda
	{
		myToolbar &tb = toolbars[tbFIND];
//		tb.wx_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
//		tb.wx_toolbar->SetToolBitmapSize(wxSize(icon_size,icon_size));
		tb.wx_toolbar->AddControl( toolbar_find_text = new wxTextCtrl(tb.wx_toolbar,mxID_TOOLBAR_FIND,wxEmptyString,wxDefaultPosition,wxSize(100,20),wxTE_PROCESS_ENTER) );
		toolbar_find_text->SetToolTip(LANG(TOOLBAR_FIND_TEXT,"Texto a Buscar"));
		mxUT::AddTool(tb.wx_toolbar,mxID_EDIT_TOOLBAR_FIND,LANG(TOOLBAR_FIND_CAPTION,"Busqueda Rapida"),ipre+_T("buscar.png"),LANG(TOOLBAR_FIND_BUTTON,"Buscar siguiente"));
//		main_window->aui_manager.AddPane(tb.wx_toolbar, wxAuiPaneInfo().Name(wxString("toolbar_")+tb.key).Caption(tb.label).ToolbarPane().Hide());
	}
	
	// barras especiales: estado de la depuracion
	{
		myToolbar &tb = toolbars[tbSTATUS];
//		tb.wx_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
//		tb.wx_toolbar->SetToolBitmapSize(wxSize(icon_size,icon_size));
		tb.wx_toolbar->AddControl( toolbar_status_text = new wxStaticText(tb.wx_toolbar,wxID_ANY,"",wxDefaultPosition,wxSize(2500,20)) );
		toolbar_status_text->SetForegroundColour(wxColour("Z DARK BLUE"));
//		main_window->aui_manager.AddPane(tb.wx_toolbar, wxAuiPaneInfo().Name(wxString("toolbar_")+tb.key).Caption(tb.label).ToolbarPane().Top().Hide());
	}
	
	// barras especiales: diff
	{
		myToolbar &tb = toolbars[tbDIFF];
//		tb.wx_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
//		tb.wx_toolbar->SetToolBitmapSize(wxSize(icon_size,icon_size));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_PREV,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_PREV,"Diferencia Anterior"),ipre+_T("diff_prev.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_PREV,"Herramientas -> Comparar Archivos -> Ir a Diferencia Anterior"));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_NEXT,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_NEXT,"Siguiente Diferencia"),ipre+_T("diff_next.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_NEXT,"Herramientas -> Comparar Archivos -> Ir a Siguiente Diferencia"));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_APPLY,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_DISCARD,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		mxUT::AddTool(tb.wx_toolbar,mxID_TOOLS_DIFF_CLEAR,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparación"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
//		main_window->aui_manager.AddPane(tb.wx_toolbar, wxAuiPaneInfo().Name(wxString("toolbar_")+tb.key).Caption(tb.label).ToolbarPane().Top().Float().LeftDockable(false).RightDockable(false).Hide());
	}
	
	// ajustar tamaños
	for(unsigned int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) AdjustToolbarSize(tb_id);
	main_window->SortToolbars(false);
}

void MenusAndToolsConfig::PopulateToolbar(int tb_id) {
	
	wxToolBar *wx_toolbar = toolbars[tb_id].wx_toolbar;
	wxString ipre=DIR_PLUS_FILE(wxString()<<icon_size,"");
	
	if (tb_id!=tbPROJECT) { // items de la base de items
		vector<myToolbarItem> &items = toolbars[tb_id].items;
		for(unsigned int i=0;i<items.size();i++)
			if (items[i].visible) 
				mxUT::AddTool(wx_toolbar,items[i].wx_id,items[i].label,ipre+items[i].icon,items[i].description,items[i].checkeable?wxITEM_CHECK:wxITEM_NORMAL);
	
	} else { // caso especial, barra de proyecto
		// elementos especificos de proyectos wx
		if (project->GetWxfbActivated()) {
			int wx_ids[3] = { mxID_TOOLS_WXFB_CONFIG, mxID_TOOLS_WXFB_INHERIT_CLASS, mxID_TOOLS_WXFB_HELP_WX };
			for(int i=0;i<3;i++) {
				myToolbarItem item(menues[mnTOOLS],wx_ids[i]);
				mxUT::AddTool(wx_toolbar,item.wx_id,item.label,ipre+item.icon,item.description,item.checkeable?wxITEM_CHECK:wxITEM_NORMAL);
			}
		}
		// herramientas personalizadas
		bool have_tool=false;
		for (int i=0;i<MAX_PROJECT_CUSTOM_TOOLS;i++) {
			if (project->custom_tools[i].on_toolbar) {
				have_tool=true;
				wxString str(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada ")); str<<i<<" ("<<project->custom_tools[i].name<<")";;
				mxUT::AddTool(wx_toolbar,mxID_CUSTOM_PROJECT_TOOL_0+i,str,ipre+wxString("projectTool")<<i<<".png",str);
			}
		}
		if (!have_tool) { // si no hay definidas herramientas personalizadas, muestra el boton para configurarlas
			myToolbarItem item(menues[mnTOOLS],mxID_TOOLS_PROJECT_TOOLS_SETTINGS);
			mxUT::AddTool(wx_toolbar,item.wx_id,item.label,ipre+item.icon,item.description,item.checkeable?wxITEM_CHECK:wxITEM_NORMAL);
		}
	}
	
}

void MenusAndToolsConfig::AdjustToolbarSize(int tb_id) {
	wxToolBar *wx_toolbar = toolbars[tb_id].wx_toolbar;
	if (wx_toolbar) { wx_toolbar->Realize(); main_window->aui_manager.GetPane(wx_toolbar).BestSize(wx_toolbar->GetBestSize()); }
}

void MenusAndToolsConfig::SetDebugMode (bool mode) {
	for(unsigned int i=0;i<items_debug.size();i++) 
		items_debug[i].Enable(mode);
}

void MenusAndToolsConfig::SetProjectMode (bool mode) {
	for(unsigned int i=0;i<items_project.size();i++) 
		items_project[i].Enable(mode);
}

int MenusAndToolsConfig::ToolbarFromTool(int tool_id) {
	for(int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) { 
		vector<myToolbarItem> &items = toolbars[tb_id].items;
		for(unsigned int i=0;i<items.size();i++) {
			if (items[i].wx_id==tool_id) return tb_id;
		}
	}
	return -1;
}

void MenusAndToolsConfig::CreateMenuesAndToolbars (mxMainWindow * _main_window) {
	main_window=_main_window;
	TransferStatesFromConfig();
	CreateToolbars();
	CreateMenues();
}

MenusAndToolsConfig::myMenuItem *MenusAndToolsConfig::GetMyMenuItem (int menu_id, int item_id) {
	vector<myMenuItem> &v = menues[menu_id].items;
	for(unsigned int i=0;i<v.size();i++) { 
		if (v[i].wx_id==item_id) return &(v[i]);
	}
	return nullptr;
}

MenusAndToolsConfig::myToolbarItem *MenusAndToolsConfig::GetMyToolbarItem (int toolbar_id, int item_id) {
	vector<myToolbarItem> &v = toolbars[toolbar_id].items;
	for(unsigned int i=0;i<v.size();i++) { 
		if (v[i].wx_id==item_id) return &(v[i]);
	}
	return nullptr;
}

void MenusAndToolsConfig::TransferStatesFromConfig() {
	// sync menu check item with toolbar visibility status
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_FILE)->Checkeable(_toolbar_visible(tbFILE));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_EDIT)->Checkeable(_toolbar_visible(tbEDIT));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_VIEW)->Checkeable(_toolbar_visible(tbVIEW));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_FIND)->Checkeable(_toolbar_visible(tbFIND));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_RUN)->Checkeable(_toolbar_visible(tbRUN));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_TOOLS)->Checkeable(_toolbar_visible(tbTOOLS));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_PROJECT)->Checkeable(_toolbar_visible(tbPROJECT));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_DEBUG)->Checkeable(_toolbar_visible(tbDEBUG));
	GetMyMenuItem(mnVIEW,mxID_VIEW_TOOLBAR_MISC)->Checkeable(_toolbar_visible(tbMISC));
	if (!config->Init.left_panels) GetMyMenuItem(mnVIEW,mxID_VIEW_LEFT_PANELS)->Hide();
	GetMyMenuItem(mnTOOLS,mxID_TOOLS_GPROF_DOT)->Checkeable(config->Init.graphviz_dot);
	GetMyMenuItem(mnTOOLS,mxID_TOOLS_GPROF_FDP)->Checkeable(config->Init.graphviz_dot);
	for (int i=0;i<MAX_CUSTOM_TOOLS;i++) {
		wxString str; str<<i<<": "<<config->custom_tools[i].name;
		wxString desc(LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables")); desc<<" -> "<<i<<": "<<config->custom_tools[i].command;
		GetMyMenuItem(mnTOOLS,mxID_CUSTOM_TOOL_0+i)->Label(str).Description(config->custom_tools[i].command).SetVisible(!config->custom_tools[i].name.IsEmpty());
		GetMyToolbarItem(mnTOOLS,mxID_CUSTOM_TOOL_0+i)->Label(str).Description(desc).SetVisible(config->custom_tools[i].on_toolbar);
	}
}

//void MenusAndToolsConfig::SetMenuItemsStates (wxMenu * wx_menu) {
//	bool vec[ecCOUNT], source=main_window->notebook_sources->GetPageCount()!=0;
//	vec[ecSOURCE]=source;
//	vec[ecPROJECT]=project;
//	vec[ecPROJECT_OR_SOURCE]=source||project;
//	vec[ecDEBUG]=debug->debugging;
//	vec[ecNOT_DEBUG]=!debug->debugging;
//	vec[ecDEBUG_PAUSED]=debug->debugging&&(!debug->waiting);
//	vec[ecDEBUG_NOT_PAUSED]=debug->debugging&&debug->waiting;
//	vec[ecNOT_DEBUG_OR_DEBUG_PAUSED]=(!debug->IsDebugging())||(!debug->waiting);
//	for(int menu_id=0;menu_id<mnCOUNT;menu_id++) {
//		if (menues[menu_id].wx_menu==wx_menu) {
//			vector<myMenuItem> &items=menues[menu_id].items;
//			for(int i=0;i<items.size();i++) { 
//				if (items[i].enabling_condition!=ecALWAYS)
//					items[i].wx_item->Enable(vec[items[i].enabling_condition]);
//			}
//			break;
//		}
//	}
//	// casos especiales
////	if (wx_menu==menues[mnVIEW].wx_menu) {
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_LINE_WRAP, LANG(MENUITEM_VIEW_LINE_WRAP,"&Ajuste de linea")).ShortCut("Alt+F11").Description("Muestra las lineas largas como en varios renglones").Icon("lineWrap.png").Checkeable(false).EnableIf(ecSOURCE));	
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_WHITE_SPACE, LANG(MENUITEM_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de &fin de linea")).Description("Muestra las lineas largas como en varios renglones").Icon("whiteSpace.png").Checkeable(false).EnableIf(ecSOURCE));	
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_CODE_STYLE, LANG(MENUITEM_VIEW_SYNTAX_HIGHLIGHT,"&Colorear Sintaxis")).ShortCut("Shift+F11").Description("Resalta el codigo con diferentes colores y formatos de fuente.").Icon("syntaxColour.png").Checkeable(false).EnableIf(ecSOURCE));	
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_FULLSCREEN, LANG(MENUITEM_VIEW_FULLSCREEN,"Ver a Pantalla &Completa")).ShortCut("F11").Description("Muestra el editor a pantalla completa, ocultando tambien los demas paneles").Icon("fullScreen.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_BEGINNER_PANEL, LANG(MENUITEM_VIEW_BEGINNER_PANEL,"Mostrar Panel de Mini-Plantillas")).Description("Muestra un panel con plantillas y estructuras basicas de c++").Icon("beginer_panel.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_LEFT_PANELS, LANG(MENUITEM_VIEW_LEFT_PANELS,"&Mostrar Panel de Arboles")).Description("Muestra el panel con los arboles de proyecto, simbolos y explorador de archivos").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_PROJECT_TREE, LANG(MENUITEM_VIEW_PROJECT_TREE,"&Mostrar Arbol de &Proyecto")).Description("Muestra el panel del arbol de proyecto/archivos abiertos").Icon("projectTree.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_EXPLORER_TREE, LANG(MENUITEM_VIEW_EXPLORER_TREE,"Mostrar &Explorardor de Archivos")).ShortCut("Ctrl+E").Description("Muestra el panel explorador de archivos").Icon("explorerTree.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_SYMBOLS_TREE, LANG(MENUITEM_VIEW_SYMBOLS_TREE,"Mostrar Arbol de &Simbolos")).Description("Analiza el codigo fuente y construye un arbol con los simbolos declarados en el mismo.").Icon("symbolsTree.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_COMPILER_TREE, LANG(MENUITEM_VIEW_COMPILER_TREE,"&Mostrar Resultados de la Compilación")).Description("Muestra un panel con la salida del compilador").Icon("compilerTree.png").Checkeable(false));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_FILE, LANG(MENUITEM_VIEW_TOOLBAR_FILE,"&Mostrar Barra de Herramientas Archivo")).Description("Muestra la barra de herramientas para el manejo de archivos")/*.Checkeable(_toolbar_visible(tbFILE))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_EDIT, LANG(MENUITEM_VIEW_TOOLBAR_EDIT,"&Mostrar Barra de Herramientas Edición")).Description("Muestra la barra de herramientas para la edición del fuente")/*.Checkeable(_toolbar_visible(tbEDIT))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_VIEW, LANG(MENUITEM_VIEW_TOOLBAR_VIEW,"&Mostrar Barra de Herramientas Ver")).Description("Muestra la barra de herramientas para las opciones de visualización")/*.Checkeable(_toolbar_visible(tbVIEW))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_FIND, LANG(MENUITEM_VIEW_TOOLBAR_FIND,"&Mostrar Barra de Busqueda Rapida")).Description("Muestra un cuadro de texto en la barra de herramientas que permite buscar rapidamente en un fuente")/*.Checkeable(_toolbar_visible(tbFIND))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_RUN, LANG(MENUITEM_VIEW_TOOLBAR_RUN,"&Mostrar Barra de Herramientas Ejecución")).Description("Muestra la barra de herramientas para la compilación y ejecución del programa")/*.Checkeable(_toolbar_visible(tbRUN))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_TOOLS, LANG(MENUITEM_VIEW_TOOLBAR_TOOLS,"&Mostrar Barra de Herramientas Herramientas")).Description("Muestra la barra de herramientas para las herramientas adicionales")/*.Checkeable(_toolbar_visible(tbTOOLS))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_PROJECT, LANG(MENUITEM_VIEW_TOOLBAR_PROJECT,"&Mostrar Barra de Herramientas Proyecto")).Description("Muestra la barra de herramientas para las herramientas personalizables propias del proyecto")/*.Checkeable(_toolbar_visible(tbPROJECT))*/.EnableIf(ecPROJECT));
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_DEBUG, LANG(MENUITEM_VIEW_TOOLBAR_DEBUG,"&Mostrar Barra de Herramientas Depuración")).Description("Muestra la barra de herramientas para la depuración del programa")/*.Checkeable(_toolbar_visible(tbDEBUG))*/);
////		AddMenuItem(mnVIEW, myMenuItem(mxID_VIEW_TOOLBAR_MISC, LANG(MENUITEM_VIEW_TOOLBAR_MISC,"&Mostrar Barra de Herramientas Miscelanea")).Description("Muestra la barra de herramientas con commandos miselaneos")/*.Checkeable(_toolbar_visible(tbMISC))*/);
////	} else if (wx_menu==menues[mnTOOLS]) {
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_CONFIG,LANG(MENUITEM_TOOLS_WXFB_CONFIG,"Configurar &Integración con wxFormBuilder...")).Description("Configura características adicionales que facilitan el uso de wxFormBuilder").Icon("wxfb_activate.png"));
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_NEW_RES,LANG(MENUITEM_TOOLS_WXFB_NEW_RESOURCE,"&Adjuntar un Nuevo Proyecto wxFB...")).Description("Crea un nuevo proyecto wxFormBuilder y lo agrega al proyecto en ZinjaI").Icon("wxfb_new_res.png"));
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_LOAD_RES,LANG(MENUITEM_TOOLS_WXFB_LOAD_RESOURCE,"&Adjuntar un Proyecto wxFB Existente...")).Description("Agrega un proyecto wxFormBuilder ya existente al proyecto en ZinjaI").Icon("wxfb_load_res.png"));
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_REGEN,LANG(MENUITEM_TOOLS_WXFB_REGENERATE,"&Regenerar Proyectos wxFB")).ShortCut("Shift+Alt+F9").Description("Ejecuta wxFormBuilder para regenerar los archivos de recurso o fuentes que correspondan").Icon("wxfb_regen.png").Map());
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(MENUITEM_TOOLS_WXFB_INHERIT,"&Generar Clase Heredada...")).Description("Genera una nueva clase a partir de las definidas por algun proyecto wxfb").Icon("wxfb_inherit.png").Map());
////		AddMenuItem(mnTOOLS, myMenuItem(mxID_TOOLS_WXFB_UPDATE_INHERIT,LANG(MENUITEM_TOOLS_WXFB_UPDATE_INHERIT,"Act&ualizar Clase Heredada...")).Description("Actualiza los metodos de una clase que hereda de las definidas por algun proyecto wxfb").Icon("wxfb_update_inherit.png").Map());
////	} else if (wx_menu==menues[mnRUN]) {
////		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_COMPILE, LANG(MENUITEM_RUN_COMPILE,"&Compilar")).ShortCut("Shift+F9").Description("Guarda y compila el fuente actual").Icon("compilar.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
////		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_CLEAN, LANG(MENUITEM_RUN_CLEAN,"&Limpiar")).ShortCut("Ctrl+Shift+F9").Description("Elimina los objetos y ejecutables compilados").Icon("limpiar.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
////		AddMenuItem(mnRUN, myMenuItem( mxID_RUN_STOP, LANG(MENUITEM_RUN_STOP,"&Detener")).Description("Detiene la ejecución del programa").Icon("detener.png").Map().EnableIf(ecPROJECT_OR_SOURCE));
////	}
//}



bool MenusAndToolsConfig::ParseToolbarConfigLine (const wxString & key, const wxString & value) {
	if (key=="icon_size") { mxUT::ToInt(value,icon_size); return true; }
	int p=key.Index('.'); if (p==wxNOT_FOUND || p==int(key.Len()-1)) return false;
	wxString tb_name=key.Mid(0,p);
	if (tb_name=="positions") {
		tb_name = key.AfterFirst('.');
		for(int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) { 
			if (toolbars[tb_id].key==tb_name) {
				toolbars[tb_id].position=value;
				return true;
			}
		}
	} else {
		for(int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) { 
			if (toolbars[tb_id].key==tb_name) {
				wxString name = key.AfterFirst('.');
				vector<myToolbarItem> &items = toolbars[tb_id].items;
				for(unsigned int i=0;i<items.size();i++) { 
					if (items[i].key==name) {
						items[i].visible = mxUT::IsTrue(value);
						return true;
					}
				}
			}
		}
	}
	return false;
}


bool MenusAndToolsConfig::LoadToolbarsSettings (const wxString & full_path) {
	wxTextFile fil(full_path);
	if (!fil.Exists()) return false;
	fil.Open();
	wxString key, value;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		key=str.BeforeFirst('=');
		value=str.AfterFirst('=');
		if (str.IsEmpty() || str[0]=='#' || str[0]=='[') continue;
		ParseToolbarConfigLine(key,value);
	}
	fil.Close();
	return true;
}

bool MenusAndToolsConfig::SaveToolbarsSettings (const wxString & full_path) {
	// open
	wxTextFile file(full_path);
	if (file.Exists()) file.Open();
	else file.Create();
	file.Clear();
	// write
	file.AddLine("[Toolbars]");
	file.AddLine(wxString("icon_size=")<<icon_size);
	for(int tb_id=0;tb_id<tbCOUNT_FULL;tb_id++) {
		file.AddLine(wxString("positions.")+toolbars[tb_id].key+"="+(wxString)toolbars[tb_id].position);
		wxString toolbar_key = toolbars[tb_id].key+".";
		vector<myToolbarItem> &items = toolbars[tb_id].items;
		for(unsigned int i=0;i<items.size();i++) {
			if (items[i].key.IsEmpty()) continue;
			file.AddLine(toolbar_key+items[i].key+(items[i].visible?"=1":"=0"));
		}
	}
	// close
	file.Write();
	file.Close();
	return true;
}

bool MenusAndToolsConfig::ParseMenuConfigLine (const wxString & key, const wxString & value) {
	int p=key.Index('.'); if (p==wxNOT_FOUND || p==int(key.Len()-1)) return false;
	wxString mn_name=key.Mid(0,p); 
	for(int mn_id=0;mn_id<mnCOUNT;mn_id++) { 
		if (menues[mn_id].key==mn_name) {
			wxString name = key.AfterFirst('.');
			vector<myMenuItem> &items = menues[mn_id].items;
			for(unsigned int i=0;i<items.size();i++) { 
				if (items[i].key==name) {
					items[i].RedefineShortcut(value);
					return true;
				}
			}
		}
	}
	return false;
}

bool MenusAndToolsConfig::LoadShortcutsSettings(const wxString &full_path) {
	wxTextFile fil(full_path);
	if (!fil.Exists()) return false;
	fil.Open();
	wxString key, value;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		key=str.BeforeFirst('=');
		value=str.AfterFirst('=');
		if (str.IsEmpty() || str[0]=='#' || str[0]=='[') continue;
		ParseMenuConfigLine(key,value);
	}
	fil.Close();
	return true;
}
	

bool MenusAndToolsConfig::SaveShortcutsSettings (const wxString & full_path) {
	// open
	wxTextFile file(full_path);
	if (file.Exists()) file.Open();
	else file.Create();
	file.Clear();
	// write
	int avoid_props = maSEPARATOR|maBEGIN_SUBMENU|maEND_SUBMENU|maDEFAULT_SHORTCUT;
	file.AddLine("[ShortCuts]");
	for(unsigned int i=0;i<mnCOUNT;i++) { 
		wxString menu_key = menues[i].key+".";
		myMenu &menu = menues[i];
		for(unsigned int j=0;j<menu.items.size();j++) {
			if (menu.items[j].properties&avoid_props) continue;
			if (menu.items[j].key.IsEmpty()) continue;
			file.AddLine(menu_key+menu.items[j].key+"="+menu.items[j].shortcut);
		}
	}
	// close
	file.Write();
	file.Close();
	return true;
}

void MenusAndToolsConfig::RecreateAllMenues ( ) {
	mapped_items.clear(); mapped_items.clear();
	TransferStatesFromConfig();
	CreateMenues();
	
}

