/** 
* @file ProjectManager.h
* @brief Definición de la clase ProjectManager y otras relacionadas
*
* ProjectManager, compile_extra_step, doxygen_configuration, project_library, 
* project_configuration, break_line_item, marked_line_item, project_file_item,
* compile_step, linking_info
**/

#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <ctime>
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/treebase.h>
#include <wx/arrstr.h>
#include "mxUtils.h"

#define PROJECT_EXT "zpr"
#define DOT_PROJECT_EXT "." PROJECT_EXT

#ifdef __WIN32__
	// maldito seas "winbase.h" (ahi se hacen defines como los que estan aca abajo, entonces cualquiera que los incluya esta cambiando los nombres)
	#ifdef MoveFile
		#undef MoveFile
	#endif
	#ifdef DeleteFile
		#undef DeleteFile
	#endif
#endif

#include "Toolchain.h" // por TOOLCHAIN_MAX_ARGS
#include "BreakPointInfo.h" // por el delete_autolist(breaklist) del destructor de project_file_item
#include "enums.h"
#include "SourceExtras.h"
#include "CustomTools.h"
class BreakPointInfo;
class mxSource;
class mxOSD;
class GenericAction;

/// Posibles ubicaciones para un paso de compilación adicional en el proceso de construcción de un proceso
enum ces_pos {
	CES_BEFORE_SOURCES=0, ///< al principio de todo, antes de compilar los fuentes
	CES_BEFORE_LIBS, ///< luego de compilar los fuentes, pero antes de enlazar las bibliotecas que genera el proyecto
	CES_BEFORE_EXECUTABLE, ///< luego de compilar los fuentes y enlazar las bibliotecas que genera el proyecto, pero antes de enlazar el ejecutable
	CES_AFTER_LINKING, ///< al final de todo, luego enlazar el ejecutable
	CES_COUNT ///< cantidad de posibles ubicaciones
};

/// Posibles tipos de Makefiles a generar
enum MakefileTypeEnum {
	MKTYPE_FULL, ///< el Makefile común, que contiene todo lo necesario
	MKTYPE_OBJS, ///< uno que contine solo las reglas de los objetos, que no depende del perfil de compilación (y se llama siempre Makefile.common), es para incluir desde otro Makefile
	MKTYPE_CONFIG ///< el que define las variables que dependen del perfil de compilación, y incluye al otro makefile que contiene la parte comun a todos los perfiles
	};

/// mecanismo de ejecución para un proyecto
enum ExecMethodEnum { 
	EMETHOD_REGULAR=0, ///< zinjai launches executable directly
	EMETHOD_WRAPPER=1, ///< zinjai launches a executable that launches the process (something working like "time" or "valgrind")
	EMETHOD_INIT=2, ///< zinjai launches an script to setup environment and then launches the executable
	EMETHOD_SCRIPT=3 ///< zinjai launches a script only, the script should launch the executable
};

enum DebugSymbolsAction {
	DBSACTION_KEEP=0,
	DBSACTION_COPY=1,
	DBSACTION_STRIP=2
};

/// para indicar si se debe esperar una tecla antes de cerrar la consola una vez finalizada la ejecución de un proyecto
enum wait_for_key_t { 
	WKEY_NEVER=0, 
	WKEY_ON_ERROR=1, 
	WKEY_ALWAYS=2,
};

struct compile_and_run_struct_single;

/** 
* @brief Paso de compilación adicional
* 
* Estructura que representa la información de un paso de compilación personalizado de una configuración.
* Es contenida por la configuración, en forma de lista doblemente enlazada ad-hoc (sin primer nodo ficticio)
**/
struct compile_extra_step {
	wxString deps; ///< archivos de los que depende este paso, puede incluir variables a reemplazar, para poder comparar y saber cuando no es necesario volver a ejecutarlo
	wxString out; ///< archivo de salida de este paso, puede incluir variables a reemplazar, para poder comparar y saber cuando no es necesario volver a ejecutarlo, si esta en blanco se ejecuta siempre
	wxString command; ///< comando que se ejecuta en este paso, puede incluir variables a reemplazar
	wxString name; ///< nombre del paso, solo para mostrar en la interfaz
	int pos; ///< un valor posible de ces_pos, indica donde va este paso en el proceso de construcción completo
	compile_extra_step *next, *prev;
	bool check_retval; ///< si debe verificar que el comando retorne 0 para saber si continuar o abortar la compilacion
	bool hide_window; ///< si debe ocultar la ventana/consola donde se ejecuta
	bool delete_on_clean; ///< si debe eliminar este archivo al hacer un clean del proyecto
	bool link_output; ///< si debe agregar el archivo de salida a la lista de objetos a enlazar
	compile_extra_step():deps(""),out(""),command(""),name("<noname>"),pos(0),next(nullptr),prev(nullptr),check_retval(false),hide_window(true),delete_on_clean(true),link_output(false) {}
};


