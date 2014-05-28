#include "ConfigManager.h"

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/textfile.h>

#include "mxUtils.h"
#include "version.h"
#include "mxMessageDialog.h"
#include "error_recovery.h"
#include "ProjectManager.h"
#include "mxPreferenceWindow.h"
#include "Language.h"
#include <wx/dir.h>
#include "mxMainWindow.h"
#include "mxColoursEditor.h"
#include "Toolchain.h"

ConfigManager *config;

ConfigManager::ConfigManager(wxString a_path) {
	config=this;
	zinjai_dir = a_path;
	color_theme::Init();
	LoadDefaults();
	LoadToolBarsDefaults();
	Toolchain::LoadToolchains();
	er_init(home_dir.char_str());
	first_run = !Load();
	// cargar archivo de internacionalizacion
	if (first_run) {
		wxDir dir("lang");
		wxString spec="*.pre", filename;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
		wxArrayString langs;
		while ( cont ) {
			langs.Add(filename.BeforeLast('.'));
			cont = dir.GetNext(&filename);
		}
		if (langs.Index("spanish")==wxNOT_FOUND) langs.Add("spanish");
		if (langs.GetCount()>1) {
			wxString newlang = wxGetSingleChoice("Select a Language:\nSeleccione el idioma:","ZinjaI",langs);
			if (newlang.Len()) Init.language_file=newlang;
		}
	}
	if (Init.language_file!="spanish")
		if (LANGERR_OK!=load_language(DIR_PLUS_FILE("lang",Init.language_file).c_str(),DIR_PLUS_FILE(home_dir,"lang_cache").c_str()))
		mxMessageDialog(NULL,"No se pudo cargar el diccionario del idioma seleccionado.\n"
						"El sistema utilizará el predeterminado (spanish).\n"
						"Could not load language file. System will use default (spanish).","ZinjaI",mxMD_OK|mxMD_WARNING).ShowModal();
		
	RecalcStuff();
}

void ConfigManager::DoInitialChecks() {
#if defined(__WIN32__)
#else
	// elegir un explorador de archivos
	if (Files.explorer_command=="<<sin configurar>>") { // tratar de detectar automaticamente un terminal adecuado
		if (utils->GetOutput("dolphin --version").Len())
			Files.explorer_command = "dolphin";
		if (utils->GetOutput("konqueror --version").Len())
			Files.explorer_command = "konqueror";
		else if (utils->GetOutput("nautilus --version").Len())
			Files.explorer_command = "nautilus";
		else if (utils->GetOutput("thunar --version").Len())
			Files.explorer_command = "thunar";
	} 
	// elegir un terminal
	if (Files.terminal_command=="<<sin configurar>>") { // tratar de detectar automaticamente un terminal adecuado
		LinuxTerminalInfo::Init();
		for(int i=0;i<LinuxTerminalInfo::count;i++) { 
			if (LinuxTerminalInfo::list[i].Test()) {
				Files.terminal_command = LinuxTerminalInfo::list[i].run_command;
				if (LinuxTerminalInfo::list[i].warning) {
					wxMessageBox(LANG1(CONFIG_TERMINAL_WARNING,"La aplicacion terminal que se ha encontrado instalada\n"
						"es <{1}>. Algunas versiones de esta terminal pueden generar\n"
						"problemas al intentar ejecutar un programa o proyecto. Si no logra\n"
						"ejecutar correctamente desde ZinjaI ninguno de los programas/proyectos\n"
						"que compile, intente configurar otra terminal (menú Archivo->Preferencias...\n"
						"pestaña \"Rutas 1\", opción \"Comando de la terminal\", la terminal\n"
						"recomendada es xterm).",LinuxTerminalInfo::list[i].name),LANG(CONFIG_TERMINAL,"Terminal de ejecucion"));
				}
			}
			break;
		}
		if (Files.terminal_command=="<<sin configurar>>") {
			mxMessageDialog(NULL,LANG(CONFIG_NO_TERMINAL_FOUND,"No se ha encontrado una terminal conocida.\n"
			                "Instale xterm, konsole, lxterminal o gnome-terminal;\n"
							"luego configure el parametro \"Comando del Terminal\"\n"
							"en la seccion \"Rutas 2\" del cuadro de \"Preferencias\"."),LANG(CONFIG_TERMINAL,"Terminal de ejecucion"),mxMD_OK|mxMD_WARNING).ShowModal();
		}
	}
	// verificar si hay compilador
	if (!Init.compiler_seen && !utils->GetOutput("g++ --version").Len()) {
		// try to use clang if g++ not found
		wxArrayString toolchains;
		Toolchain::GetNames(toolchains,true);
		if (toolchains.Index("clang")!=wxNOT_FOUND && utils->GetOutput("clang --version").Len()) {
			Files.toolchain="clang"; Toolchain::SelectToolchain(); 
		} else {
			wxString chk_message; 
			if (Files.terminal_command!="<<sin configurar>>" && utils->GetOutput("apt-get --version").Len())
				chk_message=LANG(CONFIG_APTGET_BUILD_ESSENTIAL,"Intentar instalar ahora");
			int ans = mxMessageDialog(NULL,LANG(CONFIG_COMPILER_NOT_FOUND,"No se ha encontrado un compilador para C++ (g++ o clang). Debe instalarlo\n"
				"con el gestor de paquetes que corresponda a su distribucion\n"
				"(apt-get, yum, yast, installpkg, etc.)"),LANG(CONFIG_COMPILER,"Compilador C++"),mxMD_OK|mxMD_WARNING,chk_message,true).ShowModal();
			if (ans&mxMD_CHECKED) wxExecute(Files.terminal_command+"sudo apt-get install build-essential");
		}
	} else Init.compiler_seen=true;
	// verificar si hay depurador
	if (!Init.debugger_seen && !utils->GetOutput("gdb --version").Len()) {
		wxString chk_message; 
		if (Files.terminal_command!="<<sin configurar>>" && utils->GetOutput("apt-get --version").Len())
			chk_message=LANG(CONFIG_APTGET_BUILD_ESSENTIAL,"Intentar instalar ahora");
		int ans = mxMessageDialog(NULL,LANG(CONFIG_DEBUGGER_NOT_FOUND,"No se ha encontrado el depurador (gdb). Debe instalarlo con\n"
		                "el gestor de paquetes que corresponda a su distribucion\n"
		                "(apt-get, yum, yast, installpkg, etc.)"),LANG(CONFIG_DEBUGGER,"Depurador"),mxMD_OK|mxMD_WARNING,chk_message,true).ShowModal();
		if (ans&mxMD_CHECKED) wxExecute(Files.terminal_command+"sudo apt-get install build-essential");
	}
	else Init.debugger_seen=true;
#endif	
	Toolchain::SelectToolchain();
}
	
