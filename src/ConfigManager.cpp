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
#include "MenusAndToolsConfig.h"
#include "CustomTools.h"

ConfigManager *config;

/**
* @brief Config lines from main config files related to toolbar settings, 
*
* important: it is still here just for backward compatibility, since 20140723
* these lines now belong to a different config file that is read later
*
* old toolbars and menus related config lines are processed in a delayed mode 
* because  we need to have an initialized MenusAndToolsConfig instance to do it,
* but MenusAndToolsConfig needs an initialized ConfigManager to use the proper 
* language
*
* ver la descripción de la clase para entender la secuencia de inicialización
**/
struct DelayedConfigLines {
	wxArrayString toolbars_keys;
	wxArrayString toolbars_values;
} *delayed_config_lines;

ConfigManager::ConfigManager(wxString a_path):custom_tools(MAX_CUSTOM_TOOLS) {
	config=this;
	delayed_config_lines = nullptr; 
	zinjai_dir = a_path;
	zinjai_bin_dir = DIR_PLUS_FILE(zinjai_dir,"bin");
	zinjai_third_dir = DIR_PLUS_FILE(zinjai_dir,"third");
	LoadDefaults();
	er_init(home_dir.char_str());
}

void ConfigManager::DoInitialChecks() {
#ifdef __WIN32__
#else
	
	// elegir un explorador de archivos
	if (Files.explorer_command=="<<sin configurar>>") { // tratar de detectar automaticamente un terminal adecuado
		if (mxUT::GetOutput("dolphin --version").Len())
			Files.explorer_command = "dolphin";
		if (mxUT::GetOutput("konqueror --version").Len())
			Files.explorer_command = "konqueror";
		else if (mxUT::GetOutput("nautilus --version").Len())
			Files.explorer_command = "nautilus";
		else if (mxUT::GetOutput("thunar --version").Len())
			Files.explorer_command = "thunar";
	} 
	
	// elegir un terminal
	if (Files.terminal_command=="<<sin configurar>>") { // tratar de detectar automaticamente un terminal adecuado
		wxString xterm_error_msg = LANG(CONFIG_NO_TERMINAL_FOUND,""
			"No se ha encontrado una terminal conocida. Se recomienda instalar\n"
			"xterm; luego configure el parametro \"Comando del Terminal\" en la\n"
			"pestaña \"Rutas 2\" del cuadro de \"Preferencias\".");
		LinuxTerminalInfo::Initialize();
		for(int i=0;i<=LinuxTerminalInfo::count;i++) { 
			if (LinuxTerminalInfo::list[i].Test()) {
				Files.terminal_command = LinuxTerminalInfo::list[i].run_command;
				if (LinuxTerminalInfo::list[i].warning) {
					xterm_error_msg = LANG1(CONFIG_TERMINAL_WARNING,"La aplicación terminal que se ha encontrado instalada\n"
						"es <{1}>. Algunas versiones de esta terminal pueden generar\n"
						"problemas al intentar ejecutar un programa o proyecto. Si no logra\n"
						"ejecutar correctamente desde ZinjaI ninguno de los programas/proyectos\n"
						"que compile, intente configurar otra terminal (menú Archivo->Preferencias...\n"
						"pestaña \"Rutas 1\", opción \"Comando de la terminal\", la terminal\n"
						"recomendada es xterm).",LinuxTerminalInfo::list[i].name);
				} else 
					xterm_error_msg = "";
				break;
			}
		}
		if (xterm_error_msg.Len()) {
			if (CheckComplaintAndInstall(nullptr,
				LinuxTerminalInfo::list[0].test_command,
				LANG(CONFIG_TERMINAL,"Terminal de ejecución"),
				xterm_error_msg, "xterm")) 
			{
				Files.terminal_command = LinuxTerminalInfo::list[0].run_command;
			}
		}
	}
	
	// verificar si hay compilador
	if (!Init.compiler_seen) {
		if (mxUT::GetOutput("g++ --version").Len()) {
			Init.compiler_seen = true;
		} else {
			// try to use clang if g++ not found
			wxArrayString toolchains;
			Toolchain::GetNames(toolchains,true);
			if (toolchains.Index("clang")!=wxNOT_FOUND && mxUT::GetOutput("clang --version").Len()) {
				Files.toolchain = "clang"; 
				Toolchain::SelectToolchain(); 
				Init.compiler_seen = true;
			} else {
				// show error and try to install gcc
				Init.compiler_seen = CheckComplaintAndInstall(
					nullptr,"g++ --version",LANG(CONFIG_COMPILER,"Compilador C++"),
					LANG(CONFIG_COMPILER_NOT_FOUND,"No se ha encontrado un compilador para C++ (g++ o clang). Debe instalarlo\n"
					"con el gestor de paquetes que corresponda a su distribución\n"
					"(apt-get, yum, yast, installpkg, etc.)"), 
					"build-essential");
			}
		}
	}
	
	// verificar si hay depurador
	if (!Init.debugger_seen) {
		Init.debugger_seen = CheckComplaintAndInstall(
			nullptr, "gdb --version",
			LANG(CONFIG_DEBUGGER,"Depurador"),
			LANG(CONFIG_DEBUGGER_NOT_FOUND,"No se ha encontrado el depurador (gdb). Debe instalarlo con\n"
			"el gestor de paquetes que corresponda a su distribución\n"
			"(apt-get, yum, yast, installpkg, etc.)"),
			"gdb");
	}
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
				else CFG_GENERIC_READ_DN("font_name",Styles.font_name);
				else CFG_GENERIC_READ_DN("colour_theme",Init.colour_theme);
				else if (key=="dark") { if (mxUT::IsTrue(value)) Init.colour_theme="inverted.zcs";	} // just for backward compatibility
				
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
				else CFG_BOOL_READ_DN("autocompFilters",Source.autocompFilters);
				else CFG_BOOL_READ_DN("callTips",Source.callTips);
				else CFG_BOOL_READ_DN("autocompTips",Source.autocompTips);
				else CFG_BOOL_READ_DN("autotextEnabled",Source.autotextEnabled);
				else CFG_BOOL_READ_DN("autocloseStuff",Source.autocloseStuff);
				else CFG_BOOL_READ_DN("toolTips",Source.toolTips);
				else CFG_INT_READ_DN("autoCompletion",Source.autoCompletion);
				else CFG_BOOL_READ_DN("avoidNoNewLineWarning",Source.avoidNoNewLineWarning);
			
			} else if (section=="Debug") {
				CFG_BOOL_READ_DN("autohide_panels",Debug.autohide_panels);
				else CFG_BOOL_READ_DN("autohide_toolbars",Debug.autohide_toolbars);
				else CFG_BOOL_READ_DN("allow_edition",Debug.allow_edition);
				else CFG_GENERIC_READ_DN("format",Debug.format);
				else CFG_GENERIC_READ_DN("macros_file",Debug.macros_file);
				else CFG_BOOL_READ_DN("compile_again",Debug.compile_again);
				else CFG_BOOL_READ_DN("use_colours_for_inspections",Debug.use_colours_for_inspections);
				else CFG_BOOL_READ_DN("inspections_can_have_side_effects",Debug.inspections_can_have_side_effects);
				else CFG_BOOL_READ_DN("raise_main_window",Debug.raise_main_window);
				else CFG_BOOL_READ_DN("always_debug",Debug.always_debug);
//				else CFG_BOOL_READ_DN("close_on_normal_exit",Debug.close_on_normal_exit);
				else CFG_BOOL_READ_DN("show_do_that",Debug.show_do_that);
				else CFG_BOOL_READ_DN("auto_solibs",Debug.auto_solibs);
				else CFG_BOOL_READ_DN("readnow",Debug.readnow);
				else CFG_BOOL_READ_DN("inspections_on_right",Debug.inspections_on_right);
				else CFG_BOOL_READ_DN("show_thread_panel",Debug.show_thread_panel);
				else CFG_BOOL_READ_DN("show_log_panel",Debug.show_log_panel);
				else CFG_BOOL_READ_DN("return_focus_on_continue",Debug.return_focus_on_continue);
				else CFG_BOOL_READ_DN("improve_inspections_by_type",Debug.improve_inspections_by_type);
#ifdef __linux__
				else CFG_BOOL_READ_DN("enable_core_dump",Debug.enable_core_dump);
#endif
#ifdef __WIN32__
				else CFG_BOOL_READ_DN("no_debug_heap",Debug.no_debug_heap);
#endif
				else CFG_BOOL_READ_DN("use_blacklist",Debug.use_blacklist);
				else if (key=="blacklist") Debug.blacklist.Add(value);
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
			
			} else if (section=="Help") {
//				CFG_GENERIC_READ_DN("quickhelp_index",Help.quickhelp_index);
				CFG_GENERIC_READ_DN("wxhelp_index",Help.wxhelp_index);
				else CFG_GENERIC_READ_DN("autocomp_indexes",Help.autocomp_indexes);
				else CFG_INT_READ_DN("min_len_for_completion",Help.min_len_for_completion);
				else CFG_BOOL_READ_DN("show_extra_panels",Help.show_extra_panels);
			
			} else if (section=="Init") {
				CFG_BOOL_READ_DN("left_panels",Init.left_panels);
				else CFG_BOOL_READ_DN("show_beginner_panel",Init.show_beginner_panel);
				else CFG_BOOL_READ_DN("show_welcome",Init.show_welcome);
				else CFG_BOOL_READ_DN("show_tip_on_startup",Init.show_tip_on_startup);
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
#ifndef __WIN32__
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
				else CFG_BOOL_READ_DN("use_cache_for_subcommands",Init.use_cache_for_subcommands);
				else CFG_BOOL_READ_DN("beautify_compiler_errors",Init.beautify_compiler_errors);
				else CFG_BOOL_READ_DN("fullpath_on_project_tree",Init.fullpath_on_project_tree);
				else CFG_GENERIC_READ_DN("colour_theme",Init.colour_theme);
				
			} else if (section=="Files") {
				CFG_GENERIC_READ_DN("toolchain",Files.toolchain);
				else CFG_GENERIC_READ_DN("debugger_command",Files.debugger_command);
//#ifdef __WIN32__
//				else CFG_GENERIC_READ_DN("mingw_dir",Files.mingw_dir);
//#endif
				else CFG_GENERIC_READ_DN("terminal_command",Files.terminal_command);
				else CFG_GENERIC_READ_DN("explorer_command",Files.explorer_command);
				else CFG_GENERIC_READ_DN("default_template",Files.default_template);
				else CFG_GENERIC_READ_DN("default_project",Files.default_project);
				else CFG_GENERIC_READ_DN("autocodes_file",Files.autocodes_file);
				else CFG_GENERIC_READ_DN("skin_dir",Files.skin_dir);
				else CFG_GENERIC_READ_DN("temp_dir",Files.temp_dir);
				else CFG_GENERIC_READ_DN("img_viewer",Files.img_viewer);
				else CFG_GENERIC_READ_DN("xdot_command",Files.xdot_command);
				else CFG_GENERIC_READ_DN("graphviz_dir",Files.graphviz_dir);
				else CFG_GENERIC_READ_DN("browser_command",Files.browser_command);
				else CFG_GENERIC_READ_DN("cppcheck_command",Files.cppcheck_command);
#ifndef __WIN32__
				else CFG_GENERIC_READ_DN("valgrind_command",Files.valgrind_command);
#endif
				else CFG_GENERIC_READ_DN("doxygen_command",Files.doxygen_command);
				else CFG_GENERIC_READ_DN("wxfb_command",Files.wxfb_command);
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
						Cols.inspections_grid[l]=mxUT::IsTrue(value);
				} else if (key.StartsWith("backtrace_grid_")	) {
					if (key.Mid(15).ToLong(&l) && l>=0 && l<BG_COLS_COUNT)
						Cols.backtrace_grid[l]=mxUT::IsTrue(value);
				}
