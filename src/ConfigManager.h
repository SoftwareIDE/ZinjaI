#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <wx/string.h>
#include "mxInspectionGrid.h" // por IG_COLS_COUNT
#include "mxBacktraceGrid.h" // por BG_COLS_COUNT
//#include "mxThreadGrid.h" // por TG_COLS_COUNT

#define CM_HISTORY_MAX_LEN 50

#define CFG_BOOL_READ_DN(name,what) if (key==_T(name)) what=utils->IsTrue(value)
#define CFG_BOOL_READ(where,what) if (key==_T(#what)) where.what=utils->IsTrue(value)
#define CFG_INT_READ(where,what) if (key==_T(#what)) utils->ToInt(value,where.what)
#define CFG_INT_READ_DN(name,what) if (key==_T(name)) utils->ToInt(value,what)
#define CFG_GENERIC_READ(where,what) if (key==_T(#what)) where.what=value
#define CFG_GENERIC_READ_DN(name,what) if (key==_T(name)) what=value
#define CFG_TEXT_READ_DN(name,what) if (key==_T(name)) what=utils->Line2Text(value)
#define CFG_CHAR_READ_DN(name,what) if (key==_T(name)) what=value[0]

#define CFG_BOOL_WRITE(where,what) fil.AddLine(wxString(#what"=")<<(where.what?"1":"0"))
#define CFG_BOOL_WRITE_DN(name,what) fil.AddLine(wxString(name)<<(what?"=1":"=0"))
#define CFG_GENERIC_WRITE(where,what) fil.AddLine(wxString(#what"=")<<where.what);
#define CFG_GENERIC_WRITE_DN(name,what) fil.AddLine(wxString(name"=")<<what);
#define CFG_TEXT_WRITE_DN(name,what) fil.AddLine(wxString(name"=")<<utils->Text2Line(what));
#include "mxCustomTools.h"

//! Elementos de la configuración relacionados a la depuración
struct cfgDebug {
	bool autohide_panels;
	bool autohide_toolbars;
	bool allow_edition;
	bool close_on_normal_exit;
	bool always_debug;
	bool compile_again;
	bool raise_main_window;
	bool select_modified_inspections;
	wxString blacklist;
	wxString format;
	wxString macros_file;
	bool readnow;
	bool show_do_that;
	bool inspections_on_right;
	bool show_thread_panel;
	bool show_log_panel;
	bool auto_solibs;
	bool improve_inspections_by_type;
	wxArrayString inspection_improving_template_from, inspection_improving_template_to;
};

//! Elementos de la configuración relacionados al aspecto del código
struct cfgStyles {
	int font_size;
	int print_size;
};

//! Elementos de la configuración relacionados al comportamiento de un fuente
struct cfgSource {
 	bool smartIndent;
 	bool bracketInsertion;
 	bool indentPaste;
	bool syntaxEnable;
	bool foldEnable;
	bool indentEnable;
	bool whiteSpace;
	bool wrapMode; ///< indica si el ajuste de linea esta activado para un fuente en particular
	bool lineNumber;
	bool overType;
	int tabWidth;
	bool tabUseSpaces;
	bool autoCompletion;
	bool callTips;
	bool avoidNoNewLineWarning;
	bool toolTips;
	bool autotextEnabled;
	bool autocloseStuff;
	int alignComments;
};

//! Elementos de la configuración relacionados a la ejecución
struct cfgRunning {
	bool dont_run_headers;
	bool check_includes;
	wxString cpp_compiler_options;
	wxString c_compiler_options;
	bool wait_for_key;
	bool always_ask_args;
};

//! Elementos de la configuración relacionados al sistema de ayuda
struct cfgHelp {
//	wxString quickhelp_index;
	wxString wxhelp_index;
	wxString cppreference_dir;
	wxString guihelp_dir;
	wxString autocomp_indexes;
//	wxString autocomp_dir;
	int min_len_for_completion;
	bool show_extra_panels;
};