bool ConfigManager::Load() {
	wxTextFile fil(filename);
	if (!fil.Exists()) return false;
	fil.Open();
	wxString section, key, value;
	long l;
	wxArrayString last_files; // para compatibilidad hacia atras, guarda el historial unificado y despues lo divide
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
//		wxYield();
		if (str[0]=='#') {
			continue;
		} else if (str[0]=='[') {
			section=str.AfterFirst('[').BeforeFirst(']');
		
		} else {
			key=str.BeforeFirst('=');
			value=str.AfterFirst('=');
			if (section=="Styles") {
				CFG_INT_READ_DN("print_size",Styles.print_size);
				else CFG_INT_READ_DN("font_size",Styles.font_size);
				else CFG_GENERIC_READ_DN("colour_theme",Init.colour_theme);
				else if (key=="dark") { if (utils->IsTrue(value)) Init.colour_theme="inverted.zcs";	}
				
			} else if (section=="Source") {
				CFG_INT_READ_DN("tabWidth",Source.tabWidth);
				else CFG_INT_READ_DN("alignComments",Source.alignComments);
				else CFG_BOOL_READ_DN("tabUseSpaces",Source.tabUseSpaces);
				else CFG_BOOL_READ_DN("indentPaste",Source.indentPaste);
				else CFG_BOOL_READ_DN("smartIndent",Source.smartIndent);
				else CFG_BOOL_READ_DN("bracketInsertion",Source.bracketInsertion);
				else CFG_BOOL_READ_DN("syntaxEnable",Source.syntaxEnable);
				else CFG_BOOL_READ_DN("foldEnable",Source.foldEnable);
				else CFG_BOOL_READ_DN("indentEnable",Source.indentEnable);
				else CFG_BOOL_READ_DN("whiteSpace",Source.whiteSpace);
				else CFG_BOOL_READ_DN("lineNumber",Source.lineNumber);
				else CFG_BOOL_READ_DN("overType",Source.overType);
				else CFG_BOOL_READ_DN("callTips",Source.callTips);
				else CFG_BOOL_READ_DN("autotextEnabled",Source.autotextEnabled);
				else CFG_BOOL_READ_DN("autocloseStuff",Source.autocloseStuff);
				else CFG_BOOL_READ_DN("toolTips",Source.toolTips);
				else CFG_BOOL_READ_DN("autoCompletion",Source.autoCompletion);
				else CFG_BOOL_READ_DN("avoidNoNewLineWarning",Source.avoidNoNewLineWarning);
			
			} else if (section=="Debug") {
//				CFG_BOOL_READ_DN("autoupdate_backtrace",Debug.autoupdate_backtrace);
				CFG_BOOL_READ_DN("autohide_panels",Debug.autohide_panels);
				else CFG_BOOL_READ_DN("autohide_toolbars",Debug.autohide_toolbars);
				else CFG_BOOL_READ_DN("allow_edition",Debug.allow_edition);
				else CFG_GENERIC_READ_DN("format",Debug.format);
				else CFG_GENERIC_READ_DN("blacklist",Debug.blacklist);
				else CFG_GENERIC_READ_DN("macros_file",Debug.macros_file);
				else CFG_BOOL_READ_DN("compile_again",Debug.compile_again);
				else CFG_BOOL_READ_DN("select_modified_inspections",Debug.select_modified_inspections);
				else CFG_BOOL_READ_DN("raise_main_window",Debug.raise_main_window);
				else CFG_BOOL_READ_DN("always_debug",Debug.always_debug);
				else CFG_BOOL_READ_DN("close_on_normal_exit",Debug.close_on_normal_exit);
				else CFG_BOOL_READ_DN("show_do_that",Debug.show_do_that);
				else CFG_BOOL_READ_DN("auto_solibs",Debug.auto_solibs);
				else CFG_BOOL_READ_DN("readnow",Debug.readnow);
				else CFG_BOOL_READ_DN("inspections_on_right",Debug.inspections_on_right);
				else CFG_BOOL_READ_DN("show_thread_panel",Debug.show_thread_panel);
				else CFG_BOOL_READ_DN("show_log_panel",Debug.show_log_panel);
				else CFG_BOOL_READ_DN("improve_inspections_by_type",Debug.improve_inspections_by_type);
				else if (key=="inspection_improving_template") {
					Debug.inspection_improving_template_from.Add(value.BeforeFirst('|'));
					Debug.inspection_improving_template_to.Add(value.AfterFirst('|'));
				}
				
			} else if (section=="Running") {
				CFG_GENERIC_READ_DN("compiler_options",Running.cpp_compiler_options); //just for backward compatibility
				else CFG_GENERIC_READ_DN("cpp_compiler_options",Running.cpp_compiler_options);
				else CFG_GENERIC_READ_DN("c_compiler_options",Running.c_compiler_options);
				else CFG_BOOL_READ_DN("wait_for_key",Running.wait_for_key);
				else CFG_BOOL_READ_DN("always_ask_args",Running.always_ask_args);
				else CFG_BOOL_READ_DN("check_includes",Running.check_includes);
				else CFG_BOOL_READ_DN("dont_run_headers",Running.dont_run_headers);
			
			} else if (section=="Help") {
//				CFG_GENERIC_READ_DN("quickhelp_index",Help.quickhelp_index);
				CFG_GENERIC_READ_DN("wxhelp_index",Help.wxhelp_index);
//				else CFG_GENERIC_READ_DN("tips_file",Help.tips_file);
//				else CFG_GENERIC_READ_DN("quickhelp_dir",Help.quickhelp_dir);
//				else CFG_GENERIC_READ_DN("guihelp_dir",Help.guihelp_dir);
				else CFG_GENERIC_READ_DN("autocomp_indexes",Help.autocomp_indexes);
//				else CFG_GENERIC_READ_DN("autocomp_dir",Help.autocomp_dir);
				else CFG_INT_READ_DN("min_len_for_completion",Help.min_len_for_completion);
				else CFG_BOOL_READ_DN("show_extra_panels",Help.show_extra_panels);
			
			} else if (section=="Init") {
				CFG_BOOL_READ_DN("left_panels",Init.left_panels);
				else CFG_BOOL_READ_DN("show_beginner_panel",Init.show_beginner_panel);
				else CFG_BOOL_READ_DN("show_welcome",Init.show_welcome);
				else CFG_BOOL_READ_DN("show_tip_on_startup",Init.show_tip_on_startup);
				else CFG_BOOL_READ_DN("show_splash",Init.show_splash);
				else CFG_INT_READ_DN("new_file",Init.new_file);
				else CFG_INT_READ_DN("version",Init.version);
				else CFG_INT_READ_DN("pos_x",Init.pos_x);
				else CFG_INT_READ_DN("pos_y",Init.pos_y);
				else CFG_INT_READ_DN("size_x",Init.size_x);
				else CFG_INT_READ_DN("size_y",Init.size_y);
				else CFG_BOOL_READ_DN("lang_es",Init.lang_es);
				else CFG_BOOL_READ_DN("maximized",Init.maximized);
				else CFG_INT_READ_DN("zinjai_server_port",Init.zinjai_server_port);
//				else CFG_BOOL_READ_DN("load_sharing_server",Init.load_sharing_server);
				else CFG_BOOL_READ_DN("save_project",Init.save_project);
//				else CFG_BOOL_READ_DN("close_files_for_project",Init.close_files_for_project);
				else CFG_BOOL_READ_DN("always_add_extension",Init.always_add_extension);
				else CFG_BOOL_READ_DN("autohide_menus_fs",Init.autohide_menus_fs);
				else CFG_BOOL_READ_DN("autohide_panels_fs",Init.autohide_panels_fs);
				else CFG_BOOL_READ_DN("autohide_toolbars_fs",Init.autohide_toolbars_fs);
				else CFG_BOOL_READ_DN("check_for_updates",Init.check_for_updates);
				else CFG_BOOL_READ_DN("prefer_explorer_tree",Init.prefer_explorer_tree);
				else CFG_BOOL_READ_DN("cppcheck_seen",Init.cppcheck_seen);
#if !defined(__WIN32__)
				else CFG_BOOL_READ_DN("valgrind_seen",Init.valgrind_seen);
				else CFG_BOOL_READ_DN("compiler_seen",Init.compiler_seen);
				else CFG_BOOL_READ_DN("debugger_seen",Init.debugger_seen);
#endif
				else CFG_BOOL_READ_DN("doxygen_seen",Init.doxygen_seen);
				else CFG_BOOL_READ_DN("wxfb_seen",Init.wxfb_seen);
				else CFG_BOOL_READ_DN("show_explorer_tree",Init.show_explorer_tree);
				else CFG_BOOL_READ_DN("graphviz_dot",Init.graphviz_dot);
				else CFG_INT_READ_DN("history_len",Init.history_len);
				else CFG_INT_READ_DN("inherit_num",Init.inherit_num);
//				else CFG_GENERIC_READ_DN("forced_compiler_options",Init.forced_compiler_options);
//				else CFG_GENERIC_READ_DN("forced_linker_options",Init.forced_linker_options);
				else CFG_GENERIC_READ_DN("proxy",Init.proxy);
				else CFG_GENERIC_READ_DN("language_file",Init.language_file);
				else CFG_INT_READ_DN("max_errors",Init.max_errors);
				else CFG_INT_READ_DN("max_jobs",Init.max_jobs);
				else CFG_INT_READ_DN("wrap_mode",Init.wrap_mode);
				else CFG_BOOL_READ_DN("singleton",Init.singleton);
				else CFG_BOOL_READ_DN("stop_compiling_on_error",Init.stop_compiling_on_error);
				else CFG_BOOL_READ_DN("autohide_panels",Init.autohide_panels);
				else CFG_BOOL_READ_DN("beautify_compiler_errors",Init.beautify_compiler_errors);
				else CFG_BOOL_READ_DN("fullpath_on_project_tree",Init.fullpath_on_project_tree);
				else CFG_GENERIC_READ_DN("colour_theme",Init.colour_theme);
				
			} else if (section=="Files") {
				CFG_GENERIC_READ_DN("toolchain",Files.toolchain);
				else CFG_GENERIC_READ_DN("debugger_command",Files.debugger_command);
//#if defined(__WIN32__)
//				else CFG_GENERIC_READ_DN("mingw_dir",Files.mingw_dir);
//#endif
				else CFG_GENERIC_READ_DN("terminal_command",Files.terminal_command);
				else CFG_GENERIC_READ_DN("explorer_command",Files.explorer_command);
				else CFG_GENERIC_READ_DN("default_template",Files.default_template);
				else CFG_GENERIC_READ_DN("default_project",Files.default_project);
				else CFG_GENERIC_READ_DN("autocodes_file",Files.autocodes_file);
				else CFG_GENERIC_READ_DN("skin_dir",Files.skin_dir);
				else CFG_GENERIC_READ_DN("temp_dir",Files.temp_dir);
				else CFG_GENERIC_READ_DN("img_browser",Files.img_browser);
				else CFG_GENERIC_READ_DN("xdot_command",Files.xdot_command);
				else CFG_GENERIC_READ_DN("graphviz_dir",Files.graphviz_dir);
				else CFG_GENERIC_READ_DN("browser_command",Files.browser_command);
				else CFG_GENERIC_READ_DN("cppcheck_command",Files.cppcheck_command);
#if !defined(__WIN32__)
				else CFG_GENERIC_READ_DN("valgrind_command",Files.valgrind_command);
#endif
				else CFG_GENERIC_READ_DN("doxygen_command",Files.doxygen_command);
				else CFG_GENERIC_READ_DN("wxfb_command",Files.wxfb_command);
				else CFG_GENERIC_READ_DN("parser_command",Files.parser_command);
//				else CFG_GENERIC_READ_DN("code_helper",Files.code_helper);
				else CFG_GENERIC_READ_DN("project_folder",Files.project_folder);
				else CFG_GENERIC_READ_DN("last_project_dir",Files.last_project_dir);
				else CFG_GENERIC_READ_DN("last_dir",Files.last_dir);
				else if (key.StartsWith("last_file_")) {
					last_files.Add(value);
				} else if (key.StartsWith("last_source_")) {
					if (key.Mid(12).ToLong(&l) && l>=0 && l<CM_HISTORY_MAX_LEN)
						Files.last_source[l]=value;
				} else if (key.StartsWith("last_project_")	) {
					if (key.Mid(13).ToLong(&l) && l>=0 && l<CM_HISTORY_MAX_LEN)
						Files.last_project[l]=value;
				}
				
			} else if (section=="Columns") {
				if (key.StartsWith("inspections_grid_")	) {
					if (key.Mid(17).ToLong(&l) && l>=0 && l<IG_COLS_COUNT)
						Cols.inspections_grid[l]=utils->IsTrue(value);
				} else if (key.StartsWith("backtrace_grid_")	) {
					if (key.Mid(15).ToLong(&l) && l>=0 && l<BG_COLS_COUNT)
						Cols.backtrace_grid[l]=utils->IsTrue(value);
				}
//				} else if (key.StartsWith("threadlist_grid_")	) {
//					if (key.Mid(15).ToLong(&l) && l>=0 && l<TG_COLS_COUNT)
//						Cols.threadlist_grid[l]=utils->IsTrue(value);
//				}
				
			} else if (section=="CustomTools") {
				if (key.StartsWith("name_")) {
					if (key.Mid(5).ToLong(&l) && l>=0 && l<MAX_CUSTOM_TOOLS)
						CustomTools[l].name=value;
				} else if (key.StartsWith("command_")) {
					if (key.Mid(8).ToLong(&l) && l>=0 && l<MAX_CUSTOM_TOOLS)
						CustomTools[l].command=value;
				} else if (key.StartsWith("workdir_")) {
					if (key.Mid(8).ToLong(&l) && l>=0 && l<MAX_CUSTOM_TOOLS)
						CustomTools[l].workdir=value;
				} else if (key.StartsWith("console_")) {
					if (key.Mid(8).ToLong(&l) && l>=0 && l<MAX_CUSTOM_TOOLS)
						CustomTools[l].console=utils->IsTrue(value);
				} else if (key.StartsWith("on_toolbar_")) {
					if (key.Mid(11).ToLong(&l) && l>=0 && l<MAX_CUSTOM_TOOLS)
						CustomTools[l].on_toolbar=utils->IsTrue(value);
				}
				
			} else if (section=="Toolbars") {
				CFG_INT_READ(Toolbars,icon_size);
				else CFG_BOOL_READ(Toolbars,file.new_file);
				else CFG_BOOL_READ(Toolbars,file.new_project);
				else CFG_BOOL_READ(Toolbars,file.open);
				else CFG_BOOL_READ(Toolbars,file.open_header);
				else CFG_BOOL_READ(Toolbars,file.recent_simple);
				else CFG_BOOL_READ(Toolbars,file.recent_project);
				else CFG_BOOL_READ(Toolbars,file.open_selected);
				else CFG_BOOL_READ(Toolbars,file.save);
				else CFG_BOOL_READ(Toolbars,file.save_as);
				else CFG_BOOL_READ(Toolbars,file.save_all);
				else CFG_BOOL_READ(Toolbars,file.save_project);
				else CFG_BOOL_READ(Toolbars,file.print);
				else CFG_BOOL_READ(Toolbars,file.export_html);
				else CFG_BOOL_READ(Toolbars,file.reload);
				else CFG_BOOL_READ(Toolbars,file.close);
				else CFG_BOOL_READ(Toolbars,file.close_all);
				else CFG_BOOL_READ(Toolbars,file.close_project);
				else CFG_BOOL_READ(Toolbars,file.project_config);
				else CFG_BOOL_READ(Toolbars,edit.undo);
				else CFG_BOOL_READ(Toolbars,edit.redo);
				else CFG_BOOL_READ(Toolbars,edit.cut);
				else CFG_BOOL_READ(Toolbars,edit.copy);
				else CFG_BOOL_READ(Toolbars,edit.paste);
				else CFG_BOOL_READ(Toolbars,edit.move_up);
				else CFG_BOOL_READ(Toolbars,edit.move_down);
				else CFG_BOOL_READ(Toolbars,edit.duplicate);
				else CFG_BOOL_READ(Toolbars,edit.delete_lines);
				else CFG_BOOL_READ(Toolbars,edit.goto_class);
				else CFG_BOOL_READ(Toolbars,edit.goto_line);
				else CFG_BOOL_READ(Toolbars,edit.goto_file);
				else CFG_BOOL_READ(Toolbars,edit.find);
				else CFG_BOOL_READ(Toolbars,edit.find_prev);
				else CFG_BOOL_READ(Toolbars,edit.find_next);
				else CFG_BOOL_READ(Toolbars,edit.replace);
				else CFG_BOOL_READ(Toolbars,edit.inser_header);
				else CFG_BOOL_READ(Toolbars,edit.comment);
				else CFG_BOOL_READ(Toolbars,edit.uncomment);
				else CFG_BOOL_READ(Toolbars,edit.indent);
				else CFG_BOOL_READ(Toolbars,edit.select_block);
				else CFG_BOOL_READ(Toolbars,edit.select_all);
				else CFG_BOOL_READ(Toolbars,edit.toggle_user_mark);
				else CFG_BOOL_READ(Toolbars,edit.find_user_mark);
				else CFG_BOOL_READ(Toolbars,edit.list_user_marks);
				else CFG_BOOL_READ(Toolbars,edit.autocomplete);
				else CFG_BOOL_READ(Toolbars,view.split_view);
				else CFG_BOOL_READ(Toolbars,view.white_space);
				else CFG_BOOL_READ(Toolbars,view.sintax_colour);
				else CFG_BOOL_READ(Toolbars,view.update_symbols);
				else CFG_BOOL_READ(Toolbars,view.explorer_tree);
				else CFG_BOOL_READ(Toolbars,view.project_tree);
				else CFG_BOOL_READ(Toolbars,view.compiler_tree);
				else CFG_BOOL_READ(Toolbars,view.full_screen);
				else CFG_BOOL_READ(Toolbars,view.beginner_panel);
				else CFG_BOOL_READ(Toolbars,view.next_error);
				else CFG_BOOL_READ(Toolbars,view.prev_error);
				else CFG_BOOL_READ(Toolbars,view.fold_all);
				else CFG_BOOL_READ(Toolbars,view.fold_1);
				else CFG_BOOL_READ(Toolbars,view.fold_2);
				else CFG_BOOL_READ(Toolbars,view.fold_3);
				else CFG_BOOL_READ(Toolbars,view.unfold_all);
				else CFG_BOOL_READ(Toolbars,view.unfold_1);
				else CFG_BOOL_READ(Toolbars,view.unfold_2);
				else CFG_BOOL_READ(Toolbars,view.unfold_3);
				else CFG_BOOL_READ(Toolbars,run.run);
				else CFG_BOOL_READ(Toolbars,run.run_old);
				else CFG_BOOL_READ(Toolbars,run.compile);
				else CFG_BOOL_READ(Toolbars,run.stop);
				else CFG_BOOL_READ(Toolbars,run.clean);
				else CFG_BOOL_READ(Toolbars,run.options);
				else CFG_BOOL_READ(Toolbars,run.debug);
#if !defined(__WIN32__)
				else CFG_BOOL_READ(Toolbars,run.debug_attach);
				else CFG_BOOL_READ(Toolbars,run.load_core_dump);
#endif
				else CFG_BOOL_READ(Toolbars,run.break_toggle);
				else CFG_BOOL_READ(Toolbars,run.break_options);
				else CFG_BOOL_READ(Toolbars,run.break_list);
				else CFG_BOOL_READ(Toolbars,run.inspections);
				else CFG_BOOL_READ(Toolbars,tools.draw_flow);
				else CFG_BOOL_READ(Toolbars,tools.draw_project);
				else CFG_BOOL_READ(Toolbars,tools.draw_classes);
				else CFG_BOOL_READ(Toolbars,tools.remove_comments);
				else CFG_BOOL_READ(Toolbars,tools.align_comments);
				else CFG_BOOL_READ(Toolbars,tools.preproc_mark_valid);
				else CFG_BOOL_READ(Toolbars,tools.preproc_unmark_all);
				else CFG_BOOL_READ(Toolbars,tools.preproc_expand_macros);
				else CFG_BOOL_READ(Toolbars,tools.copy_code_from_h);
				else CFG_BOOL_READ(Toolbars,tools.open_terminal);
				else CFG_BOOL_READ(Toolbars,tools.generate_makefile);
				else CFG_BOOL_READ(Toolbars,tools.exe_info);
				else CFG_BOOL_READ(Toolbars,tools.proy_stats);
				else CFG_BOOL_READ(Toolbars,tools.open_shared);
				else CFG_BOOL_READ(Toolbars,tools.share_source);
				else CFG_BOOL_READ(Toolbars,tools.share_list);
				else CFG_BOOL_READ(Toolbars,tools.diff_two_sources);
				else CFG_BOOL_READ(Toolbars,tools.diff_himself);
				else CFG_BOOL_READ(Toolbars,tools.diff_other_file);
				else CFG_BOOL_READ(Toolbars,tools.diff_clear);
				else CFG_BOOL_READ(Toolbars,tools.diff_show);
				else CFG_BOOL_READ(Toolbars,tools.diff_apply);
				else CFG_BOOL_READ(Toolbars,tools.diff_discard);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_config);
				else CFG_BOOL_READ_DN("tools.wxfb_activate",Toolbars.tools.wxfb_config);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_new_res);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_load_res);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_regen);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_inherit);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_update_inherit);
				else CFG_BOOL_READ(Toolbars,tools.wxfb_help_wx);
				else CFG_BOOL_READ(Toolbars,tools.doxy_generate);
				else CFG_BOOL_READ(Toolbars,tools.doxy_config);
				else CFG_BOOL_READ(Toolbars,tools.doxy_view);
				else CFG_BOOL_READ(Toolbars,tools.gprof_activate);
				else CFG_BOOL_READ(Toolbars,tools.gprof_show_graph);
				else CFG_BOOL_READ(Toolbars,tools.gprof_list_output);
				else CFG_BOOL_READ(Toolbars,tools.gcov_activate);
				else CFG_BOOL_READ(Toolbars,tools.gcov_reset);
				else CFG_BOOL_READ(Toolbars,tools.gcov_show_bar);
				else CFG_BOOL_READ(Toolbars,tools.custom_settings);
				else CFG_BOOL_READ(Toolbars,tools.cppcheck_run);
				else CFG_BOOL_READ(Toolbars,tools.cppcheck_config);
				else CFG_BOOL_READ(Toolbars,tools.cppcheck_view);