//				} else if (key.StartsWith("threadlist_grid_")	) {
//					if (key.Mid(15).ToLong(&l) && l>=0 && l<TG_COLS_COUNT)
//						Cols.threadlist_grid[l]=mxUT::IsTrue(value);
//				}
				
			} else if (section=="CustomTools") {
				custom_tools.ParseConfigLine(key,value);
				
			} else if (section=="Toolbars") {
				if (!delayed_config_lines) delayed_config_lines = new DelayedConfigLines;
				delayed_config_lines->toolbars_keys.Add(key);
				delayed_config_lines->toolbars_values.Add(value);
			}
		} 
	}
	
	if (Init.version<20100806) Files.terminal_command.Replace("ZinjaI - Consola de Ejecucion","${TITLE}"); // NO USAR ACENTOS, PUEDE ROMER EL X!!!! (me daba un segfault en la libICE al poner el ó en EjeuciÓn)
	if (Init.version<20101112 && Help.autocomp_indexes.Len()) Help.autocomp_indexes<<",STL_Iteradores";
	if (Init.version<20110418) Debug.use_colours_for_inspections=true;
	if (Init.version<20110420) Init.check_for_updates=true;
	
//#ifdef __WIN32__
//	if (Init.version<20120208 && !Init.forced_linker_options.Contains("-static-libstdc++")) Init.forced_linker_options<<" -static-libstdc++";
//#else
//	// " -static-libstdc++" estaba por error, solo se deberia haber agregado en windows, pero por error el if de arriba estaba fuera del #ifdef
//	if (Init.version<20120229 && Init.forced_linker_options.Contains(" -static-libstdc++")) Init.forced_linker_options.Replace(" -static-libstdc++","",false);
//#endif
	if (Init.version<20100828) {
//#ifdef __WIN32__
//		Init.forced_compiler_options<<" --show-column";
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
	
	if (Init.version<20140606 && Files.terminal_command=="xterm -T \"${TITLE}\" -e") Files.terminal_command="xterm -T \"${TITLE}\" -fa \"Liberation Mono\" -fs 12 -e";
		
	fil.Close();
	if (last_files.GetCount()) {
		int ps=0, pp=0;
		for (unsigned int i=0;i<last_files.GetCount();i++) {
			if (wxFileName(last_files[i]).GetExt().CmpNoCase(PROJECT_EXT)==0)
				Files.last_project[pp++]=last_files[i];
			else
				Files.last_source[ps++]=last_files[i];
		}
	}
	if (Init.version<20140704) {
		if (Init.proxy=="") Init.proxy="$http_proxy";
	}
	if (Init.version<20150226) {
		SetDefaultInspectionsImprovingTemplates();
	}
	
#ifdef _STC_HAS_ZASKARS_RESHOW
	if (Init.version<20141127) {
		if (Source.autoCompletion) Source.autoCompletion=2;
		Source.autocompFilters=true;
	}
#endif
	if (Init.version<20141212 && Debug.blacklist.GetCount()==1) {
		wxString orig = Debug.blacklist[0]; Debug.blacklist.Clear();
		mxUT::Split(orig,config->Debug.blacklist,true,false);
	}
	
	if (Init.version<20141218) {
		for(int i=0;i<custom_tools.GetCount();i++) { 
			if (custom_tools[i].output_mode>=2)
				custom_tools[i].output_mode++;
		}
	}

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
	
#ifdef __WIN32__
	fil.AddLine(wxString("# generado por ZinjaI-w32-")<<VERSION);
#elif defined(__APPLE__)
	fil.AddLine(wxString("# generado por ZinjaI-mac-")<<VERSION);
#else
	fil.AddLine(wxString("# generado por ZinjaI-lnx-")<<VERSION);
#endif
	fil.AddLine("[Init]");
	CFG_BOOL_WRITE_DN("left_panels",Init.left_panels);
	CFG_BOOL_WRITE_DN("show_welcome",Init.show_welcome);
	CFG_BOOL_WRITE_DN("show_beginner_panel",Init.show_beginner_panel);
	CFG_BOOL_WRITE_DN("show_tip_on_startup",Init.show_tip_on_startup);
	CFG_GENERIC_WRITE_DN("new_file",Init.new_file);
	CFG_GENERIC_WRITE_DN("version",VERSION);
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
#ifndef __WIN32__
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
	CFG_BOOL_WRITE_DN("use_cache_for_subcommands",Init.use_cache_for_subcommands);
	CFG_BOOL_WRITE_DN("beautify_compiler_errors",Init.beautify_compiler_errors);
	CFG_GENERIC_WRITE_DN("colour_theme",Init.colour_theme);
	fil.AddLine("");

	fil.AddLine("[Debug]");
	CFG_BOOL_WRITE_DN("allow_edition",Debug.allow_edition);
	CFG_BOOL_WRITE_DN("autohide_panels",Debug.autohide_panels);
	CFG_BOOL_WRITE_DN("autohide_toolbars",Debug.autohide_toolbars);
	CFG_BOOL_WRITE_DN("use_colours_for_inspections",Debug.use_colours_for_inspections);
	CFG_BOOL_WRITE_DN("inspections_can_have_side_effects",Debug.inspections_can_have_side_effects);
	CFG_BOOL_WRITE_DN("raise_main_window",Debug.raise_main_window);
	CFG_BOOL_WRITE_DN("compile_again",Debug.compile_again);
	CFG_BOOL_WRITE_DN("always_debug",Debug.always_debug);
	CFG_BOOL_WRITE_DN("use_blacklist",Debug.use_blacklist);
	if (Debug.format.Len()) CFG_GENERIC_WRITE_DN("format",Debug.format);
	for(unsigned int i=0;i<Debug.blacklist.GetCount();i++) 
		CFG_GENERIC_WRITE_DN("blacklist",Debug.blacklist[i]);
	CFG_GENERIC_WRITE_DN("macros_file",Debug.macros_file);
	CFG_BOOL_WRITE_DN("show_do_that",Debug.show_do_that);
	CFG_BOOL_WRITE_DN("show_thread_panel",Debug.show_thread_panel);
	CFG_BOOL_WRITE_DN("show_log_panel",Debug.show_log_panel);
	CFG_BOOL_WRITE_DN("inspections_on_right",Debug.inspections_on_right);
	CFG_BOOL_WRITE_DN("readnow",Debug.readnow);
	CFG_BOOL_WRITE_DN("auto_solibs",Debug.auto_solibs);
	CFG_BOOL_WRITE_DN("return_focus_on_continue",Debug.return_focus_on_continue);
	CFG_BOOL_WRITE_DN("improve_inspections_by_type",Debug.improve_inspections_by_type);
#ifdef __linux__
	CFG_BOOL_WRITE_DN("enable_core_dump",Debug.enable_core_dump);
#endif
#ifdef __WIN32__
	CFG_BOOL_WRITE_DN("no_debug_heap",Debug.no_debug_heap);
#endif
	for(unsigned int i=0;i<Debug.inspection_improving_template_from.GetCount();i++)
		CFG_GENERIC_WRITE_DN("inspection_improving_template",Debug.inspection_improving_template_from[i]+"|"+Debug.inspection_improving_template_to[i]);
	fil.AddLine("");
	
	fil.AddLine("[Styles]");
	CFG_GENERIC_WRITE_DN("print_size",Styles.print_size);
	CFG_GENERIC_WRITE_DN("font_size",Styles.font_size);
	CFG_GENERIC_WRITE_DN("font_name",Styles.font_name);
	CFG_GENERIC_WRITE_DN("colour_theme",Init.colour_theme);
	fil.AddLine("");

	fil.AddLine("[Source]");
	CFG_BOOL_WRITE_DN("smartIndent",Source.smartIndent);
	CFG_BOOL_WRITE_DN("indentPaste",Source.indentPaste);
	CFG_BOOL_WRITE_DN("bracketInsertion",Source.bracketInsertion);
	CFG_BOOL_WRITE_DN("syntaxEnable",Source.syntaxEnable);
	CFG_BOOL_WRITE_DN("foldEnable",Source.foldEnable);
	CFG_BOOL_WRITE_DN("indentEnable",Source.indentEnable);
	CFG_BOOL_WRITE_DN("whiteSpace",Source.whiteSpace);
	CFG_BOOL_WRITE_DN("lineNumber",Source.lineNumber);
	CFG_BOOL_WRITE_DN("overType",Source.overType);
	CFG_BOOL_WRITE_DN("autocompFilters",Source.autocompFilters);
	CFG_BOOL_WRITE_DN("callTips",Source.callTips);
	CFG_BOOL_WRITE_DN("autocompTips",Source.autocompTips);
	CFG_BOOL_WRITE_DN("toolTips",Source.toolTips);
	CFG_BOOL_WRITE_DN("autotextEnabled",Source.autotextEnabled);
	CFG_BOOL_WRITE_DN("autocloseStuff",Source.autocloseStuff);
	CFG_GENERIC_WRITE_DN("autoCompletion",Source.autoCompletion);
	CFG_BOOL_WRITE_DN("avoidNoNewLineWarning",Source.avoidNoNewLineWarning);
	CFG_GENERIC_WRITE_DN("alignComments",Source.alignComments);
	CFG_GENERIC_WRITE_DN("tabWidth",Source.tabWidth);
	CFG_BOOL_WRITE_DN("tabUseSpaces",Source.tabUseSpaces);
	fil.AddLine("");

	fil.AddLine("[Running]");
	CFG_GENERIC_WRITE_DN("cpp_compiler_options",Running.cpp_compiler_options);
	CFG_GENERIC_WRITE_DN("c_compiler_options",Running.c_compiler_options);
	CFG_BOOL_WRITE_DN("wait_for_key",Running.wait_for_key);
	CFG_BOOL_WRITE_DN("always_ask_args",Running.always_ask_args);
	CFG_BOOL_WRITE_DN("check_includes",Running.check_includes);
	fil.AddLine("");

	fil.AddLine("[Help]");
	CFG_GENERIC_WRITE_DN("wxhelp_index",Help.wxhelp_index);
	CFG_GENERIC_WRITE_DN("autocomp_indexes",Help.autocomp_indexes);
	CFG_GENERIC_WRITE_DN("min_len_for_completion",Help.min_len_for_completion);
	CFG_BOOL_WRITE_DN("show_extra_panels",Help.show_extra_panels);
	fil.AddLine("");

	fil.AddLine("[Files]");
	CFG_GENERIC_WRITE_DN("temp_dir",Files.temp_dir);
	CFG_GENERIC_WRITE_DN("img_viewer",Files.img_viewer);
	CFG_GENERIC_WRITE_DN("xdot_command",Files.xdot_command);
	CFG_GENERIC_WRITE_DN("skin_dir",Files.skin_dir);
	CFG_GENERIC_WRITE_DN("debugger_command",Files.debugger_command);
	CFG_GENERIC_WRITE_DN("toolchain",Files.toolchain);
//	CFG_GENERIC_WRITE_DN("compiler_command",Files.compiler_command);
//	CFG_GENERIC_WRITE_DN("compiler_c_command",Files.compiler_c_command);
	CFG_GENERIC_WRITE_DN("cppcheck_command",Files.cppcheck_command);
#ifdef __WIN32__
//	CFG_GENERIC_WRITE_DN("mingw_dir",Files.mingw_dir);
#else
	CFG_GENERIC_WRITE_DN("valgrind_command",Files.valgrind_command);
#endif
//	CFG_GENERIC_WRITE_DN("runner_command",Files.runner_command);
	CFG_GENERIC_WRITE_DN("terminal_command",Files.terminal_command);
	CFG_GENERIC_WRITE_DN("explorer_command",Files.explorer_command);
	CFG_GENERIC_WRITE_DN("default_template",Files.default_template);
	CFG_GENERIC_WRITE_DN("default_project",Files.default_project);
	CFG_GENERIC_WRITE_DN("autocodes_file",Files.autocodes_file);
	CFG_GENERIC_WRITE_DN("doxygen_command",Files.doxygen_command);
	CFG_GENERIC_WRITE_DN("wxfb_command",Files.wxfb_command);
	CFG_GENERIC_WRITE_DN("browser_command",Files.browser_command);
	CFG_GENERIC_WRITE_DN("graphviz_dir",Files.graphviz_dir);
	CFG_GENERIC_WRITE_DN("project_folder",Files.project_folder);
	CFG_GENERIC_WRITE_DN("last_dir",Files.last_dir);
	CFG_GENERIC_WRITE_DN("last_project_dir",Files.last_project_dir);
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (Files.last_source[i].Len()) fil.AddLine(wxString("last_source_")<<i<<"="<<Files.last_source[i]);
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (Files.last_project[i].Len()) fil.AddLine(wxString("last_project_")<<i<<"="<<Files.last_project[i]);
	fil.AddLine("");
	
	fil.AddLine("[Columns]");
	for (int i=0;i<IG_COLS_COUNT;i++)
		fil.AddLine(wxString("inspections_grid_")<<i<<"="<<(Cols.inspections_grid[i]?"1":"0"));
	for (int i=0;i<BG_COLS_COUNT;i++)
		fil.AddLine(wxString("backtrace_grid_")<<i<<"="<<(Cols.backtrace_grid[i]?"1":"0"));
//	for (int i=0;i<TG_COLS_COUNT;i++)
//		fil.AddLine(wxString("threadlist_grid_")<<i<<"="<<(Cols.threadlist_grid[i]?"1":"0"));
	fil.AddLine("");
	
	fil.AddLine("[CustomTools]");
	custom_tools.WriteConfig(fil);
	fil.AddLine("");
	
//	menu_data->SaveShortcutsSettings(DIR_PLUS_FILE(home_dir,"shortcuts.zsc")); // se hace en e Ok del mxShortcutsDialog
	menu_data->SaveToolbarsSettings(DIR_PLUS_FILE(home_dir,"toolbar.ztb"));
	
	fil.Write();
	fil.Close();

	return true;
}
	