//! Elementos de la configuración relacionados a la inicialización y otros aspectos generales
struct cfgInit {
	int version;
	wxString proxy;
	bool show_tip_on_startup;
	bool show_splash;
	bool show_welcome;
	bool show_beginner_panel;
	bool hide_splash;
	bool left_panels;
	int new_file;
	int pos_x,pos_y;
	int size_x,size_y;
	bool maximized;
	int zinjai_server_port;
	bool lang_es;
	bool save_project;
//	bool close_files_for_project;
	bool always_add_extension;
	bool autohide_toolbars_fs;
	bool autohide_panels_fs;
	bool autohide_menus_fs;
	bool check_for_updates;
	int history_len;
	bool prefer_explorer_tree;
	bool show_explorer_tree;
	bool graphviz_dot;
	int inherit_num;
#if !defined(_WIN32) && !defined(__WIN32__)
	bool valgrind_seen; ///< indica si se ha detectado en el sistema el ejecutable de valgrind
	bool compiler_seen; ///< indica si se ha detectado en el sistema el compilador instalado
	bool debugger_seen; ///< indica si se ha detectado en el sistema el depurador instalado
#endif
	bool cppcheck_seen; ///< indica si se ha detectado en el sistema el ejecutable de cppcheck
	bool doxygen_seen; ///< indica si se ha detectado en el sistema el depurador instalado
	bool wxfb_seen; ///< indica si se ha detectado en el sistema el depurador instalado
	int max_errors; ///< numero maximo de items el las ramas del arbol de resultados de la compilacion
	wxString language_file; ///< archivo de internacionalizacion a utilizar (.pre, pero se guarda sin extension)
	int wrap_mode; ///< indica el ajuste de linea (0=no, 1=fuentes, 2=todos), conf global
	bool singleton; ///< indica si al inciarse con argumentos, los programas simples deben buscar si hay otra instancia de zinjai
	bool stop_compiling_on_error; ///< detener compilacion de un proyecto al encontrar un error
	bool autohide_panels; ///< ocultar automaticamente los paneles dockeados en la ventana principal, y mostrar solo cuando el mouse se lleve al borde (opcion guardada)
	bool autohiding_panels; ///< ocultar automaticamente los paneles dockeados en la ventana principal, y mostrar solo cuando el mouse se lleve al borde (opcion activa)
	bool fullpath_on_project_tree; ///< mostrar rutas relativas completas en el arbol de proyecto cuando es un proyecto
	int max_jobs; ///< cantidad máxima de procesos (pasos) corriendo en paralelo al compilar un proyecto
	wxString colour_theme; ///< archivo de donde tomar el perfil de colores (si es "" se busca en .zinjai/colours.zcs, sino en zinjai/colours)
	bool beautify_compiler_errors; ///< trata de arreglar los horribles mensajes de error del compilador cuando se usan templates (como todo stl) o cosas estándar de implemetación "rebuscada" (como strings)
};

//! Elementos de la configuración relacionados a la visualización y composición de las barras de herramientas
struct cfgToolBars {
	
	int icon_size;
	
	//! Información sobre la posición y visibilidad de las barras de herramientas
	struct cfgTbPositions {
		struct pos_item {
			bool top,left,right,visible;
			long row/*,pos*/;
			pos_item():top(true),left(false),right(false),visible(false),row(0)/*,pos(0)*/ {}
			void operator=(wxString s) { 
				if (s.Len()==0) return;
				else if (s[0]=='T') { top=true; left=right=false; visible=true; }
				else if (s[0]=='R') { right=true; left=top=false; visible=true; }
				else if (s[0]=='L') { left=true; top=right=false; visible=true; }
				else if (s[0]=='t') { top=true; left=right=false; visible=false; }
				else if (s[0]=='r') { right=true; left=top=false; visible=false; }
				else if (s[0]=='l') { left=true; top=right=false; visible=false; }
				s=s.Mid(1); if (s.Len()==0) return; else s.ToLong(&row);
			}
			operator wxString() {
				wxString s;
				if (left) s<<"L"<<row; 
				else if (right) s<<"R"<<row; 
				else s<<"T"<<row;
				if (!visible) s[0]+=32;
				return s;
			}
		};
		pos_item file;
		pos_item edit;
		pos_item run;
		pos_item tools;
		pos_item view;
		pos_item misc;
		pos_item debug;
		pos_item find;
		pos_item status;
		pos_item project;
	} positions;
	