#if !defined(__WIN32__)
				else CFG_BOOL_READ(Toolbars,tools.valgrind_run);
				else CFG_BOOL_READ(Toolbars,tools.valgrind_view);
#endif
				else CFG_BOOL_READ(Toolbars,debug.start);
				else CFG_BOOL_READ(Toolbars,debug.pause);
				else CFG_BOOL_READ(Toolbars,debug.stop);
				else CFG_BOOL_READ(Toolbars,debug.step_in);
				else CFG_BOOL_READ(Toolbars,debug.step_out);
				else CFG_BOOL_READ(Toolbars,debug.step_over);
				else CFG_BOOL_READ(Toolbars,debug.function_return);
				else CFG_BOOL_READ(Toolbars,debug.jump);
				else CFG_BOOL_READ(Toolbars,debug.run_until);
				else CFG_BOOL_READ(Toolbars,debug.inspections);
				else CFG_BOOL_READ(Toolbars,debug.break_toggle);
				else CFG_BOOL_READ(Toolbars,debug.backtrace);
				else CFG_BOOL_READ(Toolbars,debug.threadlist);
				else CFG_BOOL_READ(Toolbars,debug.break_list);
				else CFG_BOOL_READ(Toolbars,debug.log_panel);
				else CFG_BOOL_READ(Toolbars,debug.inverse_exec);
				else CFG_BOOL_READ(Toolbars,debug.enable_inverse_exec);
				else CFG_BOOL_READ(Toolbars,debug.break_options);
				else CFG_BOOL_READ(Toolbars,misc.preferences);
				else CFG_BOOL_READ(Toolbars,misc.about);
				else CFG_BOOL_READ(Toolbars,misc.opinion);
				else CFG_BOOL_READ(Toolbars,misc.tutorials);
				else CFG_BOOL_READ(Toolbars,misc.help_ide);
				else CFG_BOOL_READ(Toolbars,misc.help_cpp);
				else CFG_BOOL_READ(Toolbars,misc.show_tips);
				else CFG_BOOL_READ(Toolbars,misc.find_updates);
				else CFG_BOOL_READ(Toolbars,misc.exit);
				else CFG_GENERIC_READ(Toolbars,positions.file);
				else CFG_GENERIC_READ(Toolbars,positions.edit);
				else CFG_GENERIC_READ(Toolbars,positions.view);
				else CFG_GENERIC_READ(Toolbars,positions.run);
				else CFG_GENERIC_READ(Toolbars,positions.tools);
				else CFG_GENERIC_READ(Toolbars,positions.misc);
				else CFG_GENERIC_READ(Toolbars,positions.find);
				else CFG_GENERIC_READ(Toolbars,positions.project);
				else CFG_GENERIC_READ(Toolbars,positions.debug);
				else CFG_GENERIC_READ(Toolbars,positions.status);
				else if (Init.version<20131115) {
					for(int i=0;i<MAX_CUSTOM_TOOLS;i++) { 
						CFG_BOOL_READ_DN(wxString("custom_tool_")<<i,CustomTools[i].on_toolbar);
					}
				}
			}		
		} 
	}
	
	if (Init.version<20110418) Debug.select_modified_inspections=true;
	if (Init.version<20110420) Init.check_for_updates=true;
	if (Init.version<20100806) Files.terminal_command.Replace("ZinjaI - Consola de Ejecucion","${TITLE}");
	if (Init.version<20101112 && Help.autocomp_indexes.Len()) Help.autocomp_indexes<<",STL_Iteradores";
	if (Init.version<20120929) 	{
		Debug.inspection_improving_template_from.Add("std::string");
		Debug.inspection_improving_template_to.Add("${EXP}._M_dataplus._M_p");
		Debug.inspection_improving_template_from.Add("wxString");
		Debug.inspection_improving_template_to.Add("${EXP}.m_pchData");
	}
	