/** 
* @brief Configuración para Doxygen de un proyecto
* 
* Estructura que representa la información para configurar doxygen. El archivo Doxyfile se regenera
* cada vez que hay que generar la documentación, a partir de esta estructura. Existe, cuanto mucho 
* una instancia (para el proyecto en curso). Se referencia con el puntero doxygen de
* ProjectManager, que puede ser nullptr si no se ha definido la configuarción Doxygen para dicho proyecto
* Si el puntero es valido, y el atributo save es true, se graba en el archivo de proyecto
**/
struct doxygen_configuration {
	
	wxString name; ///< nombre del proyecto para mostrar en la documentación
	wxString version; ///< versión del proyecto para mostrar en la documentación
	wxString destdir; ///< directorio donde colocar la documentación generada
	wxString extra_files; ///< archivos adicionales a procesar, ademas de lo especificado por do_headers y do_cpps
	wxString exclude_files; ///< archivos que no se deben procesar, aunque esten entre los fuentes o cpps
	bool do_headers; ///< si debe procesar los archivos de cabeceras (.h) del proyecto
	bool do_cpps; ///< si debe procesar los archivos fuentes (.cpp) del proyecto
	bool hideundocs; ///< indica si no debe generar documentación para los elementos que no tengan sus respectivos comentarios
	bool save; ///< si se debe o no recordar esta conf (guardar en el archivo de proyecto)
	bool latex; ///< indica si debe generar la documentación en formato Latex
	bool html; ///< indica si debe generar la documentación en formato HTML
	bool html_searchengine; ///< indica si debe inlcuir el motor de busqueda (basado en PHP) para la versíón HTML
	bool html_navtree; ///< indica si debe generar un panel con un árbol de navegación en la versión HTML
	bool extra_static; ///< indica si deben incluirse funciones/variables static
	bool extra_private; ///< indica si deben incluirse miembros/atributos privados
	bool preprocess; ///< indica si debe habilitar o no el preprocesado (considerar macros)
	bool use_in_quickhelp; ///< indica si se debe agregar el texto generado por doxygen a la ayuda rapida de zinjai
	wxString lang; ///< indica el lenguaje en el cual se genera la documentación (English o Spanish)
	wxString base_path; ///< indica el path base para expresar los nombres de los archivos relativos a este
	wxString extra_conf; ///< texto a incluir directamente y tal cual en el Doxyfile (para opciones no contempladas)
	
	//! inicializa la configuración con los valores por defecto
	doxygen_configuration(wxString aname) {
		name = aname;
		save = true;
		destdir = "docs";
		html = true;
		html_navtree = false;
		html_searchengine = true;
		latex = false;
		hideundocs = false;
		do_headers = do_cpps = true;
		extra_static = false;
		extra_private = false;
		preprocess = true;
		lang = "Spanish";
		use_in_quickhelp = true;
	}
	
	static wxString get_tag_index(); ///< gets the xml file generated by doxygen that maps symbols to htmls, used for adding doxygen info to quickhelp resutls
	
};

/** 
* @brief Configuración de la integración con wxFormBuilder para un proyecto
**/
struct wxfb_configuration {
	bool activate_integration;
	bool autoupdate_projects;
	bool update_class_list;
	bool update_methods;
	bool dont_show_base_classes_in_goto;
	bool set_wxfb_sources_as_readonly;
	// auxiliares para zinjai, no se guardan como opciones del proyecto ni los configura el usuario
	bool ask_if_wxfb_is_missing;  ///< indica si avisar cuando no encuentra el ejecutable del wxFormBuilder
	bool working; ///< indica si se esta actualmente regenerando algun proyecto wxfb (si wxfb está ejecutandose en segundo plano)
	SingleList<wxString> projects; ///< nombres de archivos de proyecto wxfb asociados al proyecto zinjai en la categoría Otros (se carga en Project::WxfbGetFiles, full paths)
	SingleList<wxString> sources; ///< nombres de archivos (sin extension, será .h o .cpp) asociados al proyecto generados automáticamente por wxfb, ordenados con relación a projects (se carga en Project::WxfbGetFiles, full paths)
	
	//! inicializa la configuración con los valores por defecto
	wxfb_configuration(bool enabled=true) {
		ask_if_wxfb_is_missing=true;
		activate_integration=enabled;
		autoupdate_projects=true;
		update_class_list=true;
		update_methods=true;
		dont_show_base_classes_in_goto=true;
		set_wxfb_sources_as_readonly=true;
		working=false;
	}
};