static void EnsurePathExists(const wxString path) {
	if (!wxFileName::DirExists(path)) wxFileName::Mkdir(path);
}

void ConfigManager::LoadDefaults(){

	// crear el directorio para zinjai si no existe
#ifdef __WIN32__
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),"zinjai");
#else
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),".zinjai");
#endif
	EnsurePathExists(home_dir);
	filename = DIR_PLUS_FILE(zinjai_dir,"config.here");
	if (!wxFileName::FileExists(filename)) 
		filename = DIR_PLUS_FILE(home_dir,"config");
	
	// establecer valores predeterminados para todas las estructuras
	Files.temp_dir=DIR_PLUS_FILE(home_dir,"tmp");;
	EnsurePathExists(temp_dir);
	Files.skin_dir="imgs";
	Files.graphviz_dir="graphviz";
//	Files.mingw_dir="MinGW";
#ifdef __WIN32__
	Files.toolchain="gcc-mingw32";
	Files.debugger_command="gdb";
	Files.runner_command=DIR_PLUS_FILE(zinjai_bin_dir,"runner.exe");
	Files.terminal_command="";
	Files.explorer_command="explorer";
	Files.img_viewer="";
	Files.doxygen_command="c:\\archivos de programa\\doxygen\\bin\\doxygen.exe";
	Files.wxfb_command="";