//#if defined(__WIN32__)
//	if (Init.version<20120208 && !Init.forced_linker_options.Contains("-static-libstdc++")) Init.forced_linker_options<<" -static-libstdc++";
//#else
//	// " -static-libstdc++" estaba por error, solo se deberia haber agregado en windows, pero por error el if de arriba estaba fuera del #ifdef
//	if (Init.version<20120229 && Init.forced_linker_options.Contains(" -static-libstdc++")) Init.forced_linker_options.Replace(" -static-libstdc++","",false);
//#endif
	if (Init.version<20100828) {
//#if defined(__WIN32__)
//		Init.forced_compiler_options<<_T(" --show-column");
//#endif
		if (!Running.cpp_compiler_options.Contains("-O")) {
			if (!Running.cpp_compiler_options.EndsWith(" "))
				Running.cpp_compiler_options<<" ";
			Running.cpp_compiler_options<<"-O0";
		}
	}
	
//	if (Init.version<20130730) {
//		if (Running.cpp_compiler_options.Contains("-O0"))
//			Running.cpp_compiler_options.Replace("-O0","-Og");
//	}
	
	if (Init.version<20131223) {
		if (Running.cpp_compiler_options.Contains("-Og"))
			Running.cpp_compiler_options.Replace("-Og","-O0");
	}
	
	if (Init.version<20140319) {
		if (Help.autocomp_indexes.Contains("STL"))
			Help.autocomp_indexes+=",AAA_STL,AAA_STL_11";
		if (Help.autocomp_indexes.Contains("AAA_Estandar"))
			Help.autocomp_indexes+=",AAA_Estandar_Cpp_11";
		if (Help.autocomp_indexes.Contains("AAA_Palabras"))
			Help.autocomp_indexes+=",AAA_Palabras_Reservadas_11";
	}
	
	fil.Close();
	if (last_files.GetCount()) {
		int ps=0, pp=0;
		for (unsigned int i=0;i<last_files.GetCount();i++) {
			if (wxFileName(last_files[i]).GetExt().CmpNoCase(_T(PROJECT_EXT))==0)
				Files.last_project[pp++]=last_files[i];
			else
				Files.last_source[ps++]=last_files[i];
		}
	}
	
	if (Init.colour_theme.Len())
		ctheme->Load(DIR_PLUS_FILE(DIR_PLUS_FILE(zinjai_dir,"colours"),Init.colour_theme));
	else
		ctheme->Load(DIR_PLUS_FILE(home_dir,"colours.zcs"));
	
	Init.autohiding_panels=Init.autohide_panels;
	
	return true;
}
	
