#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <ctime>
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/treebase.h>
#include "mxUtils.h"
#include <wx/arrstr.h>

#define PROJECT_EXT "zpr"

#if defined(_WIN32) || defined(__WIN32__)
// maldito seas "winbase.h"
#define MoveFile MoveFileA
#define DeleteFile DeleteFileA
#endif
class mxSource;

enum ces_pos {CES_BEFORE_SOURCES=0, CES_BEFORE_LIBS, CES_BEFORE_EXECUTABLE, CES_AFTER_LINKING, CES_COUNT};

enum makefile_type {MKTYPE_FULL,MKTYPE_OBJS,MKTYPE_CONFIG};

struct compile_and_run_struct_single;

/** 
* @brief Paso de compilaci�n adicional
* 
* Estructura que representa la informaci�n de un paso de compilaci�n personalizado de una configuraci�n.
* Es contenida por la configuraci�n, en forma de lista doblemente enlazada ad-hoc (sin primer nodo ficticio)
**/
struct compile_extra_step {
	wxString deps, out, command, name;
	int pos;
	compile_extra_step *next, *prev;
	bool done;
	bool check_retval; ///< si debe verificar que el comando retorne 0 para saber si continuar o abortar la compilacion
	bool hide_window; ///< si debe ocultar la ventana/consola donde se ejecuta
	bool delete_on_clean; ///< si debe eliminar este archivo al hacer un clean del proyecto
	bool link_output; ///< si debe agregar el archivo de salida a la lista de objetos a enlazar
	compile_extra_step():deps(_T("")),out(_T("")),command(_T("")),name(_T("<noname>")),pos(0),next(NULL),prev(NULL),done(false),check_retval(false),hide_window(true),delete_on_clean(true),link_output(false) {}
};

/** 
* @brief Configuraci�n para Doxygen de un proyecto
* 
* Estructura que representa la informaci�n para configurar doxygen. El archivo Doxyfile se regenera
* cada vez que hay que generar la documentaci�n, a partir de esta estructura. Existe, cuanto mucho 
* una instancia (para el proyecto en curso). Se referencia con el puntero doxygen_configuration de
* ProjectManager, que puede ser NULL si no se ha definido la configuarci�n Doxygen para dicho proyecto
* Si el puntero es valido, y el atributo save es true, se graba en el archivo de proyecto
**/
struct doxygen_configuration {
	wxString name; ///< nombre del proyecto para mostrar en la documentaci�n
	wxString version; ///< versi�n del proyecto para mostrar en la documentaci�n
	wxString destdir; ///< directorio donde colocar la documentaci�n generada
	wxString extra_files; ///< archivos adicionales a procesar, ademas de lo especificado por do_headers y do_cpps
	wxString exclude_files; ///< archivos que no se deben procesar, aunque esten entre los fuentes o cpps
	bool do_headers; ///< si debe procesar los archivos de cabeceras (.h) del proyecto
	bool do_cpps; ///< si debe procesar los archivos fuentes (.cpp) del proyecto
	bool hideundocs; ///< indica si no debe generar documentaci�n para los elementos que no tengan sus respectivos comentarios
	bool save; ///< si se debe o no recordar esta conf (guardar en el archivo de proyecto)
	bool latex; ///< indica si debe generar la documentaci�n en formato Latex
	bool html; ///< indica si debe generar la documentaci�n en formato HTML
	bool html_searchengine; ///< indica si debe inlcuir el motor de busqueda (basado en PHP) para la vers��n HTML
	bool html_navtree; ///< indica si debe generar un panel con un �rbol de navegaci�n en la versi�n HTML
	bool extra_static; ///< indica si deben incluirse funciones/variables static
	bool extra_private; ///< indica si deben incluirse miembros/atributos privados
	bool preprocess; ///< indica si debe habilitar o no el preprocesado (considerar macros)
	bool use_in_quickhelp; ///< indica si se debe agregar el texto generado por doxygen a la ayuda rapida de zinjai
	wxString lang; ///< indica el lenguaje en el cual se genera la documentaci�n (English o Spanish)
	wxString base_path; ///< indica el path base para expresar los nombres de los archivos relativos a este
	wxString extra_conf; ///< texto a incluir directamente y tal cual en el Doxyfile (para opciones no contempladas)
	