//	Files.browser_command="shellexecute.exe";
	Files.browser_command="";
#elif defined(__APPLE__)
	Files.toolchain="gcc";
	Files.debugger_command="gdb";
	Files.runner_command=DIR_PLUS_FILE(zinjai_bin_dir,"runner.bin");
	Files.terminal_command=DIR_PLUS_FILE(zinjai_bin_dir,"mac-terminal-wrapper.bin");
	Files.explorer_command="open";
	Files.img_viewer="open";
	Files.doxygen_command="/Applications/Doxygen.app/Contents/Resources/doxygen";
	Files.wxfb_command="/Applications/wxFormBuilder.app/Contents/MacOS/wxformbuilder";
	Files.browser_command="open";
#else
	Files.toolchain="gcc";
	Files.debugger_command="gdb";
	Files.runner_command=DIR_PLUS_FILE(zinjai_bin_dir,"runner.bin");
	Files.explorer_command="<<sin configurar>>";
	Files.terminal_command="<<sin configurar>>";
	Files.img_viewer="";
	Files.wxfb_command="wxformbuilder";
	Files.cppcheck_command="cppcheck";
	Files.valgrind_command="valgrind";
	Files.doxygen_command="doxygen";
	Files.browser_command="firefox";
#endif
	Files.project_folder=DIR_PLUS_FILE(wxFileName::GetHomeDir(),"projects");
	Files.default_template="default.tpl";
	Files.default_project="<main>";
	Files.autocodes_file=DIR_PLUS_FILE(home_dir,"autocodes");
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		Files.last_source[i]="";
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++)
		Files.last_project[i]="";
	Files.last_dir=wxFileName::GetHomeDir();
	Files.last_project_dir=wxFileName::GetHomeDir();

	Init.show_beginner_panel=false;
	Init.show_welcome=true;
	Init.show_tip_on_startup=true;
	Init.left_panels=false;
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
#ifndef __WIN32__
	Init.valgrind_seen=false;
	Init.compiler_seen=false;
	Init.debugger_seen=false;
