#ifndef MX_DEBUG_MANAGER_H
#define MX_DEBUG_MANAGER_H

#include <wx/string.h>
#include <wx/ffile.h>
#include <vector>
#include <list>
using namespace std;

#define BACKTRACE_SIZE 100
class BreakPointInfo;
class mxInspectionGrid;
class mxIEItemData;
class wxGrid;
class project_file_item;
class mxInspectionMatrix;
class mxInspectionExplorer;
class mxExternInspection;
class mxBreakList;

enum {	DI_NONE, 
		DI_CLASS, // clase A
		DI_IN_CLASS, // A.private
		DI_FATHER_CLASS, // A.B
		DI_IN_FATHER_CLASS, // A.B.private
		DI_POINTER, // int *p
		DI_CLASS_POINTER, // clase *P
		DI_IN_CLASS_POINTER, // P->private
		DI_FATHER_POINTER, // P->B
		DI_IN_FATHER_POINTER, // P->B.private
		DI_ARRAY  // int a[10]
	};

enum DEBUG_STATUS { // stages of a debug session
	DBGST_NULL, // no debugging session running
	DBGST_STARTING, // debugging session is startig, program isn't running yet
	DBGST_DEBUGGING, // debugging session running, but debugmanager talking to gdb
	DBGST_STOPPING  // debugging session is terminating, waiting for process to end
};

//! Referencia del depurador a un punto de interrupción en gdb
struct breakinfo {
	wxString fname;
	int line, n;
	breakinfo(int an, wxString afname, int aline) {
		fname=afname;
		line=aline;
		n=an;
	}
	bool operator==(breakinfo b) {
		return b.fname==fname&&b.line==line;
	}
};

/**
* @brief Información acerca de una inspección en el depurador
*
* Guarda un conjunto de inspecciones. Estas listas se generan a partir de la 
* grilla de inspecciones, y se cargan tambien ahi. Sirve para recuperar conjuntos
* de inspecciones entre sesiones de depuracion. Si es un proyecto se guarda y lee
* con el archivo de proyecto, sino se guardan temporalmente aca y por ahora se
* pierden al salir o abrir un proyecto. Cuando se cargan, las inspecciones 
* se cargan siemper como inspecciones insensibles al contexto.
**/
struct inspectlist {
	wxString name; ///< nombre de la tabla de inspecciones
	wxArrayString vars; ///< lista de expresiones a inspeccionar
	inspectlist(const wxString &aname):name(aname){}
};

//! Información acerca de una inspección en el depurador
struct inspectinfo {
	
	wxString name; ///< nombre del varible object de gdb, o comando para gdb si no es vo
	wxString expr; ///<  expresion que representa
	wxString type; ///<  tipo de dato c++ de la expresion
	bool frameless; ///< indica si debe evaluarse en cualquier frame, o hay un vo que depende de uno
	wxString frame; ///<  direccion de la frame de la inspeccion
	wxString frame_num; ///<  numero del frame (o "Error" si hay problemas)
	bool is_class; ///<  si es clase o estructura
	bool is_array; ///<  si es arreglo, matriz, etc
	bool on_scope; ///<  si todavia esta en scope
	bool always_evaluate; ///<  si hay que actualizarla siempre aunque no figure en la lista de "-var-update"
	bool use_data_evaluate; ///<  si hay que usar "-data-evaluate-expression" en lugar de "-var-evaluate-expression" para obtener su contenido
	bool watch_read; ///<  si es un watch_point que interrumpe en la lectura
	bool watch_write; ///<  si es un watch_point que interrumpe en la ecritura
	bool is_vo; ///< si es una variable objecto (caso normal), o un comando para gdb (cuando empieza con '>')
	int watch_num; ///< numero de watchpoint (para referenciar en gdb)
	bool freezed; ///< si es true no se actualiza
	
	inspectinfo() {
		freezed = frameless = false;
	}
	inspectinfo(const wxString &aname, const wxString &atype, const wxString &aexpr) {
		name = aname; 
		expr = aexpr; 
		type = atype; 
		is_class = is_array = false;
		watch_read = watch_write = false;
		is_vo = on_scope = true;
		always_evaluate = use_data_evaluate = false;
		freezed = frameless = false;
	}
};

class wxProcess;
class wxOutputStream;
class wxInputStream;
class mxSource;

