#ifndef MX_DEBUG_MANAGER_H
#define MX_DEBUG_MANAGER_H

#include <wx/string.h>
#include <wx/ffile.h>
#include <vector>
#include <list>
#include <map>
using namespace std;

#define BACKTRACE_SIZE 100
class BreakPointInfo;
class mxInspectionGrid;
class mxIEItemData;
class wxGrid;
class project_file_item;
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


/// estructura para guardar la configuración del manejo de una señal en gdb (ver Debug::GetSignals)
struct SignalHandlingInfo {
	wxString name, description;
	bool pass,print,stop;
	bool operator!=(const SignalHandlingInfo &si) { // to compare states (betweeen same signal structures)
		return si.pass!=pass||si.print!=print||si.stop!=stop;
	}
};

class wxProcess;
class wxOutputStream;
class wxInputStream;
class mxSource;
class DebugPatcher;
class DebuggerInspection;

class DebugManager;
extern DebugManager *debug;


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
	friend class DebuggerInspection;
	friend class DebugPatcher;
#ifdef _ZINJAI_DEBUG
	wxFFile debug_log_file;
#endif
private:
	friend class DebugEventListener;
	vector<SignalHandlingInfo> *signal_handlers_state; ///< signals states to be setted before running, first one has defaults, second one desired settings (if NULL no setting is required, will be created and modified my mxSignalsSettings)
	DebugPatcher *debug_patcher;
public:
	DebugPatcher *GetPatcher() { return debug_patcher; } // retorna puntero y no instancia para poder poner en ese h solo una forward declaration y evitar tener que recompilar mucho al cambiar el patcher

private:
	long gdb_version; ///< version de gdb, se consulta al iniciar la depuracion, si es 7.6 por ej, guarda 7006
	bool should_pause; ///< puede que al hacer click en la pausa no se pause realmente (que la señal que envía no llegue a término, no se por qué, pero pasa cuando hay un breakpoint de los que solo actualizan la tabla de inspecciones)
	bool has_symbols; ///< si cuando el debugger no inicia es porque no el ejecutable no tiene info de depuracion se baja esta bandera
	bool recording_for_reverse; ///< indica si se ejecuto el comando "record" para habilitar luega la ejecucion hacia atras
	bool attached; ///< indica si el programa se inicio en el depurador, o el depurador es "attacheo" mas tarde, para saber como salir
	wxArrayString black_list;
	bool stepping_in;
	bool inverse_exec;
	bool gui_is_prepared;
	long stack_depth; ///< profundidad del backtrace actual, ver current_frame
	// nota para identificaicion de frames: el id (interno de zinjai) tiene base 0, el level (usuario/gdb) tiene base 1
	long current_frame_id; ///< id interno del frame actual, se usa un nro basado en su level en el backtrace, pero inverso (el main seria 0, que figura en la salida de gdb con level=stack_depth-1)
	long GetFrameID(long level) { return stack_depth-level-1; } 
	long GetFrameLevel(long id) { return stack_depth-id-1; }
	long current_thread_id; ///< id del thread actual, lo da gdb al detenerse o al cambiar de hilo
#ifndef __WIN32__
	wxProcess *tty_process; ///< puntero al proceso de la terminal (solo en GNU/Linux), 0 si no hay ninguno
	long tty_pid; ///< pid del proceso de la terminal (solo en GNU/Linux), 0 si no hay ninguno
#endif
//	wxString current_file;
	mxSource *current_source; ///< el fuente en cual se marco la ultima posicion para ejecutar
	mxSource *notitle_source; ///< el ultimo fuente sin nombre que se le procesaron los breakpoints
	int current_handle;
	wxChar buffer[256];
	wxProcess *process;
	wxOutputStream *output;
	wxInputStream *input;
	long pid; ///< pid del proceso de gdb
	long child_pid; ///< pid del proceso que esta siendo depurado, o 0, se averigua bajo demanda con FindOutChildPid
	bool FindOutChildPid(); ///< @brief intenta determinar el pid del proceso depurado
//	wxString EscapeString(wxString str, bool add_comillas=false); // paso a mxutils
	wxString last_error; ///< para evitar pasar strings por referencia a cada rato (ver ModifyInspection)
private:
	DebugManager();
public:
	static void Initialize();
	~DebugManager();
public:
	wxString SendCommand(wxString command);
	wxString SendCommand(wxString cmd1,wxString cmd2);
	wxString SendCommand(wxString command, int i);
	wxString last_command, last_answer;
	bool /*backtrace_visible,*/ threadlist_visible;
	DEBUG_STATUS status;
private:
	bool debugging; ///< indica que hay una sesion de depuracion en marcha (en cualquier estado)
	bool waiting; ///< indica que se esta esperando una respuesta a un comando gdb, por lo que no se puede enviar otro
	bool running; ///< indica que se esta estarando por un comando gdb que involucra la real ejecucion del programa, y no es consulta o configuracion del estado