/** 
* @brief Configuración para CppCheck de un proyecto
* 
* Estructura que representa la información para configurar cppcheck. Existe, cuanto mucho 
* una instancia (para el proyecto en curso). Se referencia con el puntero cppcheck de
* ProjectManager, que puede ser nullptr si no se ha definido la configuarción Doxygen para dicho proyecto
* Si el puntero es valido, y el atributo save es true, se graba en el archivo de proyecto
**/
struct cppcheck_configuration {
	bool copy_from_config; ///< indica si las configuraciones (-D -U) se toman del campo de macros de active_configuration, o de config_d y config_u
	bool exclude_headers; ///< no pasarle los .h, que los encuentre por inclusion desde los cpps
	wxString config_d; ///< macros a definir para el analisis (-D...)
	wxString config_u; ///< macros a no definir para el analisis (-U...)
	wxString style; ///< verificaciones adicionales (--enable=...)
	wxString platform; ///< verificaciones especificas para una plataforma (--platform=...)
	wxString standard; ///< verificaciones especificas para un standard (--std=...)
	wxString suppress_file; ///< archivo con lista de supresiones (--suppressions-list=)
	wxString suppress_ids; ///< lista de supresiones (--suppress=...)
	bool inline_suppr; ///< activar supresiones inline (--inline-suppr)
	wxString exclude_list; ///< lista de archivos del proyecto que no seran analizados
	wxString additional_files; ///< lista de archivos no registrados en el proyecto a agregar para el analisis
	bool save_in_project; ///< indica si hay que guardar esta configuración en el archivo de proyecto
	//! inicializa la configuración con los valores por defecto
	cppcheck_configuration() {
		exclude_headers=true;
		copy_from_config=true;
		style="all";
		inline_suppr=true;
		save_in_project=false;
	}
};


/**
* @brief Guarda información sobre una libreria a generar con el proyecto
**/
struct project_library {
	wxString libname; ///< nombre amigable
	wxString path; ///< archivo de salida (destino)
	wxString sources; ///< fuentes que poner dentro
	wxString extra_link; ///< opciones extra para el linker
	bool is_static; ///< estatica o dinamica
	bool default_lib; ///< por defecto para nuevos fuentes
	bool need_relink; ///< temporal, para AnalizeConfig y PrepareForBuilding
	wxString objects_list; ///< temporal, para AnalizeConfig y PrepareForBuilding
	wxString parsed_extra; ///< temporal, para AnalizeConfig y PrepareForBuilding
	wxString filename; ///< archivo de salida (ruta completa del destino), para AnalizeConfig y PrepareForBuilding
	project_library *next;
	project_library *prev;
	project_library(project_library *aprev=nullptr) {
		is_static=true;
		need_relink=false;
		next=nullptr;
		prev=aprev;
	}
};

/** 
* @brief Perfil de compilación y ejecución para un proyecto
* 
* Estructura que representa la información de un perfil de compilacion (paths, banderas, librerias
* nivel de info de debug, nivel de warnings, pasos personalizados, etc. Puede haber muchas por proyecto
* (actualmente hasta 100). ProjectManager tiene un arreglo configurations (creado dinamicamente) para
* guardarlas, y un contador configurations_count para saber la cantidad
**/
struct project_configuration {

	project_configuration *bakup; ///< copia antes de modificar en el dialogo de opciones, para usar si se cancela
	
	wxString name; ///< nombre de la configuracion (unico y definido por usuario)
	
	wxString toolchain; ///< nombre del toolchain que utiliza, o \<default\> para tomar el por defecto de zinjai
	wxString toolchain_arguments[TOOLCHAIN_MAX_ARGS]; ///< argumentos para el Toolchain
	
	wxString working_folder; ///< directorio de trabajo para la ejecución
	bool always_ask_args; ///< mostrar el dialogo que pide los argumentos antes de ejecutar
	wxString args; ///< argumentos para la ejecucion
	int exec_method; ///< como se debe ejecutar (las opciones estan en ExecMethodEnum)
	wxString exec_script; ///< script para exec_method>0
	int wait_for_key; ///< esperar una tecla antes de cerrar la consola luego de la ejecución (0=nunca, 1=solo en caso de error, 2=siempre)
	wxString env_vars; ///< lista con valores para asignar o reemplazar en las variables de entorno antes de ejecutar
	
	wxString temp_folder; ///< directorio temporal donde poner los objetos de la compilacion
	wxString output_file; ///< archivo de salida de la compilacion = ruta del ejecutable
	wxString icon_file; ///< archivo con el icono para compilar como recurso (solo windows)
	wxString manifest_file; ///< archivo con el manifest.xml para compilar como recurso (solo windows)
	