/**
* @brief Administra la comunicación entre la interfaz y el depurador gdb
**/
class DebugManager {
	friend class mxApplication;
	friend class mxMainWindow;
	friend class mxInspectionGrid;
	friend class mxBacktraceGrid;
	friend class mxInspectionExplorer;
	friend class mxInspectionMatrix;
#ifdef DEBUG
	wxFFile debug_log_file;
#endif
private:
	bool should_pause; ///< puede que al hacer click en la pausa no se pause realmente (que la señal que envía no llegue a término, no se por qué, pero pasa cuando hay un breakpoint de los que solo actualizan la tabla de inspecciones)
	bool has_symbols; ///< si cuando el debugger no inicia es porque no el ejecutable no tiene info de depuracion se baja esta bandera
	mxInspectionGrid *inspection_grid;
	bool recording_for_reverse; ///< indica si se ejecuto el comando "record" para habilitar luega la ejecucion hacia atras
	bool attached; ///< indica si el programa se inicio en el depurador, o el depurador es "attacheo" mas tarde, para saber como salir
	wxArrayString black_list;
	bool stepping_in;
	bool inverse_exec;
	bool gui_is_prepared;
	list<mxExternInspection*> extern_list;
	wxString current_frame, current_frame_num;
	wxString frames_addrs[BACKTRACE_SIZE];
	wxString frames_nums[BACKTRACE_SIZE];
	int last_backtrace_size;
	int inspections_count;
	vector<inspectinfo> inspections;
#if !defined(_WIN32) && !defined(__WIN32__)
	wxProcess *tty_process;
	long tty_pid;
	bool tty_running;
#endif
	wxString current_file;
	mxSource *current_source; ///< el fuente en cual se marco la ultima posicion para ejecutar
	mxSource *notitle_source; ///< el ultimo fuente sin nombre que se le procesaron los breakpoints
	int current_handle;
	wxChar buffer[256];
	wxProcess *process;
	wxOutputStream *output;
	wxInputStream *input;
	long pid;
	wxString SendCommand(wxString command);
	wxString SendCommand(wxString cmd1,wxString cmd2);
	wxString SendCommand(wxString command, int i);
//	wxString EscapeString(wxString str, bool add_comillas=false); // paso a mxutils
	wxString RewriteExpressionForBreaking(wxString expr);
	wxString last_error; ///< para evitar pasar strings por referencia a cada rato (ver ModifyInspection)
	BreakPointInfo *pause_breakpoint; ///< puntero al breakpoint que hay que agregar/sacar en una pausa, para el usuario los modifica durante la ejecución
public:
	wxString last_command, last_answer;
	DEBUG_STATUS status;
	bool debugging, running, waiting, /*backtrace_visible,*/ threadlist_visible;
	bool really_running; ///< solo para indicar desde al WaitAnswer que esta esperando el resultado de una ejecución del programa que se esta depurando y no de un comando gdb para setear o averiguar algo (para que en el primer caso haga yield y en el otro no)
	DebugManager();
	~DebugManager();
	void BacktraceClean();
	bool Start(bool update); ///< starts debugging for current project
	bool Start(bool update, mxSource *source); ///< starts debugging for a simple program
	bool Start(wxString workdir, wxString exe, wxString args, bool show_console, bool wait_for_key);
	bool Stop();
	bool Run();
	void StepIn();
	void StepOver();
	void StepOut();
	wxString GetAddress(wxString fname, int line); ///< Devuelve la direccion de memoria donde comienza una linea de codigo
	bool Jump(wxString fname, int line); ///< Mueve el program counter a un punto especifico del fuente
	bool RunUntil(wxString fname, int line); ///< Continua la ejecucion hasta llegar a un determinado punto en el fuente
	bool Return(wxString what);
	void Pause();
	void Continue();
	bool MarkCurrentPoint(wxString cf=_T(""), int cline=-1, int cmark=-1);
	void HowDoesItRuns();
	void CloseSource(mxSource *source);
	void SetStateText(wxString text, bool refresh=false);
	void SetBreakPoints(mxSource *source);
	int SetLiveBreakPoint(BreakPointInfo *_bpi);
	int SetBreakPoint(BreakPointInfo *_bpi);
	void SetBreakPointEnable(int num, bool enable, bool once=false);
	void SetBreakPointOptions(int num, int ignore_count);
	bool SetBreakPointOptions(int num, wxString condition);
	int GetBreakHitCount(int num);
	void DeleteBreakPoint(BreakPointInfo *_bpi);
	wxString GetValueFromAns(wxString ans, wxString key, bool crop = false, bool fix_slash=false);
	wxString GetSubValueFromAns(wxString ans, wxString key1, wxString key2, bool crop=false, bool fix_slash=false);
	wxString InspectExpression(wxString var, bool pretty=false);
	wxString WaitAnswer();
	bool Backtrace(bool dont_select_if_first=false);
#if !defined(_WIN32) && !defined(__WIN32__)
	void TtyProcessKilled();
#endif
	void ProcessKilled();
	bool DuplicateInspection(int num);
	bool ModifyInspection(int num, wxString expr, bool force_new=false);
	bool DeleteInspection(int num);
	bool ModifyInspectionValue(int num, wxString value);
	bool ModifyInspectionFormat(int num, wxString format);
	bool ModifyInspectionWatch(int num, bool read, bool write);
	bool UpdateInspection();
	wxString GetNextItem(wxString &ans, int &from);
	bool SelectFrame(wxString framenum, int idx);
	bool SelectFrameForInspeccion(wxString addr);
	bool ToggleInspectionFreeze(int n);
	bool DoThat(wxString what);
	bool BreakCompoundInspection(int n);
	bool CreateVO(wxString expr, wxString &name, wxString &type, int &children);
	wxString GetVOValue(wxString name);
	bool DeleteVO(wxString name);
	int GetVOChildrenData(mxIEItemData **data, wxString name, wxString main_expr, wxString main_frame, int what_is);
	void ClearInspections();
	bool GetArgs (wxArrayString &args, wxString level);
	bool GetLocals (wxArrayString &locals, wxString level);
	void MakeRoomForInspections(int pos, int cant);
	void PopulateBreakpointsList(mxBreakList *break_list, bool also_watchpoints);
//	BreakPointInfo *FindBreakInfoFromNumber(int _id, bool use_gdb_id=true);
	void RegisterExternInspection(mxExternInspection* ei);
	void UnRegisterExternInspection(mxExternInspection* ei);
//	void RegisterMatrix(mxInspectionMatrix* matrix);
//	void UnRegisterMatrix(mxInspectionMatrix* matrix);
//	void RegisterExplorer(mxInspectionExplorer* explorer);
//	void UnRegisterExplorer(mxInspectionExplorer* explorer);
	
