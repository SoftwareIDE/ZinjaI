/**
* @file mxMainWindowMenus
* @brief Contiene las implementaciones de los métodos que gestionan la creación y 
*        visualización de los menúes y barras de herramientas de la ventana principal
**/

#include "mxMainWindow.h"
#include "mxUtils.h"
#include "Language.h"
#include "ids.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxArt.h"

/**
* @brief Crea los menúes de la ventana principal y los configura en el modo inicial (no debug, no project)
**/
void mxMainWindow::CreateMenues() {
	
	menu.menu = new wxMenuBar;
	wxString ipre=DIR_PLUS_FILE("16","");
	
	menu.file = new wxMenu;
	PopulateMenuFile(ipre);
	menu.menu->Append(menu.file, LANG(MENUITEM_FILE,"&Archivo"));
	
	menu.edit = new wxMenu;
	PopulateMenuEdit(ipre);
	menu.menu->Append(menu.edit, LANG(MENUITEM_EDIT,"&Edicion"));
	
	menu.view = new wxMenu;
	PopulateMenuView(ipre);
#if defined(__APPLE__)
	menu.menu->Append(menu.view, LANG(MENUITEM_VIEW,"Ver"));
#else
	menu.menu->Append(menu.view, LANG(MENUITEM_VIEW,"&Ver"));
#endif
	
	menu.run = new wxMenu;
	PopulateMenuRun(ipre);
	menu.menu->Append(menu.run, LANG(MENUITEM_RUN,"E&jecucion"));
	
	menu.debug = new wxMenu;
	PopulateMenuDebug(ipre);
	menu.menu->Append(menu.debug, LANG(MENUITEM_DEBUG,"&Depuracion"));
	
	menu.tools = new wxMenu;
	PopulateMenuTools(ipre);
	menu.menu->Append(menu.tools, LANG(MENUITEM_TOOLS,"&Herramientas"));
	
	menu.help = new wxMenu;
	PopulateMenuHelp(ipre);
	menu.menu->Append(menu.help, LANG(MENUITEM_HELP,"A&yuda"));
	
	SetMenusForDebugging(false);
	SetMenusForProject(false);
	
	SetMenuBar(menu.menu);
}