bool ConfigManager::Save(){
	wxTextFile fil(filename);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
#if defined(__WIN32__)
	fil.AddLine(wxString(_T("# generado por ZinjaI-w32-"))<<VERSION);
#elif defined(__APPLE__)
	fil.AddLine(wxString(_T("# generado por ZinjaI-mac-"))<<VERSION);
#else
	fil.AddLine(wxString(_T("# generado por ZinjaI-lnx-"))<<VERSION);
#endif
	fil.AddLine(_T("[Init]"));
	CFG_BOOL_WRITE_DN("left_panels",Init.left_panels);
	CFG_BOOL_WRITE_DN("show_welcome",Init.show_welcome);
	CFG_BOOL_WRITE_DN("show_beginner_panel",Init.show_beginner_panel);
	CFG_BOOL_WRITE_DN("show_tip_on_startup",Init.show_tip_on_startup);
	CFG_BOOL_WRITE_DN("show_splash",Init.show_splash);
	CFG_GENERIC_WRITE_DN("new_file",Init.new_file);
	CFG_GENERIC_WRITE_DN("version",Init.version);
	CFG_GENERIC_WRITE_DN("pos_x",Init.pos_x);
	CFG_GENERIC_WRITE_DN("pos_y",Init.pos_y);
	CFG_GENERIC_WRITE_DN("size_x",Init.size_x);
	CFG_GENERIC_WRITE_DN("size_y",Init.size_y);
	CFG_BOOL_WRITE_DN("maximized",Init.maximized);
	CFG_BOOL_WRITE_DN("lang_es",Init.lang_es);
	CFG_GENERIC_WRITE_DN("zinjai_server_port",Init.zinjai_server_port);
	CFG_BOOL_WRITE_DN("save_project",Init.save_project);
//	CFG_BOOL_WRITE_DN("close_files_for_project",Init.close_files_for_project);
	CFG_BOOL_WRITE_DN("always_add_extension",Init.always_add_extension);
	CFG_BOOL_WRITE_DN("autohide_toolbars_fs",Init.autohide_toolbars_fs);
	CFG_BOOL_WRITE_DN("autohide_panels_fs",Init.autohide_panels_fs);
	CFG_BOOL_WRITE_DN("autohide_menus_fs",Init.autohide_menus_fs);
	CFG_BOOL_WRITE_DN("check_for_updates",Init.check_for_updates);
	CFG_BOOL_WRITE_DN("prefer_explorer_tree",Init.prefer_explorer_tree);
	CFG_BOOL_WRITE_DN("cppcheck_seen",Init.cppcheck_seen);
#if !defined(_WIN32) && !defined(__WIN32__)
	CFG_BOOL_WRITE_DN("valgrind_seen",Init.valgrind_seen);
	CFG_BOOL_WRITE_DN("compiler_seen",Init.compiler_seen);
	CFG_BOOL_WRITE_DN("debugger_seen",Init.debugger_seen);
#endif
	CFG_BOOL_WRITE_DN("doxygen_seen",Init.doxygen_seen);
	CFG_BOOL_WRITE_DN("wxfb_seen",Init.wxfb_seen);
	CFG_BOOL_WRITE_DN("show_explorer_tree",Init.show_explorer_tree);
	CFG_BOOL_WRITE_DN("graphviz_dot",Init.graphviz_dot);
	CFG_GENERIC_WRITE_DN("inherit_num",Init.inherit_num);
	CFG_GENERIC_WRITE_DN("history_len",Init.history_len);
	CFG_GENERIC_WRITE_DN("proxy",Init.proxy);
	CFG_GENERIC_WRITE_DN("language_file",Init.language_file);
	CFG_GENERIC_WRITE_DN("max_errors",Init.max_errors);
	CFG_GENERIC_WRITE_DN("max_jobs",Init.max_jobs);
	CFG_GENERIC_WRITE_DN("wrap_mode",Init.wrap_mode);
	CFG_BOOL_WRITE_DN("singleton",Init.singleton);
	CFG_BOOL_WRITE_DN("stop_compiling_on_error",Init.stop_compiling_on_error);
	CFG_BOOL_WRITE_DN("autohide_panels",Init.autohide_panels);
	CFG_BOOL_WRITE_DN("fullpath_on_project_tree",Init.fullpath_on_project_tree);
	CFG_BOOL_WRITE_DN("beautify_compiler_errors",Init.beautify_compiler_errors);
	CFG_GENERIC_WRITE_DN("colour_theme",Init.colour_theme);
	fil.AddLine(_(""));

	fil.AddLine(_T("[Debug]"));
	CFG_BOOL_WRITE_DN("allow_edition",Debug.allow_edition);
	CFG_BOOL_WRITE_DN("autohide_panels",Debug.autohide_panels);
	CFG_BOOL_WRITE_DN("autohide_toolbars",Debug.autohide_toolbars);
	CFG_BOOL_WRITE_DN("select_modified_inspections",Debug.select_modified_inspections);
	CFG_BOOL_WRITE_DN("raise_main_window",Debug.raise_main_window);
	CFG_BOOL_WRITE_DN("compile_again",Debug.compile_again);
	CFG_BOOL_WRITE_DN("always_debug",Debug.always_debug);
	CFG_BOOL_WRITE_DN("close_on_normal_exit",Debug.close_on_normal_exit);
	if (Debug.format.Len()) CFG_GENERIC_WRITE_DN("format",Debug.format);
	CFG_GENERIC_WRITE_DN("blacklist",Debug.blacklist);
	CFG_GENERIC_WRITE_DN("macros_file",Debug.macros_file);
	CFG_BOOL_WRITE_DN("show_do_that",Debug.show_do_that);
	CFG_BOOL_WRITE_DN("show_thread_panel",Debug.show_thread_panel);
	CFG_BOOL_WRITE_DN("show_log_panel",Debug.show_log_panel);
	CFG_BOOL_WRITE_DN("inspections_on_right",Debug.inspections_on_right);
	CFG_BOOL_WRITE_DN("readnow",Debug.readnow);
	CFG_BOOL_WRITE_DN("auto_solibs",Debug.auto_solibs);
	CFG_BOOL_WRITE_DN("improve_inspections_by_type",Debug.improve_inspections_by_type);
	for(unsigned int i=0;i<Debug.inspection_improving_template_from.GetCount();i++)
		CFG_GENERIC_WRITE_DN("inspection_improving_template",Debug.inspection_improving_template_from[i]+"|"+Debug.inspection_improving_template_to[i]);
	fil.AddLine(_(""));
	
	fil.AddLine(_T("[Styles]"));
	CFG_GENERIC_WRITE_DN("print_size",Styles.print_size);
	CFG_GENERIC_WRITE_DN("font_size",Styles.font_size);
	CFG_GENERIC_WRITE_DN("colour_theme",Init.colour_theme);
	fil.AddLine(_T(""));

	fil.AddLine(_T("[Source]"));
	CFG_BOOL_WRITE_DN("smartIndent",Source.smartIndent);
	CFG_BOOL_WRITE_DN("indentPaste",Source.indentPaste);
	CFG_BOOL_WRITE_DN("bracketInsertion",Source.bracketInsertion);
	CFG_BOOL_WRITE_DN("syntaxEnable",Source.syntaxEnable);
	CFG_BOOL_WRITE_DN("foldEnable",Source.foldEnable);
	CFG_BOOL_WRITE_DN("indentEnable",Source.indentEnable);
	CFG_BOOL_WRITE_DN("whiteSpace",Source.whiteSpace);
	CFG_BOOL_WRITE_DN("lineNumber",Source.lineNumber);
	CFG_BOOL_WRITE_DN("overType",Source.overType);
	CFG_BOOL_WRITE_DN("callTips",Source.callTips);
	CFG_BOOL_WRITE_DN("toolTips",Source.toolTips);
	CFG_BOOL_WRITE_DN("autotextEnabled",Source.autotextEnabled);
	CFG_BOOL_WRITE_DN("autocloseStuff",Source.autocloseStuff);
	CFG_BOOL_WRITE_DN("autoCompletion",Source.autoCompletion);
	CFG_BOOL_WRITE_DN("avoidNoNewLineWarning",Source.avoidNoNewLineWarning);
	CFG_GENERIC_WRITE_DN("alignComments",Source.alignComments);
	CFG_GENERIC_WRITE_DN("tabWidth",Source.tabWidth);
	CFG_BOOL_WRITE_DN("tabUseSpaces",Source.tabUseSpaces);
	fil.AddLine(_T(""));

	fil.AddLine(_T("[Running]"));
	CFG_GENERIC_WRITE_DN("cpp_compiler_options",Running.cpp_compiler_options);
	CFG_GENERIC_WRITE_DN("c_compiler_options",Running.c_compiler_options);
	CFG_BOOL_WRITE_DN("wait_for_key",Running.wait_for_key);
	CFG_BOOL_WRITE_DN("always_ask_args",Running.always_ask_args);
	CFG_BOOL_WRITE_DN("dont_run_headers",Running.dont_run_headers);
	CFG_BOOL_WRITE_DN("check_includes",Running.check_includes);
	fil.AddLine(_T(""));

	fil.AddLine(_T("[Help]"));
	CFG_GENERIC_WRITE_DN("wxhelp_index",Help.wxhelp_index);
	CFG_GENERIC_WRITE_DN("autocomp_indexes",Help.autocomp_indexes);
	CFG_GENERIC_WRITE_DN("min_len_for_completion",Help.min_len_for_completion);
	CFG_BOOL_WRITE_DN("show_extra_panels",Help.show_extra_panels);
	fil.AddLine(_T(""));

	fil.AddLine(_T("[Files]"));
	CFG_GENERIC_WRITE_DN("temp_dir",Files.temp_dir);
	CFG_GENERIC_WRITE_DN("img_browser",Files.img_browser);
	CFG_GENERIC_WRITE_DN("xdot_command",Files.xdot_command);
	CFG_GENERIC_WRITE_DN("skin_dir",Files.skin_dir);
//	CFG_GENERIC_WRITE_DN("parser_command",Files.parser_command);
	CFG_GENERIC_WRITE_DN("debugger_command",Files.debugger_command);
	CFG_GENERIC_WRITE_DN("toolchain",Files.toolchain);
//	CFG_GENERIC_WRITE_DN("compiler_command",Files.compiler_command);
//	CFG_GENERIC_WRITE_DN("compiler_c_command",Files.compiler_c_command);
	CFG_GENERIC_WRITE_DN("cppcheck_command",Files.cppcheck_command);
#if defined(__WIN32__)
//	CFG_GENERIC_WRITE_DN("mingw_dir",Files.mingw_dir);
#else
	CFG_GENERIC_WRITE_DN("valgrind_command",Files.valgrind_command);
#endif
//	CFG_GENERIC_WRITE_DN("runner_command",Files.runner_command);
	CFG_GENERIC_WRITE_DN("terminal_command",Files.terminal_command);
	CFG_GENERIC_WRITE_DN("explorer_command",Files.explorer_command);
	CFG_GENERIC_WRITE_DN("default_template",Files.default_template);
	CFG_GENERIC_WRITE_DN("default_project",Files.default_project);
//	CFG_GENERIC_WRITE_DN("templates_dir",Files.templates_dir);
	CFG_GENERIC_WRITE_DN("autocodes_file",Files.autocodes_file);
	CFG_GENERIC_WRITE_DN("doxygen_command",Files.doxygen_command);
	CFG_GENERIC_WRITE_DN("wxfb_command",Files.wxfb_command);
	CFG_GENERIC_WRITE_DN("browser_command",Files.browser_command);
	CFG_GENERIC_WRITE_DN("graphviz_dir",Files.graphviz_dir);
//	CFG_GENERIC_WRITE_DN("code_helper",Files.code_helper);
	CFG_GENERIC_WRITE_DN("project_folder",Files.project_folder);
	CFG_GENERIC_WRITE_DN("last_dir",Files.last_dir);
	CFG_GENERIC_WRITE_DN("last_project_dir",Files.last_project_dir);
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (Files.last_source[i].Len()) fil.AddLine(wxString(_T("last_source_"))<<i<<_T("=")<<Files.last_source[i]);
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (Files.last_project[i].Len()) fil.AddLine(wxString(_T("last_project_"))<<i<<_T("=")<<Files.last_project[i]);
	fil.AddLine(_T(""));
	
	fil.AddLine(_T("[Columns]"));
	for (int i=0;i<IG_COLS_COUNT;i++)
		fil.AddLine(wxString(_T("inspections_grid_"))<<i<<_T("=")<<(Cols.inspections_grid[i]?_T("1"):_T("0")));
	for (int i=0;i<BG_COLS_COUNT;i++)
		fil.AddLine(wxString(_T("backtrace_grid_"))<<i<<_T("=")<<(Cols.backtrace_grid[i]?_T("1"):_T("0")));
//	for (int i=0;i<TG_COLS_COUNT;i++)
//		fil.AddLine(wxString(_T("threadlist_grid_"))<<i<<_T("=")<<(Cols.threadlist_grid[i]?_T("1"):_T("0")));
	fil.AddLine(_T(""));
	
	fil.AddLine(_T("[CustomTools]"));
	for (int i=0;i<MAX_CUSTOM_TOOLS;i++) {
		if (CustomTools[i].command.Len()) {
			fil.AddLine(wxString(_T("name_"))<<i<<_T("=")<<CustomTools[i].name);
			fil.AddLine(wxString(_T("command_"))<<i<<_T("=")<<CustomTools[i].command);
			fil.AddLine(wxString(_T("workdir_"))<<i<<_T("=")<<CustomTools[i].workdir);
			fil.AddLine(wxString(_T("console_"))<<i<<_T("=")<<(CustomTools[i].console?_T("1"):_T("0")));
			fil.AddLine(wxString(_T("on_toolbar_"))<<i<<_T("=")<<(CustomTools[i].on_toolbar?_T("1"):_T("0")));
		}
	}
	fil.AddLine(_T(""));
	
	fil.AddLine(_T("[Toolbars]"));
	CFG_GENERIC_WRITE(Toolbars,icon_size);
	CFG_BOOL_WRITE(Toolbars,file.new_file);
	CFG_BOOL_WRITE(Toolbars,file.new_project);
	CFG_BOOL_WRITE(Toolbars,file.open);
	CFG_BOOL_WRITE(Toolbars,file.open_header);
	CFG_BOOL_WRITE(Toolbars,file.recent_simple);
	CFG_BOOL_WRITE(Toolbars,file.recent_project);
	CFG_BOOL_WRITE(Toolbars,file.open_selected);
	CFG_BOOL_WRITE(Toolbars,file.save);
	CFG_BOOL_WRITE(Toolbars,file.save_as);
	CFG_BOOL_WRITE(Toolbars,file.save_all);
	CFG_BOOL_WRITE(Toolbars,file.save_project);
	CFG_BOOL_WRITE(Toolbars,file.print);
	CFG_BOOL_WRITE(Toolbars,file.export_html);
	CFG_BOOL_WRITE(Toolbars,file.reload);
	CFG_BOOL_WRITE(Toolbars,file.close);
	CFG_BOOL_WRITE(Toolbars,file.close_all);
	CFG_BOOL_WRITE(Toolbars,file.close_project);
	CFG_BOOL_WRITE(Toolbars,file.project_config);
	CFG_BOOL_WRITE(Toolbars,edit.undo);
	CFG_BOOL_WRITE(Toolbars,edit.redo);
	CFG_BOOL_WRITE(Toolbars,edit.cut);
	CFG_BOOL_WRITE(Toolbars,edit.copy);
	CFG_BOOL_WRITE(Toolbars,edit.paste);
	CFG_BOOL_WRITE(Toolbars,edit.move_up);
	CFG_BOOL_WRITE(Toolbars,edit.move_down);
	CFG_BOOL_WRITE(Toolbars,edit.duplicate);
	CFG_BOOL_WRITE(Toolbars,edit.delete_lines);
	CFG_BOOL_WRITE(Toolbars,edit.goto_class);
	CFG_BOOL_WRITE(Toolbars,edit.goto_line);
	CFG_BOOL_WRITE(Toolbars,edit.goto_file);
	CFG_BOOL_WRITE(Toolbars,edit.find);
	CFG_BOOL_WRITE(Toolbars,edit.find_prev);
	CFG_BOOL_WRITE(Toolbars,edit.find_next);
	CFG_BOOL_WRITE(Toolbars,edit.replace);
	CFG_BOOL_WRITE(Toolbars,edit.inser_header);
	CFG_BOOL_WRITE(Toolbars,edit.comment);
	CFG_BOOL_WRITE(Toolbars,edit.uncomment);
	CFG_BOOL_WRITE(Toolbars,edit.indent);
	CFG_BOOL_WRITE(Toolbars,edit.select_block);
	CFG_BOOL_WRITE(Toolbars,edit.select_all);
	CFG_BOOL_WRITE(Toolbars,edit.toggle_user_mark);
	CFG_BOOL_WRITE(Toolbars,edit.find_user_mark);
	CFG_BOOL_WRITE(Toolbars,edit.list_user_marks);
	CFG_BOOL_WRITE(Toolbars,edit.autocomplete);
	CFG_BOOL_WRITE(Toolbars,run.run);
	CFG_BOOL_WRITE(Toolbars,run.run_old);
	CFG_BOOL_WRITE(Toolbars,run.compile);
	CFG_BOOL_WRITE(Toolbars,run.stop);
	CFG_BOOL_WRITE(Toolbars,run.clean);
	CFG_BOOL_WRITE(Toolbars,run.options);
	CFG_BOOL_WRITE(Toolbars,run.debug);
#if !defined(__WIN32__)
	CFG_BOOL_WRITE(Toolbars,run.debug_attach);
	CFG_BOOL_WRITE(Toolbars,run.load_core_dump);
#endif
	CFG_BOOL_WRITE(Toolbars,run.break_toggle);
	CFG_BOOL_WRITE(Toolbars,run.break_options);
	CFG_BOOL_WRITE(Toolbars,run.break_list);
	CFG_BOOL_WRITE(Toolbars,run.inspections);
	CFG_BOOL_WRITE(Toolbars,tools.draw_flow);
	CFG_BOOL_WRITE(Toolbars,tools.draw_classes);
	CFG_BOOL_WRITE(Toolbars,tools.draw_project);
	CFG_BOOL_WRITE(Toolbars,tools.remove_comments);
	CFG_BOOL_WRITE(Toolbars,tools.align_comments);
	CFG_BOOL_WRITE(Toolbars,tools.preproc_mark_valid);
	CFG_BOOL_WRITE(Toolbars,tools.preproc_unmark_all);
	CFG_BOOL_WRITE(Toolbars,tools.preproc_expand_macros);
	CFG_BOOL_WRITE(Toolbars,tools.copy_code_from_h);
	CFG_BOOL_WRITE(Toolbars,tools.open_terminal);
	CFG_BOOL_WRITE(Toolbars,tools.generate_makefile);
	CFG_BOOL_WRITE(Toolbars,tools.proy_stats);
	CFG_BOOL_WRITE(Toolbars,tools.exe_info);
	CFG_BOOL_WRITE(Toolbars,tools.open_shared);
	CFG_BOOL_WRITE(Toolbars,tools.share_source);
	CFG_BOOL_WRITE(Toolbars,tools.share_list);
	CFG_BOOL_WRITE(Toolbars,tools.diff_two_sources);
	CFG_BOOL_WRITE(Toolbars,tools.diff_himself);
	CFG_BOOL_WRITE(Toolbars,tools.diff_other_file);
	CFG_BOOL_WRITE(Toolbars,tools.diff_clear);
	CFG_BOOL_WRITE(Toolbars,tools.diff_apply);
	CFG_BOOL_WRITE(Toolbars,tools.diff_discard);
	CFG_BOOL_WRITE(Toolbars,tools.diff_show);
	CFG_BOOL_WRITE(Toolbars,tools.doxy_generate);
	CFG_BOOL_WRITE(Toolbars,tools.doxy_config);
	CFG_BOOL_WRITE(Toolbars,tools.doxy_view);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_config);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_regen);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_new_res);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_load_res);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_inherit);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_update_inherit);
	CFG_BOOL_WRITE(Toolbars,tools.wxfb_help_wx);
	CFG_BOOL_WRITE(Toolbars,tools.gprof_activate);
	CFG_BOOL_WRITE(Toolbars,tools.gprof_show_graph);
	CFG_BOOL_WRITE(Toolbars,tools.gprof_list_output);
	CFG_BOOL_WRITE(Toolbars,tools.gcov_activate);
	CFG_BOOL_WRITE(Toolbars,tools.gcov_reset);
	CFG_BOOL_WRITE(Toolbars,tools.gcov_show_bar);
	CFG_BOOL_WRITE(Toolbars,tools.custom_settings);
	for(int i=0;i<MAX_CUSTOM_TOOLS;i++)
		CFG_BOOL_WRITE_DN((wxString("custom_tool_")<<i),CustomTools[i].on_toolbar);
	CFG_BOOL_WRITE(Toolbars,tools.cppcheck_run);
	CFG_BOOL_WRITE(Toolbars,tools.cppcheck_config);
	CFG_BOOL_WRITE(Toolbars,tools.cppcheck_view);