#endif
	Init.wxfb_seen=false;
	Init.doxygen_seen=false;
	Init.singleton=true;
	Init.stop_compiling_on_error=true;
	Init.autohide_panels=false;
	Init.use_cache_for_subcommands=true;
	Init.beautify_compiler_errors=true;
	Init.fullpath_on_project_tree=false;

	Styles.print_size=8;
	Styles.font_size=10;
	Styles.font_name=wxFont(10,wxMODERN,wxNORMAL,wxNORMAL).GetFaceName();
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
	Source.autoCompletion=2;
	Source.autocompFilters=true;
	Source.callTips=true;
	Source.autocompTips=true;
	Source.avoidNoNewLineWarning=true;

	Running.cpp_compiler_options="-Wall -pedantic-errors -O0";
	Running.c_compiler_options="-Wall -pedantic-errors -O0 -lm";
	Running.wait_for_key=true;
	Running.always_ask_args=false;
	Running.check_includes=true;
	
#ifdef __WIN32__
	Init.proxy="";
#else
	Init.proxy="$http_proxy";
#endif
	Init.language_file="spanish";
	Init.max_errors=500;
	Init.max_jobs=wxThread::GetCPUCount();
	if (Init.max_jobs<1) Init.max_jobs=1;
	
#ifdef __WIN32__
	Help.wxhelp_index="MinGW\\wx\\docs\\wx_contents.html";