	//! Determina los componentes de la barra de herramientas "Archivo"
	struct cfgTbFile {
		bool new_file;
		bool new_project;
		bool open;
		bool open_header;
		bool recent_simple;
		bool recent_project;
		bool open_selected;
		bool save;
		bool save_as;
		bool save_all;
		bool save_project;
		bool print;
		bool export_html;
		bool reload;
		bool close;
		bool close_all;
		bool close_project;
		bool project_config;
	} file;
	
	//! Determina los componentes de la barra de herramientas "Edición"
	struct cfgTbEdit {
		bool undo;
		bool redo;
		bool cut;
		bool copy;
		bool paste;
		bool move_up;
		bool move_down;
		bool duplicate;
		bool delete_lines;
		bool goto_class;
		bool goto_line;
		bool goto_file;
		bool find;
		bool find_prev;
		bool find_next;
		bool replace;
		bool inser_header;
		bool comment;
		bool uncomment;
		bool indent;
		bool select_block;
		bool select_all;
		bool toggle_user_mark;
		bool find_user_mark;
		bool list_user_marks;
		bool autocomplete;
	} edit;
	
	//! Determina los componentes de la barra de herramientas "Ver"
	struct cfgTbView {
		bool split_view;
		bool white_space;
		bool line_wrap;
		bool sintax_colour;
		bool update_symbols;
		bool explorer_tree;
		bool project_tree;
		bool compiler_tree;
		bool full_screen;
		bool beginner_panel;
		bool next_error;
		bool prev_error;
		bool fold_all;
		bool fold_1;
		bool fold_2;
		bool fold_3;
		bool unfold_all;
		bool unfold_1;
		bool unfold_2;
		bool unfold_3;
	} view;
	
	//! Determina los componentes de la barra de herramientas "Ejecución"
	struct cfgTbRun {
		bool run;
		bool run_old;
		bool compile;
		bool stop;
		bool clean;
		bool options;
		bool debug;
#if !defined(__WIN32__)
		bool debug_attach;
		bool load_core_dump;
#endif
		bool break_toggle;
		bool break_options;
		bool break_list;
		bool inspections;
	} run;
	
	//! Determina los componentes de la barra de herramientas "Herramientas"
	struct cfgTbTools {
		bool draw_flow;
		bool draw_classes;
		bool draw_project;
		bool remove_comments;
		bool align_comments;
		bool preproc_mark_valid;
		bool preproc_unmark_all;
		bool preproc_expand_macros;
		bool copy_code_from_h;
		bool open_terminal;
		bool generate_makefile;
		bool proy_stats;
		bool exe_info;
		bool open_shared;
		bool share_source;
		bool share_list;
		bool diff_two_sources;
		bool diff_himself;
		bool diff_other_file;
		bool diff_clear;
		bool diff_show;
		bool diff_apply;
		bool diff_discard;
		bool doxy_generate;
		bool doxy_config;
		bool doxy_view;
		bool wxfb_config;
		bool wxfb_new_res;
		bool wxfb_load_res;
		bool wxfb_regen;
		bool wxfb_inherit;
		bool wxfb_update_inherit;
		bool wxfb_help_wx;
		bool gprof_activate;
		bool gprof_show_graph;
		bool gprof_list_output;
		bool gcov_activate;
		bool gcov_reset;
		bool gcov_show_bar;
		bool cppcheck_run;
		bool cppcheck_config;
		bool cppcheck_view;
#if !defined(__WIN32__)
		bool valgrind_run;
		bool valgrind_view;
#endif
//		bool custom[10];
		bool custom_settings;
	} tools;
	