#if !defined(__WIN32__)
	CFG_BOOL_WRITE(Toolbars,tools.valgrind_run);
	CFG_BOOL_WRITE(Toolbars,tools.valgrind_view);
#endif
	CFG_BOOL_WRITE(Toolbars,view.white_space);
	CFG_BOOL_WRITE(Toolbars,view.split_view);
	CFG_BOOL_WRITE(Toolbars,view.line_wrap);
	CFG_BOOL_WRITE(Toolbars,view.sintax_colour);
	CFG_BOOL_WRITE(Toolbars,view.update_symbols);
	CFG_BOOL_WRITE(Toolbars,view.explorer_tree);
	CFG_BOOL_WRITE(Toolbars,view.project_tree);
	CFG_BOOL_WRITE(Toolbars,view.compiler_tree);
	CFG_BOOL_WRITE(Toolbars,view.full_screen);
	CFG_BOOL_WRITE(Toolbars,view.beginner_panel);
	CFG_BOOL_WRITE(Toolbars,view.next_error);
	CFG_BOOL_WRITE(Toolbars,view.prev_error);
	CFG_BOOL_WRITE(Toolbars,view.fold_all);
	CFG_BOOL_WRITE(Toolbars,view.fold_1);
	CFG_BOOL_WRITE(Toolbars,view.fold_2);
	CFG_BOOL_WRITE(Toolbars,view.fold_3);
	CFG_BOOL_WRITE(Toolbars,view.unfold_all);
	CFG_BOOL_WRITE(Toolbars,view.unfold_1);
	CFG_BOOL_WRITE(Toolbars,view.unfold_2);
	CFG_BOOL_WRITE(Toolbars,view.unfold_3);
	CFG_BOOL_WRITE(Toolbars,debug.start);
	CFG_BOOL_WRITE(Toolbars,debug.pause);
	CFG_BOOL_WRITE(Toolbars,debug.stop);
	CFG_BOOL_WRITE(Toolbars,debug.step_in);
	CFG_BOOL_WRITE(Toolbars,debug.step_out);
	CFG_BOOL_WRITE(Toolbars,debug.step_over);
	CFG_BOOL_WRITE(Toolbars,debug.function_return);
	CFG_BOOL_WRITE(Toolbars,debug.jump);
	CFG_BOOL_WRITE(Toolbars,debug.run_until);
	CFG_BOOL_WRITE(Toolbars,debug.inspections);
	CFG_BOOL_WRITE(Toolbars,debug.backtrace);
	CFG_BOOL_WRITE(Toolbars,debug.threadlist);
	CFG_BOOL_WRITE(Toolbars,debug.break_list);
	CFG_BOOL_WRITE(Toolbars,debug.break_toggle);
	CFG_BOOL_WRITE(Toolbars,debug.log_panel);
	CFG_BOOL_WRITE(Toolbars,debug.inverse_exec);
	CFG_BOOL_WRITE(Toolbars,debug.enable_inverse_exec);
	CFG_BOOL_WRITE(Toolbars,debug.break_options);
	CFG_BOOL_WRITE(Toolbars,misc.preferences);
	CFG_BOOL_WRITE(Toolbars,misc.about);
	CFG_BOOL_WRITE(Toolbars,misc.opinion);
	CFG_BOOL_WRITE(Toolbars,misc.tutorials);
	CFG_BOOL_WRITE(Toolbars,misc.help_ide);
	CFG_BOOL_WRITE(Toolbars,misc.help_cpp);
	CFG_BOOL_WRITE(Toolbars,misc.show_tips);
	CFG_BOOL_WRITE(Toolbars,misc.find_updates);
	CFG_GENERIC_WRITE(Toolbars,positions.file);	
	CFG_GENERIC_WRITE(Toolbars,positions.edit);	
	CFG_GENERIC_WRITE(Toolbars,positions.view);	
	CFG_GENERIC_WRITE(Toolbars,positions.run);	
	CFG_GENERIC_WRITE(Toolbars,positions.tools);	
	CFG_GENERIC_WRITE(Toolbars,positions.misc);	
	CFG_GENERIC_WRITE(Toolbars,positions.find);	
	CFG_GENERIC_WRITE(Toolbars,positions.project);	
	CFG_GENERIC_WRITE(Toolbars,positions.debug);	
	CFG_GENERIC_WRITE(Toolbars,positions.status);	
	
	fil.Write();
	fil.Close();

	return true;
}
	