	wxString compiling_extra; ///< parametros adicionales para el compilador
	wxString macros; ///< lista de macros a definir (con -D)
	wxString old_macros; ///< lista de macros a definir en la ultima corrida (para comparar y saber que recompilar por cambio de macros)
	wxString headers_dirs; ///< lista de rutas adicionales donde buscar cabeceras (para pasar con -I)
	int warnings_level; ///< nivel de advertencias a mostrar por el compilador: 0=ninguna 1=default 2=todas
	bool pedantic_errors; ///< forzar a cumplir el estandar (-pedantic-errors)
	wxString std_c; ///< versión del estándar a utilizar para los fuentes C
	wxString std_cpp; ///< versión del estándar a utilizar para los fuentes C++
	int debug_level; ///< nivel de información de depuración a colocar al compilar: 0=g0 1=g1 2=g2
	int optimization_level; ///< nivel optimización para los binarios: 0=O0 1=O1 2=O2 3=O3 4=Os, 5=Og, 6=Ofast
	wxString linking_extra; ///< parametros adicionales para el enlazador (se llama a travez de gcc/g++, no directo)
	wxString libraries_dirs; ///< rutas adicionales para buscar librerias (para pasar con -L)
	wxString libraries; ///< librearias para enlazar (para pasar con -l)
	int strip_executable; ///< que hacer durante el enlazado con la info de depuracion (las opciones estan en el enum DebugSymbolsAction)
	bool console_program; ///< marcar como programa de consola (sino, no usar el runner y compilar con -mwindows)
	compile_extra_step *extra_steps; ///< puntero al primer item de la lista de pasos adicionales para la compilacion (sin primer nodo ficticio), nullptr si no hay pasos extra
	project_library *libs_to_build; ///< bibliotecas a construir, lista enlazada sin primer nodo ficticio
	bool dont_generate_exe; ///< no generar ejecutable, solo bibliotecas
	HashStringString *by_src_compiling_options; ///< argumentos de compilacion adicionales por fuente (solo guarda los fuentes que no usan la configuracion por defecto)
	
	//! inicializa la configuración con los valores por defecto
	project_configuration(wxString pname, wxString cname) {
		by_src_compiling_options = new HashStringString();
		libs_to_build=nullptr;
		bakup=nullptr;
		name=cname;
		working_folder="";
		always_ask_args=false;
		exec_script="";
		args="";
		exec_method=EMETHOD_REGULAR;
		wait_for_key=WKEY_ALWAYS;
		temp_folder=cname;
		output_file=DIR_PLUS_FILE(cname,pname+_T(BINARY_EXTENSION));
		compiling_extra="";
		macros="";
		headers_dirs="";
		warnings_level=1;
		pedantic_errors=false;
		debug_level=2;
		optimization_level=0;
		linking_extra="";
		libraries_dirs="";
		libraries="";
		strip_executable=DBSACTION_KEEP;
		console_program=true;
		dont_generate_exe=false;
		extra_steps=nullptr;
		toolchain="";
		std_c=std_cpp="";
		for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) toolchain_arguments[i]="${DEFAULT}";
	}
	
	/// inicializa una nueva configuracion a partir de una existente
	project_configuration(wxString cname, project_configuration *copy_from) {
		(*this)=*copy_from; // this copy is not correct, copies ptrs
		bakup=nullptr;
		name=cname;
		if (extra_steps) {
			compile_extra_step *aux = extra_steps = new compile_extra_step(*extra_steps);
			while (aux->next) { 
				compile_extra_step * niu = new compile_extra_step(*aux->next); 
				aux->next = niu; niu->prev=aux;
				aux=niu;
			}
		}
		if (libs_to_build) {
			project_library *aux = libs_to_build = new project_library(*libs_to_build);
			while (aux->next) { 
				project_library * niu = new project_library(*aux->next); 
				aux->next = niu; niu->prev=aux;
				aux=niu;
			}
		}
		by_src_compiling_options = new HashStringString();
		*by_src_compiling_options = *(copy_from->by_src_compiling_options);
	}

};

