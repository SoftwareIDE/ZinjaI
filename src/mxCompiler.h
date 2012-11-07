#ifndef MXCOMPILER_H
#define MXCOMPILER_H
#include <wx/treectrl.h>
#include <wx/string.h>
#include <wx/timer.h>

class wxProcess;
class mxSource;

//! Informacion asociada a un item del arbol de resultados de compilacion, para guardar los que no se ve (por ejemplo el path completo)
class mxCompilerItemData:public wxTreeItemData {
public:
	wxString file_info;
	mxCompilerItemData(wxString fi):file_info(fi) {};
	void Set(wxString &s) { file_info=s; }
};

enum MXC_OUTPUT {
	MXC_GCC, ///< output should be parsed and added to compiler tree (regular output)
	MXC_SIMIL_GCC, ///< output should be added to compiler tree, but not parsed (for gcc-like compilers)
	MXC_EXTRA, ///< output should not be parsed, it's a project's extra compiling step
	MXC_EXTERN, ///< output should be redirected to main_window->extern_compiler_output (it's from an extern toolchain)
	MXC_NULL ///< undefined, should not be used
};

//! Informaci�n acerca de una compilaci�n en proceso
struct compile_and_run_struct_single {
	bool killed; ///< indica si fue interrumpido adrede, para usar en OnProcessTerminate
	bool for_debug; ///< indica si la compilacion fue para luego depurar en lugar de ejecutar normalmente
	bool compiling; ///< indica si esta compilando/enlazando o ejecutando
	wxString step_label; ///< nombre del paso especial que se esta ejecutando (para los extra step, usar con ouput_type=MXC_EXTRA)
	MXC_OUTPUT output_type; ///< indica el tipo de salida
	bool linking; ///< indica si esta compilando o enlazando (cuando compiling=true, parece que solo se usa para llamar a mxCompiler::CheckForExecutablePermision)
	wxProcess *process; ///< puntero al proceso en ejecucion
	long int pid; ///< process id del proceso en ejecucion, 0 si no se pudo lanzar
	bool run_after_compile; ///< indica si hay que ejecutar cuando termine de compilar y enlazar
	int parsing_flag;
	wxString parsing_aux;
	wxTreeItemId last_error_item; ///< ultima error/advertencia (para poner las "note:")
	wxTreeItemId last_all_item; ///< ultima linea en la rama "toda la salida"
	mxCompilerItemData *last_item_data;
	wxArrayString full_output; ///< guarda toda la salida sin procesar		
	wxString valgrind_cmd; ///< prefijo para la ejecucion con la llamada a valgrind
	compile_and_run_struct_single(const compile_and_run_struct_single *o);
	compile_and_run_struct_single(const char *name);
	~compile_and_run_struct_single();
	bool last_error_item_IsOk;
	compile_and_run_struct_single *next, *prev;
#ifdef DEBUG
	const char* mname;
	static int count;
#endif
	wxString GetInfo();
};

//! Agrupa las funciones relacionadas al lanzamiento y gestion de los procesos de compilacion
class mxCompiler {
private:
	wxTreeCtrl *tree;
	wxTreeItemId state,warnings,errors,all;
public:
	mxCompiler(wxTreeCtrl *atree, wxTreeItemId s, wxTreeItemId e, wxTreeItemId w, wxTreeItemId a);
	bool IsCompiling();
	int NumCompilers();
	wxString GetCompilingStatusText();
	
	void CompileSource (mxSource *source, bool run, bool debug);
	void BuildOrRunProject(bool run, bool debug, bool prepared);
	void ParseSomeErrors(compile_and_run_struct_single *compile_and_run);
	void ParseCompilerOutput(compile_and_run_struct_single *compile_and_run, bool success);
	void ResetCompileData();
	compile_and_run_struct_single  *compile_and_run_single; // lista simplemente doblemente enlazada, sin nodo ficticio
	
	// compile_and_run_struct_common
	mxSource *last_compiled; ///< ultimo fuente compilado
	mxSource *last_runned; ///< ultimo fuente ejecutado
	wxString last_caption; ///< titulo de la pestana del ultimo fuente ejecutado
	wxTimer *timer; ///< timer que actualiza el arbol de compilacion mientras compila
	int num_errors; ///< cantidad de errores de la compilacion
	int num_warnings; ///< cantidad de avisos de la compilacion
	int num_all; ///< cantidad de cualquier cosa de la compilacion
	wxArrayString full_output; ///< guarda toda la salida sin procesar
	wxString valgrind_cmd; ///< indica si la proxima ejecuci�n se hace con valgrind
	bool CheckForExecutablePermision(wxString file); ///< en linux, verifica que tenga permisos de ejecucion
	void UnSTD(wxString &line); ///< hace m�s legibles errores reemplazando templates y tipos conocidos (como allocators, char_traits, etc)
};

extern mxCompiler *compiler;

#endif

