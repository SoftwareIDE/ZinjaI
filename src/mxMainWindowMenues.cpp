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
#include "MenusAndToolsConfig.h"

wxMenu *menu_file = new wxMenu;
wxMenu *menu_edit = new wxMenu;
wxMenu *menu_view = new wxMenu;
wxMenu *menu_run = new wxMenu;
wxMenu *menu_debug = new wxMenu;
wxMenu *menu_tools = new wxMenu;

/**
* @brief Crea los menúes de la ventana principal y los configura en el modo inicial (no debug, no project)
**/
void mxMainWindow::CreateMenues() {
	
	wxMenuBar *menu_bar = new wxMenuBar;
	
	SetMenuBar(menu_bar);

	// create regular menues
	menu_data = new MenusAndToolsConfig(menu_bar);
	menu_data->LoadMenuData();
	menu_data->CreateMenues();
	
	// create some special submenues
	UpdateCustomTools(false);
	wxString ipre=DIR_PLUS_FILE("16","recent");
	for(int k=0;k<2;k++) { 
		wxString *cfglast = k==0?config->Files.last_project:config->Files.last_source;
		wxMenuItem **mnihistory = k==0?menu_data->file_project_history:menu_data->file_source_history;
		wxMenu *mnurecent = menu_data->GetMenu(k==0?mxID_FILE_PROJECT_RECENT:mxID_FILE_SOURCE_RECENT);
		int history_id = k==0?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0;
		for (int i=0;i<config->Init.history_len;i++)
			if (!cfglast[i].IsEmpty()) 
				mnihistory[i] = utils->AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],wxString(ipre+"recent")<<i<<(".png"),i);
	}
	
	// set items state
	menu_data->SetDebugMode(false);
	menu_data->SetProjectMode(false);

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

void mxMainWindow::UpdateInHistory(wxString filename, bool is_project) {
	wxString ipre=DIR_PLUS_FILE("16","recent");
	wxString *cfglast = is_project?config->Files.last_project:config->Files.last_source;
	wxMenuItem **mnihistory = is_project?menu_data->file_project_history:menu_data->file_source_history;
	wxMenu *mnurecent = menu_data->GetMenu(is_project?mxID_FILE_PROJECT_RECENT:mxID_FILE_SOURCE_RECENT);
	int history_id = is_project?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0, i;
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
	
	
	wxMenu *menu_custom_tools = menu_data->GetMenu(mxID_TOOLS_CUSTOM_TOOLS);
	if (!for_project) {
		wxString ipre=DIR_PLUS_FILE("16","customTool");
		for (int i=0;i<count;i++) {
			if (menu_data->tools_custom_item[i])
				menu_custom_tools->Remove(menu_data->tools_custom_item[i]);
			menu_data->tools_custom_item[i]=NULL;
		}
		int c=0;
		for (int i=0;i<count;i++) {
			if (tools[i].name.Len() && tools[i].command.Len())
				menu_data->tools_custom_item[i] = utils->AddItemToMenu(menu_custom_tools, mxID_CUSTOM_TOOL_0+i,tools[i].name,"",tools[i].command,SKIN_FILE(wxString(ipre)<<i<<".png"),c++);
		}
	}
	
	if (toolbar) CreateToolbars(toolbar);
	if (main_window) main_window->aui_manager.Update();
}