#else
	Help.wxhelp_index="docs/wx/wx_contents.html";
#endif
	Help.cppreference_dir="cppreference/en";
	Help.guihelp_dir="guihelp";
	Help.autocomp_indexes="AAA_Directivas_de_Preprocesador,AAA_Estandar_C,AAA_Estandar_Cpp,AAA_STL,AAA_Palabras_Reservadas,AAA_Estandar_Cpp_11,AAA_STL_11,AAA_Palabras_Reservadas_11";
	Help.min_len_for_completion=3;
	Help.show_extra_panels=true;
	
	Debug.use_colours_for_inspections = true;
	Debug.inspections_can_have_side_effects = false;
	Debug.allow_edition = false;
	Debug.autohide_panels = true;
	Debug.autohide_toolbars = true;
//	Debug.close_on_normal_exit = true;
	Debug.always_debug = false;
	Debug.raise_main_window = true;
	Debug.compile_again = true;
	Debug.format = "";
	Debug.macros_file = "debug_macros.gdb";
	Debug.inspections_on_right = false;
	Debug.show_thread_panel = false;
	Debug.show_log_panel = false;
	Debug.auto_solibs = false;
	Debug.readnow = false;
	Debug.show_do_that = false;
	Debug.return_focus_on_continue = true;
	Debug.improve_inspections_by_type = true;
#ifdef __linux__
	Debug.enable_core_dump = false;
#endif
#ifdef __WIN32__
	Debug.no_debug_heap = true;
#endif
	Debug.use_blacklist = true;
//	SetDefaultInspectionsImprovingTemplates(); // not needed, done (only on first run) in ConfigManager::Load when version<20140924
	
	for (int i=0;i<IG_COLS_COUNT;i++)
		Cols.inspections_grid[i]=true;
	for (int i=0;i<BG_COLS_COUNT;i++)
		Cols.backtrace_grid[i]=true;
//	for (int i=0;i<TG_COLS_COUNT;i++)
//		Cols.threadlist_grid[i]=true;
	
}

bool ConfigManager::CheckWxfbPresent() {
	if (config->Init.wxfb_seen && !config->Files.wxfb_command.IsEmpty()) return true;
	boolFlagGuard wxfb_working_guard(project?&(project->GetWxfbConfiguration()->working):nullptr);
	wxString out;
#ifdef __WIN32__
	if (!config->Files.wxfb_command.IsEmpty())
		out = mxUT::GetOutput(mxUT::Quotize(config->Files.wxfb_command)+" -h",true);
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\archivos de programa\\wxformbuilder\\wxformbuilder.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\Program Files\\wxformbuilder\\wxformbuilder.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\wxformbuilder\\wxformbuilder.exe"))
			out=config->Files.wxfb_command="c:\\Program Files (x86)\\wxformbuilder\\wxformbuilder.exe";
	}
#else
	if (config->Files.wxfb_command.IsEmpty()) config->Files.wxfb_command = "wxformbuilder";
#endif
	if (!project->GetWxfbConfiguration(false)->ask_if_wxfb_is_missing) return false;
	config->Init.wxfb_seen = out.Len() || CheckComplaintAndInstall(
		main_window,
		mxUT::Quotize(config->Files.wxfb_command)+" -h",
		LANG(GENERAL_WARNING,"Advertencia"),
		LANG(PROJMNGR_WXFB_NOT_FOUND,"El proyecto utiliza wxFormBuilder, pero este software\n"
		"no se encuentra correctamente instalado/configurado en\n"
		"su PC. Para descargar e instalar wxFormsBuilder dirijase\n"
		"a http://wxformbuilder.org. Si ya se encuentra instalado,\n"
		"configure su ubicación en la pestaña \"Rutas 2\" del\n"
		"dialogo de \"Preferencias\" (menu \"Archivo\")."),
		"wxformbuilder", "http://wxformbuilder.org");
	return config->Init.wxfb_seen;
}

bool ConfigManager::CheckDoxygenPresent() {
	if (config->Init.doxygen_seen) return true;
	wxString out;
#ifdef __WIN32__
	if (config->Files.doxygen_command.Len())
		out = mxUT::GetOutput(mxUT::Quotize(config->Files.doxygen_command)+" --version",true);
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\doxygen\\bin\\doxygen.exe"))
			out = config->Files.doxygen_command="c:\\archivos de programa\\doxygen\\bin\\doxygen.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\doxygen\\bin\\doxygen.exe"))
			out = config->Files.doxygen_command="c:\\Program Files\\doxygen\\bin\\doxygen.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\doxygen\\bin\\doxygen.exe"))
			out = config->Files.doxygen_command="c:\\Program Files (x86)\\doxygen\\bin\\doxygen.exe";
	}