	//! inicializa la configuraci�n con los valores por defecto
	doxygen_configuration(wxString aname) {
		name = aname;
		save = true;
		destdir = _T("docs");
		html = true;
		html_navtree = false;
		html_searchengine = true;
		latex = false;
		hideundocs = false;
		do_headers = do_cpps = true;
		extra_static = false;
		extra_private = false;
		preprocess = true;
		lang = _T("Spanish");
		use_in_quickhelp = true;
	}
	
	static wxString get_tag_index();
	
};


/**
* @brief Guarda informaci�n sobre una libreria a generar con el proyecto
**/
struct project_library {
	wxString libname; ///< nombre amigable
	wxString filename; ///< archivo de salida (destino)
	wxString path; ///< archivo de salida (destino)
	wxString sources; ///< fuentes que poner dentro
	wxString extra_link; ///< opciones extra para el linker
	bool is_static; ///< estatica o dinamica
	bool default_lib; ///< por defecto para nuevos fuentes
	bool need_relink; ///< temporal, para AnalizeConfig y PrepareForBuilding
	wxString objects_list; ///< temporal, para AnalizeConfig y PrepareForBuilding
	wxString parsed_extra; ///< temporal, para AnalizeConfig y PrepareForBuilding
	project_library *next;
	project_library *prev;
	project_library(project_library *aprev=NULL) {
		is_static=true;
		need_relink=false;
		next=NULL;
		prev=aprev;
	}
};

/** 
* @brief Perfil de compilaci�n y ejecuci�n para un proyecto
* 
* Estructura que representa la informaci�n de un perfil de compilacion (paths, banderas, librerias
* nivel de info de debug, nivel de warnings, pasos personalizados, etc. Puede haber muchas por proyecto
* (actualmente hasta 100). ProjectManager tiene un arreglo configurations (creado dinamicamente) para
* guardarlas, y un contador configurations_count para saber la cantidad
**/
struct project_configuration {

	project_configuration *bakup; ///< copia antes de modificar en el dialogo de opciones, para usar si se cancela
	
	wxString name; ///< nombre de la configuracion (unico y definido por usuario)
	
	wxString working_folder; ///< directorio de trabajo para la ejecuci�n
	bool always_ask_args; ///< mostrar el dialogo que pide los argumentos antes de ejecutar
	wxString args; ///< argumentos para la ejecucion
	int wait_for_key; ///< esperar una tecla antes de cerrar la consola luego de la ejecuci�n (0=nunca, 1=solo en caso de error, 2=siempre)
	
	wxString temp_folder; ///< directorio temporal donde poner los objetos de la compilacion
	wxString output_file; ///< archivo de salida de la compilacion = ruta del ejecutable
	wxString icon_file; ///< archivo con el icono para compilar como recurso (solo windows)
	wxString manifest_file; ///< archivo con el manifest.xml para compilar como recurso (solo windows)
	
	wxString compiling_extra; ///< parametros adicionales para el compilador
	wxString macros; ///< lista de macros a definir (con -D)
	wxString old_macros; ///< lista de macros a definir en la ultima corrida (para comparar y saber que recompilar por cambio de macros)
	wxString headers_dirs; ///< lista de rutas adicionales donde buscar cabeceras (para pasar con -I)
	int warnings_level; ///< nivel de advertencias a mostrar por el compilador: 0=ninguna 1=default 2=todas
	bool ansi_compliance; ///< forzar a cumplir el estandar (-pedantic-errors)
	int debug_level; ///< nivel de informaci�n de depuraci�n a colocar al compilar: 0=g0 1=g1 2=g2
	int optimization_level; ///< nivel optimizaci�n para los binarios: 0=O0 1=O1 2=O2 3=O3 4=Os
	bool enable_profiling; ///< habilitar la generaci�n de informaci�n de profiling (para gprof, -pg)