//! Información acerca de una linea de código resaltada por el usuario
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
* Representa un archivo de un proyecto. Funciona también como nodo para una lista
* enlazada. ProjectManager tiene los tres punteros que inician las tres listas (fuentes,
* cabeceras y otros). Cada estructura guarda el nombre del archivo, sus listas breakpoints
* y lineas resaltadas y algunas banderas.
**/
struct project_file_item { // para armar las listas (doblemente enlazadas) de archivos del proyecto
	wxString name;
	wxTreeItemId item;
	SourceExtras extras; ///< breakpoints, highlighted lines, cursor position
	bool force_recompile; ///< indica que se debe recompilar independientemente de la fecha de modificacion (por ejemplo, si lo va a modificar un paso adicional)
	project_library *lib; ///< a que biblioteca pertenece (no siempre es correcto, se rehace con analize_config)
	bool read_only; ///< indica que se debe abrir como solo lectura (porque es generado por una herramienta externa)
	bool hide_symbols; ///< indica si sus métodos y funciones se deben tener en cuenta para el cuadro "Ir a funcion/clase/método"
	eFileType where; ///< indica en qué categoria de archivos está asociado al proyecto (s=sources, h=headers, o=otros)
	wxString compiling_options; ///< campo generado por AnaliceConfig, con las opciones finales y reales, ya procesadas
	void Init() { // parte comun a ambos constructores
		force_recompile=false;
		read_only=false;
		hide_symbols=false;
		lib=nullptr;
	}
	project_file_item (const wxString &n, const wxTreeItemId &i, const eFileType w) {
		Init();
		name=n;
		item=i;
		where=w;
	}
	project_file_item () {
		Init();
		where=FT_NULL;
	}
	/// true=c++, false=c
	bool IsCppOrJustC() {
		return name.Len()<=2 || (name[name.Len()-1]!='c'&&name[name.Len()-1]!='C') || name[name.Len()-2]!='.';
	}
	wxString GetBinName(const wxString &temp_dir) {
		return DIR_PLUS_FILE(temp_dir,wxFileName(name).GetName()+".o");
	}
};

enum ces_type{
	CNS_VOID, ///< primer paso, ficticio, solo para inicializar la lista de pasos
	CNS_SOURCE, ///< compilación un fuente del proyecto
	CNS_BARRIER, ///< paso ficticio que no ejecuta nada, pero está para separar grupos de pasos paralelizables
	CNS_EXTRA, ///< para pasos adicionales que configure el usuario en la pestaña "Secuencia"
	CNS_LINK, ///< enlazado de una biblioteca o del ejecutable
	CNS_ICON, ///< compilación del archivo de recursos que arma zinjai en windows para el manifest y/o el ícono
	CNS_DEBUGSYM, ///< paso que extrae los simbolos de depuracion de un ejecutable o de una biblioteca a un archivo separado
	CNS_EXTERN ///< para extern toolchains (llamar a make por ejemplo)
};

/** 
* @brief Estructura que representa un paso genérico del proceso de compilación.
* 
* Estructura que representa un paso genérico del proceso de compilación. Puede contener
* info para compilar un fuente normal, un paso personalizado, o enlazar. El ProyectManager 
* tiene una lista de estos que empieza en first_compile_step (sin primer nodo ficticio).
* Esta lista se arma en PrepareForBuilding, y cada vez que se llama a CompileNext se 
* ejecuta el primer paso, y se elimina de la lista.
**/
struct compile_step {
	ces_type type; ///< que se hace en este paso (compilar fuente, paso personalizado, enlazar
	bool start_parallel; ///< indica si en este paso se debe comenzar a paralelizar (es decir, si es el primer paso de una etapa paralelizable)
	void *what; ///< puntero al project_file_item si es un fuente, al compile_extra_step si es un paso personalizado, o basura si para enlazar
	compile_step *next; ///< puntero al siguiente paso (lista simplemente enlazada)
	//! constructor
	compile_step(ces_type t, void *w) : type(t),start_parallel(false),what(w),next(nullptr) {}
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

struct stripping_info {
	wxString fullpath;
	wxString filename;
	int current_step;
	stripping_info(wxString _fullpath, wxString _filename="", int _curren_step=0):
		fullpath(_fullpath),filename(_filename),current_step(_curren_step) {
			if (filename.IsEmpty()) filename=wxFileName(fullpath).GetFullName();
		};
};


/** 
* @brief Representa un proyecto
* 
* Contiene sus perfiles de ejecución y configuración, sus archivos, su información 
* adicional, y encapsula todos los métodos específicos para admistrar, compilar y 
* ejecutar proyectos. No se puede crear en blanco, sino que siempre a partir de un 
* archivo. En la aplicación existe un puntero proyect que siempre apunta al proyecto 
* actual; o es nullptr si no hay proyecto abierto.
**/
class ProjectManager {
	
	ProjectManager(const ProjectManager &); ///< esta clase no es copiable
	void operator=(const ProjectManager &); ///< esta clase no es copiable
	