	//! Determina los componentes de la barra de herramientas "Depuración"
	struct cfgTbDebug {
		bool start;
		bool break_toggle;
		bool pause;
		bool stop;
		bool step_in;
		bool step_out;
		bool step_over;
		bool function_return;
		bool jump;
		bool run_until;
		bool inspections;
		bool threadlist;
		bool backtrace;
		bool break_list;
		bool break_options;
		bool inverse_exec;
		bool enable_inverse_exec;
		bool log_panel;
	} debug;
	
	//! Determina los componentes de la barra de herramientas "Miscelánea"
	struct cfgTbMisc {
		bool preferences;
		bool about;
		bool opinion;
		bool tutorials;
		bool help_ide;
		bool help_cpp;
		bool show_tips;
		bool find_updates;
		bool exit;
	} misc;
	
	//! Determina que columnas se veran el las tables
};

//! Elementos de la configuración relacionados a rutas de archivos y directorios
struct cfgFiles {
	wxString temp_dir;
	wxString skin_dir;
	wxString default_template; 
	wxString default_project; 
//	wxString templates_dir; 
//	wxString mingw_dir;
	wxString toolchain;
	wxString runner_command;
	wxString debugger_command;
	wxString parser_command;
	wxString terminal_command; ///< comando para abrir un terminal y ejecutar algo en el
	wxString explorer_command; ///< comando para abrir un explorador de archivos
//	wxString code_helper;
	wxString project_folder;
	wxString graphviz_dir;
	wxString xdot_command; ///< nice python app for displaying graphs
	wxString img_browser;
	wxString last_source[CM_HISTORY_MAX_LEN];
	wxString last_project[CM_HISTORY_MAX_LEN];
	wxString last_dir;
	wxString last_project_dir;
	wxString browser_command;
	wxString cppcheck_command;
	wxString valgrind_command;
	wxString doxygen_command;
	wxString wxfb_command;
	wxString autocodes_file;
};

struct cfgCols {
	bool inspections_grid[IG_COLS_COUNT];
	bool backtrace_grid[BG_COLS_COUNT];
//	bool threadlist_grid[TG_COLS_COUNT];
};

/**
* @brief Administra la configuración completa del sistema
**/
class ConfigManager {
public:
	wxString filename;
	wxString home_dir;
	wxString zinjai_dir;
	wxString temp_dir;
//	wxString mingw_real_path;
	cfgSource Source;
	cfgToolBars Toolbars;
	bool first_run;
	cfgRunning Running;
	cfgHelp Help;
	cfgStyles Styles;
	cfgInit Init;
	cfgFiles Files;
	cfgCols Cols;
	cfgDebug Debug;
	cfgCustomTool CustomTools[MAX_CUSTOM_TOOLS];
	ConfigManager(wxString a_path);
	void DoInitialChecks();
	bool Load();
	bool Save();	
	void LoadDefaults();
	void LoadToolBarsDefaults();
	
	void RecalcStuff(); /// @brief arma las cadenas (paths) que no se graban en la configuracion, pero que dependen de esta (ejemplo: temp_dir)
	
	/// @brief Verifica si esta instalado y configurado el path para llamar a wxFormBuilder
	bool CheckWxfbPresent();
	/// @brief Verifica si esta instalado y configurado el path para llamar a Doxygen
	bool CheckDoxygenPresent();
	/// @brief Verifica si esta instalado y configurado el path para llamar a CppCheck
	bool CheckCppCheckPresent();
	
	/// @brief return default compiler arguments for simple programs (for_cpp=true: c++(g++) arguments, for_cpp=false, c(gcc) arguments)
	wxString GetDefaultCompilerOptions(bool for_cpp) { return for_cpp?Running.cpp_compiler_options:Running.c_compiler_options; }
	
};

extern ConfigManager *config;

#endif