	wxString linking_extra; ///< parametros adicionales para el enlazador (se llama a travez de gcc/g++, no directo)
	wxString libraries_dirs; ///< rutas adicionales para buscar librerias (para pasar con -L)
	wxString libraries; ///< librearias para enlazar (para pasar con -l)
	bool strip_executable; ///< stripear el ejecutable (-s)
	bool console_program; ///< marcar como programa de consola (sino, no usar el runner y compilar con -mwindows)
	compile_extra_step *extra_steps; ///< puntero al primer item de la lista de pasos adicionales para la compilacion (sin primer nodo ficticio), NULL si no hay pasos extra
	project_library *libs_to_build; ///< bibliotecas a construir, lista enlazada sin primer nodo ficticio
	bool dont_generate_exe; ///< no generar ejecutable, solo bibliotecas
		
	//! inicializa la configuraci�n con los valores por defecto
	project_configuration(wxString pname, wxString cname) {
		libs_to_build=NULL;
		bakup=NULL;
		name=cname;
		working_folder=_T("");
		always_ask_args=false;
		args=_T("");
		wait_for_key=2;
		temp_folder=cname;
		output_file=DIR_PLUS_FILE(cname,pname+_T(BINARY_EXTENSION));
		compiling_extra=_T("");
		macros=_T("");
		headers_dirs=_T("");
		warnings_level=1;
		ansi_compliance=false;
		enable_profiling=false;
		debug_level=2;
		optimization_level=0;
		linking_extra=_T("");
		libraries_dirs=_T("");
		libraries=_T("");
		strip_executable=false;
		console_program=true;
		dont_generate_exe=false;
		extra_steps=NULL;
	}
};

//! Informaci�n acerca de un punto de interrupci�n en un fuente
struct break_line_item {
	int handle, num, line;
	break_line_item *next, *prev;
	bool enabled, error,valid_cond, only_once;
	int ignore_count;
	wxString cond;
	break_line_item(int ln, break_line_item *prv, break_line_item *nxt=NULL) {
		next=nxt; prev=prv; ignore_count=0;
		line=ln; enabled=true; error=false;
		valid_cond=true; only_once=false;
	}
};

//! Informaci�n acerca de una linea de c�digo resaltada por el usuario
struct marked_line_item {
	int line;
	marked_line_item *next;
	marked_line_item(int ln, marked_line_item *nxt) {
		next=nxt;
		line=ln;
	}
};

class project_library;

/** 
* @brief Representa un archivo de un proyecto
* 
* Representa un archivo de un proyecto. Funciona tambi�n como nodo para una lista
* enlazada. ProjectManager tiene los tres punteros que inician las tres listas (fuentes,
* cabeceras y otros). Cada estructura guarda el nombre del archivo, sus listas breakpoints
* y lineas resaltadas y algunas banderas.
**/
struct file_item { // para armar las listas (doblemente enlazadas) de archivos del proyecto
	wxString name;
	wxTreeItemId item;
	file_item *prev, *next; ///< para enlazar los nodos de la lista
	break_line_item *breakpoints; ///< primer item de la lista de breakpoints del fuente (sin primer elemento ficticio, se asigna con GetSourceExtras)
	marked_line_item *markers; ///< primer item de la lista de lineas resaltadas del fuente (sin primer elemento ficticio, se asigna con GetSourceExtras)
	bool force_recompile; ///< indica se debe recompilar independientemente de la fecha de modificacion (por ejemplo, si lo va a modificar un paso adicional)
	int cursor; ///< posicion del cursor en el texto (se asigna con GetSourceExtras)
	project_library *lib; ///< a que biblioteca pertenece (no siempre es correcto, se rehace con analize_config)
	
	file_item (file_item *p, wxString &n, wxTreeItemId &i) {
		force_recompile=false;
		cursor=0;
		breakpoints=NULL;
		markers=NULL;
		name=n;
		item=i;
		prev=p;
		next=NULL;
	}
	file_item () {
		force_recompile=false;
		name=_T("");
		breakpoints=NULL;
		markers=NULL;
		cursor=0;
		next=NULL;
		prev=NULL;
	}
	void ClearExtras(bool all=false) {
		if (all) { // solo se borran si se destruye el archivo... pero no cuando se "recolocan" los extras en el fuente
			break_line_item *bit = breakpoints, *bit2;
			while (bit) {
				bit2=bit->next;
				delete bit;
				bit=bit2;
			}
			breakpoints=NULL;
		}
		marked_line_item *mit = markers, *mit2;
		while (mit) {
			mit2=mit->next;
			delete mit;
			mit=mit2;
		}
		markers=NULL;
	}
	~file_item() {
		ClearExtras(true);
	}
};