	friend wxString doxygen_configuration::get_tag_index();
public:
	GenericAction *post_compile_action; ///< what to do if building finishes ok, is set in last CompileNext call
	compile_step *first_compile_step;
	bool loading; ///< indica que se esta cargando, para que otros eventos sepan
	time_t compile_startup_time;
	int version_required; ///< minima version de ZinjaI requerida para abrir el proyecto sin perder nada
	int steps_count; ///< temporal para contar los pasos totales para una compilacion
	int current_step; ///< temporal para contar cuantos pasos van durante una compilacion
	float actual_progress; ///< temporal para calcular el porcentaje de progreso segun las cuentas de pasos
	float progress_step; ///< temporal para guardar de a cuanto avanza la barra de progreso al avanzar un paso
	bool force_relink;	///< indica si debe reenlazar si o si en la proxima compilacion, aunque el exe esté al día
	cppcheck_configuration *cppcheck; ///< configuracion para ejecutar cppcheck, nullptr si no esta definido en el proyecto
private:
	wxfb_configuration *wxfb; ///< configuración de la integración con wxfb (si es nullptr no está activada)
	doxygen_configuration *doxygen; ///< configuracion para el Doxyfile, nullptr si no esta definido en el proyecto
public:
	doxygen_configuration* GetDoxygenConfiguration();
	wxfb_configuration* GetWxfbConfiguration(bool create_activated=true);
	bool GetWxfbActivated() { return wxfb && wxfb->activate_integration; }
	project_configuration *active_configuration; ///< puntero a la configuracion activa
	project_configuration *configurations[100]; ///< arreglo de configuraciones, ver configurations_count
	int configurations_count; ///< cantidad de configuraciones validas en configurations
	wxString project_name; ///< el nombre bonito del proyecto
	wxString filename; ///< el archivo del proyecto
	wxString path; ///< la carpeta del proyecto
	GlobalList<project_file_item*> files_all;
	LocalList<project_file_item*> files_sources;
	LocalList<project_file_item*> files_headers;
	LocalList<project_file_item*> files_others;
	wxString autocomp_extra; ///< indices de autocompletado adicionales para este proyecto
	wxString autocodes_file; ///< archivo con definiciones de autocodigos adicionales
	wxString macros_file; ///< archivo con definiciones de macros para gdb
	wxArrayString inspection_improving_template_from, inspection_improving_template_to;
	CustomToolsPack custom_tools; ///< herramientas personalizables asociadas al proyecto
private:
	wxString temp_folder; ///< guarda la ruta de temporales (objetos) completa para la configuracion actual, solo para uso interno
	wxString temp_folder_short; ///< guarda la ruta de temporales (objetos) relativa (como la ingresa el usuario), solo para uso interno
public:
	wxString GetTempFolder(bool create=false);
	wxString GetTempFolderEx(wxString path, bool create=false);
	wxString executable_name; ///< ubicacion final del ejecutable (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString linking_options; ///< argumentos para el enlazado del ejecutable (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString cpp_compiling_options; ///< argumentos para las compilaciones (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString c_compiling_options; ///< argumentos para las compilaciones (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString objects_list; ///< lista de objetos y bibliotecas (con -L) para usar en la linea de compilacion (se llena en AnalizeConfig, lo usan PrepareForBuilding, ExportMakefile, ...)
	wxString exe_deps; ///< temporal, para pasar de AnalizeConfig a ExportMakefile las dependencias del ejecutable (la diff con objects_list esta en los -l de las bibliotecas)
	int custom_tabs; ///< tipo de tabulado en los fuentes, si es 0, hereda de la configuracion de zinjai, si no usa ese
	bool tab_use_spaces; ///< indica si los tabs en los fuentes deben reemplazarse por espacios (solo si custom_tabs no es 0)
	bool config_analized; ///< indica si ya se actualizo la info para compilar (se usa en AnalizeConfig cuando force=false)
	bool compile_was_ok; ///< indica si se arrastra algun error de compilacion de pasos anteriores
	
	wxString last_dir; ///< ultimo path utilizado por el usuario, para los cuadros de abrir...
	bool modified; ///< indica si algo cambió en la configuración del proyecto
	wxArrayString warnings; ///< advertencias generadas por ZinjaI que deben agregarse mas tarde al arbol de resultados
	
	project_configuration *GetConfig(wxString name); ///< busca una configuracion por nombre, y devuelve un puntero si la encuentra, nullptr si no
	
	bool GenerateDoxyfile(wxString fname); ///< escribe el archivo de configuración para Doxygen
	wxString WxfbGetSourceFile(wxString fbp_file); ///< funcion auxiliar que devuelve el nombre (path+nombre, sin extension) de los archivos que genera el archivo .fbp que recibe
	
	ProjectManager(wxFileName filename); ///< loads a project from a file (just_created=true when its called from new project wizard)
	~ProjectManager();
	wxString GetFileName();
	int GetFileList(wxArrayString &array, eFileType cuales=FT_NULL, bool relative_paths=false);
	project_file_item *FindFromName(wxString name); ///< busca a partir del nombre de archivo (solo, sin path)
	project_file_item *FindFromItem(wxTreeItemId &tree_item);
	wxString GetNameFromItem(wxTreeItemId &tree_item, bool relative=false);
	project_file_item *HasFile(wxFileName file); ///< busca una archivo en el proyecto por su ruta completa
	bool Save(bool as_template=false);
	void MoveFirst(wxTreeItemId &tree_item);
	