#else
	if (!config->Files.doxygen_command.Len()) config->Files.doxygen_command="doxygen";
#endif
	config->Init.doxygen_seen = out.Len() || CheckComplaintAndInstall(
		main_window,
		mxUT::Quotize(config->Files.doxygen_command)+" --version",
		LANG(GENERAL_WARNING,"Advertencia"),
		LANG(MAINW_DOXYGEN_MISSING,"Doxygen no se encuentra correctamente instalado/configurado\n"
		"en su pc. Para descargar e instalar Doxygen dirijase a\n"
		"http://www.doxygen.org. Si ya se encuentra instalado,\n"
		"configure su ubiciación en la pestaña \"Rutas 2\" del\n"
		"dialog de \"Preferencias\" (menu \"Archivo\")."),
		"doxygen","http://www.doxygen.org");
	return config->Init.doxygen_seen;
}

#ifndef __WIN32__
bool ConfigManager::CheckValgrindPresent() {
	if (config->Init.valgrind_seen) return true;
	if (!config->Files.valgrind_command.Len()) config->Files.valgrind_command="valgrind";
	config->Init.valgrind_seen = CheckComplaintAndInstall(
		main_window,
		mxUT::Quotize(config->Files.valgrind_command)+" --version",
		LANG(GENERAL_WARNING,"Advertencia"),
		LANG(MAINW_VALGRIND_MISSING,"Valgrind no se encuentra correctamente instalado/configurado\n"
		"en su pc. Para descargar e instalar Doxygen dirijase a\n"
		"http://www.valgrind.org. Si ya se encuentra instalado,\n"
		"configure su ubiciacion en la pestaña \"Rutas 2\" del\n"
		"dialog de \"Preferencias\" (menu \"Archivo\")."),
		"valgrind","http://valgrind.org");
	return config->Init.valgrind_seen;
}
#endif