enum ces_type{CNS_VOID,CNS_SOURCE,CNS_BARRIER,CNS_EXTRA,CNS_LINK,CNS_ICON};

/** 
* @brief Estructura que representa un paso gen�rico del proceso de compilaci�n.
* 
* Estructura que representa un paso gen�rico del proceso de compilaci�n. Puede contener
* info para compilar un fuente normal, un paso personalizado, o enlazar. El ProyectManager 
* tiene una lista de estos que empieza en first_compile_step (sin primer nodo ficticio).
* Esta lista se arma en PrepareForBuilding, y cada vez que se llama a CompileNext se 
* ejecuta el primer paso, y se elimina de la lista.
**/
struct compile_step {
	ces_type type; ///< que se hace en este paso (compilar fuente, paso personalizado, enlazar
	bool start_parallel; ///< indica si en este paso se debe comenzar a paralelizar (es decir, si es el primer paso de una etapa paralelizable)
	void *what; ///< puntero al file_item si es un fuente, al compile_extra_step si es un paso personalizado, o basura si para enlazar
	compile_step *next; ///< puntero al siguiente paso (lista simplemente enlazada)
	//! constructor
	compile_step(ces_type t, void *w) : type(t),start_parallel(false),what(w),next(NULL) {}
};


/// estructura auxiliar para el puntero what del compile_step cuando type==LINK_*
struct linking_info {
	wxString command;
	wxString output_file;
	wxString objects_list;
	wxString extra_args;
	bool *flag_relink;
	linking_info(wxString cmd, wxString output, wxString objects, wxString args, bool *flag) :
		command(cmd), output_file(output), objects_list(objects),extra_args(args),flag_relink(flag) {};
};


/** 
* @brief Representa un proyecto
* 
* Contiene sus perfiles de ejecuci�n y configuraci�n, sus archivos, su informaci�n 
* adicional, y encapsula todos los m�todos espec�ficos para admistrar, compilar y 
* ejecutar proyectos. No se puede crear en blanco, sino que siempre a partir de un 
* archivo. En la aplicaci�n existe un puntero proyect que siempre apunta al proyecto 
* actual; o es NULL si no hay proyecto abierto.
**/
class ProjectManager {
public:
	compile_step *first_compile_step;
	bool loading; ///< indica que se esta cargando, para que otros eventos sepan
	time_t compile_startup_time;
	int version_required; ///< minima version de ZinjaI requerida para abrir el proyecto sin perder nada
	int steps_count; ///< temporal para contar los pasos totales para una compilacion
	int current_step; ///< temporal para contar cuantos pasos van durante una compilacion
	float actual_progress; ///< temporal para calcular el porcentaje de progreso segun las cuentas de pasos
	float progress_step; ///< temporal para guardar de a cuanto avanza la barra de progreso al avanzar un paso
	bool force_relink;	///< indica si debe reenlazar si o si en la proxima compilacion, aunque el exe est� al d�a
	bool use_wxfb; ///< indica si esta activada la integracion con wxFormBuilder
	bool auto_wxfb; ///< indica si la integracion con wxFormBuilder es automatica (regenera solo al activar la ventana)
	bool generating_wxfb; ///< indica si la generacion o actualizacion esta en proceso
	bool wxfb_ask; ///< indica si avisar cuando no encuentra el ejecutable del wxFormBuilder
	wxArrayString wxfbHeaders;
	doxygen_configuration *doxygen; ///< configuracion para el Doxyfile, NULL si no esta definido en el proyecto
	project_configuration *active_configuration; ///< puntero a la configuracion activa
	project_configuration *configurations[100]; ///< arreglo de configuraciones, ver configurations_count
	int configurations_count; ///< cantidad de configuraciones validas en configurations
	wxString project_name; ///< el nombre bonito del proyecto
	wxString filename; ///< el archivo del proyecto
	wxString path; ///< la carpeta del proyecto
	file_item *first_header; ///< primeros nodos (ficticios) de la lista de archivos de cabeceras
	file_item *first_source; ///< primeros nodos (ficticios) de la lista de archivos fuentes
	file_item *first_other; ///< primeros nodos (ficticios) de la lista de otros archivos
	wxString autocomp_extra; ///< indices de autocompletado adicionales para este proyecto
	wxString autocodes_file; ///< archivo con definiciones de autocodigos adicionales
	wxString macros_file; ///< archivo con definiciones de macros para gdb
	wxString temp_folder; ///< ruta completa a la carpeta de temporales
	wxString temp_folder_short; ///< ruta relativa (tal cual ingresa el usuario) a la carpeta de temporales
	wxString executable_name; ///< ubicacion final del ejecutable (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString linking_options; ///< argumentos para el enlazado del ejecutable (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString compiling_options; ///< argumentos para las compilaciones (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString objects_list; ///< lista de objetos y bibliotecas (con -L) para usar en la linea de compilacion (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString exe_deps; ///< temporal, para pasar de AnalizeConfig a ExportMakefile las dependencias del ejecutable (la diff con objects_list esta en los -l de las bibliotecas)
	int custom_tabs; ///< tipo de tabulado en los fuentes, si es 0, hereda de la configuracion de zinjai, si no usa ese
	bool tab_use_spaces; ///< indica si los tabs en los fuentes deben reemplazarse por espacios (solo si custom_tabs no es 0)
	bool config_analized; ///< indica si ya se actualizo la info para compilar (se usa en AnalizeConfig cuando force=false)
	bool compile_was_ok; ///< indica si se arrastra algun error de compilacion de pasos anteriores
	