	/// guarda todos los archivos del proyecto que esten abiertos
	void SaveAll(bool save_project=true);
	
	/// arma el comando para ejecutar de un paso de compilación personalizado, reemplazando todas las variables de sus campos
	wxString GetCustomStepCommand(const compile_extra_step *step, wxString mingw_dir, wxString &deps, wxString &output);
	/// arma el comando para ejecutar de un paso de compilación personalizado, reemplazando todas las variables de sus campos
	wxString GetCustomStepCommand(const compile_extra_step *step);
	
	/// Determina si el archivo item usa alguna de las macros de la lista macros
	bool DependsOnMacro(project_file_item *item, wxArrayString &macros);
	/// Guarda todo y marca cuales archivos hay que recompilar, devuelve falso si no hay que compilar ninguno ni reenlazar el ejecutable
	bool PrepareForBuilding(project_file_item *only_one=nullptr);
	/// Ejecuta el próximo paso para la construcción del proyecto
	long int CompileNext(compile_and_run_struct_single *compile_and_run, wxString &object_name);
	/// Compila el archivo de recursos temporal del proyecto (generado por PrepareForBuilding), se invoca a través de CompileNext, no directamente
	long int CompileIcon(compile_and_run_struct_single *compile_and_run, wxString icon_file);
	/// Ejecuta un paso de compilación de un fuente del proyecto en la construcción de un proceso, se invoca a través de CompileNext, o del otro CompileFile, no directamente
	long int CompileFile(compile_and_run_struct_single *compile_and_run, project_file_item *item);
	/// Ejecuta un paso compilación adicional, se invoca a través de CompileNext, no directamente
	long int CompileExtra(compile_and_run_struct_single *compile_and_run, compile_extra_step *step);
	/// Ejecuta el paso de enlazado del ejecutable o de una biblioteca que genera un proyecto, se invoca a través de CompileNext, no directamente
	long int Link(compile_and_run_struct_single *compile_and_run, linking_info *info);
	/// Ejecuta el paso de extracción de los simbolos de depuración a un archivo separado, se invoca a través de CompileNext, no directamente
	long int Strip(compile_and_run_struct_single *compile_and_run, stripping_info *info);
	/// Ejecuta el comando de compilación para un toolchan externo (como make) (build=true compila, build=false limpia)
	long int CompileWithExternToolchain(compile_and_run_struct_single *compile_and_run, bool build=true);

	bool RenameFile(wxTreeItemId &tree_item, wxString new_name);
	/// Compila un solo fuente del proyecto, preparando la configuración, fuera del proceso de construcción general
	long int CompileFile(compile_and_run_struct_single *compile_and_run, wxFileName filename);
	
	void SetFileReadOnly(project_file_item *item, bool read_only);
	void SetFileHideSymbols(project_file_item *item, bool hide_symbols);
	
	void MoveFile(wxTreeItemId &tree_item, eFileType where);
	/// Removes a file from the project, and optionally also erases it from the disk
	void DeleteFile(project_file_item *item, bool also_delete_from_disk);
	/// Removes a file from the project and optionally from the disk, and optionally also the complement (include gui actions to confirm, also param is for internal use)
	bool DeleteFile(wxTreeItemId tree_item);
	project_file_item *AddFile (eFileType where, wxFileName name, bool sort_tree=true);
	
	/// Determina cuales son los projectos wxfb asociados al proyecto zinjai y sus respectivos fuentes (busca archivos .fbp en Otros, guarda los resultados en wxfb->projects y wxfb->sources)
	void WxfbGetFiles();
	/// Sets read_only and hide_symbols properties for project_file_items from wxfb projects
	void WxfbSetFileProperties(bool change_read_only, bool read_only_value, bool change_hide_symbols, bool hide_symbols_value);
	/// Regenera uno o todos los proyecto wxFormBuilder
	bool WxfbGenerate(bool show_osd=false, project_file_item *cual=nullptr);
private:
	/// Funcion auxiliar para el otro WxfbGenerate, esta es la que realmente hace el trabajo, la otra funcion de wrapper para seleccionar cuales proyectos
	bool WxfbGenerate(wxString fbp_file, wxString fbase, bool force_regen, mxOSD **osd);
public:
	/// auxiliar struct for retrieving wxfb related data from parser data (used to send the old state from WxfbAutoCheckStep1 to WxfbAutoCheckStep2)
	struct WxfbAutoCheckData {
		SingleList<wxString> wxfb_classes; /// list of wxfb's generated classes before the update (step 1); if after  the update (step 2) there are new classes here zinjai can offer creating new inherited ones
		SingleList<wxString> user_classes; /// list of wxfb inherited classes, if some of this classes don't have a base class anymore (was deleted in wxfb), zinjai can offer deleting them
		SingleList<wxString> user_fathers; /// list fathers of classes in user_classes (same size, same indexes)
		WxfbAutoCheckData(); /// initializes the lists with info from the current project (erases previous existing data)
	};
	/// Regenerar proyectos o actualizar clases de wxFormBuilder si es necesario (parte 1)
	void WxfbAutoCheckStep1();
	/// Regenerar proyectos o actualizar clases de wxFormBuilder si es necesario (parte 2)
	void WxfbAutoCheckStep2(WxfbAutoCheckData *old_data);
	