	bool Attach(long pid, mxSource *source);
	
	/** @brief Inicia el depurador para explorar un archivo core de GNU/Linux **/
	bool LoadCoreDump(wxString core_file, mxSource *source);
	
	/** @brief Genera un core dump. Se utiliza durante la depuración para guardar un estado en una pausa. **/
	bool SaveCoreDump(wxString core_file);
	
	/** @brief Resetea los atributos de estado para comenzar un nuevo proceso de depuración **/
	void ResetDebuggingStuff();
	
	/** @brief Devuelve la salida de un comando sin los agregados de mi, para usar con las inspecciones-macro **/
	wxString GetMacroOutput(wxString cmd, bool keep_endl=false);
	
	bool EnableInverseExec();
	bool ToggleInverseExec();

	vector<inspectlist*> inspections_tables;
	void SaveInspectionsTable(wxString name);
	void SaveInspectionsTable(inspectlist *il);
	void LoadInspectionsTable(wxString name);
	void LoadInspectionsTable(inspectlist *il);
	void DeleteInspectionsTable(wxString name);
	inspectlist *GetInspectionsTable(wxString name, bool create_if_not_exists=false);
	void ClearSavedInspectionTables();
	void UpdateFramelessInspection();
	void SetFramelessInspection(int i);
	
	/// @brief los fuentes avisan al destruirse, para evitar usar punteros invalidos
	void UnregisterSource(mxSource *src); 
	
	/// @brief modifica una inspeccion cuando no hay sesion de depuración en curso
	bool OffLineInspectionModify(int i, wxString value);
	/// @brief elimina inspecciones cuando no hay sesion de depuración en curso
	bool OffLineInspectionDelete(int i=-1);
	
	void ListThreads();
	void ThreadListClean();
	void SelectThread(wxString id);
	
	/// @brief habilita o deshabilita el mostrado completo de arreglos (set print elements ... en gdb), para deshabilitar desde ventanas como mxInspectionPrint, normalmente debe estar habilitado
	void SetFullOutput(bool on=false);
	
	/// @brief muestra un mensaje de alerta/ayuda cuando no se pudo colocar un breakpoint
	void ShowBreakPointLocationErrorMessage(BreakPointInfo *_bpi);
	void ShowBreakPointConditionErrorMessage(BreakPointInfo *_bpi);
	
	/// @brief  calls -var-create and returns its output, but applying improving_inspections_templates
	wxString CreateVO(wxString &expr, wxString &type);
};

extern DebugManager *debug;

#endif