	wxString last_dir; ///< ultimo path utilizado por el usuario, para los cuadros de abrir...
	bool modified; ///< indica si algo cambi� en la configuraci�n del proyecto
	wxArrayString warnings; ///< advertencias generadas por ZinjaI que deben agregarse mas tarde al arbol de resultados
	
	project_configuration *GetConfig(wxString name); ///< busca una configuracion por nombre, y devuelve un puntero si la encuentra, NULL si no
	
	bool GenerateDoxyfile(wxString fname); ///< escribe el archivo de configuraci�n para Doxygen
	wxString WxfbGetSourceFile(wxString fbp_file); ///< funcion auxiliar que devuelve el nombre (path+nombre, sin extension) de los archivos que genera el archivo .fbp que recibe
	
	ProjectManager(wxFileName filename);
	~ProjectManager();
	wxString GetFileName();
	int GetFileList(wxArrayString &array, char cuales='*', bool relative_paths=false);
	file_item *FindFromName(wxString name, char *where=NULL); ///< busca a partir del nombre de archivo (solo, sin path)
	file_item *FindFromItem(wxTreeItemId &tree_item);
	wxString GetNameFromItem(wxTreeItemId &tree_item, bool relative=false);
	file_item *HasFile(wxFileName file); ///< busca una archivo en el proyecto por su ruta completa
	bool Save();
	void MoveFirst(wxTreeItemId &tree_item);
	
	//! guarda todos los archivos del proyecto que esten abiertos
	void SaveAll(bool save_project=true);
	