	/// Agrega al proyecto un clase heredada de alguna de las diseñadas en wxFormBuilder
	bool WxfbNewClass(wxString base_name, wxString name);
	/// Actualiza una clase heredada de alguna de las diseñadas en wxFormBuilder
	bool WxfbUpdateClass(wxString wxfb_class, wxString user_class);
	/// Genera un makefile para el proyecto a partir de active_configuration
	void ExportMakefile(wxString make_file, bool exec_comas, wxString mingw_dir, MakefileTypeEnum mktype, bool cmake_style=false);
	/// Parsea la configuración del proyecto (active_configuration) para generar los argumentos necesarios para invocar al compilador
	void AnalizeConfig(wxString path, bool exec_comas, wxString mingw_dir, bool force=true);
	/// sets environment variables for running/compiling the project (LD_LIBRARY_PATH, PROJECT_BIN, PROJECT_PATH)
	void SetEnvironment(bool set, bool for_running);
	/// Lanza el ejecutable
	long int Run();
	/// not to be called directly, but trough DebugManager::Start(bool)
	bool Debug(); 
	///< remove temporary files for active configuration
	void Clean(); 
	///< remove temporary files for all configurations
	void CleanAll(); 
	wxString GetPath();
	/// Carga todos los breakpoints del proyecto en gdb
	void SetBreakpoints();
	wxString GetExePath();
	
	/** @brief Agrega una biblioteca a construir de una configuración **/
	project_library *AppendLibToBuild(project_configuration *conf);
	
	/** @brief Elimina una biblioteca a construir de una configuración **/
	bool DeleteLibToBuild(project_configuration *conf, project_library *lib_to_del);
	
	/** @brief Elimina una biblioteca a construir de una configuración **/
	project_library *GetLibToBuild(project_configuration *conf, wxString libname);
	
	/// @brief Asocia los fuenes a las bibliotecas de una configuracion (llena el puntero lib de project_file_item)
	void AssociateLibsAndSources(project_configuration *conf=nullptr);
	/// @brief Guarda las asociaciones de los fuentes (puntero lib de project_file_item) en la configuracion que recibe
	void SaveLibsAndSourcesAssociation(project_configuration *conf=nullptr);
	
	/** @brief Añade un paso de compilación personalizado a una configuración  **/
	compile_extra_step *InsertExtraSteps(project_configuration *conf, wxString name, wxString cmd, int pos);
	
	/** @brief Mueve una paso de compilación personalizado hacia arriba o hacia abajo en la secuencia **/
	void MoveExtraSteps(project_configuration *conf, compile_extra_step *step, int delta);
	
	/** @brief Busca un paso de compilación personalizado por nombre en una configuración **/
	compile_extra_step *GetExtraStep(project_configuration *conf, wxString name);
	
	/** @brief Elimina un paso de compilación personalizado de una configuración **/
	bool DeleteExtraStep(project_configuration *conf, compile_extra_step *step);
	
	/** @brief Determina si debe ejecutarse o no un paso personalizado en la proxima compilación **/
	bool ShouldDoExtraStep(compile_extra_step *step);
	
	/** @brief Reemplaza nombre de proyecto y demás cosas por el nombre, para usar al crear desde un template **/
	void FixTemplateData(wxString name);
	
	/// @brief Activa o desactiva el uso de wxFormsBuilder para el proyecto y verifica si encuentra el ejecutable (de wxfb)
	void ActivateWxfb(bool do_activate);
	
	/// @brief Devuelve la minima versión necesaria para que otro ZinjaI más viejo abra correctamente el proyecto
	int GetRequiredVersion();
	
	/// @brief Actualiza el arbol de simbolos (parsea los abiertos y cerrados que se hayan modificado)
	void UpdateSymbols();
	
	/// @brief Dibuja un grafo con las relaciones entre los fuentes como arcos y los tamaños como colores
	void DrawGraph();
	
	/// @brief Defines with project_configuration is the active one and inform main window to adecuate gui
	void SetActiveConfiguration(project_configuration *aconf);

};

extern ProjectManager *project;

#endif