void ConfigManager::LoadDefaults(){

	// crear el directorio para zinjai si no existe
#if defined(__WIN32__)
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("zinjai"));
#else
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T(".zinjai"));
#endif
	if (!wxFileName::DirExists(home_dir)) wxFileName::Mkdir(home_dir);
	filename = DIR_PLUS_FILE(zinjai_dir,"config.here");
	if (!wxFileName::FileExists(filename)) 
		filename = DIR_PLUS_FILE(home_dir,_T("config"));
	
	// establecer valores predeterminados para todas las estructuras
	Files.temp_dir=home_dir;
	Files.skin_dir=_T("imgs");
	Files.graphviz_dir=_T("graphviz");
//	Files.mingw_dir=_T("MinGW");
#if defined(__WIN32__)
	Files.toolchain="gcc-mingw32";
	Files.parser_command=_T("cbrowser.exe");
	Files.debugger_command=_T("gdb");
	Files.runner_command=_T("runner.exe");
	Files.terminal_command=_T("");
	Files.explorer_command=_T("explorer");
	Files.img_browser=_T("graphviz\\viewerw.exe");
	Files.doxygen_command=_T("c:\\archivos de programa\\doxygen\\bin\\doxygen.exe");
	Files.wxfb_command=_T("");
//	Files.browser_command=_T("shellexecute.exe");
	Files.browser_command=_T("");
#elif defined(__APPLE__)
	Files.toolchain="gcc";
	Files.parser_command=_T("./cbrowser");
	Files.debugger_command=_T("gdb");
	Files.runner_command=_T("./runner.bin");
	Files.terminal_command=_T("./mac-terminal-wrapper.bin");
	Files.explorer_command=_T("open");
	Files.img_browser=_T("open");
	Files.doxygen_command=_T("/Applications/Doxygen.app/Contents/Resources/doxygen");
	Files.wxfb_command=_T("/Applications/wxFormBuilder.app/Contents/MacOS/wxformbuilder");
	Files.browser_command=_T("open");
#else
	Files.toolchain="gcc";
	Files.parser_command=DIR_PLUS_FILE(zinjai_dir,_T("cbrowser"));
	Files.debugger_command=_T("gdb");
	Files.runner_command=DIR_PLUS_FILE(zinjai_dir,_T("runner.bin"));
	Files.explorer_command=_T("<<sin configurar>>");
	Files.terminal_command=_T("<<sin configurar>>");
	Files.img_browser=DIR_PLUS_FILE(zinjai_dir,_T("graphviz/viewer.bin"));
	Files.wxfb_command=_T("wxformbuilder");
	Files.cppcheck_command=_T("cppcheck");
	Files.valgrind_command=_T("valgrind");
	Files.doxygen_command=_T("doxygen");
	Files.browser_command=_T("firefox");
#endif
	Files.project_folder=DIR_PLUS_FILE(wxFileName::GetHomeDir(),"projects");
	Files.default_template="default.tpl";
	Files.default_project="<main>";
//	Files.templates_dir="templates";
	Files.autocodes_file=DIR_PLUS_FILE(home_dir,"autocodes");
//	Files.code_helper=DIR_PLUS_FILE(_T("quickhelp"),_T("codehelper.txt"));
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		Files.last_source[i]=_T("");
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		Files.last_project[i]=_T("");
	Files.last_dir=wxFileName::GetHomeDir();
	Files.last_project_dir=wxFileName::GetHomeDir();

	Init.show_beginner_panel=false;
	Init.show_welcome=true;
	Init.show_tip_on_startup=true;
	Init.left_panels=false;
	Init.show_splash=true;
	Init.new_file=2;
	Init.version=0;
	Init.pos_x=Init.pos_y=0;
	Init.size_x=Init.size_y=0;
	Init.maximized=true;
	Init.lang_es=false;
	Init.zinjai_server_port=46527;
//	Init.load_sharing_server=false;
	Init.save_project=false;
//	Init.close_files_for_project=false;
	Init.always_add_extension=false;
	Init.autohide_toolbars_fs=true;
	Init.autohide_menus_fs=false;
	Init.autohide_panels_fs=true;
	Init.check_for_updates=false;
	Init.inherit_num=3;
	Init.history_len=10;
	Init.prefer_explorer_tree=false;
	Init.graphviz_dot=true;
	Init.show_explorer_tree=false;
	
	Init.cppcheck_seen=false;
#if !defined(__WIN32__)
	Init.valgrind_seen=false;
	Init.compiler_seen=false;
	Init.debugger_seen=false;
#endif
	Init.wxfb_seen=false;
	Init.doxygen_seen=false;
	Init.singleton=true;
	Init.stop_compiling_on_error=true;
	Init.autohide_panels=false;
	Init.beautify_compiler_errors=true;
	Init.fullpath_on_project_tree=false;

	Styles.print_size=8;
	Styles.font_size=10;
	Init.colour_theme="default.zcs";
	
	Init.wrap_mode=1;
	Source.smartIndent=true;
	Source.indentPaste=true;
	Source.bracketInsertion=true;
	Source.syntaxEnable=true;
	Source.whiteSpace=false;
	Source.lineNumber=true;
	Source.indentEnable=true;
	Source.foldEnable=true;
	Source.overType=false;
	Source.autocloseStuff=false;
	Source.autotextEnabled=true;
	Source.toolTips=true;
	Source.alignComments=80;
	Source.tabWidth=4;
	Source.tabUseSpaces=false;
	Source.autoCompletion=true;
	Source.callTips=true;
	Source.avoidNoNewLineWarning=true;

	Running.cpp_compiler_options=_T("-Wall -pedantic-errors -O0");
	Running.c_compiler_options=_T("-Wall -pedantic-errors -O0 -lm");
	Running.wait_for_key=true;
	Running.always_ask_args=false;
	Running.dont_run_headers=false;
	Running.check_includes=true;
	
	Init.proxy="";
	Init.language_file=_T("spanish");
	Init.max_errors=500;
	Init.max_jobs=wxThread::GetCPUCount();
	if (Init.max_jobs<1) Init.max_jobs=1;
	
#if defined(__WIN32__)
	Help.wxhelp_index=_T("MinGW\\wx\\docs\\wx_contents.html");
//	Help.quickhelp_index="quickhelp\\index";
#else
	Help.wxhelp_index=_T("docs/wx/wx_contents.html");
//	Help.quickhelp_index="quickhelp/index";
#endif
//	Help.tips_file=DIR_PLUS_FILE(_T("quickhelp"),_T("tips"));
//	Help.quickhelp_dir="quickhelp";
	Help.cppreference_dir="cppreference/en";
	Help.guihelp_dir="guihelp";
//	Help.autocomp_dir="autocomp";
	Help.autocomp_indexes=_T("AAA_Directivas_de_Preprocesador,AAA_Estandar_C,AAA_Estandar_Cpp,AAA_STL,AAA_Palabras_Reservadas,AAA_Estandar_Cpp_11,AAA_STL_11,AAA_Palabras_Reservadas_11");
	Help.min_len_for_completion=3;
	Help.show_extra_panels=true;
	
	Debug.select_modified_inspections = true;
	Debug.allow_edition = false;
	Debug.autohide_panels = true;
	Debug.autohide_toolbars = true;
//	Debug.autoupdate_backtrace = true;
	Debug.close_on_normal_exit = true;
	Debug.always_debug = false;
	Debug.raise_main_window = true;
	Debug.compile_again = true;
	Debug.format = _T("");
	Debug.macros_file = _T("debug_macros.gdb");
	Debug.blacklist = _T("");
	Debug.inspections_on_right = false;
	Debug.show_thread_panel = false;
	Debug.show_log_panel = false;
	Debug.auto_solibs = false;
	Debug.readnow = false;
	Debug.show_do_that = false;
	Debug.improve_inspections_by_type = true;
	
	for (int i=0;i<IG_COLS_COUNT;i++)
		Cols.inspections_grid[i]=true;
	for (int i=0;i<BG_COLS_COUNT;i++)
		Cols.backtrace_grid[i]=true;
//	for (int i=0;i<TG_COLS_COUNT;i++)
//		Cols.threadlist_grid[i]=true;
	
}