	/// Copia la posici�n del cursor, las breakpoints y la lista de lineas resaltadas desde un mxSource a un file_item del proyecto
	void GetSourceExtras(mxSource *source, file_item *item=NULL);
	/// Copia la posici�n del cursor, las breakpoints y la lista de lineas resaltadas desde un file_item del proyecto a un mxSource
	void SetSourceExtras(mxSource *source, file_item *item=NULL);
	/// Determina si el archivo item usa alguna de las macros de la lista macros
	bool DependsOnMacro(file_item *item, wxArrayString &macros);
	bool PrepareForBuilding(file_item *only_one=NULL);
	long int CompileNext(compile_and_run_struct_single *compile_and_run, wxString &object_name);
	long int CompileIcon(compile_and_run_struct_single *compile_and_run, wxString icon_file);
	long int CompileFile(compile_and_run_struct_single *compile_and_run, wxFileName filename);
	long int CompileFile(compile_and_run_struct_single *compile_and_run, file_item *item);
	long int CompileExtra(compile_and_run_struct_single *compile_and_run, compile_extra_step *step);
	long int CompileItem(compile_and_run_struct_single *compile_and_run, wxTreeItemId &tree_item);
	long int Link(compile_and_run_struct_single *compile_and_run, linking_info *info);
	long int Run(compile_and_run_struct_single *compile_and_run);
	bool RenameFile(wxTreeItemId &tree_item, wxString new_name);
	bool MoveFile(wxTreeItemId &tree_item, char where);
	bool DeleteFile(wxTreeItemId &tree_item, bool also=false);
	file_item *AddFile (char where, wxFileName name, bool sort_tree=true);
	void ExportMakefile(wxString make_file, bool exec_comas, wxString mingw_dir, makefile_type mktype);
	void AnalizeConfig(wxString path, bool exec_comas, wxString mingw_dir, bool force=true);
	/// Regenera uno o todos los proyecto wxFormBuilder
	bool WxfbGenerate(bool show_osd=false, file_item *cual=NULL);
	/// Regenerar proyectos o actualizar clases de wxFormBuilder si es necesario
	void WxfbAutoCheck();
	/// Agrega al proyecto un clase heredada de alguna de las dise�adas en wxFormBuilder
	bool WxfbNewClass(wxString base_name, wxString name);
	/// Actualiza una clase heredada de alguna de las dise�adas en wxFormBuilder
	bool WxfbUpdateClass(wxString fname, wxString cname);
	void Clean();
	bool Debug();
	wxString GetPath();
	/// Carga todos los breakpoints del proyecto en gdb
	int SetBreakpoints();
	wxString GetExePath();
	
	/** @brief Agrega una biblioteca a construir de una configuraci�n **/
	project_library *AppendLibToBuild(project_configuration *conf);
	
	/** @brief Elimina una biblioteca a construir de una configuraci�n **/
	bool DeleteLibToBuild(project_configuration *conf, project_library *lib_to_del);
	
	/** @brief Elimina una biblioteca a construir de una configuraci�n **/
	project_library *GetLibToBuild(project_configuration *conf, wxString libname);
	
	/// @brief Asocia los fuenes a las bibliotecas de una configuracion (llena el puntero lib de file_item)
	void AssociateLibsAndSources(project_configuration *conf=NULL);
	/// @brief Guarda las asociaciones de los fuentes (puntero lib de file_item) en la configuracion que recibe
	void SaveLibsAndSourcesAssociation(project_configuration *conf=NULL);
	
	/** @brief A�ade un paso de compilaci�n personalizado a una configuraci�n  **/
	compile_extra_step *InsertExtraSteps(project_configuration *conf, wxString name, wxString cmd, int pos);
	
	/** @brief Mueve una paso de compilaci�n personalizado hacia arriba o hacia abajo en la secuencia **/
	void MoveExtraSteps(project_configuration *conf, compile_extra_step *step, int delta);
	
	/** @brief Busca un paso de compilaci�n personalizado por nombre en una configuraci�n **/
	compile_extra_step *GetExtraStep(project_configuration *conf, wxString name);
	
	/** @brief Elimina un paso de compilaci�n personalizado de una configuraci�n **/
	bool DeleteExtraStep(project_configuration *conf, compile_extra_step *step);
	
	/** @brief Determina si debe ejecutarse o no un paso personalizado en la proxima compilaci�n **/
	bool ShouldDoExtraStep(compile_extra_step *step);
	
	/** @brief Reemplaza nombre de proyecto y dem�s cosas por el nombre, para usar al crear desde un template **/
	void FixTemplateData(wxString name);
	
	/// @brief Activa el uso de wxFormsBuilder para el proyecto y verifica si encuentra el ejecutabel
	void ActivateWxfb();
	
	/// @brief Devuelve la minima versi�n necesaria para que otro ZinjaI m�s viejo abra correctamente el proyecto
	int GetRequiredVersion();
	
	/// @brief Actualiza el arbol de simbolos (parsea los abiertos y cerrados que se hayan modificado)
	void UpdateSymbols();
	
	/// @brief Dibuja un grafo con las relaciones entre los fuentes como arcos y los tama�os como colores
	void DrawGraph();
		
};

extern ProjectManager *project;

#endif