bool ConfigManager::CheckCppCheckPresent() {
	if (config->Init.cppcheck_seen) return true;
	wxString out;
#ifdef __WIN32__
	if (config->Files.cppcheck_command.Len())
		out = mxUT::GetOutput(mxUT::Quotize(config->Files.cppcheck_command)+" --version",true);
	if (!out.Len()) {
		if (wxFileName::FileExists("c:\\archivos de programa\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\archivos de programa\\cppcheck\\cppcheck.exe";
		else if (wxFileName::FileExists("c:\\Program Files\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\Program Files\\cppcheck\\cppcheck.exe";
		else if (wxFileName::FileExists("c:\\Program Files (x86)\\cppcheck\\cppcheck.exe"))
			out=config->Files.cppcheck_command="c:\\Program Files (x86)\\cppcheck\\cppcheck.exe";
	}
#else
	if (!config->Files.cppcheck_command.Len()) config->Files.cppcheck_command="cppcheck";
#endif
	config->Init.cppcheck_seen = out.Len() || CheckComplaintAndInstall(
		main_window, 
		mxUT::Quotize(config->Files.cppcheck_command)+" --version",
		LANG(GENERAL_WARNING,"Advertencia"),
		LANG(MAINW_CPPCHECK_MISSING,"CppCheck no se encuentra correctamente instalado/configurado\n"
		"en su pc. Para descargar e instalar CppCheck dirijase a\n"
		"http://cppcheck.sourceforge.net. Si ya se encuentra instalado,\n"
		"configure su ubiciación en la pestaña \"Rutas 2\" del\n"
		"dialog de \"Preferencias\" (menu \"Archivo\")."),
		"cppcheck","http://cppcheck.sourceforge.net");
	return config->Init.cppcheck_seen;
}

void ConfigManager::RecalcStuff ( ) {
	// setup some required paths
	temp_dir = DIR_PLUS_FILE(zinjai_dir,Files.temp_dir);
	if (zinjai_dir.EndsWith("\\")||zinjai_dir.EndsWith("/")) zinjai_dir.RemoveLast();
	if (temp_dir.EndsWith("\\")||temp_dir.EndsWith("/")) temp_dir.RemoveLast();
	// poner el idioma del compilador en castellano
	if (config->Init.lang_es) {
		wxSetEnv("LANG","es_ES");
		wxSetEnv("LANGUAGE","es_ES");
	} else {
		wxSetEnv("LANG","en_US");
		wxSetEnv("LANGUAGE","en_US");
	}
}

void ConfigManager::FinishiLoading ( ) {
	
	// load language translations
	if (Init.language_file!="spanish") {
		if (LANGERR_OK!=load_language(DIR_PLUS_FILE("lang",Init.language_file).c_str(),DIR_PLUS_FILE(home_dir,"lang_cache").c_str()))
			mxMessageDialog(nullptr,"No se pudo cargar el diccionario del idioma seleccionado.\n"
			"El sistema utilizará el predeterminado (spanish).\n"
			"Could not load language file. System will use default (spanish).","ZinjaI",mxMD_OK|mxMD_WARNING).ShowModal();
	}
	
	// load syntax highlighting colors' scheme
	color_theme::Initialize();
	if (Init.colour_theme.IsEmpty()) ctheme->Load(DIR_PLUS_FILE(home_dir,"colours.zcs"));
	else ctheme->Load(mxUT::WichOne(Init.colour_theme,"colours",true));
	
	// check if extern tools are present and set some paths
	Toolchain::LoadToolchains();
	DoInitialChecks(); 
	RecalcStuff();
	
	// create regular menus and toolbars' data
	menu_data = new MenusAndToolsConfig();
	if (delayed_config_lines) { // old way
		for(unsigned int i=0;i<delayed_config_lines->toolbars_keys.GetCount();i++)
			menu_data->ParseToolbarConfigLine(delayed_config_lines->toolbars_keys[i],delayed_config_lines->toolbars_values[i]); 
		delete delayed_config_lines; delayed_config_lines=nullptr;
	} else { // new way
		menu_data->LoadShortcutsSettings(DIR_PLUS_FILE(home_dir,"shortcuts.zsc"));
		menu_data->LoadToolbarsSettings(DIR_PLUS_FILE(home_dir,"toolbar.ztb"));
	}
	if (Init.version<20141030) { 
		menu_data->GetToolbarPosition(MenusAndToolsConfig::tbDEBUG)="t3";
		menu_data->GetToolbarPosition(MenusAndToolsConfig::tbSTATUS)="t3";
		menu_data->GetToolbarPosition(MenusAndToolsConfig::tbPROJECT)="T1";
	}
}

bool ConfigManager::Initialize(const wxString & a_path) {
	config = new ConfigManager(a_path);
	bool first_time = !config->Load();
	if (first_time) {
		config->SetDefaultInspectionsImprovingTemplates();
	}
	return first_time;
}


void ConfigManager::AddInspectionImprovingTemplate(const wxString &from, const wxString &to) {
	if (Debug.inspection_improving_template_from.Index(from)!=wxNOT_FOUND) return;
	Debug.inspection_improving_template_from.Add(from);
	Debug.inspection_improving_template_to.Add(to);
}

void ConfigManager::SetDefaultInspectionsImprovingTemplates ( ) {
	AddInspectionImprovingTemplate("std::string","${EXP}._M_dataplus._M_p");
	AddInspectionImprovingTemplate("std::vector<${T}, std::allocator<${T}> >",">pvector ${EXP}");
	AddInspectionImprovingTemplate("std::list<${T}, std::allocator<${T}> >",">plist ${EXP} ${T}");
	AddInspectionImprovingTemplate("std::map<${T1}, ${T2}, ${C}, std::allocator<${P}> >",">pmap ${EXP} ${T1} ${T2}");
	AddInspectionImprovingTemplate("std::stack<${T}, std::deque<${T}, std::allocator<${T}> > >",">pstack ${EXP}");
	AddInspectionImprovingTemplate("std::set<${T}, ${C}, std::allocator<${T}> >",">pset ${EXP} ${T}");
	AddInspectionImprovingTemplate("std::deque<${T}, std::allocator<${T}> >",">pdeque ${EXP}");
	AddInspectionImprovingTemplate("std::queue<${T}, std::deque<${T}, std::allocator<${T}> > >",">pqueue ${EXP}");
	AddInspectionImprovingTemplate("std::priority_queue<${T}, std::vector<${T}, std::allocator<${T}> >, ${C} >",">ppqueue ${EXP}");
	AddInspectionImprovingTemplate("std::bitset<${N}>",">pbitset ${EXP}");
}

bool ConfigManager::CheckComplaintAndInstall(wxWindow *parent, const wxString &check_command, const wxString &what, const wxString &error_msg, const wxString &pkgname, const wxString &website) {
	wxString check_output = mxUT::GetOutput(check_command,true);
	if (check_output.Len() && !check_output.StartsWith("execvp")) return true; // si anda, ya esta instalada
	wxString chk_message = GetTryToInstallCheckboxMessage(); // ver si tenemos apt-get
	if (!chk_message && website.Len()) // si no lo tenemos, talvez tengamos el link al sitio de descarga
		chk_message = LANG(CONFIG_GOTO_PACKAGE_WEBSITE,"Abrir sitio el web de esta herramienta");
	int ans = mxMessageDialog(parent,error_msg,what,mxMD_OK|mxMD_WARNING,chk_message,true).ShowModal(); // informar/preguntar
	if (ans&mxMD_CHECKED) {
		if (GetTryToInstallCheckboxMessage()) { // si había apt-get, 
			TryToInstallWithAptGet(parent,what,pkgname); // intentar instalar
			check_output = mxUT::GetOutput(check_command,true);
			if (check_output.Len() && !check_output.StartsWith("execvp")) // si anda, ya esta instalada
				return true; 
			// si falló apt-get, avisar e intentar abrir el sitio web de descarga
			ans = mxMessageDialog(
				nullptr,LANG(CONFIG_APTGET_FAILED,"Falló la instalación automática."),what,mxMD_OK|mxMD_WARNING,
				(website.Len()?(LANG(CONFIG_GOTO_PACKAGE_WEBSITE,"Abrir sitio el web de esta herramienta")):""),true
				).ShowModal();
		}
		if (ans&mxMD_CHECKED) { // si no había apt-get, o si fallo (por eso repito la pregunta del if), abrir el sitio web
			mxUT::OpenInBrowser(website);
		}
	}
	return false;
}

void ConfigManager::TryToInstallWithAptGet (wxWindow * parent, const wxString & what, const wxString & pkgname) {
	mxMessageDialog(parent,LANG(CONFIG_ABOUT_TO_APTGET,"A continuación se intentará instalar el software faltante en una nueva\n"
		"terminal. Podría requerir ingresar la contraseña del administrador.\n"
		"ZinjaI continuará cuando se cierre dicha terminal."),what).ShowModal();
	wxExecute(mxUT::GetCommandForRunningInTerminal(
		wxString("ZinjaI - sudo apt-get install ")+pkgname,
		wxString("sudo apt-get install ")+pkgname ),wxEXEC_SYNC);
}

wxString ConfigManager::GetTryToInstallCheckboxMessage ( ) {
#ifdef __linux__
	if (mxUT::GetOutput("apt-get --version").Len())	
		return LANG(CONFIG_APTGET_BUILD_ESSENTIAL,"Intentar instalar ahora");
#endif
	return "";
}