void ConfigManager::LoadToolBarsDefaults() {
	Toolbars.tools.custom_settings=true;
	Toolbars.icon_size=24;
	Toolbars.file.new_file=true;
	Toolbars.file.new_project=false;
	Toolbars.file.open=true;
	Toolbars.file.recent_simple=false;
	Toolbars.file.recent_project=false;
	Toolbars.file.open_header=false;
	Toolbars.file.open_selected=false;
	Toolbars.file.save=true;
	Toolbars.file.save_as=true;
	Toolbars.file.save_all=false;
	Toolbars.file.save_project=false;
	Toolbars.file.print=false;
	Toolbars.file.export_html=false;
	Toolbars.file.reload=false;
	Toolbars.file.close=false;
	Toolbars.file.close_all=false;
	Toolbars.file.close_project=false;
	Toolbars.file.project_config=false;
	Toolbars.edit.undo=true;
	Toolbars.edit.redo=true;
	Toolbars.edit.cut=true;
	Toolbars.edit.copy=true;
	Toolbars.edit.paste=true;
	Toolbars.edit.move_up=false;
	Toolbars.edit.move_down=false;
	Toolbars.edit.duplicate=false;
	Toolbars.edit.delete_lines=false;
	Toolbars.edit.goto_class=true;
	Toolbars.edit.goto_line=true;
	Toolbars.edit.goto_file=true;
	Toolbars.edit.find=true;
	Toolbars.edit.find_prev=false;
	Toolbars.edit.find_next=true;
	Toolbars.edit.replace=true;
	Toolbars.edit.inser_header=true;
	Toolbars.edit.comment=true;
	Toolbars.edit.uncomment=true;
	Toolbars.edit.indent=true;
	Toolbars.edit.select_block=false;
	Toolbars.edit.select_all=false;
	Toolbars.edit.toggle_user_mark=false;
	Toolbars.edit.find_user_mark=false;
	Toolbars.edit.list_user_marks=false;
	Toolbars.edit.autocomplete=false;
	Toolbars.view.white_space=true;
	Toolbars.view.split_view=true;
	Toolbars.view.line_wrap=true;
	Toolbars.view.sintax_colour=true;
	Toolbars.view.update_symbols=true;
	Toolbars.view.explorer_tree=false;
	Toolbars.view.project_tree=false;
	Toolbars.view.compiler_tree=false;
	Toolbars.view.full_screen=true;
	Toolbars.view.beginner_panel=true;
	Toolbars.view.next_error=true;
	Toolbars.view.prev_error=true;
	Toolbars.view.fold_all=false;
	Toolbars.view.fold_1=false;
	Toolbars.view.fold_2=false;
	Toolbars.view.fold_3=false;
	Toolbars.view.unfold_all=false;
	Toolbars.view.unfold_1=false;
	Toolbars.view.unfold_2=false;
	Toolbars.view.unfold_3=false;
	Toolbars.run.run=true;
	Toolbars.run.run_old=false;
	Toolbars.run.compile=true;
	Toolbars.run.stop=true;
	Toolbars.run.clean=false;
	Toolbars.run.options=true;
	Toolbars.run.debug=true;
#if !defined(__WIN32__)
	Toolbars.run.load_core_dump=false;
	Toolbars.run.debug_attach=false;
#endif
	Toolbars.run.break_toggle=false;
	Toolbars.run.break_options=false;
	Toolbars.run.break_list=false;
	Toolbars.run.inspections=false;
	Toolbars.tools.draw_flow=false;
	Toolbars.tools.draw_classes=false;
	Toolbars.tools.draw_project=false;
	Toolbars.tools.remove_comments=false;
	Toolbars.tools.copy_code_from_h=false;
	Toolbars.tools.align_comments=false;
	Toolbars.tools.preproc_mark_valid=false;
	Toolbars.tools.preproc_unmark_all=false;
	Toolbars.tools.preproc_expand_macros=false;
	Toolbars.tools.open_terminal=false;
	Toolbars.tools.generate_makefile=false;
	Toolbars.tools.proy_stats=false;
	Toolbars.tools.exe_info=false;
	Toolbars.tools.open_shared=false;
	Toolbars.tools.share_source=false;
	Toolbars.tools.share_list=false;
	Toolbars.tools.diff_two_sources=false;
	Toolbars.tools.diff_himself=false;
	Toolbars.tools.diff_other_file=false;
	Toolbars.tools.diff_clear=false;
	Toolbars.tools.diff_apply=false;
	Toolbars.tools.diff_show=false;
	Toolbars.tools.diff_discard=false;
	Toolbars.tools.doxy_generate=false;
	Toolbars.tools.doxy_config=false;
	Toolbars.tools.doxy_view=false;
	Toolbars.tools.gprof_activate=false;
	Toolbars.tools.gprof_show_graph=false;
	Toolbars.tools.gprof_list_output=false;
	Toolbars.tools.gcov_activate=false;
	Toolbars.tools.gcov_reset=false;
	Toolbars.tools.gcov_show_bar=false;
	Toolbars.tools.cppcheck_run=false;
	Toolbars.tools.cppcheck_config=false;
	Toolbars.tools.cppcheck_view=false;
#if !defined(__WIN32__)
	Toolbars.tools.valgrind_run=false;
	Toolbars.tools.valgrind_view=false;
#endif
	Toolbars.tools.wxfb_config=false;
	Toolbars.tools.wxfb_new_res=false;
	Toolbars.tools.wxfb_load_res=false;
	Toolbars.tools.wxfb_regen=false;
	Toolbars.tools.wxfb_inherit=false;
	Toolbars.tools.wxfb_update_inherit=false;
	Toolbars.tools.wxfb_help_wx=false;
	Toolbars.debug.start=true;
	Toolbars.debug.pause=true;
	Toolbars.debug.stop=true;
	Toolbars.debug.step_in=true;
	Toolbars.debug.step_out=true;
	Toolbars.debug.step_over=true;
	Toolbars.debug.function_return=true;
	Toolbars.debug.jump=true;
	Toolbars.debug.run_until=true;
	Toolbars.debug.inspections=false;
	Toolbars.debug.backtrace=false;
	Toolbars.debug.threadlist=false;
	Toolbars.debug.break_toggle=false;
	Toolbars.debug.break_list=true;
	Toolbars.debug.log_panel=false;
	Toolbars.debug.inverse_exec=false;
	Toolbars.debug.enable_inverse_exec=false;
	Toolbars.debug.break_options=false;
	Toolbars.misc.preferences=true;
	Toolbars.misc.about=false;
	Toolbars.misc.opinion=true;
	Toolbars.misc.tutorials=false;
	Toolbars.misc.help_ide=true;
	Toolbars.misc.help_cpp=true;
	Toolbars.misc.show_tips=false;
	Toolbars.misc.find_updates=false;
	Toolbars.misc.exit=false;
	Toolbars.positions.file="T:1:0";
	Toolbars.positions.edit="T:1:1";
	Toolbars.positions.run="T:1:2";
	Toolbars.positions.view="t:1:3";
	Toolbars.positions.tools="t:1:4";
	Toolbars.positions.find="T:1:5";
	Toolbars.positions.misc="T:1:6";
	Toolbars.positions.project="T:1:7";
	Toolbars.positions.debug="t:3:0";
	Toolbars.positions.status="t:3:1";
	
}

bool ConfigManager::CheckWxfbPresent() {
	if (config->Init.wxfb_seen) return true;
	wxString out;
	if (config->Files.wxfb_command.Len())
		out = utils->GetOutput(wxString(_T("\""))<<config->Files.wxfb_command<<_T("\" -h"),true);
#ifdef __WIN32__
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\archivos de programa\\wxformbuilder\\wxformbuilder.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\Program Files\\wxformbuilder\\wxformbuilder.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\Program Files (x86)\\wxformbuilder\\wxformbuilder.exe";
	}
#endif
	if (project->GetWxfbConfiguration(false)->ask_if_wxfb_is_missing && (out.Len()==0 || out.Find(_T("bash"))!=wxNOT_FOUND || out.Find(_T("exec"))!=wxNOT_FOUND)) {
		/*int res = */mxMessageDialog(main_window,LANG(PROJMNGR_WXFB_NOT_FOUND,"El proyecto utiliza wxFormBuilder, pero este software\n"
			"no se ecuentra correctamente instalado/configurado en\n"
			"su pc. Para descargar e instalar wxFormsBuilder dirijase\n"
			"a http://wxformbuilder.org. Si ya se encuentra instalado,\n"
			"configure su ubicacion en la pestaña \"Rutas 2\" del\n"
			"dialogo de \"Preferencias\" (menu \"Archivo\")."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING/*,LANG(GENERAL_DONT_REMEMBER_AGAIN,"No volver a recordar"),false*/).ShowModal();
		/*if (res&mxMD_CHECKED)*/ project->GetWxfbConfiguration()->ask_if_wxfb_is_missing=false;
		return false;
	} else {
		config->Init.wxfb_seen=true;
		return true;
	}
}

bool ConfigManager::CheckDoxygenPresent() {
	if (config->Init.doxygen_seen) return true;
	wxString out;
	if (config->Files.doxygen_command.Len())
		out = utils->GetOutput(wxString(_T("\""))<<config->Files.doxygen_command<<_T("\" --version"),true);
#ifdef __WIN32__
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\doxygen\\bin\\doxygen.exe"))
			out=config->Files.doxygen_command="c:\\archivos de programa\\doxygen\\bin\\doxygen.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\doxygen\\bin\\doxygen.exe"))
			out=config->Files.doxygen_command="c:\\Program Files\\doxygen\\bin\\doxygen.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\doxygen\\bin\\doxygen.exe"))
			out=config->Files.doxygen_command="c:\\Program Files (x86)\\doxygen\\bin\\doxygen.exe";
	}
#endif
	if ((out.Len()==0 || out.Find(_T("bash"))!=wxNOT_FOUND || out.Find(_T("exec"))!=wxNOT_FOUND)) {
		mxMessageDialog(main_window,LANG(MAINW_DOXYGEN_MISSING,"Doxygen no se ecuentra correctamente instalado/configurado\n"
			"en su pc. Para descargar e instalar CppCheck dirijase a\n"
			"http://www.doxygen.org. Si ya se encuentra instalado,\n"
			"configure su ubiciacion en la pestania \"Rutas 2\" del\n"
			"dialog de \"Preferencias\" (menu \"Archivo\")."),
			LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();
		return false;
	} else {
		config->Init.doxygen_seen=true;
		return true;
	}
}

bool ConfigManager::CheckCppCheckPresent() {
	if (config->Init.cppcheck_seen) return true;
	wxString out;
	if (config->Files.cppcheck_command.Len())
		out = utils->GetOutput(wxString(_T("\""))<<config->Files.cppcheck_command<<_T("\" --version"),true);
#ifdef __WIN32__
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\archivos de programa\\cppcheck\\cppcheck.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\Program Files\\cppcheck\\cppcheck.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\Program Files (x86)\\cppcheck\\cppcheck.exe";
	}
#endif
	if ((out.Len()==0 || out.Find(_T("bash"))!=wxNOT_FOUND || out.Find(_T("exec"))!=wxNOT_FOUND)) {
		mxMessageDialog(main_window,LANG(MAINW_CPPCHECK_MISSING,"CppCheck no se ecuentra correctamente instalado/configurado\n"
			"en su pc. Para descargar e instalar CppCheck dirijase a\n"
			"http://cppcheck.sourceforge.net. Si ya se encuentra instalado,\n"
			"configure su ubiciacion en la pestania \"Rutas 2\" del\n"
			"dialog de \"Preferencias\" (menu \"Archivo\")."),
			LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();
		return false;
	} else {
		config->Init.cppcheck_seen=true;
		return true;
	}
}

void ConfigManager::RecalcStuff ( ) {
	temp_dir = DIR_PLUS_FILE(zinjai_dir,Files.temp_dir);
	if (zinjai_dir.EndsWith("\\")||zinjai_dir.EndsWith("/")) zinjai_dir.RemoveLast();
	if (temp_dir.EndsWith("\\")||temp_dir.EndsWith("/")) temp_dir.RemoveLast();
}