void mxMainWindow::PopulateMenuFile(const wxString &ipre) {
	utils->AddItemToMenu(menu.file, wxID_NEW, LANG(MENUITEM_FILE_NEW,"&Nuevo..."),_T("Ctrl+N"),_T("Crear un nuevo archivo"),ipre+_T("nuevo.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PROJECT, LANG(MENUITEM_FILE_NEW_PROJECT,"&Nuevo Proyecto..."),_T("Ctrl+Shift+N"),_T("Crear un nuevo proyecto"),ipre+_T("proyecto.png"));
	menu.file_open=utils->AddItemToMenu(menu.file, wxID_OPEN, LANG(MENUITEM_FILE_OPEN,"&Abrir..."),_T("Ctrl+O"),_T("Abrir un archivo o un proyecto existente..."),ipre+_T("abrir.png"));
	utils->AddSubMenuToMenu(menu.file, menu.file_source_recent = new wxMenu,LANG(MENUITEM_FILE_RECENT_SOURCES,"Fuentes Abiertos Recientemente"),_T("Muestra los ultimos archivos abiertos como programas simples"),ipre+_T("recentSimple.png"));
	utils->AddSubMenuToMenu(menu.file, menu.file_project_recent = new wxMenu,LANG(MENUITEM_FILE_RECENT_PROJECTS,"Proyectos Abiertos Recientemente"),_T("Muestra los ultimos proyectos abiertos"),ipre+_T("recentProject.png"));
//	utils->AddItemToMenu(menu.file, mxID_FILE_OPEN_H, LANG(MENUITEM_FILE_OPEN_H,"Abrir &h/cpp complementario"),_T("F12"),_T("Si se esta editando un .h/.cpp, abrir el .cpp/.h homonimo."),ipre+_T("abrirp.png"));
//	utils->AddItemToMenu(menu.file, mxID_FILE_OPEN_SELECTED, LANG(MENUITEM_FILE_OPEN_SELECTED,"Abrir Se&leccionado"),_T("Ctrl+Enter"),_T("Abrir el archivo seleccionado en el codigo"),ipre+_T("abrirh.png"));
	utils->AddItemToMenu(menu.file, wxID_SAVE, LANG(MENUITEM_FILE_SAVE,"&Guardar"),_T("Ctrl+S"),_T("Guardar el archivo actual"),ipre+_T("guardar.png"));
	utils->AddItemToMenu(menu.file, wxID_SAVEAS, LANG(MENUITEM_FILE_SAVE_AS,"G&uardar Como..."),_T("Ctrl+Shift+S"),_T("Guardar el archivo actual con otro nombre..."),ipre+_T("guardarComo.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_SAVE_ALL, LANG(MENUITEM_FILE_SAVE_ALL,"Guardar &Todo..."),_T("Ctrl+Alt+Shift+S"),_T("Guarda todos los archivos abiertos y el proyecto actual..."),ipre+_T("guardarTodo.png"));
	menu.file_save_project = utils->AddItemToMenu(menu.file, mxID_FILE_SAVE_PROJECT, LANG(MENUITEM_FILE_SAVE_PROJECT,"Guar&dar Proyecto"),_T("Ctrl+Alt+S"),_T("Guardar la configuaricion actual del proyecto"),ipre+_T("guardarProyecto.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_EXPORT_HTML, LANG(MENUITEM_FILE_EXPORT_HTML,"Exportar a HTML..."),"",_T("Genera un archiv HTML con el codigo fuente"),ipre+_T("exportHtml.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PRINT, LANG(MENUITEM_FILE_PRINT,"&Imprimir..."),"",_T("Imprime el codigo fuente actual"),ipre+_T("imprimir.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_RELOAD, LANG(MENUITEM_FILE_RELOAD,"&Recargar"),_T("Ctrl+Shift+R"),_T("Recarga la version en disco del archivo actual."),ipre+_T("recargar.png"));
	utils->AddItemToMenu(menu.file, wxID_CLOSE, LANG(MENUITEM_FILE_CLOSE,"&Cerrar"),_T("Ctrl+W"),_T("Cerrar el archivo actual"),ipre+_T("cerrar.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_CLOSE_ALL, LANG(MENUITEM_FILE_CLOSE_ALL,"Cerrar &Todo"),_T("Ctrl+Alt+Shift+W"),_T("Cierra todos los archivos abiertos"),ipre+_T("cerrarTodo.png"));
	menu.file_close_project = utils->AddItemToMenu(menu.file, mxID_FILE_CLOSE_PROJECT, LANG(MENUITEM_FILE_CLOSE_PROJECT,"Cerrar Pro&yecto"),_T("Ctrl+Shift+W"),_T("Cierra el proyecto actual"),ipre+_T("cerrarProyecto.png"));
	menu.file->AppendSeparator();
	menu.file_project_config = utils->AddItemToMenu(menu.file, mxID_FILE_PROJECT_CONFIG, LANG(MENUITEM_FILE_PROJECT_CONFIG,"&Configuracion del Proyecto..."),_T("Ctrl+Shift+P"),_T("Configurar las propiedades generales de un proyecto..."),ipre+_T("projectConfig.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PREFERENCES, LANG(MENUITEM_FILE_PREFERENCES,"&Preferencias..."),_T("Ctrl+P"),_T("Configurar el entorno..."),ipre+_T("preferencias.png"));
	menu.file->AppendSeparator();
	utils->AddItemToMenu(menu.file, wxID_EXIT, LANG(MENUITEM_FILE_EXIT,"&Salir"),_T("Alt+F4"),_T("Salir del programa!"),ipre+_T("salir.png"));
	menu.file_source_history=new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (i<config->Init.history_len && config->Files.last_source[i][0]!=0)
			menu.file_source_history[i] = utils->AddItemToMenu(menu.file_source_recent, mxID_FILE_SOURCE_HISTORY_0+i,config->Files.last_source[i],"",config->Files.last_source[i],wxString(ipre+_T("recent"))<<i<<(_T(".png")));
		else
			menu.file_source_history[i] = NULL;
	}
	menu.file_source_recent->AppendSeparator();
	utils->AddItemToMenu(menu.file_source_recent, mxID_FILE_SOURCE_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas..."),"",_T("Muestra un dialogo con la lista completa de archivos recientes"),ipre+_T("recentMore.png"));
	menu.file_project_history=new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (i<config->Init.history_len && config->Files.last_project[i][0]!=0)
			menu.file_project_history[i] = utils->AddItemToMenu(menu.file_project_recent, mxID_FILE_PROJECT_HISTORY_0+i, config->Files.last_project[i],"",config->Files.last_project[i],wxString(ipre+_T("recent"))<<i<<(_T(".png")));
		else
			menu.file_project_history[i]=NULL;
	}
	menu.file_project_recent->AppendSeparator();
	utils->AddItemToMenu(menu.file_project_recent, mxID_FILE_PROJECT_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas..."),"",_T("Muestra un dialogo con la lista completa de archivos recientes"),ipre+_T("recentMore.png"));
}

void mxMainWindow::PopulateMenuEdit(const wxString &ipre) {
	utils->AddItemToMenu(menu.edit, wxID_UNDO, LANG(MENUITEM_EDIT_UNDO,"&Deshacer"),_T("Ctrl+Z"),_T("Deshacer el ultimo cambio"),ipre+_T("deshacer.png"));
	utils->AddItemToMenu(menu.edit, wxID_REDO, LANG(MENUITEM_EDIT_REDO,"&Rehacer"),_T("Ctrl+Shift+Z"),_T("Rehacer el ultimo cambio desecho"),ipre+_T("rehacer.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, wxID_CUT, LANG(MENUITEM_EDIT_CUT,"C&ortar"),_T("Ctrl+X"),_T("Cortar la seleccion al portapapeles"),ipre+_T("cortar.png"));
	utils->AddItemToMenu(menu.edit, wxID_COPY, LANG(MENUITEM_EDIT_COPY,"&Copiar"),_T("Ctrl+C"),_T("Copiar la seleccion al portapapeles"),ipre+_T("copiar.png"));
	utils->AddItemToMenu(menu.edit, wxID_PASTE, LANG(MENUITEM_EDIT_PASTE,"&Pegar"),_T("Ctrl+V"),_T("Pegar el contenido del portapapeles"),ipre+_T("pegar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_TOGGLE_LINES_UP, LANG(MENUITEM_EDIT_LINES_UP,"Mover Hacia Arriba"),_T("Ctrl+T"),_T("Mueve la o las lineas seleccionadas hacia arriba"),ipre+_T("toggleLinesUp.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_TOGGLE_LINES_DOWN, LANG(MENUITEM_EDIT_LINES_DOWN,"Mover Hacia Abajo"),_T("Ctrl+Shift+T"),_T("Mueve la o las lineas seleccionadas hacia abajo"),ipre+_T("toggleLinesDown.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_DUPLICATE_LINES, LANG(MENUITEM_EDIT_DUPLICATE_LINES,"&Duplicar Linea(s)"),_T("Ctrl+L"),_T("Copia la linea actual del cursor, o las lineas seleccionadas, nuevamente a continuacion"),ipre+_T("duplicarLineas.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_DELETE_LINES, LANG(MENUITEM_EDIT_DELETE_LINES,"&Eliminar Linea(s)"),_T("Shift+Ctrl+L"),_T("Elimina la linea actual del cursor nuevamente, o las lineas seleccionadas"),ipre+_T("borrarLineas.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_FUNCTION, LANG(MENUITEM_EDIT_GOTO_FUNCTION,"&Ir a Funcion/Clase/Metodo..."),_T("Ctrl+Shift+G"),_T("Abrir el fuente con la declaracion de una funcion, clase o metodo..."),ipre+_T("irAFuncion.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_GOTO_FILE,"&Ir a Archivo..."),_T("Ctrl+Shift+F"),_T("Abrir un archivo en particular buscandolo por parte de su nombre..."),ipre+_T("irAArchivo.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO, LANG(MENUITEM_EDIT_GOTO_LINE,"&Ir a Linea..."),_T("Ctrl+G"),_T("Mover el cursor a una linea determinada en el archivo..."),ipre+_T("irALinea.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND, LANG(MENUITEM_EDIT_FIND,"&Buscar..."),_T("Ctrl+F"),_T("Buscar una cadena en el archivo..."),ipre+_T("buscar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND_PREV, LANG(MENUITEM_EDIT_FIND_PREV,"Buscar &Anterior"),_T("Shift+F3"),_T("Repetir la ultima busqueda a partir del cursor hacia atras"),ipre+_T("buscarAnterior.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND_NEXT, LANG(MENUITEM_EDIT_FIND_NEXT,"Buscar &Siguiente"),_T("F3"),_T("Repetir la ultima busqueda a partir del cursor"),ipre+_T("buscarSiguiente.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_REPLACE, LANG(MENUITEM_EDIT_REPLACE,"&Reemplazar..."),_T("Ctrl+R"),_T("Reemplazar una cadena con otra en el archivo..."),ipre+_T("reemplazar.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, mxID_EDIT_INSERT_HEADER, LANG(MENUITEM_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),_T("Ctrl+H"),_T("Si es posible, inserta el #include necesario para utilizar la funcion/clase en la cual se encuentra el cursor."),ipre+_T("insertarInclude.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_COMMENT, LANG(MENUITEM_EDIT_COMMENT,"Comentar"),_T("Ctrl+D"),_T("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea"),ipre+_T("comentar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_UNCOMMENT, LANG(MENUITEM_EDIT_UNCOMMENT,"Descomentar"),_T("Shift+Ctrl+D"),_T("Descomente el texto seleccionado eliminando \"//\" de cada linea"),ipre+_T("descomentar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_INDENT, LANG(MENUITEM_EDIT_INDENT,"Indentar Blo&que"),_T("Ctrl+I"),_T("Corrige el indentado de un bloque de codigo agregando o quitando tabs segun corresponda"),ipre+_T("indent.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_BRACEMATCH, LANG(MENUITEM_EDIT_BRACEMATCH,"Seleccionar Blo&que"),_T("Ctrl+M"),_T("Seleccionar todo el bloque correspondiente a la llave o parentesis sobre el cursor"),ipre+_T("mostrarLlave.png"));
	utils->AddItemToMenu(menu.edit, wxID_SELECTALL, LANG(MENUITEM_EDIT_SELECT_ALL,"&Seleccionar Todo"),_T("Ctrl+A"),_T("Seleccionar todo el contenido del archivo"),ipre+_T("seleccionarTodo.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_MARK_LINES, LANG(MENUITEM_EDIT_HIGHLIGHT_LINES,"&Resaltar Linea(s)/Quitar Resaltado"),_T("Ctrl+B"),_T("Resalta la linea pintandola de otro color"),ipre+_T("marcar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_MARK, LANG(MENUITEM_EDIT_FIND_HIGHLIGHTS,"Buscar &Resaltado"),_T("Ctrl+Shift+B"),_T("Mueve el cursor a la siguiente linea resaltada"),ipre+_T("irAMarca.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_LIST_MARKS, LANG(MENUITEM_EDIT_LIST_HIGHLIGHTS,"&Listar Lineas Resaltadas"),_T("Ctrl+Alt+B"),_T("Muestra una lista de las lineas marcadas en todos los archivos"),ipre+_T("listarMarcas.png"));
//	utils->AddItemToMenu(menu.edit, mxID_EDIT_FORCE_AUTOCOMPLETE, LANG(MENUITEM_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),_T("Ctrl+Space"),_T("Muestra el menu emergente de autocompletado"),ipre+_T("autocompletar.png"));
	//#ifdef DEBUG
//	utils->AddItemToMenu(menu.edit, mxID_EDIT_FUZZY_AUTOCOMPLETE, _T("Autocompletar extendido"),_T("Ctrl+alt+Space"),_T("Muestra el menu emergente de autocompletado"),ipre+_T("autocompletar.png"));
	//#endif
}

void mxMainWindow::PopulateMenuView(const wxString &ipre) {
	wxMenu *fold_menu, *toolbars_menu;
	utils->AddItemToMenu(menu.view, mxID_VIEW_DUPLICATE_TAB, LANG(MENUITEM_VIEW_SPLIT_VIEW,"&Duplicar vista"),"","",ipre+_T("duplicarVista.png"));
	menu.view_line_wrap = utils->AddCheckToMenu(menu.view, mxID_VIEW_LINE_WRAP, LANG(MENUITEM_VIEW_LINE_WRAP,"&Ajuste de linea"),_T("Alt+F11"),_T("Muestra las lineas largas como en varios renglones"), false);	
	menu.view_white_space = utils->AddCheckToMenu(menu.view, mxID_VIEW_WHITE_SPACE, LANG(MENUITEM_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de &fin de linea"),"",_T("Muestra las lineas largas como en varios renglones"), false);	
	menu.view_code_style = utils->AddCheckToMenu(menu.view, mxID_VIEW_CODE_STYLE, LANG(MENUITEM_VIEW_SYNTAX_HIGHLIGHT,"&Colorear Sintaxis"),_T("Shift+F11"),_T("Resalta el codigo con diferentes colores y formatos de fuente."), false);	
	utils->AddItemToMenu(menu.view, mxID_VIEW_CODE_COLOURS, LANG(MENUITEM_VIEW_CODE_COLOURS,"Configurar esquema de colores..."),"","",ipre+_T("preferencias.png"));
	utils->AddSubMenuToMenu(menu.view, fold_menu = new wxMenu,LANG(MENUITEM_VIEW_FOLDING,"Plegado"),_T("Muestra opciones para plegar y desplegar codigo en distintos niveles"),ipre+_T("folding.png"));
	menu.view->AppendSeparator();
	menu.view_fullscreen = utils->AddCheckToMenu(menu.view, mxID_VIEW_FULLSCREEN, LANG(MENUITEM_VIEW_FULLSCREEN,"Ver a Pantalla &Completa"),_T("F11"),_T("Muestra el editor a pantalla completa, ocultando tambien los demas paneles"), false);
	menu.view_beginner_panel = utils->AddCheckToMenu(menu.view, mxID_VIEW_BEGINNER_PANEL, LANG(MENUITEM_VIEW_BEGINNER_PANEL,"Mostrar Panel de Mini-Plantillas"),"",_T("Muestra un panel con plantillas y estructuras basicas de c++"), false);
	if (config->Init.left_panels)
		menu.view_left_panels = utils->AddCheckToMenu(menu.view, mxID_VIEW_LEFT_PANELS, LANG(MENUITEM_VIEW_LEFT_PANELS,"&Mostrar Panel de Arboles"),"",_T("Muestra el panel con los arboles de proyecto, simbolos y explorador de archivos"), false);
	project_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_PROJECT_TREE, LANG(MENUITEM_VIEW_PROJECT_TREE,"&Mostrar Arbol de &Proyecto"),"",_T("Muestra el panel del arbol de proyecto/archivos abiertos"), false);
	explorer_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_EXPLORER_TREE, LANG(MENUITEM_VIEW_EXPLORER_TREE,"Mostrar &Explorardor de Archivos"),_T("Ctrl+E"),_T("Muestra el panel explorador de archivos"), false);
	symbols_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_SYMBOLS_TREE, LANG(MENUITEM_VIEW_SYMBOLS_TREE,"Mostrar Arbol de &Simbolos"),"",_T("Analiza el codigo fuente y construye un arbol con los simbolos declarados en el mismo."), false);
	compiler_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_COMPILER_TREE, LANG(MENUITEM_VIEW_COMPILER_TREE,"&Mostrar Resultados de la Compilacion"),"",_T("Muestra un panel con la salida del compilador"), false);
//	utils->AddItemToMenu(menu.view, mxID_VIEW_UPDATE_SYMBOLS, LANG(MENUITEM_VIEW_UPDATE_SYMBOLS,"&Actualizar Arbol de Simbolos"),_T("F2"),_T("Actualiza el arbol de simbolos."),ipre+_T("simbolos.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_HIDE_BOTTOM, _T("&Ocultar paneles inferiores"),_T("Escape"),_T("Oculta los paneles de informacion de compilacion y ayuda rapida."),ipre+_T("hideBottom.png"));
	utils->AddSubMenuToMenu(menu.view, toolbars_menu = new wxMenu,LANG(MENUITEM_VIEW_TOOLBARS,"Barras de herramientas"),"","");
	menu.view_toolbar_file = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_FILE, LANG(MENUITEM_VIEW_TOOLBAR_FILE,"&Mostrar Barra de Herramientas Archivo"),"",_T("Muestra la barra de herramientas para el manejo de archivos"), config->Toolbars.positions.file.visible);
	menu.view_toolbar_edit = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_EDIT, LANG(MENUITEM_VIEW_TOOLBAR_EDIT,"&Mostrar Barra de Herramientas Edicion"),"",_T("Muestra la barra de herramientas para la edicion del fuente"), config->Toolbars.positions.edit.visible);
	menu.view_toolbar_view = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_VIEW, LANG(MENUITEM_VIEW_TOOLBAR_VIEW,"&Mostrar Barra de Herramientas Ver"),"",_T("Muestra la barra de herramientas para las opciones de visualizacion"), config->Toolbars.positions.view.visible);
	menu.view_toolbar_find = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_FIND, LANG(MENUITEM_VIEW_TOOLBAR_FIND,"&Mostrar Barra de Busqueda Rapida"),"",_T("Muestra un cuadro de texto en la barra de herramientas que permite buscar rapidamente en un fuente"), config->Toolbars.positions.find.visible);
	menu.view_toolbar_run = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_RUN, LANG(MENUITEM_VIEW_TOOLBAR_RUN,"&Mostrar Barra de Herramientas Ejecucion"),"",_T("Muestra la barra de herramientas para la compilacion y ejecucion del programa"), config->Toolbars.positions.run.visible);
	menu.view_toolbar_tools = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_TOOLS, LANG(MENUITEM_VIEW_TOOLBAR_TOOLS,"&Mostrar Barra de Herramientas Herramientas"),"",_T("Muestra la barra de herramientas para las herramientas adicionales"), config->Toolbars.positions.tools.visible);
	menu.view_toolbar_project = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_PROJECT, LANG(MENUITEM_VIEW_TOOLBAR_PROJECT,"&Mostrar Barra de Herramientas Proyecto"),"",_T("Muestra la barra de herramientas para las herramientas personalizables propias del proyecto"), config->Toolbars.positions.project.visible);
	menu.view_toolbar_debug = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_DEBUG, LANG(MENUITEM_VIEW_TOOLBAR_DEBUG,"&Mostrar Barra de Herramientas Depuracion"),"",_T("Muestra la barra de herramientas para la depuracion del programa"), config->Toolbars.positions.debug.visible);
	menu.view_toolbar_misc = utils->AddCheckToMenu(toolbars_menu, mxID_VIEW_TOOLBAR_MISC, LANG(MENUITEM_VIEW_TOOLBAR_MISC,"&Mostrar Barra de Herramientas Miscelanea"),"",_T("Muestra la barra de herramientas con commandos miselaneos"), config->Toolbars.positions.misc.visible);
	utils->AddItemToMenu(toolbars_menu, mxID_VIEW_TOOLBARS_CONFIG, LANG(MENUITEM_VIEW_TOOLBARS_CONFIG,"&Configurar..."),"","",ipre+_T("preferencias.png"));
	menu.view->AppendSeparator();
#if !defined(_WIN32) && !defined(__WIN32__)
	utils->AddItemToMenu(menu.view, mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior"),_T("Ctrl+>"),_T("Selecciona el error/advertencia anterior de la salida del compilador."),ipre+_T("errorPrev.png"));
#else
	utils->AddItemToMenu(menu.view, mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior"),_T("Ctrl+Shift+<"),_T("Selecciona el error/advertencia anterior de la salida del compilador."),ipre+_T("errorPrev.png"));
#endif
	utils->AddItemToMenu(menu.view, mxID_VIEW_NEXT_ERROR, LANG(MENUITEM_VIEW_NEXT_ERROR,"&Ir a siguiente error"),_T("Ctrl+<"),_T("Selecciona el proximo error/advertencia de la salida del compilador."),ipre+_T("errorNext.png"));
//	menu.view->AppendSeparator();
#if !defined(_WIN32) && !defined(__WIN32__)
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_PREV, LANG(MENUITEM_VIEW_PREV_SOURCE,"&Fuente anterior"),_T("Ctrl+PageUp"),_T("Selecciona la pestana del fuente anterior al actual en el area de codigo."),ipre+_T("notebookPrev.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_NEXT, LANG(MENUITEM_VIEW_NEXT_SOURCE,"&Fuente siguiente"),_T("Ctrl+PageDown"),_T("Selecciona la pestana del fuente siguiente al actual en el area de codigo."),ipre+_T("notebookNext.png"));
#else
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_PREV, LANG(MENUITEM_VIEW_PREV_SOURCE,"&Fuente anterior"),_T("Ctrl+Shift+Tab"),_T("Selecciona la pestana del fuente anterior al actual en el area de codigo."),ipre+_T("notebookPrev.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_NEXT, LANG(MENUITEM_VIEW_NEXT_SOURCE,"&Fuente siguiente"),_T("Ctrl+Tab"),_T("Selecciona la pestana del fuente siguiente al actual en el area de codigo."),ipre+_T("notebookNext.png"));
#endif
	
#ifdef __APPLE__
	utils->AddItemToMenu(fold_menu, mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea"),"","",ipre+_T("foldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea"),"","",ipre+_T("unfoldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel"),"",_T("Cierra todos los bolques del primer nivel"),ipre+_T("fold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel"),"",_T("Cierra todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel"),"",_T("Cierra todos los bolques del tercer nivel"),ipre+_T("fold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel"),"",_T("Cierra todos los bolques del cuarto nivel"),ipre+_T("fold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel"),"",_T("Cierra todos los bolques del quinto nivel"),ipre+_T("fold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles"),"",_T("Cierra todos los bolques de todos los niveles"),ipre+_T("foldAll.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel"),"",_T("Abre todos los bolques del primer nivel"),ipre+_T("unfold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel"),"",_T("Abre todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel"),"",_T("Abre todos los bolques del tercer nivel"),ipre+_T("unfold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel"),"",_T("Abre todos los bolques del cuarto nivel"),ipre+_T("unfold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel"),"",_T("Abre todos los bolques del quinto nivel"),ipre+_T("unfold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles"),"",_T("Abre todos los bolques de todos los niveles"),ipre+_T("unfoldAll.png"));
#else
	utils->AddItemToMenu(fold_menu, mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea"),_T("Alt+Up"),"",ipre+_T("foldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea"),_T("Alt+Down"),"",ipre+_T("unfoldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel"),_T("Ctrl+1"),_T("Cierra todos los bolques del primer nivel"),ipre+_T("fold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel"),_T("Ctrl+2"),_T("Cierra todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel"),_T("Ctrl+3"),_T("Cierra todos los bolques del tercer nivel"),ipre+_T("fold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel"),_T("Ctrl+4"),_T("Cierra todos los bolques del cuarto nivel"),ipre+_T("fold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel"),_T("Ctrl+5"),_T("Cierra todos los bolques del quinto nivel"),ipre+_T("fold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles"),_T("Ctrl+0"),_T("Cierra todos los bolques de todos los niveles"),ipre+_T("foldAll.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel"),_T("Alt+1"),_T("Abre todos los bolques del primer nivel"),ipre+_T("unfold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel"),_T("Alt+2"),_T("Abre todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel"),_T("Alt+3"),_T("Abre todos los bolques del tercer nivel"),ipre+_T("unfold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel"),_T("Alt+4"),_T("Abre todos los bolques del cuarto nivel"),ipre+_T("unfold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel"),_T("Alt+5"),_T("Abre todos los bolques del quinto nivel"),ipre+_T("unfold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles"),_T("Alt+0"),_T("Abre todos los bolques de todos los niveles"),ipre+_T("unfoldAll.png"));
#endif
}

void mxMainWindow::PopulateMenuRun(const wxString &ipre) {
	menu.run_run = utils->AddItemToMenu(menu.run, mxID_RUN_RUN, LANG(MENUITEM_RUN_RUN,"&Ejecutar..."),_T("F9"),_T("Guarda y compila si es necesario, luego ejecuta el programa"),ipre+_T("ejecutar.png"));
	utils->AddItemToMenu(menu.run, mxID_RUN_RUN_OLD, LANG(MENUITEM_RUN_OLD,"Ejecutar (sin recompilar)..."),_T("Ctrl+F9"),_T("Ejecuta el binario existente sin recompilar primero"),ipre+_T("ejecutar_old.png"));
	menu.run_compile = utils->AddItemToMenu(menu.run, mxID_RUN_COMPILE, LANG(MENUITEM_RUN_COMPILE,"&Compilar"),_T("Shift+F9"),_T("Guarda y compila el fuente actual"),ipre+_T("compilar.png"));
//	menu.run_build = utils->AddItemToMenu(menu.run, mxID_RUN_BUILD, LANG(MENUITEM_RUN_BUILD,"Co&nstruir"),_T("Ctrl+F9"),_T("Guarda y compila todo el proyecto"),ipre+_T("construir.png")));
	menu.run_clean = utils->AddItemToMenu(menu.run, mxID_RUN_CLEAN, LANG(MENUITEM_RUN_CLEAN,"&Limpiar"),_T("Ctrl+Shift+F9"),_T("Elimina los objetos y ejecutables compilados"),ipre+_T("limpiar.png"));
	menu.run_stop = utils->AddItemToMenu(menu.run, mxID_RUN_STOP, LANG(MENUITEM_RUN_STOP,"&Detener"),"",_T("Detiene la ejecucion del programa"),ipre+_T("detener.png"));
	utils->AddItemToMenu(menu.run, mxID_RUN_CONFIG, LANG(MENUITEM_RUN_OPTIONS,"&Opciones..."),_T("Alt+F9"),_T("Configura la compilacion y ejecucion de los programas"),ipre+_T("opciones.png"));
}

void mxMainWindow::PopulateMenuDebug(const wxString &ipre) {
	menu.debug_run = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RUN, LANG(MENUITEM_DEBUG_START,"&Iniciar/Continuar"),_T("F5"),"",ipre+_T("depurar.png"));
	menu.debug_pause = utils->AddItemToMenu(menu.debug, mxID_DEBUG_PAUSE, LANG(MENUITEM_DEBUG_PAUSE,"Interrum&pir"),"","",ipre+_T("pausar.png"));
	menu.debug_stop = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STOP, LANG(MENUITEM_DEBUG_STOP,"&Detener"),_T("Shift+F5"),"",ipre+_T("detener.png"));
	menu.debug_step_in = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_IN, LANG(MENUITEM_DEBUG_STEP_IN,"Step &In"),_T("F6"),"",ipre+_T("step_in.png"));
	menu.debug_step_over = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_OVER, LANG(MENUITEM_DEBUG_STEP_OVER,"Step &Over"),_T("F7"),"",ipre+_T("step_over.png"));
	menu.debug_step_out = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_OUT, LANG(MENUITEM_DEBUG_STEP_OUT,"Step O&ut"),_T("Shift+F6"),"",ipre+_T("step_out.png"));
	menu.debug_run_until = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RUN_UNTIL, LANG(MENUITEM_DEBUG_RUN_UNTIL,"Ejecutar &Hasta el Cursor"),_T("Shift+F7"),"",ipre+_T("run_until.png"));
	menu.debug_return = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RETURN, LANG(MENUITEM_DEBUG_RETURN,"&Return"),_T("Ctrl+F6"),"",ipre+_T("return.png"));
	menu.debug_jump = utils->AddItemToMenu(menu.debug, mxID_DEBUG_JUMP, LANG(MENUITEM_DEBUG_JUMP,"Continuar Desde Aqui"),_T("Ctrl+F5"),"",ipre+_T("debug_jump.png"));
	menu.debug->AppendSeparator();
	menu.debug_toggle_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_TOGGLE_BREAKPOINT, LANG(MENUITEM_DEBUG_TOGGLE_BREAKPOINT,"&Agregar/quitar Breakpoint"),_T("F8"),"",ipre+_T("breakpoint.png"));
	menu.debug_toggle_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_BREAKPOINT_OPTIONS, LANG(MENUITEM_DEBUG_BREAKPOINT_OPTIONS,"&Opciones del Breakpoint..."),_T("Ctrl+F8"),"",ipre+_T("breakpoint_options.png"));
	menu.debug_list_breakpoints = utils->AddItemToMenu(menu.debug, mxID_DEBUG_LIST_BREAKPOINTS, LANG(MENUITEM_DEBUG_LIST_BREAKPOINTS,"&Listar Watch/Break points..."),_T("Shift+F8"),"",ipre+_T("breakpoint_list.png"));
	menu.debug->AppendSeparator();
	menu.debug_inspect = utils->AddItemToMenu(menu.debug, mxID_DEBUG_INSPECT, LANG(MENUITEM_DEBUG_INSPECT,"Panel de In&specciones"),"","",ipre+_T("inspect.png"));
	menu.debug_backtrace = utils->AddItemToMenu(menu.debug, mxID_DEBUG_BACKTRACE, LANG(MENUITEM_DEBUG_BACKTRACE,"&Trazado Inverso"),"","",ipre+_T("backtrace.png"));
	menu.debug_threadlist = utils->AddItemToMenu(menu.debug, mxID_DEBUG_THREADLIST, LANG(MENUITEM_DEBUG_THREADLIST,"&Hilos de Ejecucion"),"","",ipre+_T("threadlist.png"));
	utils->AddItemToMenu(menu.debug, mxID_DEBUG_LOG_PANEL, LANG(MENUITEM_DEBUG_SHOW_LOG_PANEL,"&Mostrar mensajes del depurador"),"","",ipre+_T("debug_log_panel.png"));
	menu.debug->AppendSeparator();
	
#ifndef __WIN32__
	wxMenu *more_menu = new wxMenu;
	menu.debug_attach = utils->AddItemToMenu(more_menu, mxID_DEBUG_ATTACH, LANG(MENUITEM_DEBUG_ATTACH,"&Adjuntar..."),"","",ipre+_T("debug_attach.png"));
	menu.debug_load_core_dump = utils->AddItemToMenu(more_menu, mxID_DEBUG_CORE_DUMP, LANG(MENUITEM_DEBUG_LOAD_CORE_DUMP,"Cargar &Volcado de Memoria..."),"","",ipre+_T("core_dump.png"));
	menu.debug_generate_core_dump = utils->AddItemToMenu(more_menu, mxID_DEBUG_CORE_DUMP,LANG(MENUITEM_SAVE_CORE_DUMP,"Guardar &Volcado de Memoria..."),"","",ipre+_T("core_dump.png"));
	menu.debug_enable_inverse_execution = utils->AddCheckToMenu(more_menu, mxID_DEBUG_ENABLE_INVERSE_EXEC, LANG(MENUITEM_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras"),"","",false);
	menu.debug_inverse_execution = utils->AddCheckToMenu(more_menu, mxID_DEBUG_INVERSE_EXEC, LANG(MENUITEM_DEBUG_INVERSE,"Ejecutar Hacia Atras"),"","",false);
	menu.debug_set_signals = utils->AddItemToMenu(more_menu, mxID_DEBUG_SET_SIGNALS, LANG(MENUITEM_DEBUG_SET_SIGNALS,"Configurar comportamiento ante señales..."),"","",ipre+_T("debug_singals_set.png"));
	menu.debug_send_signal = utils->AddItemToMenu(more_menu, mxID_DEBUG_SEND_SIGNAL, LANG(MENUITEM_DEBUG_SEND_SIGNALS,"Enviar señal..."),"","",ipre+_T("debug_singal_send.png"));
	menu.debug_gdb_command = utils->AddItemToMenu(more_menu, mxID_DEBUG_GDB_COMMAND, LANG(MENUITEM_DEBUG_GDB_COMMAND,"Introducir comandos gdb..."),"","",ipre+_T("gdb_command.png"));
	menu.debug_patch = utils->AddItemToMenu(more_menu, mxID_DEBUG_PATCH, LANG(MENUITEM_DEBUG_PATCH,"Actualizar ejecutable..."),"","",ipre+_T("debug_patch.png"));
	utils->AddSubMenuToMenu(menu.debug, more_menu,LANG(MENUITEM_DEBUG_MORE,"Más..."),"",ipre+"share.png");
#endif
	if (config->Debug.show_do_that) menu.debug_function_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_DO_THAT, LANG(MENUITEM_DEBUG_DO_THAT,"DO_THAT"),"","",ipre+_T("do_that.png"));
}


void mxMainWindow::PopulateMenuTools(const wxString &ipre) { 
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_FLOW, LANG(MENUITEM_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de &Flujo..."),"",_T("Genera un diagrama de flujo a partir del bloque de codigo actual"),ipre+_T("flujo.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_CLASSES, LANG(MENUITEM_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar &Jerarquia de Clases..."),"","",ipre+_T("clases.png"));
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_CODE_COPY_FROM_H, LANG(MENUITEM_TOOLS_CODE_COPY_FROM_H,"Implementar Métodos/Funciones faltantes..."),_T("Ctrl+Shift+H"),"",ipre+_T("copy_code_from_h.png"));
	
	wxMenu *comments_menu = new wxMenu;
	utils->AddItemToMenu(comments_menu, mxID_TOOLS_ALIGN_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_ALIGN_COMMENTS,"&Alinear Comentarios..."),"",_T("Mueve todos los comentarios hacia una determinada columna"),ipre+_T("align_comments.png"));
	utils->AddItemToMenu(comments_menu, mxID_EDIT_COMMENT, LANG(MENUITEM_TOOLS_COMMENTS_COMMENT,"&Comentar"),"",_T("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea"),ipre+_T("comentar.png"));
	utils->AddItemToMenu(comments_menu, mxID_EDIT_UNCOMMENT, LANG(MENUITEM_TOOLS_COMMENTS_UNCOMMENT,"&Descomentar"),"",_T("Descomente el texto seleccionado eliminando \"//\" de cada linea"),ipre+_T("descomentar.png"));
	utils->AddItemToMenu(comments_menu, mxID_TOOLS_REMOVE_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_DELETE_COMMENTS,"&Eliminar Comentarios"),"",_T("Quita todos los comentarios del codigo fuente o de la seleccion"),ipre+_T("remove_comments.png"));
	utils->AddSubMenuToMenu(menu.tools, comments_menu,LANG(MENUITEM_TOOLS_COMMENTS,"Coment&arios"),_T("Permite alinear o quitar los comentarios del codigo"),ipre+_T("comments.png"));
	
	wxMenu *preproc_menu = new wxMenu;
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_EXPAND_MACROS, LANG(MENUITEM_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros"),_T("Ctrl+Shift+M"),"",ipre+_T("preproc_expand_macros.png"));
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_MARK_VALID, LANG(MENUITEM_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas"),_T("Ctrl+Alt+M"),"",ipre+_T("preproc_mark_valid.png"));
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_UNMARK_ALL, LANG(MENUITEM_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas"),_T("Ctrl+Alt+Shift+M"),"",ipre+_T("preproc_unmark_all.png"));
	preproc_menu->AppendSeparator();
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_HELP, LANG(MENUITEM_TOOLS_PREPROC_HELP,"Ayuda..."),"","",ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, preproc_menu,LANG(MENUITEM_TOOLS_PREPROC,"Preprocesador"),_T("Muestra información generada por el preprocesador de C++"),ipre+_T("preproc.png"));
	
	menu.tools->AppendSeparator();
	
	menu.tools_makefile = utils->AddItemToMenu(menu.tools, mxID_TOOLS_MAKEFILE, LANG(MENUITEM_TOOLS_GENERATE_MAKEFILE,"&Generar Makefile..."),"",_T("Genera el Makefile a partir de los fuentes y la configuracion seleccionada"),ipre+_T("makefile.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_CONSOLE, LANG(MENUITEM_TOOLS_OPEN_TERMINAL,"Abrir Co&nsola..."),"",_T("Inicia una terminal para interactuar con el interprete de comandos del sistema operativo"),ipre+_T("console.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_EXE_PROPS, LANG(MENUITEM_TOOLS_EXE_INFO,"&Propiedades del Ejecutable..."),"",_T("Muestra informacion sobre el archivo compilado"),ipre+_T("exeinfo.png"));
	menu.tools_stats = utils->AddItemToMenu(menu.tools, mxID_TOOLS_PROJECT_STATISTICS, LANG(MENUITEM_TOOLS_PROJECT_STATISTICS,"E&stadisticas del Proyecto..."),"",_T("Muestra informacion estadistica sobre los fuentes y demas archivos del proyecto"),ipre+_T("proystats.png"));
	menu.tools_proy_graph = utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_PROJECT, LANG(MENUITEM_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),"","",ipre+_T("draw_project.png"));
	
	wxMenu *share_menu = new wxMenu;
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_OPEN, LANG(MENUITEM_TOOLS_SHARE_OPEN,"&Abrir compartido..."),"",_T("Abre un archivo compartido por otra PC en la red local."),ipre+_T("abrirs.png"));
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_SHARE, LANG(MENUITEM_TOOLS_SHARE_SHARE,"&Compartir actual..."),"",_T("Comparte el archivo en la red local."),ipre+_T("compartir.png"));
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_LIST, LANG(MENUITEM_TOOLS_SHARE_LIST,"&Ver lista de compartidos propios..."),"",_T("Comparte el archivo en la red local."),ipre+_T("share_list.png"));
	share_menu->AppendSeparator();
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_HELP, LANG(MENUITEM_TOOLS_SHARE_HELP,"A&yuda..."),"",_T("Muestra ayuda acerca de la comparticion de archivos en ZinjaI"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, share_menu,LANG(MENUITEM_TOOLS_SHARE,"Compartir Archivos en la &Red Local"),_T("Permite enviar o recibir codigos fuentes a traves de una red LAN"),ipre+_T("share.png"));
	
	menu.tools->AppendSeparator();
	
	wxMenu *diff_menu = new wxMenu;
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_TWO, LANG(MENUITEM_TOOLS_DIFF_TWO,"&Dos fuentes abiertos..."),"",_T("Compara dos archivos de texto abiertos y los colorea segun sus diferencias"),ipre+_T("diff_sources.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_DISK, LANG(MENUITEM_TOOLS_DIFF_DISK,"&Fuente actual contra archivo en disco..."),"",_T("Compara un archivo abierto contra un archivo en disco y lo colorea segun sus diferencias"),ipre+_T("diff_source_file.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_HIMSELF, LANG(MENUITEM_TOOLS_DIFF_HIMSELF,"&Cambios en fuente actual contra su version en disco..."),"",_T("Compara un archivos abierto y modificado contra su version en disco y lo colorea segun sus diferencias"),ipre+_T("diff_source_himself.png"));
	diff_menu->AppendSeparator();
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_PREV, LANG(MENUITEM_TOOLS_DIFF_PREV,"Ir a Diferencia Anterior"),_T("Shift+Alt+PageUp"),"",ipre+_T("diff_prev.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_NEXT, LANG(MENUITEM_TOOLS_DIFF_NEXT,"Ir a Siguiente Diferencia"),_T("Shift+Alt+PageDown"),"",ipre+_T("diff_next.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_SHOW, LANG(MENUITEM_TOOLS_DIFF_SHOW,"Mostrar Cambio"),_T("Alt+Shift+Ins"),_T("Muestra en un globo emergente el cambio a aplicar"),ipre+_T("diff_show.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_APPLY, LANG(MENUITEM_TOOLS_DIFF_APPLY,"Apl&icar Cambio"),_T("Alt+Ins"),_T("Aplica el cambio marcado en la linea actual"),ipre+_T("diff_apply.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_DISCARD, LANG(MENUITEM_TOOLS_DIFF_DISCARD,"De&scartar Cambio"),_T("Alt+Del"),_T("Descarta el cambio marcado en la linea actual"),ipre+_T("diff_discard.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_CLEAR, LANG(MENUITEM_TOOLS_DIFF_CLEAR,"&Borrar Marcas"),_T("Alt+Shift+Del"),_T("Quita los colores y marcas que se agregaron en un fuente producto de una comparacion"),ipre+_T("diff_clear.png"));
	diff_menu->AppendSeparator();
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_HELP, LANG(MENUITEM_TOOLS_DIFF_HELP,"A&yuda..."),"",_T("Muestra ayuda acerca de la comparacion de fuentes en ZinjaI"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, diff_menu,LANG(MENUITEM_TOOLS_DIFF,"&Comparar Archivos (diff)"),_T("Muestra opciones para plegar y desplegar codigo en distintos niveles"),ipre+_T("diff.png"));
	
	wxMenu *doxy_menu = new wxMenu;
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_GENERATE, LANG(MENUITEM_TOOLS_DOXYGEN_GENERATE,"&Generar..."),_T("Ctrl+Shift+F1"),_T("Ejecuta doxygen para generar la documentacion de forma automatica"),ipre+_T("doxy_run.png"));
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_CONFIG, LANG(MENUITEM_TOOLS_DOXYGEN_CONFIGURE,"&Configurar..."),"",_T("Permite establecer opciones para el archivo de configuracion de doxygen"),ipre+_T("doxy_config.png"));
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_VIEW, LANG(MENUITEM_TOOLS_DOXYGEN_VIEW,"&Ver..."),_T("Ctrl+F1"),_T("Abre un explorador y muestra la documentacion generada"),ipre+_T("doxy_view.png"));
	doxy_menu->AppendSeparator();
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_HELP, LANG(MENUITEM_TOOLS_DOXYGE_HELP,"A&yuda..."),"",_T("Muestra una breve ayuda acerca de la integracion de Doxygen en ZinjaI"),ipre+_T("ayuda.png"));
	menu.tools_doxygen = utils->AddSubMenuToMenu(menu.tools, doxy_menu,LANG(MENUITEM_TOOLS_DOXYGEN,"Generar &Documentación (doxygen)"),_T("Doxygen permite generar automaticamente documentacion a partir del codigo y sus comentarios"),ipre+_T("doxy.png"));
	
	wxMenu *wxfb_menu = new wxMenu;
	menu.tools_wxfb_config = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_CONFIG, LANG(MENUITEM_TOOLS_WXFB_CONFIG,"Configurar &Integracion con wxFormBuilder..."),"",_T("Añade los pasos necesarios a la compilacion para utilizar wxFormBuilder"),ipre+_T("wxfb_activate.png"));
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_NEW_RES, LANG(MENUITEM_TOOLS_WXFB_NEW_RESOURCE,"&Adjuntar un Nuevo Proyecto wxFB..."),"",_T("Crea un nuevo proyecto wxFormBuilder y lo agrega al proyecto en ZinjaI"),ipre+_T("wxfb_new_res.png"));
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_LOAD_RES, LANG(MENUITEM_TOOLS_WXFB_LOAD_RESOURCE,"&Adjuntar un Proyecto wxFB Existente..."),"",_T("Agrega un proyecto wxFormBuilder ya existente al proyecto en ZinjaI"),ipre+_T("wxfb_load_res.png"));
	menu.tools_wxfb_regen = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_REGEN, LANG(MENUITEM_TOOLS_WXFB_REGENERATE,"&Regenerar Proyectos wxFB"),_T("Shift+Alt+F9"),_T("Ejecuta wxFormBuilder para regenerar los archivos de recurso o fuentes que correspondan"),ipre+_T("wxfb_regen.png"));
	menu.tools_wxfb_inherit = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_INHERIT_CLASS, LANG(MENUITEM_TOOLS_WXFB_INHERIT,"&Generar Clase Heredada..."),"",_T("Genera una nueva clase a partir de las definidas por algun proyecto wxfb"),ipre+_T("wxfb_inherit.png"));
	menu.tools_wxfb_update_inherit = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_UPDATE_INHERIT, LANG(MENUITEM_TOOLS_WXFB_UPDATE_INHERIT,"Act&ualizar Clase Heredada..."),"",_T("Actualiza los metodos de una clase que hereda de las definidas por algun proyecto wxfb"),ipre+_T("wxfb_update_inherit.png"));
	wxfb_menu->AppendSeparator();
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_HELP_WX, LANG(MENUITEM_TOOLS_WXFB_REFERENCE,"Referencia &wxWidgets..."),"",_T("Muestra la ayuda de la biblioteca wxWidgets"),ipre+_T("ayuda_wx.png"));
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_HELP, LANG(MENUITEM_TOOLS_WXFB_HELP,"A&yuda wxFB..."),"",_T("Muestra una breve ayuda acerca de la integracion de wxFormBuilder en ZinjaI"),ipre+_T("ayuda.png"));
	menu.tools_wxfb = utils->AddSubMenuToMenu(menu.tools, wxfb_menu,LANG(MENUITEM_TOOLS_WXFB,"Diseñar &Interfases (wxFormBuilder)"),_T("Diseño visual de interfaces con la biblioteca wxWidgets"),ipre+_T("wxfb.png"));
	
	wxMenu *cppcheck_menu = new wxMenu;
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_RUN, LANG(MENUITEM_TOOLS_CPPCHECK_RUN,"Iniciar..."),"","",ipre+_T("cppcheck_run.png"));
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_CONFIG, LANG(MENUITEM_TOOLS_CPPCHECK_CONFIG,"Configurar..."),"","",ipre+_T("cppcheck_config.png"));
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_VIEW, LANG(MENUITEM_TOOLS_CPPCHECK_VIEW,"Mostrar Panel de Resultados"),"","",ipre+_T("cppcheck_view.png"));
	cppcheck_menu->AppendSeparator();
	utils->AddItemToMenu(cppcheck_menu,mxID_TOOLS_CPPCHECK_HELP, LANG(MENUITEM_TOOLS_CPPCHECK_HELP,"A&yuda..."),"","",ipre+_T("ayuda.png"));
	menu.tools_cppcheck = utils->AddSubMenuToMenu(menu.tools, cppcheck_menu,LANG(MENUITEM_TOOLS_CPPCHECK,"Análisis Estático (cppcheck)"),"",ipre+_T("cppcheck.png"));
	
	wxMenu *gprof_menu = new wxMenu;
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_SET, LANG(MENUITEM_TOOLS_GPROF_ACTIVATE,"Habilitar/Deshabilitar"),"",_T("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable."),ipre+_T("comp_for_prof.png"));
	wxMenu *gprof_gv_menu = new wxMenu;
	menu.tools_gprof_dot = utils->AddCheckToMenu(gprof_gv_menu, mxID_TOOLS_GPROF_DOT, LANG(MENUITEM_TOOLS_GPROF_DOT,"dot"),"",_T("dot"),config->Init.graphviz_dot);
	menu.tools_gprof_fdp = utils->AddCheckToMenu(gprof_gv_menu, mxID_TOOLS_GPROF_FDP, LANG(MENUITEM_TOOLS_GPROF_FDP,"fdp"),"",_T("fdp"),!config->Init.graphviz_dot);
	utils->AddSubMenuToMenu(gprof_menu, gprof_gv_menu,LANG(MENUITEM_TOOLS_GPROF_LAYOUT,"Algoritmo de Dibujo"),_T("Permite seleccionar entre dos algoritmos diferentes para dibujar el grafo"),ipre+_T("dotfdp.png"));
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_SHOW, LANG(MENUITEM_TOOLS_GPROF_SHOW,"Visualizar Resultados (grafo)..."),"",_T("Muestra graficamente la informacion de profiling de la ultima ejecucion."),ipre+_T("showgprof.png"));
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_LIST, LANG(MENUITEM_TOOLS_GPROF_LIST,"Listar Resultados (texto)"),"",_T("Muestra la informacion de profiling de la ultima ejecucion sin procesar."),ipre+_T("listgprof.png"));
	gprof_menu->AppendSeparator();
	utils->AddItemToMenu(gprof_menu,mxID_TOOLS_GPROF_HELP, LANG(MENUITEM_TOOLS_GPROF_HELP,"A&yuda..."),"",_T("Muestra ayuda acerca de como generar e interpretar la informacion de profiling"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, gprof_menu,LANG(MENUITEM_TOOLS_GPROF,"Perfil de Ejecución (gprof)"),_T("Gprof permite analizar las llamadas a funciones y sus tiempos de ejecucion."),ipre+_T("gprof.png"));
	
	wxMenu *gcov_menu = new wxMenu;
	utils->AddItemToMenu(gcov_menu, mxID_TOOLS_GCOV_SET, LANG(MENUITEM_TOOLS_GCOV_ACTIVATE,"Habilitar/Deshabilitar"),"",_T("Añade/remueve los argumentos necesarios a la configuración de compilación y reconstruye el ejecutable."),ipre+_T("gcov_set.png"));
	utils->AddItemToMenu(gcov_menu, mxID_TOOLS_GCOV_SHOW, LANG(MENUITEM_TOOLS_GCOV_SHOW_BAR,"Mostrar barra de resultados"),"",_T("Muestra un panel con los conteos por linea en el margen izquierdo de la ventana."),ipre+_T("gcov_show.png"));
	utils->AddItemToMenu(gcov_menu, mxID_TOOLS_GCOV_RESET, LANG(MENUITEM_TOOLS_GCOV_RESET,"Eliminar resultados"),"",_T("Elimina los archivos de resultados generados por el test de cobertura."),ipre+_T("gcov_reset.png"));
	gcov_menu->AppendSeparator();
	utils->AddItemToMenu(gcov_menu,mxID_TOOLS_GCOV_HELP, LANG(MENUITEM_TOOLS_GCOV_HELP,"A&yuda..."),"",_T("Muestra ayuda acerca de como generar e interpretar la informacion del test de cobertura"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, gcov_menu,LANG(MENUITEM_TOOLS_GCOV,"&Test de Cobertura (experimental, gcov)"),_T("Gcov permite contabilizar cuantas veces se ejecuta cada linea del código fuente."),ipre+_T("gcov.png"));
	
#if !defined(_WIN32) && !defined(__WIN32__)
	wxMenu *valgrind_menu = new wxMenu;
	utils->AddItemToMenu(valgrind_menu, mxID_TOOLS_VALGRIND_RUN, LANG(MENUITEM_TOOLS_VALGRIND_RUN,"Ejecutar..."),"","",ipre+_T("valgrind_run.png"));
	utils->AddItemToMenu(valgrind_menu, mxID_TOOLS_VALGRIND_VIEW, LANG(MENUITEM_TOOLS_VALGRIND_VIEW,"Mostrar Panel de Resultados"),"","",ipre+_T("valgrind_view.png"));
	valgrind_menu->AppendSeparator();
	utils->AddItemToMenu(valgrind_menu,mxID_TOOLS_VALGRIND_HELP, LANG(MENUITEM_TOOLS_VALGRIND_HELP,"A&yuda..."),"","",ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, valgrind_menu,LANG(MENUITEM_TOOLS_VALGRIND,"Análisis Dinámico (valgrind)"),_T("Valgrind permite analizar el uso de memoria dinamica para detectar perdidas y otros errores"),ipre+_T("valgrind.png"));
#endif
	
	utils->AddSubMenuToMenu(menu.tools, menu.tools_custom_menu = new wxMenu,LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables"),"",ipre+_T("customTools.png"));
	menu.tools_custom_item=new wxMenuItem*[10];
	for (int i=0;i<10;i++) menu.tools_custom_item[i] = NULL;
	menu.tools_custom_menu->AppendSeparator();
	utils->AddItemToMenu(menu.tools_custom_menu,mxID_TOOLS_CUSTOM_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_CUSTOM_TOOLS_SETTINGS,"&Configurar (generales)..."),"","",ipre+_T("customToolsSettings.png"));
	menu.tools_project_tools = utils->AddItemToMenu(menu.tools_custom_menu,mxID_TOOLS_PROJECT_TOOLS_SETTINGS, LANG(MENUITEM_TOOLS_PROJECT_TOOLS_SETTINGS,"&Configurar (de proyecto)..."),"","",ipre+_T("projectToolsSettings.png"));
	utils->AddItemToMenu(menu.tools_custom_menu,mxID_TOOLS_CUSTOM_HELP, LANG(MENUITEM_TOOLS_CUSTOM_HELP,"A&yuda..."),"","",ipre+_T("ayuda.png"));	
	UpdateCustomTools(false);
	
	menu.tools->AppendSeparator();
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_CREATE_TEMPLATE, LANG(MENUITEM_TOOLS_CREATE_TEMPLATE,"Guardar como nueva plantilla..."),"",_T("Permite guardar el programa simple o proyecto actual como plantilla"),ipre+_T("create_template.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_INSTALL_COMPLEMENTS, LANG(MENUITEM_TOOLS_INSTALL_COMPLEMENTS,"Instalar Complementos..."),"",_T("Permite instalar un complemento ya descargado para ZinjaI"),ipre+_T("updates.png"));
	
}

void mxMainWindow::PopulateMenuHelp(const wxString &ipre) { 
	
	utils->AddItemToMenu(menu.help,mxID_HELP_ABOUT, LANG(MENUITEM_HELP_ABOUT,"Acerca de..."),"",_T("Acerca de..."),ipre+_T("acercaDe.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_TUTORIAL, LANG(MENUITEM_HELP_TUTORIALS,"Tutoriales..."),"",_T("Abre el cuadro de ayuda y muestra el indice de tutoriales disponibles"),ipre+_T("tutoriales.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_GUI, LANG(MENUITEM_HELP_ZINJAI,"Ayuda sobre ZinjaI..."),_T("F1"),_T("Muestra la ayuda sobre el uso y las caracteristicas de este entorno..."),ipre+_T("ayuda.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_CPP, LANG(MENUITEM_HELP_CPP,"Referencia C/C++..."),_T("Alt+F1"),_T("Muestra una completa referencia sobre el lenguaje"),ipre+_T("referencia.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_TIP, LANG(MENUITEM_HELP_TIPS,"&Mostrar sugerencias de uso..."),"",_T("Muestra sugerencias sobre el uso del programa..."),ipre+_T("tip.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_OPINION, LANG(MENUITEM_HELP_OPINION,"Enviar sugerencia o reportar error..."),"",_T("Permite acceder a los foros oficiales de ZinjaI para dejar sugerencias, comentarios o reportar errores"),ipre+_T("opinion.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_UPDATES, LANG(MENUITEM_HELP_UPDATES,"&Buscar actualizaciones..."),"",_T("Comprueba a traves de Internet si hay versiones mas recientes de ZinjaI disponibles..."),ipre+_T("updates.png"));
}


void mxMainWindow::CreateToolbars(wxToolBar *wich_one, bool delete_old) {
#define _aux_ctb_new(name) toolbar_##name = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER | (config->Toolbars.positions.name.top?wxTB_HORIZONTAL:wxTB_VERTICAL));
#define _aux_ctb_aui(name,label) aui_manager.AddPane(toolbar_##name, wxAuiPaneInfo().Name("toolbar_"#name).Caption(label).ToolbarPane().Hide());
	wxString ipre=DIR_PLUS_FILE(wxString()<<config->Toolbars.icon_size,"");
	if (wich_one) {
		if (delete_old) { 
#define _aux_ctb_redo(name,label) if (wich_one == toolbar_##name) { \
			aui_manager.DetachPane(wich_one); wich_one->Destroy(); \
			_aux_ctb_new(name); wich_one=toolbar_##name; \
			_aux_ctb_aui(name,label); }
			_aux_ctb_redo(file,LANG(CAPTION_TOOLBAR_FILE,"Archivo"));
			_aux_ctb_redo(edit,LANG(CAPTION_TOOLBAR_EDIT,"Edición"));
			_aux_ctb_redo(view,LANG(CAPTION_TOOLBAR_VIEW,"Ver"));
			_aux_ctb_redo(run,LANG(CAPTION_TOOLBAR_RUN,"Ejecución"));
			_aux_ctb_redo(tools,LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"));
			_aux_ctb_redo(misc,LANG(CAPTION_TOOLBAR_MISC,"Miscelánea"));
			_aux_ctb_redo(project,LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"));
			_aux_ctb_redo(debug,LANG(CAPTION_TOOLBAR_DEBUG,"Depuración"));
		} else
			wich_one->ClearTools();
		wich_one->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
	} else {
		if (!wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,wxString()<<config->Toolbars.icon_size))) {
			wxString icsz = wxString()<<config->Toolbars.icon_size<<_T("x")<<config->Toolbars.icon_size;
			mxMessageDialog(this,
				wxString()<<LANG1(MAIN_WINDOW_NO_ICON_SIZE,""
				"El tema de iconos seleccionado no tiene iconos del tamano elegido (<{1}>)\n"
				"Se utilizaran los iconos del tamano predeterminado (16x16).\n"
				"Para modificarlo utilice el cuadro de Preferencias (menu Archivo).",wxString()<<icsz),
				LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
			config->Toolbars.icon_size=16;
			ipre=DIR_PLUS_FILE(_T("16"),"");
		}
		
#define _aux_ctb_init(name,label) _aux_ctb_new(name); toolbar_##name->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size)); _aux_ctb_aui(name,label)
		_aux_ctb_init(file,LANG(CAPTION_TOOLBAR_FILE,"Archivo"));
		_aux_ctb_init(edit,LANG(CAPTION_TOOLBAR_EDIT,"Edición"));
		_aux_ctb_init(view,LANG(CAPTION_TOOLBAR_VIEW,"Ver"));
		_aux_ctb_init(run,LANG(CAPTION_TOOLBAR_RUN,"Ejecución"));
		_aux_ctb_init(tools,LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"));
		_aux_ctb_init(misc,LANG(CAPTION_TOOLBAR_MISC,"Miscelánea"));
		_aux_ctb_init(project,LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"));
		_aux_ctb_init(debug,LANG(CAPTION_TOOLBAR_DEBUG,"Depuración"));
	}
	
#define _aux_ctb_realize(tb) tb->Realize(); aui_manager.GetPane(tb).BestSize(tb->GetBestSize());
	
	if (!wich_one || wich_one==toolbar_tools) {
		
		if (config->Toolbars.tools.draw_flow) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_FLOW,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de Flujo..."),ipre+_T("flujo.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_FLOWCHART,"Herramientas -> Dibujar Diagrama de Flujo..."));
		if (config->Toolbars.tools.draw_classes) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_CLASSES,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar Jerarquia de Clases..."),ipre+_T("clases.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_CLASS_HIERARCHY,"Herramientas -> Dibujar Jerarquia de Clases..."));
		if (config->Toolbars.tools.copy_code_from_h) utils->AddTool(toolbar_tools,mxID_TOOLS_CODE_COPY_FROM_H,LANG(TOOLBAR_CAPTION_TOOLS_COPY_CODE_FROM_H,"Implementar Metodos/Funciones Faltantes..."),ipre+_T("copy_code_from_h.png"),LANG(TOOLBAR_DESC_TOOLS_COPY_CODE_FROM_H,"Herramientas -> Implementar Metodos/Funciones Faltantes..."));
		if (config->Toolbars.tools.align_comments) utils->AddTool(toolbar_tools,mxID_TOOLS_ALIGN_COMMENTS,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_ALIGN_COMMENTS,"Alinear Comentarios..."),ipre+_T("align_comments.png"),LANG(TOOLBAR_TOOLS_DESC_COMMENTS_ALIGN_COMMENTS,"Herramientas -> Comentarios -> Alinear Comentarios..."));
		if (config->Toolbars.tools.remove_comments) utils->AddTool(toolbar_tools,mxID_TOOLS_REMOVE_COMMENTS,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_DELETE_COMMENTS,"Eliminar Comentarios"),ipre+_T("remove_comments.png"),LANG(TOOLBAR_DESC_TOOLS_COMMENTS_DELETE_COMMENTS,"Herramientas -> Comtearios -> Eliminar Comentarios"));
		if (config->Toolbars.tools.preproc_mark_valid) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_MARK_VALID,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas"),ipre+_T("preproc_mark_valid.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_MARK_VALID,"Herramientas -> Preprocesador -> Marcar Lineas No Compiladas"));
		if (config->Toolbars.tools.preproc_unmark_all) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_UNMARK_ALL,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas"),ipre+_T("preproc_unmark_all.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_UNMARK_ALL,"Herramientas -> Preprocesador -> Borrar Marcas"));
		if (config->Toolbars.tools.preproc_expand_macros) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_EXPAND_MACROS,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros"),ipre+_T("preproc_expand_macros.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_EXPAND_MACROS,"Herramientas -> Preprocesador -> Expandir Macros"));
		if (config->Toolbars.tools.generate_makefile) utils->AddTool(toolbar_tools,mxID_TOOLS_MAKEFILE,LANG(TOOLBAR_CAPTION_TOOLS_GENERATE_MAKEFILE,"Generar Makefile..."),ipre+_T("makefile.png"),LANG(TOOLBAR_DESC_TOOLS_GENERATE_MAKEFILE,"Herramientas -> Generar Makefile..."));
		if (config->Toolbars.tools.open_terminal) utils->AddTool(toolbar_tools,mxID_TOOLS_CONSOLE,LANG(TOOLBAR_CAPTION_TOOLS_OPEN_TERMINAL,"Abrir terminal..."),ipre+_T("console.png"),LANG(TOOLBAR_DESC_OPEN_TERMINAL,"Herramientas -> Abrir consola..."));
		if (config->Toolbars.tools.exe_info) utils->AddTool(toolbar_tools,mxID_TOOLS_EXE_PROPS,LANG(TOOLBAR_CAPTION_TOOLS_EXE_INFO,"Propiedades del Ejecutable..."),ipre+_T("exeinfo.png"),LANG(TOOLBAR_DESC_EXE_INFO,"Herramientas -> Propiedades del Ejecutable..."));
		if (config->Toolbars.tools.proy_stats) utils->AddTool(toolbar_tools,mxID_TOOLS_PROJECT_STATISTICS,LANG(TOOLBAR_CAPTION_TOOLS_PROY_STATS,"Estadisticas del Proyecto..."),ipre+_T("proystats.png"),LANG(TOOLBAR_DESC_PROY_STATS,"Herramientas -> Estadisticas del Proyecto..."));
		if (config->Toolbars.tools.draw_classes) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_CLASSES,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),ipre+_T("draw_project.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_PROJECT,"Herramientas -> Grafo del Proyecto..."));
		if (config->Toolbars.tools.open_shared) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_OPEN,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_OPEN,"Abrir Archivo Compartido..."),ipre+_T("abrirs.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_OPEN,"Herramientas -> Compartir Archivos en la Red Local -> Abrir Compartido..."));
		if (config->Toolbars.tools.share_source) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_SHARE,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_SHARE,"Compartir Fuente..."),ipre+_T("compartir.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_SHARE,"Herramientas -> Compartir Archivos en la Red Local -> Compartir Actual..."));
		if (config->Toolbars.tools.share_list) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_LIST,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_LIST,"Listar Compartidos Propios..."),ipre+_T("share_list.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_LIST,"Herramientas -> Compartir Archivos en la Red Local -> Listar Compartidos Propios..."));
		if (config->Toolbars.tools.diff_two_sources) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_TWO,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_TWO,"Compara Dos Fuentes Abiertos..."),ipre+_T("diff_sources.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_TWO,"Herramientas -> Comparar Archivos -> Dos Fuentes Abiertos"));
		if (config->Toolbars.tools.diff_other_file) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_DISK,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISK,"Comparar Fuente con Archivo en Disco..."),ipre+_T("diff_source_file.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISK,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Archivo en Disco"));
		if (config->Toolbars.tools.diff_himself) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_HIMSELF,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_HIMSELF,"Comparar Cambios en el Fuente con su Version en Disco..."),ipre+_T("diff_source_himself.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_HIMSELF,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Version en Disco"));
		if (config->Toolbars.tools.diff_show) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		if (config->Toolbars.tools.diff_apply) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_APPLY,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		if (config->Toolbars.tools.diff_discard) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_DISCARD,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		if (config->Toolbars.tools.diff_clear) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_CLEAR,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
		if (config->Toolbars.tools.doxy_generate) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_GENERATE,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_GENERATE,"Generar Documentacion Doxygen..."),ipre+_T("doxy_run.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_GENERATE,"Herramientas -> Generar Documentacion -> Generar..."));
		if (config->Toolbars.tools.doxy_config) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_CONFIG,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_CONFIGURE,"Configurar Documentacion Doxygen..."),ipre+_T("doxy_config.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_CONFIGURE,"Herramientas -> Generar Documentacion -> Configurar..."));
		if (config->Toolbars.tools.doxy_view) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_VIEW,"Ver Documentacion Doxygen..."),ipre+_T("doxy_view.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_VIEW,"Herramientas -> Generar Documentacion -> Ver..."));
		if (config->Toolbars.tools.wxfb_config) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_CONFIG,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_CONFIG,"Activar Integracion wxFormBuilder"),ipre+_T("wxfb_activate.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_ACTIVATE,"Herramientas -> wxFormBuilder -> Configurar Integracion wxFormBuilder"));
		if (config->Toolbars.tools.wxfb_new_res) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_NEW_RES,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_NEW_RESOURCE,"Adjuntar un Nuevo Proyecto wxFB"),ipre+_T("wxfb_new_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_NEW_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Nuevo Proyecto wxFB"));
		if (config->Toolbars.tools.wxfb_load_res) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_LOAD_RES,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_LOAD_RESOURCE,"Adjuntar un Proyecto wxFB Existente"),ipre+_T("wxfb_load_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_LOAD_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Proyecto wxFB Existente"));
		if (config->Toolbars.tools.wxfb_regen) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_REGEN,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REGENERATE,"Regenerar Proyectos wxFB"),ipre+_T("wxfb_regen.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REGENERATE,"Herramientas -> wxFormBuilder -> Regenerar Proyectos wxFB"));
		if (config->Toolbars.tools.wxfb_inherit) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),ipre+_T("wxfb_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_INHERIT,"Herramientas -> wxFormBuilder -> Generar Clase Heredada..."));
		if (config->Toolbars.tools.wxfb_update_inherit) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_UPDATE_INHERIT,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_UPDATE_INHERIT,"Actualizar Clase Heredada..."),ipre+_T("wxfb_update_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_UPDATE_INHERIT,"Herramientas -> wxFormBuilder -> Actualizar Clase Heredada..."));
		if (config->Toolbars.tools.wxfb_help_wx) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_HELP_WX,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),ipre+_T("ayuda_wx.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REFERENCE,"Herramientas -> wxFormBuilder -> Referencia wxWidgets..."));
		if (config->Toolbars.tools.cppcheck_run) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_RUN,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_RUN,"Comenzar Analisis Estatico..."),ipre+_T("cppcheck_run.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_RUN,"Herramientas -> Analisis Estatico -> Iniciar..."));
		if (config->Toolbars.tools.cppcheck_config) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_CONFIG,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_CONFIG,"Configurar Analisis Estatico..."),ipre+_T("cppcheck_config.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_CONFIG,"Herramientas -> Analisis Estatico-> Configurar..."));
		if (config->Toolbars.tools.cppcheck_view) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_VIEW,"Mostrar Resultados del Analisis Estatico"),ipre+_T("cppcheck_view.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_VIEW,"Herramientas -> Analisis Estatico -> Mostrar Panel de Resultados"));
		if (config->Toolbars.tools.gprof_activate) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_SET,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_ACTIVATE,"Activar Perfilado de Ejecucion"),ipre+_T("comp_for_prof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_ACTIVATE,"Herramientas -> Perfil de Ejecucion -> Activar"));
		if (config->Toolbars.tools.gprof_show_graph) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_SHOW,"Graficar Resultados del Perfilado de Ejecucion"),ipre+_T("showgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_SHOW,"Herramientas -> Perfil de Ejecucion -> Visualizar Resultados (grafo)..."));
		if (config->Toolbars.tools.gprof_list_output) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_LIST,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_LIST,"Listar Resultados del Perfilado de Ejecucion"),ipre+_T("listgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_LIST,"Herramientas -> Perfil de Ejecucion -> Listar Resultados (texto)..."));
		if (config->Toolbars.tools.gcov_activate) utils->AddTool(toolbar_tools,mxID_TOOLS_GCOV_SET,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_ACTIVATE,"Activar Test de Cobertura"),ipre+_T("gcov_set.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_ACTIVATE,"Herramientas -> Test de Cobertura -> Habilitar/Deshabilitar"));
		if (config->Toolbars.tools.gcov_show_bar) utils->AddTool(toolbar_tools,mxID_TOOLS_GCOV_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_RESET,"Mostrar barra de resultados de Test de Cobertura"),ipre+_T("gcov_show.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_SHOW_BAR,"Herramientas -> Test de Cobertura -> Mostrar Barra de Resultados"));
		if (config->Toolbars.tools.gcov_reset) utils->AddTool(toolbar_tools,mxID_TOOLS_GCOV_RESET,LANG(TOOLBAR_CAPTION_TOOLS_GCOV_SHOW_BAR,"Eliminar resultados de Test de Cobertura"),ipre+_T("gcov_reset.png"),LANG(TOOLBAR_DESC_TOOLS_GCOV_RESET,"Herramientas -> Test de Cobertura -> Eliminar Resultados"));
#if !defined(__WIN32__)
		if (config->Toolbars.tools.valgrind_run) utils->AddTool(toolbar_tools,mxID_TOOLS_VALGRIND_RUN,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_RUN,"Ejecutar Para Analisis Dinamico"),ipre+_T("valgrind_run.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_RUN,"Herramientas -> Analisis Dinámico -> Ejecutar..."));
		if (config->Toolbars.tools.valgrind_view) utils->AddTool(toolbar_tools,mxID_TOOLS_VALGRIND_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_VIEW,"Mostrar Resultados del Analisis Dinamico"),ipre+_T("valgrind_view.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_VIEW,"Herramientas -> Analisis Dinámico -> Mostrar Panel de Resultados"));
#endif
		for (int i=0;i<MAX_CUSTOM_TOOLS;i++) {
			if (config->CustomTools[i].on_toolbar) {
				wxString str(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada ")); str<<i<<" ("<<config->CustomTools[i].name<<")";
				utils->AddTool(toolbar_tools,mxID_CUSTOM_TOOL_0+i,str,ipre+wxString("customTool")<<i<<".png",str);
			}
		}
		if (config->Toolbars.tools.custom_settings) utils->AddTool(toolbar_tools,mxID_TOOLS_CUSTOM_TOOLS_SETTINGS,LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),ipre+_T("customToolsSettings.png"),LANG(TOOLBAR_DESC_TOOLS_CUSTOM_TOOLS_SETTINGS,"Herramientas -> Herramientas Personalizadas -> Configurar (generales)..."));
		_aux_ctb_realize(toolbar_tools);
	}
	
	if (wich_one==toolbar_project) {
		if (project->GetWxfbActivated()) {
			utils->AddTool(toolbar_project,mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),ipre+_T("wxfb_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_INHERIT,"Herramientas -> wxFormBuilder -> Generar Clase Heredada..."));
			utils->AddTool(toolbar_project,mxID_TOOLS_WXFB_HELP_WX,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),ipre+_T("ayuda_wx.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REFERENCE,"Herramientas -> wxFormBuilder -> Referencia wxWidgets..."));
		}
		bool have_tool=false;
		for (int i=0;i<MAX_PROJECT_CUSTOM_TOOLS;i++) {
			if (project->custom_tools[i].on_toolbar) {
				have_tool=true;
				wxString str(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada ")); str<<i<<" ("<<project->custom_tools[i].name<<")";;
				utils->AddTool(toolbar_project,mxID_CUSTOM_PROJECT_TOOL_0+i,str,ipre+wxString("projectTool")<<i<<".png",str);
			}
		}
		if (!have_tool) utils->AddTool(toolbar_project,mxID_TOOLS_PROJECT_TOOLS_SETTINGS,LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),ipre+_T("projectToolsSettings.png"),LANG(TOOLBAR_DESC_TOOLS_PROJECT_TOOLS_SETTINGS,"Herramientas -> Herramientas Personalizadas -> Configurar (de proyecto)..."));
		_aux_ctb_realize(toolbar_project);
	}
	
	if (!wich_one || wich_one==toolbar_view) {
		
		if (config->Toolbars.view.split_view) utils->AddTool(toolbar_view,mxID_VIEW_DUPLICATE_TAB,LANG(TOOLBAR_CAPTION_VIEW_SPLIT_VIEW,"Duplicar Vista"),ipre+_T("duplicarVista.png"),LANG(TOOLBAR_DESC_VIEW_SPLIT_VIEW,""));
		if (config->Toolbars.view.line_wrap) utils->AddTool(toolbar_view,mxID_VIEW_LINE_WRAP,LANG(TOOLBAR_CAPTION_VIEW_LINE_WRAP,"Ajuste de Linea"),ipre+_T("lineWrap.png"),LANG(TOOLBAR_DESC_VIEW_LINE_WRAP,""));
		if (config->Toolbars.view.white_space) utils->AddTool(toolbar_view,mxID_VIEW_WHITE_SPACE,LANG(TOOLBAR_CAPTION_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de de fin de linea"),ipre+_T("whiteSpace.png"),LANG(TOOLBAR_DESC_VIEW_WHITE_SPACES,""));
		if (config->Toolbars.view.sintax_colour) utils->AddTool(toolbar_view,mxID_VIEW_CODE_STYLE,LANG(TOOLBAR_CAPTION_VIEW_SYNTAX_HIGHLIGHT,"Colorear Sintaxis"),ipre+_T("syntaxColour.png"),LANG(TOOLBAR_DESC_VIEW_SYNTAX_HIGHLIGHT,""));
		if (config->Toolbars.view.update_symbols) utils->AddTool(toolbar_view,mxID_VIEW_UPDATE_SYMBOLS,LANG(TOOLBAR_CAPTION_VIEW_SYMBOLS_TREE,"Arbol de simbolos"),ipre+_T("symbolsTree.png"),LANG(TOOLBAR_DESC_VIEW_SYMBOLS_TREE,""));
		if (config->Toolbars.view.explorer_tree) utils->AddTool(toolbar_view,mxID_VIEW_EXPLORER_TREE,LANG(TOOLBAR_CAPTION_VIEW_EXPLORER_TREE,"Explorador de Archivos"),ipre+_T("explorerTree.png"),LANG(TOOLBAR_DESC_VIEW_EXPLORER_TREE,""));
		if (config->Toolbars.view.project_tree) utils->AddTool(toolbar_view,mxID_VIEW_PROJECT_TREE,LANG(TOOLBAR_CAPTION_VIEW_PROJECT_TREE,"Arbol de Proyecto"),ipre+_T("projectTree.png"),LANG(TOOLBAR_DESC_VIEW_PROJECT_TREE,""));
		if (config->Toolbars.view.compiler_tree) utils->AddTool(toolbar_view,mxID_VIEW_COMPILER_TREE,LANG(TOOLBAR_CAPTION_VIEW_COMPILER_TREE,"Resultados de La Compilacion"),ipre+_T("compilerTree.png"),LANG(TOOLBAR_DESC_VIEW_COMPILER_TREE,""));
		if (config->Toolbars.view.full_screen) utils->AddTool(toolbar_view,mxID_VIEW_FULLSCREEN,LANG(TOOLBAR_CAPTION_VIEW_FULLSCREEN,"Pantalla Completa"),ipre+_T("fullScreen.png"),LANG(TOOLBAR_DESC_VIEW_FULLSCREEN,""));
		if (config->Toolbars.view.beginner_panel) utils->AddTool(toolbar_view,mxID_VIEW_BEGINNER_PANEL,LANG(TOOLBAR_CAPTION_VIEW_BEGINNER_PANEL,"Panel de Asistencias"),ipre+_T("beginer_panel.png"),LANG(TOOLBAR_DESC_VIEW_BEGINER_PANEL,""));
		if (config->Toolbars.view.prev_error) utils->AddTool(toolbar_view,mxID_VIEW_PREV_ERROR,LANG(TOOLBAR_CAPTION_VIEW_PREV_ERROR,"Ir a Error Anterior"),ipre+_T("errorPrev.png"),LANG(TOOLBAR_DESC_VIEW_PREV_ERROR,""));
		if (config->Toolbars.view.next_error) utils->AddTool(toolbar_view,mxID_VIEW_NEXT_ERROR,LANG(TOOLBAR_CAPTION_VIEW_NEXT_ERROR,"Ir a siguiente error"),ipre+_T("errorNext.png"),LANG(TOOLBAR_DESC_VIEW_NEXT_ERROR,""));
		if (config->Toolbars.view.fold_all) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_ALL,LANG(TOOLBAR_CAPTION_FOLD_ALL_LEVELS,"Plegar Todos los Niveles"),ipre+_T("foldAll.png"),LANG(TOOLBAR_DESC_FOLD_ALL_LEVELS,""));
		if (config->Toolbars.view.fold_1) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_1,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_1,"Plegar el Primer Nivel"),ipre+_T("fold1.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_1,""));
		if (config->Toolbars.view.fold_2) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_2,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_2,"Plegar el Segundo Nivel"),ipre+_T("fold2.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_2,""));
		if (config->Toolbars.view.fold_3) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		if (config->Toolbars.view.unfold_all) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_ALL,LANG(TOOLBAR_CAPTION_UNFOLD_ALL_LEVELS,"Desplegar Todos los Niveles"),ipre+_T("unfoldAll.png"),LANG(TOOLBAR_DESC_UNFOLD_ALL_LEVELS,""));
		if (config->Toolbars.view.unfold_1) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_1,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_1,"Desplegar el Primer Nivel"),ipre+_T("unfold1.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_1,""));
		if (config->Toolbars.view.unfold_2) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_2,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_2,"Desplegar el Segundo Nivel"),ipre+_T("unfold2.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_2,""));
		if (config->Toolbars.view.unfold_3) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		_aux_ctb_realize(toolbar_view);
	}
	
	if (!wich_one || wich_one==toolbar_file) {
		
		if (config->Toolbars.file.new_file) utils->AddTool(toolbar_file,wxID_NEW,LANG(TOOLBAR_CAPTION_FILE_NEW,"Nuevo..."),ipre+_T("nuevo.png"),LANG(TOOLBAR_DESC_FILE_NEW,"Archivo -> Nuevo..."));
		if (config->Toolbars.file.new_project) utils->AddTool(toolbar_file,mxID_FILE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_NEW_PROJECT,"Nuevo Proyecto..."),ipre+_T("proyecto.png"),LANG(TOOLBAR_DESC_FILE_NEW_PROJECT,"Archivo -> Nuevo Proyecto..."));
		if (config->Toolbars.file.open) utils->AddTool(toolbar_file,wxID_OPEN,LANG(TOOLBAR_CAPTION_FILE_OPEN,"Abrir..."),ipre+_T("abrir.png"),LANG(TOOLBAR_DESC_FILE_OPEN,"Archivo -> Abrir..."));
		if (config->Toolbars.file.recent_simple) utils->AddTool(toolbar_file,mxID_FILE_SOURCE_HISTORY_MORE,LANG(TOOLBAR_CAPTION_FILE_RECENT_SOURCES,"Fuentes Recientes..."),ipre+_T("recentSimple.png"),LANG(TOOLBAR_DESC_FILE_RECENT_SOURCES,"Archivo -> Fuentes Abiertos Recientemente..."));
		if (config->Toolbars.file.recent_project) utils->AddTool(toolbar_file,mxID_FILE_PROJECT_HISTORY_MORE,LANG(TOOLBAR_CAPTION_FILE_RECENT_PROJECTS,"Proyectos Recientes..."),ipre+_T("recentProject.png"),LANG(TOOLBAR_DESC_FILE_RECENT_PROJECTS,"Archivo -> Proyectos Abiertos Recientemete..."));
		if (config->Toolbars.file.open_header) utils->AddTool(toolbar_file,mxID_FILE_OPEN_H,LANG(TOOLBAR_CAPTION_FILE_OPEN_H,"Abrir h/cpp Complementario"),ipre+_T("abrirp.png"),LANG(TOOLBAR_DESC_FILE_OPEN_H,"Abrir h/cpp Complementario"));
		if (config->Toolbars.file.open_selected) utils->AddTool(toolbar_file,mxID_FILE_OPEN_SELECTED,LANG(TOOLBAR_CAPTION_FILE_OPEN_SELECTED,"Abrir Seleccionado"),ipre+_T("abrirh.png"),LANG(TOOLBAR_DESC_FILE_OPEN_SELECTED,"Archivo -> Abrir Seleccionado"));
		if (config->Toolbars.file.save) utils->AddTool(toolbar_file,wxID_SAVE,LANG(TOOLBAR_CAPTION_FILE_SAVE,"Guardar"),ipre+_T("guardar.png"),LANG(TOOLBAR_DESC_FILE_SABE,"Archivo -> Guardar"));
		if (config->Toolbars.file.save_as) utils->AddTool(toolbar_file,wxID_SAVEAS,LANG(TOOLBAR_CAPTION_FILE_SAVE_AS,"Guardar Como..."),ipre+_T("guardarComo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_AS,"Archivo -> Guardar Como..."));
		if (config->Toolbars.file.save_all) utils->AddTool(toolbar_file,mxID_FILE_SAVE_ALL,LANG(TOOLBAR_CAPTION_FILE_SAVE_ALL,"Guardar Todo"),ipre+_T("guardarTodo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_ALL,"Archivo -> Guardar Todo"));
		if (config->Toolbars.file.save_project) utils->AddTool(toolbar_file,mxID_FILE_SAVE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_SAVE_PROJECT,"Guardar Proyecto"),ipre+_T("guardarProyecto.png"),LANG(TOOLBAR_DESC_FILE_SAVE_PROJECT,"Archivo -> Guardar Proyecto"));
		if (config->Toolbars.file.export_html) utils->AddTool(toolbar_file,mxID_FILE_EXPORT_HTML,LANG(TOOLBAR_CAPTION_FILE_EXPORT_HTML,"Exportar a HTML..."),ipre+_T("exportHtml.png"),LANG(TOOLBAR_DESC_FILE_EXPORT_HTML,"Archivo -> Exportar a HTML..."));
		if (config->Toolbars.file.print) utils->AddTool(toolbar_file,mxID_FILE_PRINT,LANG(TOOLBAR_CAPTION_FILE_PRINT,"Imprimir..."),ipre+_T("imprimir.png"),LANG(TOOLBAR_DESC_FILE_PRINT,"Archivo -> Imprimir..."));
		if (config->Toolbars.file.reload) utils->AddTool(toolbar_file,mxID_FILE_RELOAD,LANG(TOOLBAR_CAPTION_FILE_RELOAD,"Recargar"),ipre+_T("recargar.png"),LANG(TOOLBAR_DESC_FILE_RELOAD,"Archivo -> Recargar"));
		if (config->Toolbars.file.close) utils->AddTool(toolbar_file,wxID_CLOSE,LANG(TOOLBAR_CAPTION_FILE_CLOSE,"Cerrar"),ipre+_T("cerrar.png"),LANG(TOOLBAR_DESC_FILE_CLOSE,"Archivo -> Cerrar"));
		if (config->Toolbars.file.close_all) utils->AddTool(toolbar_file,mxID_FILE_CLOSE_ALL,LANG(TOOLBAR_CAPTION_FILE_CLOSE_ALL,"Cerrar Todo"),ipre+_T("cerrarTodo.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_ALL,"Archivo -> Cerrar Todo"));
		if (config->Toolbars.file.close_project) utils->AddTool(toolbar_file,mxID_FILE_CLOSE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_CLOSE_PROJECT,"Cerrar Proyecto"),ipre+_T("cerrarProyecto.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_PROJECT,"Archivo -> Cerrar Proyecto"));
		if (config->Toolbars.file.project_config) utils->AddTool(toolbar_file,mxID_FILE_PROJECT_CONFIG,LANG(TOOLBAR_CAPTION_FILE_PROJECT_CONFIG,"Configuracion de proyecto..."),ipre+_T("projectConfig.png"),LANG(TOOLBAR_DESC_FILE_PROJECT_CONFIG,"Archivo -> Configuracion del Proyecto..."));
		_aux_ctb_realize(toolbar_file);
	}
	
	if (!wich_one || wich_one==toolbar_misc) {
		if (config->Toolbars.misc.preferences) utils->AddTool(toolbar_misc,mxID_FILE_PREFERENCES,LANG(TOOLBAR_CAPTION_FILE_PREFERENCES,"Preferencias..."),ipre+_T("preferencias.png"),LANG(TOOLBAR_DESC_FILE_PREFERENCES,"Archivo -> Preferencias..."));
		if (config->Toolbars.misc.tutorials) utils->AddTool(toolbar_misc,mxID_HELP_TUTORIAL,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Tutoriales..."),ipre+_T("tutoriales.png"),LANG(TOOLBAR_DESC_HELP_TUTORIALS,"Ayuda -> Tutoriales..."));
		if (config->Toolbars.misc.help_ide) utils->AddTool(toolbar_misc,mxID_HELP_GUI,LANG(TOOLBAR_CAPTION_HELP_ZINJAI,"Ayuda Sobre ZinjaI..."),ipre+_T("ayuda.png"),LANG(TOOLBAR_DESC_HELP_ZINJAI,"Ayuda -> Ayuda sobre ZinjaI..."));
		if (config->Toolbars.misc.help_cpp) utils->AddTool(toolbar_misc,mxID_HELP_CPP,LANG(TOOLBAR_CAPTION_HELP_CPP,"Ayuda Sobre C/C++..."),ipre+_T("referencia.png"),LANG(TOOLBAR_DESC_HELP_CPP,"Ayuda -> Ayuda sobre C++..."));
		if (config->Toolbars.misc.show_tips) utils->AddTool(toolbar_misc,mxID_HELP_TIP,LANG(TOOLBAR_CAPTION_HELP_TIPS,"Sugerencias..."),ipre+_T("tip.png"),LANG(TOOLBAR_DESC_HELP_TIPS,"Ayuda -> Mostrar Sugerencias..."));
		if (config->Toolbars.misc.about) utils->AddTool(toolbar_misc,mxID_HELP_ABOUT,LANG(TOOLBAR_CAPTION_HELP_ABOUT,"Acerca de..."),ipre+_T("acercaDe.png"),LANG(TOOLBAR_DESC_HELP_ABOUT,"Ayuda -> Acerca de..."));
		if (config->Toolbars.misc.opinion) utils->AddTool(toolbar_misc,mxID_HELP_OPINION,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Enviar sugerencia o reportar error..."),ipre+_T("opinion.png"),LANG(TOOLBAR_DESC_HELP_OPINION,"Ayuda -> Opina sobre ZinjaI..."));
		if (config->Toolbars.misc.find_updates) utils->AddTool(toolbar_misc,mxID_HELP_UPDATES,LANG(TOOLBAR_CAPTION_HELP_UPDATES,"Buscar Actualizaciones..."),ipre+_T("updates.png"),LANG(TOOLBAR_DESC_HELP_UPDATES,"Ayuda -> Buscar Actualizaciones..."));
		if (config->Toolbars.misc.exit) utils->AddTool(toolbar_misc,wxID_EXIT,LANG(TOOLBAR_CAPTION_FILE_EXIT,"Salir"),ipre+_T("salir.png"),LANG(TOOLBAR_DESC_FILE_EXIT,"Archivo -> Salir"));
		_aux_ctb_realize(toolbar_misc);
	}
	
	if (!wich_one || wich_one==toolbar_edit) {
		
		if (config->Toolbars.edit.undo) utils->AddTool(toolbar_edit,wxID_UNDO,LANG(TOOLBAR_CAPTION_EDIT_UNDO,"Deshacer"),ipre+_T("deshacer.png"),LANG(TOOLBAR_DESC_EDIT_UNDO,"Editar -> Deshacer"));
		if (config->Toolbars.edit.redo) utils->AddTool(toolbar_edit,wxID_REDO,LANG(TOOLBAR_CAPTION_EDIT_REDO,"Rehacer"),ipre+_T("rehacer.png"),LANG(TOOLBAR_DESC_EDIT_REDO,"Editar -> Rehacer"));
		if (config->Toolbars.edit.copy) utils->AddTool(toolbar_edit,wxID_COPY,LANG(TOOLBAR_CAPTION_EDIT_COPY,"Copiar"),ipre+_T("copiar.png"),LANG(TOOLBAR_DESC_EDIT_COPY,"Editar -> Copiar"));
		if (config->Toolbars.edit.cut) utils->AddTool(toolbar_edit,wxID_CUT,LANG(TOOLBAR_CAPTION_EDIT_CUT,"Cortar"),ipre+_T("cortar.png"),LANG(TOOLBAR_DESC_EDIT_CUT,"Editar -> Cortar"));
		if (config->Toolbars.edit.paste) utils->AddTool(toolbar_edit,wxID_PASTE,LANG(TOOLBAR_CAPTION_EDIT_PASTE,"Pegar"),ipre+_T("pegar.png"),LANG(TOOLBAR_DESC_EDIT_PASTE,"Editar -> Pegar"));
		if (config->Toolbars.edit.move_up) utils->AddTool(toolbar_edit,mxID_EDIT_TOGGLE_LINES_UP,LANG(TOOLBAR_CAPTION_EDIT_LINES_UP,"Mover Hacia Arriba"),ipre+_T("toggleLinesUp.png"),LANG(TOOLBAR_DESC_EDIT_LINES_UP,"Editar -> Mover Hacia Arriba"));
		if (config->Toolbars.edit.move_down) utils->AddTool(toolbar_edit,mxID_EDIT_TOGGLE_LINES_DOWN,LANG(TOOLBAR_CAPTION_EDIT_LINES_DOWN,"Mover Hacia Abajo"),ipre+_T("toggleLinesDown.png"),LANG(TOOLBAR_DESC_EDIT_LINES_DOWN,"Editar -> Mover Hacia Abajo"));
		if (config->Toolbars.edit.duplicate) utils->AddTool(toolbar_edit,mxID_EDIT_DUPLICATE_LINES,LANG(TOOLBAR_CAPTION_EDIT_DUPLICATE_LINES,"Duplicar Linea(s)"),ipre+_T("duplicarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DUPLICATE_LINES,"Editar -> Duplicar Linea(s)"));
		if (config->Toolbars.edit.delete_lines) utils->AddTool(toolbar_edit,mxID_EDIT_DELETE_LINES,LANG(TOOLBAR_CAPTION_EDIT_DELETE_LINES,"Eliminar Linea(s)"),ipre+_T("borrarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DELETE_LINES,"Editar -> Eliminar Linea(s)"));
//	toolbar_edit->AddSeparator();
		if (config->Toolbars.edit.goto_line) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO,LANG(TOOLBAR_CAPTION_EDIT_GOTO_LINE,"Ir a Linea..."),ipre+_T("irALinea.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_LINE,"Editar -> Ir a linea..."));
		if (config->Toolbars.edit.goto_class) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_FUNCTION,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FUNCTION,"Ir a Clase/Metodo/Funcion..."),ipre+_T("irAFuncion.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FUNCTION,"Editar -> Ir a Clase/Metodo/Funcion..."));
		if (config->Toolbars.edit.goto_file) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_FILE,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FILE,"Ir a Archivo..."),ipre+_T("irAArchivo.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FILE,"Editar -> Ir a Archivo..."));
		if (config->Toolbars.edit.find) utils->AddTool(toolbar_edit,mxID_EDIT_FIND,LANG(TOOLBAR_CAPTION_EDIT_FIND,"Buscar..."),ipre+_T("buscar.png"),LANG(TOOLBAR_DESC_EDIT_FIND,"Editar -> Buscar..."));
		if (config->Toolbars.edit.find_prev) utils->AddTool(toolbar_edit,mxID_EDIT_FIND_PREV,LANG(TOOLBAR_CAPTION_EDIT_FIND_PREV,"Buscar anterior"),ipre+_T("buscarAnterior.png"),LANG(TOOLBAR_DESC_EDIT_FIND_PREV,"Editar -> Buscar Anterior"));
		if (config->Toolbars.edit.find_next) utils->AddTool(toolbar_edit,mxID_EDIT_FIND_NEXT,LANG(TOOLBAR_CAPTION_EDIT_FIND_NEXT,"Buscar siguiente"),ipre+_T("buscarSiguiente.png"),LANG(TOOLBAR_DESC_EDIT_FIND_NEXT,"Editar -> Buscar Siguiente"));
		if (config->Toolbars.edit.replace) utils->AddTool(toolbar_edit,mxID_EDIT_REPLACE,LANG(TOOLBAR_CAPTION_EDIT_REPLACE,"Reemplazar..."),ipre+_T("reemplazar.png"),LANG(TOOLBAR_DESC_EDIT_REPLACE,"Editar -> Reemplazar..."));
		
		if (config->Toolbars.edit.inser_header) utils->AddTool(toolbar_edit,mxID_EDIT_INSERT_HEADER,LANG(TOOLBAR_CAPTION_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),ipre+_T("insertarInclude.png"),LANG(TOOLBAR_DESC_EDIT_INSERT_INCLUDE,"Editar -> Insertar #include Correspondiente"));
		if (config->Toolbars.edit.comment) utils->AddTool(toolbar_edit,mxID_EDIT_COMMENT,LANG(TOOLBAR_CAPTION_EDIT_COMMENT,"Comentar"),ipre+_T("comentar.png"),LANG(TOOLBAR_DESC_EDIT_COMMNENT,"Editar -> Comentar"));
		if (config->Toolbars.edit.uncomment) utils->AddTool(toolbar_edit,mxID_EDIT_UNCOMMENT,LANG(TOOLBAR_CAPTION_EDIT_UNCOMMENT,"Descomentar"),ipre+_T("descomentar.png"),LANG(TOOLBAR_DESC_EDIT_UNCOMMENT,"Editar -> Descomentar"));
		if (config->Toolbars.edit.indent) utils->AddTool(toolbar_edit,mxID_EDIT_INDENT,LANG(TOOLBAR_CAPTION_EDIT_INDENT,"Indentar"),ipre+_T("indent.png"),LANG(TOOLBAR_DESC_EDIT_INDENT,"Editar -> Indentar"));
		if (config->Toolbars.edit.select_block) utils->AddTool(toolbar_edit,mxID_EDIT_BRACEMATCH,LANG(TOOLBAR_CAPTION_EDIT_BRACEMATCH,"Seleccionar Bloque"),ipre+_T("mostrarLlave.png"),LANG(TOOLBAR_DESC_EDIT_BRACEMATCH,"Editar -> Seleccionar Bloque"));
		if (config->Toolbars.edit.select_all) utils->AddTool(toolbar_edit,wxID_SELECTALL,LANG(TOOLBAR_CAPTION_EDIT_SELECT_ALL,"Seleccionar Todo"),ipre+_T("seleccionarTodo.png"),LANG(TOOLBAR_DESC_EDIT_SELECT_ALL,"Editar -> Seleccionar Todo"));
		if (config->Toolbars.edit.toggle_user_mark) utils->AddTool(toolbar_edit,mxID_EDIT_MARK_LINES,LANG(TOOLBAR_CAPTION_EDIT_HIGHLIGHT_LINES,"Resaltar Linea(s)/Quitar Resaltado"),ipre+_T("marcar.png"),LANG(TOOLBAR_DESC_EDIT_HIGHLIGHT_LINES,"Editar -> Resaltar Linea(s)/Quitar Resaltado"));
		if (config->Toolbars.edit.find_user_mark) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_MARK,LANG(TOOLBAR_CAPTION_EDIT_FIND_HIGHLIGHTS,"Buscar Resaltado"),ipre+_T("irAMarca.png"),LANG(TOOLBAR_DESC_EDIT_FIND_HIGHLIGHTS,"Editar -> Buscar Resaltado"));
		if (config->Toolbars.edit.list_user_marks) utils->AddTool(toolbar_edit,mxID_EDIT_LIST_MARKS,LANG(TOOLBAR_CAPTION_EDIT_LIST_HIGHLIGHTS,"Listar Lineas Resaltadas"),ipre+_T("listarMarcas.png"),LANG(TOOLBAR_DESC_EDIT_LIST_HIGHLIGHTS,"Editar -> Listar Lineas Resaltadas"));
		if (config->Toolbars.edit.autocomplete) utils->AddTool(toolbar_edit,mxID_EDIT_FORCE_AUTOCOMPLETE,LANG(TOOLBAR_CAPTION_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),ipre+_T("autocompletar.png"),LANG(TOOLBAR_DESC_EDIT_FORCE_AUTOCOMPLETE,"Editar -> Autocompletar"));
		
		_aux_ctb_realize(toolbar_edit);
	}
	
	if (!wich_one || wich_one==toolbar_debug) {
		if (config->Toolbars.debug.start) utils->AddTool(toolbar_debug,mxID_DEBUG_RUN,LANG(TOOLBAR_CAPTION_DEBUG_START,"Comenzar/Continuar Depuracion"),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_DEBUG_START,"Depurar -> Comenzar/Continuar"));
		if (config->Toolbars.debug.pause) utils->AddTool(toolbar_debug,mxID_DEBUG_PAUSE,LANG(TOOLBAR_CAPTION_DEBUG_PAUSE,"Interrumpir Ejecucion en Depuracion"),ipre+_T("pausar.png"),LANG(TOOLBAR_DESC_DEBUG_PAUSE,"Depurar -> Interrumpir"));
		if (config->Toolbars.debug.stop) utils->AddTool(toolbar_debug,mxID_DEBUG_STOP,LANG(TOOLBAR_CAPTION_DEBUG_STOP,"Detener Depuracion"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_DEBUG_STOP,"Depurar -> Detener"));
		if (config->Toolbars.debug.step_in) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_IN,LANG(TOOLBAR_CAPTION_DEBUG_STEP_IP,"Step In"),ipre+_T("step_in.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_IN,"Depurar -> Step In"));
		if (config->Toolbars.debug.step_over) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_OVER,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OVER,"Step Over"),ipre+_T("step_over.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OVER,"Depurar -> Step Over"));
		if (config->Toolbars.debug.step_out) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_OUT,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OUT,"Step Out"),ipre+_T("step_out.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OUT,"Depurar -> Step Out"));
		if (config->Toolbars.debug.run_until) utils->AddTool(toolbar_debug,mxID_DEBUG_RUN_UNTIL,LANG(TOOLBAR_CAPTION_DEBUG_RUN_UNTIL,"Ejecutar Hasta El Cursor"),ipre+_T("run_until.png"),LANG(TOOLBAR_DESC_DEBUG_RUN_UNTIL,"Depurar -> Ejecutar Hasta el Cursos"));
		if (config->Toolbars.debug.function_return) utils->AddTool(toolbar_debug,mxID_DEBUG_RETURN,LANG(TOOLBAR_CAPTION_DEBUG_RETURN,"Return"),ipre+_T("return.png"),LANG(TOOLBAR_DESC_DEBUG_RETURN,"Depurar -> Return"));
		if (config->Toolbars.debug.jump) utils->AddTool(toolbar_debug,mxID_DEBUG_JUMP,LANG(TOOLBAR_CAPTION_DEBUG_JUMP,"Continuar desde aqui"),ipre+_T("debug_jump.png"),LANG(TOOLBAR_DESC_DEBUG_JUMP,"Depurar -> Continuar Desde Aqui"));
#if !defined(_WIN32) && !defined(__WIN32__)
		if (config->Toolbars.debug.enable_inverse_exec)  utils->AddTool(toolbar_debug,mxID_DEBUG_ENABLE_INVERSE_EXEC,LANG(TOOLBAR_CAPTION_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras"),ipre+_T("reverse_enable.png"),LANG(TOOLBAR_DESC_DEBUG_ENABLE_INVERSE,"Depurar -> Habilitar Ejecucion Hacia Atras"),wxITEM_CHECK);
		if (config->Toolbars.debug.inverse_exec)  utils->AddTool(toolbar_debug,mxID_DEBUG_INVERSE_EXEC,LANG(TOOLBAR_CAPTION_DEBUG_INVERSE,"Ejecutar Hacia Atras"),ipre+_T("reverse_toggle.png"),LANG(TOOLBAR_DESC_DEBUG_INVERSE,"Depurar -> Ejecutar Hacia Atras..."),wxITEM_CHECK);
#endif
		if (config->Toolbars.debug.inspections) utils->AddTool(toolbar_debug,mxID_DEBUG_INSPECT,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
		if (config->Toolbars.debug.backtrace) utils->AddTool(toolbar_debug,mxID_DEBUG_BACKTRACE,LANG(TOOLBAR_CAPTION_BACKTRACE,"Trazado Inverso"),ipre+_T("backtrace.png"),LANG(TOOLBAR_DESC_BACKTRACE,"Depurar -> Trazado Inverso"));
		if (config->Toolbars.debug.threadlist) utils->AddTool(toolbar_debug,mxID_DEBUG_THREADLIST,LANG(TOOLBAR_CAPTION_THREADLIST,"Hilos de Ejecucion"),ipre+_T("threadlist.png"),LANG(TOOLBAR_DESC_THREADLIST,"Depurar -> Hilos de Ejecucion"));
		
		if (config->Toolbars.debug.break_toggle) utils->AddTool(toolbar_debug,mxID_DEBUG_TOGGLE_BREAKPOINT,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		if (config->Toolbars.debug.break_options) utils->AddTool(toolbar_debug,mxID_DEBUG_BREAKPOINT_OPTIONS,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		if (config->Toolbars.debug.break_list) utils->AddTool(toolbar_debug,mxID_DEBUG_LIST_BREAKPOINTS,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		if (config->Toolbars.debug.log_panel)  utils->AddTool(toolbar_debug,mxID_DEBUG_LOG_PANEL,LANG(TOOLBAR_CAPTION_DEBUG_SHOW_LOG_PANEL,"Mostrar Mensajes del Depurador"),ipre+_T("debug_log_panel.png"),LANG(TOOLBAR_DESC_DEBUG_SHOW_LOG_PANEL,"Depurar -> Mostrar Mensajes del Depurador..."));
		
		_aux_ctb_realize(toolbar_debug);
	}
	
	if (!wich_one || wich_one==toolbar_run) {
		if (config->Toolbars.run.compile) utils->AddTool(toolbar_run,mxID_RUN_COMPILE,LANG(TOOLBAR_CAPTION_RUN_COMPILE,"Compilar"),ipre+_T("compilar.png"),LANG(TOOLBAR_DESC_RUN_COMPILE,"Ejecucion -> Compilar"));
		if (config->Toolbars.run.run) utils->AddTool(toolbar_run,mxID_RUN_RUN,LANG(TOOLBAR_CAPTION_RUN_RUN,"Guardar, Compilar y Ejecutar..."),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_RUN_RUN,"Ejecucion -> Guardar, Compilar y Ejecutar"));
		if (config->Toolbars.run.run_old) utils->AddTool(toolbar_run,mxID_RUN_RUN_OLD,LANG(TOOLBAR_CAPTION_RUN_OLD,"Ejecutar Sin Recompilar..."),ipre+_T("ejecutar_old.png"),LANG(TOOLBAR_DESC_RUN_OLD,"Ejecucion -> Ejecutar Sin Recompilar"));
		if (config->Toolbars.run.stop) utils->AddTool(toolbar_run,mxID_RUN_STOP,LANG(TOOLBAR_CAPTION_RUN_STOP,"Detener"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_RUN_STOP,"Ejecucion -> Detener"));
		if (config->Toolbars.run.clean) utils->AddTool(toolbar_run,mxID_RUN_CLEAN,LANG(TOOLBAR_CAPTION_RUN_CLEAN,"Limpiar"),ipre+_T("limpiar.png"),LANG(TOOLBAR_DESC_RUN_CLEAN,"Ejecucion -> Limpiar"));
		if (config->Toolbars.run.options) utils->AddTool(toolbar_run,mxID_RUN_CONFIG,LANG(TOOLBAR_CAPTION_RUN_OPTIONS,"Opciones de compilacion y ejecucion..."),ipre+_T("opciones.png"),LANG(TOOLBAR_DESC_RUN_OPTIONS,"Ejecucion -> Opciones..."));
		if (config->Toolbars.run.debug) utils->AddTool(toolbar_run,mxID_DEBUG_RUN,LANG(TOOLBAR_CAPTION_DEBUG_RUN,"Depurar"),ipre+_T("depurar.png"),LANG(TOOLBAR_DESC_DEBUG_RUN,"Depurar -> Iniciar"));
#if !defined(__WIN32__)		
		if (config->Toolbars.run.debug_attach) utils->AddTool(toolbar_run,mxID_DEBUG_ATTACH,LANG(TOOLBAR_CAPTION_DEBUG_ATTACH,"Attachear Depurador"),ipre+_T("debug_attach.png"),LANG(TOOLBAR_DESC_DEBUG_ATTACH,"Depurar -> Adjuntar"));
		if (config->Toolbars.run.load_core_dump) utils->AddTool(toolbar_run,mxID_DEBUG_CORE_DUMP,LANG(TOOLBAR_CAPTION_DEBUG_CORE_DUMP,"Cargar Volcado de Memoria"),ipre+_T("core_dump.png"),LANG(TOOLBAR_DESC_DEBUG_CORE_DUMP,"Depurar -> Cargar Volcado de Memoria"));
#endif
		if (config->Toolbars.run.break_toggle) utils->AddTool(toolbar_run,mxID_DEBUG_TOGGLE_BREAKPOINT,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		if (config->Toolbars.run.break_options) utils->AddTool(toolbar_run,mxID_DEBUG_BREAKPOINT_OPTIONS,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		if (config->Toolbars.run.break_list) utils->AddTool(toolbar_run,mxID_DEBUG_LIST_BREAKPOINTS,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		if (config->Toolbars.run.inspections) utils->AddTool(toolbar_run,mxID_DEBUG_INSPECT,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
		_aux_ctb_realize(toolbar_run);
	}
	
	if (wich_one) {
		aui_manager.GetPane(wich_one).BestSize(wich_one->GetBestSize());
	} else {
		toolbar_find = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_find->SetToolBitmapSize(wxSize(ICON_SIZE,ICON_SIZE));
		toolbar_find_text = new wxTextCtrl(toolbar_find,mxID_TOOLBAR_FIND,wxEmptyString,wxDefaultPosition,wxSize(100,20),wxTE_PROCESS_ENTER);
		toolbar_find->AddControl( toolbar_find_text );
		toolbar_find_text->SetToolTip(LANG(TOOLBAR_FIND_TEXT,"Texto a Buscar"));
		utils->AddTool(toolbar_find,mxID_EDIT_TOOLBAR_FIND,LANG(TOOLBAR_FIND_CAPTION,"Busqueda Rapida"),ipre+_T("buscar.png"),LANG(TOOLBAR_FIND_BUTTON,"Buscar siguiente"));
		_aux_ctb_realize(toolbar_find);
		aui_manager.AddPane(toolbar_find, wxAuiPaneInfo().Name("toolbar_find").Caption(LANG(CAPTION_TOOLBAR_FIND,"Busqueda")).ToolbarPane().Hide());
		
		toolbar_status = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_status->SetToolBitmapSize(wxSize(ICON_SIZE,ICON_SIZE));
		toolbar_status->AddControl( toolbar_status_text = new wxStaticText(toolbar_status,wxID_ANY,"",wxDefaultPosition,wxSize(2500,20)) );
		toolbar_status_text->SetForegroundColour(wxColour(_T("Z DARK BLUE")));
		_aux_ctb_realize(toolbar_status);
		aui_manager.AddPane(toolbar_status, wxAuiPaneInfo().Name("toolbar_status").Caption(LANG(CAPTION_TOOLBAR_STATUS,"Status Toolbar")).ToolbarPane().Top().Hide());
		
		toolbar_diff = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_diff->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_PREV,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_PREV,"Diferencia Anterior"),ipre+_T("diff_prev.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_PREV,"Herramientas -> Comparar Archivos -> Ir a Diferencia Anterior"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_NEXT,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_NEXT,"Siguiente Diferencia"),ipre+_T("diff_next.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_NEXT,"Herramientas -> Comparar Archivos -> Ir a Siguiente Diferencia"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_APPLY,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_DISCARD,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_CLEAR,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
		_aux_ctb_realize(toolbar_diff);
		aui_manager.AddPane(toolbar_diff, wxAuiPaneInfo().Name("toolbar_diff").Caption(LANG(CAPTION_TOOLBAR_DIFF,"Diff")).ToolbarPane().Top().Float().LeftDockable(false).RightDockable(false).Hide());
		
		SortToolbars(false);
	}
}

void mxMainWindow::UpdateInHistory(wxString filename) {
	wxString ipre=DIR_PLUS_FILE("16","recent");
	bool is_project = wxFileName(filename).GetExt().CmpNoCase(PROJECT_EXT)==0;
	int i;
	wxString *cfglast = is_project?config->Files.last_project:config->Files.last_source;
	wxMenuItem **mnihistory = is_project?menu.file_project_history:menu.file_source_history;
	wxMenu *mnurecent = is_project?menu.file_project_recent:menu.file_source_recent;
	int history_id = is_project?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0;
	
	for (i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (cfglast[i][0]==0 || cfglast[i]==filename)
			break;
	if (i==CM_HISTORY_MAX_LEN) 
		i--;
	for (int j=i;j>0;j--)
		cfglast[j]=cfglast[j-1];
	cfglast[0]=filename;
	// actualizar el menu archivo
	for (i=0;i<config->Init.history_len;i++) {
		wxString icon_fname=wxString("recent")<<i<<".png";
#if defined(_WIN32) || defined(__WIN32__)
		if (mnihistory[i])
			mnurecent->Remove(mnihistory[i]);
		if (cfglast[i][0])
			mnihistory[i] = utils->AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],icon_fname,i);
#else
		if (cfglast[i][0])
			if (mnihistory[i]) {
				icon_fname=SKIN_FILE_OPT(icon_fname);
				if (wxFileName::FileExists(icon_fname))
					mnihistory[i]->SetBitmap(wxBitmap(icon_fname,wxBITMAP_TYPE_PNG));
				mnurecent->SetLabel(mnihistory[i]->GetId(),cfglast[i]);
			} else {
				mnihistory[i] = utils->AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],icon_fname,i);
			}
			else
				break;
#endif
	}
}


void mxMainWindow::UpdateCustomTools(bool for_project) {
	
	wxToolBar *toolbar=for_project?toolbar_project:toolbar_tools;
	int count=(for_project?MAX_PROJECT_CUSTOM_TOOLS:MAX_CUSTOM_TOOLS);
	cfgCustomTool *tools=(for_project?project->custom_tools:config->CustomTools);
	
	if (!for_project) {
		wxString ipre=DIR_PLUS_FILE("16","customTool");
		for (int i=0;i<count;i++) {
			if (menu.tools_custom_item[i])
				menu.tools_custom_menu->Remove(menu.tools_custom_item[i]);
			menu.tools_custom_item[i]=NULL;
		}
		int c=0;
		for (int i=0;i<count;i++) {
			if (tools[i].name.Len() && tools[i].command.Len())
				menu.tools_custom_item[i] = utils->AddItemToMenu(menu.tools_custom_menu, mxID_CUSTOM_TOOL_0+i,tools[i].name,"",tools[i].command,SKIN_FILE(wxString(ipre)<<i<<".png"),c++);
		}
	}
	
	if (toolbar) CreateToolbars(toolbar);
	if (main_window) main_window->aui_manager.Update();
}

void mxMainWindow::SetMenusForDebugging(bool debug_mode) {
	menu.debug_stop->Enable(debug_mode);
	menu.debug_return->Enable(debug_mode);
	menu.debug_step_out->Enable(debug_mode);
	menu.debug_step_over->Enable(debug_mode);
	menu.debug_step_in->Enable(debug_mode);
	menu.debug_run_until->Enable(debug_mode);
	menu.debug_jump->Enable(debug_mode);
	menu.debug_pause->Enable(debug_mode);
	menu.debug_patch->Enable(debug_mode);
	menu.debug_gdb_command->Enable(debug_mode);
	menu.debug_send_signal->Enable(debug_mode);
	menu.debug_set_signals->Enable(debug_mode);
#ifndef __WIN32__
	menu.debug_attach->Enable(!debug_mode);
	menu.debug_enable_inverse_execution->Enable(debug_mode);
	menu.debug_inverse_execution->Enable(false);
	menu.debug_load_core_dump->Enable(!debug_mode);
	menu.debug_generate_core_dump->Enable(debug_mode);
#endif
}

void mxMainWindow::SetMenusForProject(bool project_mode) {
	menu.file_open->SetItemLabel(project_mode?LANG(MENUITEM_FILE_OPEN_ON_PROJECT,"&Abrir/Agregar al proyecto..."):LANG(MENUITEM_FILE_OPEN,"&Abrir..."));
	menu.view_toolbar_project->Enable(project_mode);
	menu.tools_doxygen->Enable(project_mode);
	menu.tools_cppcheck->Enable(project_mode);
	menu.tools_wxfb->Enable(project_mode);
	menu.tools_wxfb_config->Enable(project_mode);
	menu.tools_wxfb_update_inherit->Enable(false);
	menu.tools_project_tools->Enable(project_mode);
	menu.tools_proy_graph->Enable(project_mode);
	menu.tools_proy_graph->Enable(project_mode);
	menu.tools_makefile->Enable(project_mode);
	menu.tools_stats->Enable(project_mode);
	menu.file_save_project->Enable(project_mode);
	menu.file_close_project->Enable(project_mode);
	menu.file_project_config->Enable(project_mode);
}