public:
	bool CanTalkToGDB() { return debugging && !waiting; }
	bool IsPaused() { return debugging && !running; }
	bool IsDebugging() { return debugging; }
	
	void BacktraceClean();
	bool Start(bool update); ///< starts debugging for current project
	bool Start(bool update, mxSource *source); ///< starts debugging for a simple program
	bool Start(wxString workdir, wxString exe, wxString args, bool show_console, bool wait_for_key); ///< common code Starting a program (the other two Starts will end up calling this one)
	bool SpecialStart(mxSource *source, const wxString &gdb_command, const wxString &status_message, bool should_continue);
	void Start_ConfigureGdb(); ///< sends commands to gdb to set its initial state (common code for Start, Attach and LoadCoreDump)
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
	bool MarkCurrentPoint(wxString cf="", int cline=-1, int cmark=-1);
	void HowDoesItRuns();
	void SetStateText(wxString text, bool refresh=false);
	void SetBreakPoints(mxSource *source, bool quiet=false);
	int SetLiveBreakPoint(BreakPointInfo *_bpi);
	int SetBreakPoint(BreakPointInfo *_bpi, bool quiet=false);
	void SetBreakPointEnable(int num, bool enable, bool once=false);
	void SetBreakPointOptions(int num, int ignore_count);
	bool SetBreakPointOptions(int num, wxString condition);
	int GetBreakHitCount(int num);
	bool DeleteBreakPoint(BreakPointInfo *_bpi);
	wxString GetValueFromAns(wxString ans, wxString key, bool crop = false, bool fix_slash=false);
	wxString GetSubValueFromAns(wxString ans, wxString key1, wxString key2, bool crop=false, bool fix_slash=false);
	wxString InspectExpression(wxString var, bool full=false);
	wxString WaitAnswer();
private:
	bool backtrace_shows_args; ///< determine wheter backtrace table should show an extra column with arguments (with values) for each function in the stack
public:
	void SetBacktraceShowsArgs(bool show);
	bool UpdateBacktrace(bool set_frame=true);
#ifndef __WIN32__
	void TtyProcessKilled();
#endif
	void ProcessKilled();
	void UpdateInspections();
	wxString GetNextItem(wxString &ans, int &from);
	/**
	* Cambia el frame actual, solo en gdb, no hace nada de interfaz ni mantenimiento
	*
	* cualquiera de los dos argumentos identifica un frame, pasar uno y dejar el otro en -1 para que se calcule solo
	**/
	bool SelectFrame(long frame_id, long frame_level); 
	bool ToggleInspectionFreeze(int n);
	bool DoThat(wxString what);
	void PopulateBreakpointsList(mxBreakList *break_list, bool also_watchpoints);

private:
	map<wxString,wxString> watchpoints;
public:
	wxString AddWatchPoint(const wxString &expression, bool read, bool write); 
	bool DeleteWatchPoint(const wxString &num);
	
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

//	vector<inspectlist*> inspections_tables;
//	void SaveInspectionsTable(wxString name);
//	void SaveInspectionsTable(inspectlist *il);
//	void LoadInspectionsTable(wxString name);
//	void LoadInspectionsTable(inspectlist *il);
//	void DeleteInspectionsTable(wxString name);
//	inspectlist *GetInspectionsTable(wxString name, bool create_if_not_exists=false);
//	void ClearSavedInspectionTables();
	
	/// @brief los fuentes avisan al destruirse, para evitar usar punteros invalidos
	void UnregisterSource(mxSource *src); 
	
	void UpdateThreads();
	void ThreadListClean();
	bool SelectThread(long thread_id);
	
	void SendSignal(const wxString &signame);
	bool GetSignals(vector<SignalHandlingInfo> &v);
	bool SetSignalHandling(SignalHandlingInfo &si, int i=-1);
	
	/// @brief habilita o deshabilita el mostrado completo de arreglos (set print elements ... en gdb), para deshabilitar desde ventanas como mxInspectionPrint, normalmente debe estar habilitado
	void SetFullOutput(bool on=false, bool force=false);
	
	/// @brief muestra un mensaje de alerta/ayuda cuando no se pudo colocar un breakpoint
	void ShowBreakPointLocationErrorMessage(BreakPointInfo *_bpi);
	void ShowBreakPointConditionErrorMessage(BreakPointInfo *_bpi);
	
	/// @ struct for executing tasks while debugger is runnin... the task is saved with this class, and a debugger pause is triggered, so the debugger can run it an continue
	class OnPauseAction { 
	public: 
		virtual void Do()=0; /// action to perform on pause
		virtual bool Invalidate(void *ptr){ return false; } /// to avoid action on deleted objects
		virtual ~OnPauseAction(){}
	};
	OnPauseAction *on_pause_action;
	bool PauseFor(OnPauseAction *action);
	void InvalidatePauseEvent(void *ptr);
	
public:
	struct TemporaryScopeChange {
		long orig_frame_id, orig_thread_id;
		TemporaryScopeChange(long frame_id=-1, long thread_id=-1) : 
			orig_frame_id(debug->current_frame_id), 
			orig_thread_id(debug->current_thread_id) { 
				if (frame_id!=-1) ChangeTo(frame_id,thread_id);
			}
		void ChangeTo(long frame_id, long thread_id=-1) {
			if (thread_id!=-1 && debug->current_thread_id!=thread_id) debug->SelectThread(thread_id);
			if (debug->current_frame_id!=frame_id) debug->SelectFrame(frame_id,-1);
		}
		void ChangeIfNeeded(DebuggerInspection *di);
		void Reset() {
			if (debug->current_thread_id!=orig_thread_id) debug->SelectThread(orig_thread_id);
			if (debug->current_frame_id!=orig_frame_id) debug->SelectFrame(orig_frame_id,-1);
		}
		~TemporaryScopeChange() { Reset(); }
	};
	
};


#endif

