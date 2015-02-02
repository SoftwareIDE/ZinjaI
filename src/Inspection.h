#ifndef INSPECTION_H
#define INSPECTION_H
#include <map>
#include <wx/string.h>
#include "DebugManager.h"
#include "mxUtils.h"
#include "SingleList.h"
#include "Flag.h"
#include <wx/msgdlg.h>
using namespace std;

////! Información acerca de una inspección en el depurador

/// Tipo/estado de una inspeccion
enum DEBUG_INSPECTION_EXPRESSION_TYPE {
	DIT_GDB_COMMAND, ///< la expresion es en realidad una macro o comando para gdb
	DIT_VARIABLE_OBJECT, ///< la expresion tienen un variable object asociado
	DIT_PENDING, ///< la expresión creará un vo, pero su creación esta en cola para la próxima pausa
	DIT_GHOST, ///< vo que no está en el mapa ni recibe actualizaciones, solo existe porque otras dependen de ella (num_children!=0, ver VOBreak)
	DIT_ERROR ///< la expresión debía crear un vo, pero ocurrió un error al intentarlo
};

#define DIF_FRAMELESS 1
#define DIF_IN_SCOPE 2
#define DIF_REQUIRES_MANUAL_UPDATE 4
#define DIF_DONT_USE_HELPER 8
#define DIF_AUTO_IMPROVE 16
#define DIF_IS_INTERNAL_HELPER 32
#define DIF_FULL_OUTPUT 64

#ifdef _INSPECTION_LOG
#include <sstream>
#include <wx/ffile.h>
	extern wxFFile inspection_log_file;
#endif

enum GDB_VO_FORMAT { GVF_NATURAL, GVF_BINARY, GVF_OCTAL, GVF_DECIMAL, GVF_HEXADECIMAL };

enum DEBUG_INSPECTION_MESSAGE { DIMSG_PENDING, DIMSG_OUT_OF_SCOPE, DIMSG_ERROR };

class DebuggerInspection;

///< class base que heredarán los componentes visuales para ser notificados de los cambios en el estado de las inspecciones de forma individual
class myDIEventHandler {
	bool owned_by_the_inspection; ///< if true, the inspections will do the delete
public:
	myDIEventHandler(bool _owned_by_the_inspection=false) : owned_by_the_inspection(_owned_by_the_inspection) {}
	virtual void OnDICreated(DebuggerInspection *di) {}
	virtual void OnDIError(DebuggerInspection *di) {}
	virtual void OnDIValueChanged(DebuggerInspection *di) {}
	virtual void OnDIOutOfScope(DebuggerInspection *di) {}
	virtual void OnDIInScope(DebuggerInspection *di) {}
	virtual void OnDINewType(DebuggerInspection *di) {}
	virtual ~myDIEventHandler() {};
};

///< class base que heredarán los componentes visuales para ser notificados de los cambios en el estado de la depuracion
class myDIGlobalEventHandler {
	bool registered;
public:
	myDIGlobalEventHandler();
	void UnRegister();
	virtual ~myDIGlobalEventHandler();
	/// debug session is starting
	virtual void OnDebugStart() {}
	/// debug session is ending
	virtual void OnDebugStop() {}
	/// debug session is paused, before updating inspections
	virtual void OnDebugPausePre() {}
	/// debug session is paused, after updating inspections
	virtual void OnDebugPausePost() {}
};

class myCompoundHelperDIEH : public myDIEventHandler {
	DebuggerInspection *helper_parent;
	friend class DebuggerInspection;
public:
	myCompoundHelperDIEH(DebuggerInspection *parent):myDIEventHandler(true),helper_parent(parent) {}
	virtual void OnDIValueChanged(DebuggerInspection *di);
};

class myUserHelperDIEH : public myDIEventHandler {
	DebuggerInspection *helper_parent;
public:
	myUserHelperDIEH(DebuggerInspection *parent):myDIEventHandler(true),helper_parent(parent) {}
	virtual void OnDIValueChanged(DebuggerInspection *di);
};

/**
* @brief Clase para gestionar todo lo relacionado a las inspecciones a nivel zinjai-gdb (no interfaz de usuario)
*
* Esta clase representa en sus atributos regulares una inspeccion, que puede ser "consumida"
* por cualquier componente de la interfaz. La inspeccion encapsula la comunicación con el depurador
* y la gestion de su estado interno en zinjai, para que las clases que la usan solo se preocupen
* por cómo mostrarla. Además tiene métodos staticos para gestionar el conjunto completo de inspecciones
* de forma de poner acá toda la comunicación con gdb que tenga que ver con inspecciones y que
* DebugManager solo llame a un par de métodos y se desligue del tema.
**/
struct DebuggerInspection {

#ifdef _INSPECTION_LOG
#	define __null_inspection__ (DebuggerInspection*)0xFFFFFFF
	class CallLogger {
		static int lev;
		DebuggerInspection *di;
		const char *method;
	public:
		CallLogger(const char *_method, DebuggerInspection *_di=__null_inspection__) : di(_di),method(_method) {
			stringstream cerr;
			cerr<<"DI("<<di<<")::"<<string((++lev)*2,' ')<<method<<"  in";
			if (di!=__null_inspection__) cerr<<": dtype="<<di->dit_type<<" vtype="<<di->value_type<<" expr="<<di->expression<<"  vo="<<di->variable_object<<" "<<(di->parent?"p":"")<<(di->helper?"h":"")<<(di->di_children?"c":"")<<(di->IsFrameless()?"f":"")<<(di->IsInScope()?"s":"");
			cerr<<endl;
			inspection_log_file.Write(cerr.str().c_str()); inspection_log_file.Flush();
		}
		~CallLogger() { 
			stringstream cerr;
			cerr<<"DI("<<di<<")::"<<string((lev--)*2,' ')<<method<<" out";
			if (di!=__null_inspection__) cerr<<": dtype="<<di->dit_type<<" vtype="<<di->value_type<<" expr="<<di->expression<<"  vo="<<di->variable_object<<" "<<(di->parent?"p":"")<<(di->helper?"h":"")<<(di->di_children?"c":"")<<(di->IsFrameless()?"f":"")<<(di->IsInScope()?"s":"");
			cerr<<endl;
			inspection_log_file.Write(cerr.str().c_str()); inspection_log_file.Flush();
		}
	};
#	define __debug_log_method__ CallLogger _call_logger_(__FUNCTION__,this)
#	define __debug_log_static_method__ CallLogger _call_logger_(__FUNCTION__)
#	define __debug_log_message__(msg) { stringstream cerr; cerr<<msg<<endl; inspection_log_file.Write(cerr.str().c_str()); } inspection_log_file.Flush()
#else
#	define __debug_log_method__ 
#	define __debug_log_static_method__ 
#	define __debug_log_message__(msg)
#endif
	
	/// inspecciones a reestablecer al reiniciar la depuración, guarda todas las útiles para el usuario
	static SingleList<DebuggerInspection*> all_inspections;
	
	/// asocia los variable_objects (por nombre, como lo da gdb) a las instancias de esta clase que los representan internamente en ZinjaI
	typedef map<wxString,DebuggerInspection*> vo2di_type;
	static vo2di_type vo2di_map;
	
	/// busca una instancia de esta clase DebuggerInspection a partir del nombre de su variable-object en gdb
	static DebuggerInspection *GetFromVO(const wxString &variable_object) {
		map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(variable_object);
		if (it!=vo2di_map.end()) return it->second; else return NULL;
	}
	
	
	typedef void (DebuggerInspection::*pending_action)();
	
	/**
	* Estructura para encolar acciones que se invoacn cuando el depurador esta ocupado y entonces deben pospuestas para la próxima pausa (ver ProcessPendingActions)
	**/
	struct DIPendingAction {
		DebuggerInspection *inspection; ///< inspeccion a la que se refiere
		pending_action action; ///< metodo a ejecutar (si es null, delete)
		bool requires_debug_pause; ///< si es true solo se ejecuta si los eventos pendientes se procesan en una pausa de la depuracion (para cuando son comandos gdb), sino simplemente se ignora
		bool dont_run_now; ///< para cuando se agrega en la cola desde una llamada de la propia cola, para que no se procese a continuacion, sino que se deje para la proxima pasada por la cola
		DIPendingAction() {}
		DIPendingAction(DebuggerInspection *_inspection, pending_action _action, bool _requires_debug_pause, bool _dont_run_now):
			inspection(_inspection),action(_action),requires_debug_pause(_requires_debug_pause),dont_run_now(_dont_run_now) {}
	};
	
	/// lista de acciones en cola para ejecutarse cuando el depurador se detenga
	static SingleList<DIPendingAction> pending_actions;
	
	static void AddPendingAction(DebuggerInspection *inspection, pending_action action, bool requires_debug_pause, bool dont_run_now=false) {
		__debug_log_message__( "DI::AddPendingAction: "<<(void*)action<<"  insp="<<inspection );
		pending_actions.Add(DIPendingAction(inspection,action,requires_debug_pause,dont_run_now));
	}
	
	
	/**
	* @brief metodo a usar antes de intentar dialogar con gdb... 
	*
	* Si no se puede dialogar ahora (esta ejecutando), retorna falso y además
	* encola el intento para que se intente nuevamente cuando se pueda (en la proxima pausa)
	**/
	bool TryToExec(pending_action action, bool requires_debug_pause) {
		if (debug->waiting) {
			AddPendingAction(this,action,requires_debug_pause);
			return false;
		} else if (debug->debugging || !requires_debug_pause) {
			(this->*action)();
			return true;
		}
		return true;
	}
		
	static void ProcessPendingActions() {
		__debug_log_static_method__;
		if (debug->debugging && debug->waiting) {
			DEBUG_INFO("ERROR: ProcessPendingActions: debug->debugging && debug->waiting");
		}
		int dont_run_now_count = 0, initial_size = pending_actions.GetSize();
		for(int i=0;i<pending_actions.GetSize();i++) {
			DIPendingAction &pa=pending_actions[i];
			if (!pa.inspection) continue; // skip actions for deleted inspections
			if (i>=initial_size && pa.dont_run_now) { 
				pa.dont_run_now=false;
				pending_actions[dont_run_now_count++]=pa;
			} else if (pa.action) {
				if (debug->debugging||!pa.requires_debug_pause)
					(pa.inspection->*pa.action)();
			} else {
				delete pa.inspection; // action=NULL signfica que hay que eliminar el objeto
				// delete all pending actions for this inspection to avoid future references
				for(int j=0;j<pending_actions.GetSize();j++) { 
					if (j!=i && pending_actions[j].inspection==pa.inspection) 
						pending_actions[j].inspection=NULL;
				}
			}
		}
		pending_actions.Resize(dont_run_now_count);
	}
	
	/// para notificar los cambios a los componentes de interfaz que utilizan estas inspecciones
	void GenerateEvent(void (myDIEventHandler::*foo)(DebuggerInspection*)) { 
		if (/* !is_frozen && */consumer) (consumer->*foo)(this);
	}
	
	
	friend class myCompoundHelperDIEH;
	friend class myUserHelperDIEH;
	
	friend class myDIGlobalEventHandler;
	static SingleList<myDIGlobalEventHandler*> global_consumers;
	
public:
	
	static void OnDebugStart();
	static void OnDebugPause();
	static void OnDebugStop();
	
	/// Actualiza todas las inspecciones (consultando a gdb, se debe invocar desde DebugManager cuando hay una pausa/interrupción en la ejecución)
	static void UpdateAllVO(const wxString &voname="*");
	static void UpdateAllManual();
	
	
private:
	DEBUG_INSPECTION_EXPRESSION_TYPE dit_type; ///< si es una vo, una macro, todavía no se creo, o fallo la creacion
//	bool requieres_manual_update; ///< false, para los VOs comunes, true para los comandos gdb y para VOs que son intermediarios para otros VOs (helpers de compuestas)
	// inf definida por el usuario/consumidor de la inspeccion
	wxString expression; ///< expresión que está siendo inspeccionada
	wxString short_expression; ///< solo para expresiones hijas, contiene solo la parte hija, y no es valida como tal (ej, se descompuso x en x.a y x.b, expression tendria x.a/x.b, mientras que short_expression a/b), solo lo tienen quienes se generaron con Break
	Flag flags; /// combinacion de DIF_FRAMELESS, DIF_IN_SCOPE, DIF_REQUIRES_MANUAL_UPDATE, DIF_DONT_USE_HELPER
	wxString variable_object; ///< si es variable object (ver dit_type) guarda el nombre de la vo, sino el comando gdb que se evalua
//	bool is_frameless; ///< si su valor está asociado a un frame/scope particular o no (en gdb se conocen como "floating" variable objects)
	long thread_id, frame_id; ///< si no es frameless, aqui se guarda el scope al que está asociada
//	bool is_in_scope; ///< autoexplicativo (solo para vo, los comandos gdb siempre tendran true)
//	bool is_frozen; ///< autoexplicativo (solo para vo, los comandos gdb siempre tendran true)
	myDIEventHandler *consumer; ///< puntero a quien utiliza esta inspección, para notificarle los cambios
	// informacion "calculada"
	wxString value_type; ///< tipo de dato c/c++ del resultado de la expresión
	long num_children; ///< si es una inspeccion compuesta, cantidad de "partes" (campos en un struct, elementos en un arreglo, etc)
	wxString gdb_value; ///< valor tal como lo da gdb, sin alterar
	DebuggerInspection *helper; ///< vo auxiliar para mostrar mejor este cuando es compuesto, helper sera responsable de actualizar this->gdb_value y generar sus eventos
	DebuggerInspection *parent; ///< otro vo del cual depende este (el otro era compuesto, este es hijo)
	int di_children; ///< cantidad de vo hijos que dependen de este
	GDB_VO_FORMAT vo_value_format; ///< con que formato debe gdb mostrar el valor, solo para tipos simples
	
	void VODelete() {
		__debug_log_method__;
		debug->SendCommand("-var-delete ",variable_object);
	}
	
	bool VOCreate() {
		__debug_log_method__;
		wxString cmd = "-var-create - ";
		if (IsFrameless()) cmd<<"@ "; else cmd<<"* "; 
		thread_id=debug->current_thread_id; frame_id=debug->current_frame_id;
		wxString ans = debug->SendCommand(cmd,mxUT::EscapeString(expression,true));
		if (ans.Left(5)!="^done") return false;
		flags.Set(DIF_IN_SCOPE); // si no existen en el scope actual gdb no la crea, aunque se frameless
		variable_object = debug->GetValueFromAns(ans,"name",true);
		value_type = debug->GetValueFromAns(ans,"type",true);
		gdb_value = debug->GetValueFromAns(ans,"value",true);
		debug->GetValueFromAns(ans,"numchild",true).ToLong(&num_children);
		vo2di_map[variable_object] = this;
		return true;
	}
	
//	void VOSetFrozen() {
//		debug->SendCommand(wxString("-var-set-frozen ")+variable_object,is_frozen?" 1":" 0");
//	}
	
	void VOSetFormat() {
		const char *format;
		switch (vo_value_format) {
			case GVF_NATURAL: format=" natural"; break;
			case GVF_BINARY: format=" binary"; break;
			case GVF_OCTAL: format=" octal"; break;
			case GVF_DECIMAL: format=" decimal"; break;
			case GVF_HEXADECIMAL: format=" hexadecimal"; break;
			default: return;
		}
		debug->SendCommand(wxString("-var-set-format ")+variable_object,format);
		VOEvaluate();
	}
	
	void VOEvaluate() {
		__debug_log_method__;
		wxString ans = debug->SendCommand(wxString("-var-evaluate-expression "),variable_object);
		gdb_value = debug->GetValueFromAns(ans,"value",true,true);
	}
	
	bool VOAssign(const wxString &new_value) {
		__debug_log_method__;
		wxString ans = debug->SendCommand(wxString("-var-assign ")<<variable_object<<" "<<mxUT::EscapeString(new_value,true));
		return ans.StartsWith("^done");
	}
	
	void DeleteHelper() {
		delete helper->consumer;
		helper->consumer=NULL; 
		helper->Destroy(); 
		helper=NULL;
	}
	
	bool SetupChildInspection() {
		__debug_log_method__;
		// si habia, borrar la inspeccion auxiliar previa
		if (helper) 
			DeleteHelper();
		// si no hay que intentar simplificar esta
		if (flags.Get(DIF_DONT_USE_HELPER)) return false;
		// si hay una mejora automática para este tipo
		if (flags.Get(DIF_AUTO_IMPROVE)&&AutoImprove()) return true;
		// si no tiene hijos, no necesita la inspeccion auxiliar
		if (!IsCompound() && !flags.Get(DIF_FULL_OUTPUT)) return false;
		// si tiene hijos, intentar crear la expresion auxiliar
		helper = /*DebuggerInspection::*/Create( 
			wxString("&(")<<expression<<")",
			FlagIf(DIF_FRAMELESS,IsFrameless()) | FlagIf(DIF_FULL_OUTPUT,flags.Get(DIF_FULL_OUTPUT)) 
			| DIF_IS_INTERNAL_HELPER | DIF_DONT_USE_HELPER,
			new myCompoundHelperDIEH(this), true );
		if (helper->dit_type==DIT_ERROR) { DeleteHelper(); return false; }
		helper->MakeEvaluationExpressionForParent(this);
		helper->flags.Set(DIF_REQUIRES_MANUAL_UPDATE);
		helper->UpdateValue(false);
		return true;
	}
	
	/// for automatic inspections improvement
	bool SetHelperInspection(const wxString &new_expression) {
		__debug_log_method__;
		// si habia, borrar la inspeccion auxiliar previa
		if (helper) DeleteHelper();
		// intentar crear la expresion auxiliar
		helper = /*DebuggerInspection::*/Create(
			new_expression,
			FlagIf(DIF_FRAMELESS,IsFrameless()) | FlagIf(DIF_FULL_OUTPUT,flags.Get(DIF_FULL_OUTPUT)),
			new myUserHelperDIEH(this), true );
		if (helper->dit_type==DIT_ERROR) { DeleteHelper(); return false; }
		gdb_value = helper->gdb_value; // Create does a first evaluation
		return true;
	}
	
public:
	void DeleteHelperInspection() {
		if (helper) DeleteHelper();
		UpdateValue(true);
	}
	
	bool AskGDBIfIsEditable() {
		if (!debug->debugging||debug->waiting) return false;
		if (dit_type!=DIT_VARIABLE_OBJECT) return false;
		wxString ans = debug->SendCommand("-var-show-attributes ",variable_object);
		if (ans.Contains("noneditable")) return false;
		else return ans.Contains("editable");
	}
	
	void ForceVOUpdate() {
		if (!debug->debugging||debug->waiting) return;
		if (dit_type!=DIT_VARIABLE_OBJECT) return;
		UpdateAllVO(variable_object);
	}
	
	
private:
	/// solo debe ser llamado cuando dit_type==DIT_HELPER_VO
	void MakeEvaluationExpressionForParent(DebuggerInspection *parent) {
		__debug_log_method__;
		const wxString &type = parent->value_type;
		int i=type.Len(); int  plev=0,pend=-1,pbeg=i;
		while (--i>=0) {
			if (type[i]==']') { if (plev++==0) pend=i; }
			else if (type[i]=='[') { if (--plev==0) pbeg=i; }
			else if (type[i]<'0'||type[i]>'9') break;
		}
		wxString mtype = type.Mid(0,pbeg); mtype.Replace("&","",true);
		while (mtype.Contains("::") && debug->SendCommand(wxString("p (")<<mtype<<"*)0x0").StartsWith("^error")) // gdb seems to simplify some nested typenames and the does not recognize them with their full scoped name
			mtype=mtype.AfterFirst(':').Mid(1);
		wxString mvalue = gdb_value; // evaluar con un vo algo como "&(p)" puede dar "0xBOO <tipo_de_p>", solo queremos la direccion
		if (mvalue.StartsWith("0x")&&mvalue.Contains(" ")) mvalue=mvalue.BeforeFirst(' ');
		if (pbeg!=-1 && pbeg+1<pend) // arreglo
			expression = wxString("*((")<<mtype<<"(*)"<<type.Mid(pend+1)<<")"<<mvalue<<")@"<<type.Mid(pbeg+1,pend-pbeg-1);
		else // clase
			expression = wxString("*((")<<mtype<<"*)"<<mvalue<<")";
	}
	
	void CreateVO() {
		__debug_log_method__;
		if (VOCreate()) {
			dit_type = DIT_VARIABLE_OBJECT;
			if (!SetupChildInspection()) {
				if (vo_value_format!=GVF_NATURAL) VOSetFormat();
//				VOEvaluate(); // no es necesario, el create ya da un value
			}
//			if (is_frozen) VOSetFrozen();
			/*else */GenerateEvent(&myDIEventHandler::OnDICreated);
			// this is an ugly hack for a gdb-bug... when you create a not-frameless vo for an expresion, 
			// previous frameless vos for the same expression may show a wrong result (tested: bad on gdb 7.6.1, good on 7.8.0)
			if (!IsFrameless() && debug->gdb_version<7008) RecreateAllFramelessInspections(expression);
		} else {
			dit_type=DIT_ERROR; 
			GenerateEvent(&myDIEventHandler::OnDIError);
			// si no se pudo crear en este scope, probar otra vez en el proximo
			if (IsFrameless()) 
				AddPendingAction(this,&DebuggerInspection::CreateVO,true,true);
		}
	}
	
	bool AutoImprove() {
		__debug_log_method__;
		wxString new_expr;
		bool ret = TryToImproveExpression(value_type,new_expr,expression);
		if (ret) SetHelperInspection(new_expr);
		return ret;
	}
	
	void RecreateAllFramelessInspections(const wxString &expression);
	
	/// las instancias de los clientes de esta clase se construyen solo a través de Create (que usará este ctor)
	DebuggerInspection(DEBUG_INSPECTION_EXPRESSION_TYPE type, const wxString &expr, const Flag &flags, myDIEventHandler *event_handler=NULL) :
		dit_type(type),
		expression(expr),
		flags( FlagIf(DIF_REQUIRES_MANUAL_UPDATE,type==DIT_GDB_COMMAND) | flags.Get() | DIF_IN_SCOPE ),
//		is_frozen(false),
		consumer(event_handler),
		helper(NULL),
		parent(NULL),
		di_children(0),
		vo_value_format(GVF_NATURAL)
		{ 
			__debug_log_method__;
		};
	
	/// ctor para una vo hija, creada por VOBreak
	DebuggerInspection(DebuggerInspection *_parent, const wxString &vo_name, const wxString &expr, const wxString &type, int num_child) : 
		dit_type(DIT_VARIABLE_OBJECT),
		expression(expr),
		flags(DIF_IN_SCOPE|(_parent->IsFrameless()?DIF_FRAMELESS:0)),
		variable_object(vo_name),
		thread_id(_parent->thread_id),
		frame_id(_parent->frame_id),
//		is_frozen(_parent->is_frozen), 
		consumer(_parent->consumer), 
		value_type(type),
		num_children(num_child),
		helper(NULL),
		parent(_parent),
		di_children(0),
		vo_value_format(GVF_NATURAL)
		{ 
			__debug_log_method__;
		};
	
	void RemoveParentLink() {
		if (--parent->di_children==0 && parent->dit_type==DIT_GHOST)  parent->Destroy(); 
		parent=NULL;
	}
	
	/// las instancias se destruyen a través de Destroy, esto evita que alguien de afuera le quiera hacer delete
	~DebuggerInspection() {
		__debug_log_method__;
	}; 
		
	DebuggerInspection(const DebuggerInspection &); ///< esta clase no es copiable
	void operator=(const DebuggerInspection &); ///< esta clase no es copiable
	
public:

	/**
	* @brief Crea una instancia a partir de una expresion
	*
	* @param init_now si es falso, no se inicializa (no se registra en gdb la variable_object).
	*                 se debe llamar inmediatamente a Init con la instancia... esto es para que
	*                 el que la crea pueda conocer el puntero antes de recibir los eventos
	**/ 
	static DebuggerInspection *Create(const wxString &expr, Flag flags, myDIEventHandler *event_handler=NULL, bool init_now=true) {
		__debug_log_static_method__;
		bool is_cmd = expr.size()&&expr[0]=='>';
		DebuggerInspection *di = new DebuggerInspection(is_cmd?DIT_GDB_COMMAND:DIT_PENDING,expr,is_cmd?DIF_FRAMELESS:flags,event_handler);
		if (is_cmd) di->value_type="<gdb command>";
		all_inspections.Add(di);
		if (init_now) di->Init();
		return di;
	}
	
	void FirstManualEvaluation() {
		__debug_log_method__;
		UpdateValue(false);
		GenerateEvent(&myDIEventHandler::OnDICreated);
	}
	
	/**
	* @brief registra la vo de una inspeccion en gdb, ver DebuggerInspection::Create
	*
	* @retval true si se completó la inicialización ahora (o si no era necesario 
	* hacer nada), false si quedó pendiente para la próxima pausa
	**/ 
	bool Init() {
		__debug_log_method__;
		if (dit_type==DIT_PENDING) { // si es variable-object y estamos en depuracion, crearla en gdb
			return (!debug->debugging || TryToExec(&DebuggerInspection::CreateVO,true)); 
		} else if (dit_type==DIT_GDB_COMMAND) {
			variable_object = expression.Mid(1);
			return (!debug->debugging || TryToExec(&DebuggerInspection::FirstManualEvaluation,true));
		}
		return true;
	}
	
	/// delete lógico, el real se hará en ProcessPendingActions (considera las dependencias, y funciona para cualquier tipo de inspeccion)
	void Destroy() {
		__debug_log_method__;
		if (dit_type==DIT_VARIABLE_OBJECT||dit_type==DIT_GHOST) { // si tenia una vo asociado....
			// si tenía otra inspección auxiliar asociada, eliminar esa también
			if (helper) DeleteHelper(); // poner en NULL por si queda como DIT_GHOST
			// si no tiene dependencias, eliminarla en gdb
			if (di_children==0) {
				if (debug->debugging) TryToExec(&DebuggerInspection::VODelete,true);
				// si es hijo (dependiente, no helper), restarle un hijo al padre, y si llega a 0 y es GHOST eliminarlo tambien
				if (parent) RemoveParentLink();
			}
			// eliminarla del mapa para que ya no reciba actualizaciones (si no estaba ya eliminada)
			if (dit_type!=DIT_GHOST) {
				map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(variable_object);
				if (it!=vo2di_map.end()) vo2di_map.erase(it); // el if siempre debería dar true
				else { DEBUG_INFO("ERROR: Inspection::Destroy: it==vo2di_map.end(): "<<variable_object); }
			}
		}
		// quitarla de la lista total de inspecciones a reestablecer al reiniciar la depuración
		if (dit_type!=DIT_GHOST) all_inspections.Remove(all_inspections.Find(this));
		// si quedan hijos que dependen de este, dejar como fantasma, sino hacerle el delete
		if (dit_type==DIT_VARIABLE_OBJECT && di_children!=0) dit_type=DIT_GHOST;
		else AddPendingAction(this,NULL,false);
	}
	
	bool ModifyValue(const wxString &new_value) {
		__debug_log_method__;
		if (dit_type!=DIT_VARIABLE_OBJECT) return false;
		if (!debug->CanTalkToGDB()) {
			class OnPauseModifyInspectionValue : public DebugManager::OnPauseAction {
			wxString command;
			public: 
				OnPauseModifyInspectionValue(DebuggerInspection *di, const wxString &new_value) {
					command = wxString("-var-assign ")<<di->variable_object<<" "<<mxUT::EscapeString(new_value,true);
				}
				void Do() { debug->SendCommand(command); }
			};
			debug->PauseFor(new OnPauseModifyInspectionValue(this,new_value));
			return true;
		}
		return VOAssign(new_value);
	}
	
//	void Freeze() {
//		is_frozen=true;
//		if (dit_type==DIT_VARIABLE_OBJECT && debug->debugging) 
//			TryToExec(&DebuggerInspection::VOSetFrozen,true);
//	}

	void SetFormat(GDB_VO_FORMAT format) { 
		vo_value_format=format;
		if (dit_type==DIT_VARIABLE_OBJECT && debug->debugging) TryToExec(&DebuggerInspection::VOSetFormat,true);
	}
	
//	void UnFreeze() {
//		is_frozen=false;
//		if (dit_type==DIT_VARIABLE_OBJECT && debug->debugging) {
//			TryToExec(&DebuggerInspection::VOSetFrozen,true);
////			TryToExec(&DebuggerInspection::VOEvaluate,true);
//		}
//		if (dit_type!=DIT_VARIABLE_OBJECT || helper) UpdateValue();
//	}
	
	/**
	* @brief break an inspections based on a compound vo, into its child vos
	*
	* If its variable objects' num_child is different from 0, this will create
	* a new inspections for avery child vo. Original vo and inspection will remain
	* valid.
	* @param skip_visibility_groups		a class has public, private and/or protected
	*                                   as children vos, and the real attributes 
	*                                   are inside thoose, one more level in. If
	*                                   skip_visibility_groups==true will also 
	*                                   break those children vos, and return their
	*									children instead.
	* @param recursive_on_inheritance 	an inherited class has a vo named as its
	*									base class as child, and the real inherited
	*									stuff as children of that child. If
	*									recursive_on_inheritance=true will also
	*                                   break those children vos, and return their
	*									children instead.
	**/
	bool Break(SingleList<DebuggerInspection*> &children, bool skip_visibility_groups, bool recursive_on_inheritance);
	
	DEBUG_INSPECTION_EXPRESSION_TYPE GetDbiType() { return dit_type; }
	const wxString &GetExpression() const { return expression; }
	const wxString &GetValue() const { return gdb_value; }
	const wxString &GetValueType() const { return value_type; }
	bool IsFrameless() { return flags.Get(DIF_FRAMELESS); }
	bool IsInScope() { return flags.Get(DIF_IN_SCOPE); }
//	bool IsFrozen() { return is_frozen; }
	bool RequiresManualUpdate() { return helper?helper->RequiresManualUpdate():flags.Get(DIF_REQUIRES_MANUAL_UPDATE); }
	bool UpdateValue(bool generate_event=true) { // solo para cuando RequiresManualUpdate()==true
		__debug_log_method__;
		if (!debug->CanTalkToGDB()) return false;
		if (dit_type==DIT_VARIABLE_OBJECT) { // si es vo.... 
			if (helper) return helper->UpdateValue(true); // ...o bien tiene un helper (y el evento del helper actualiza this)...
			if (flags.Get(DIF_IS_INTERNAL_HELPER)) { // ...o bien es el helper de un compuesto...
				gdb_value = debug->InspectExpression(expression,flags.Get(DIF_FULL_OUTPUT));
				DebuggerInspection *helper_parent = reinterpret_cast<myCompoundHelperDIEH*>(consumer)->helper_parent;
				if (helper_parent->gdb_value!=gdb_value) {
					helper_parent->gdb_value=gdb_value;
					if (generate_event) helper_parent->GenerateEvent(&myDIEventHandler::OnDIValueChanged);
					return true;
				}
			} else { // ... o era un vo comun y no requeria actualizacion manual (se usa por ej luego de eliminar un helper)
				wxString old_value = gdb_value;
				VOEvaluate(); 
				if (generate_event && old_value!=gdb_value) GenerateEvent(&myDIEventHandler::OnDIValueChanged);
			}
		} else if (dit_type==DIT_GDB_COMMAND) { // si es comando/macro gdb...
			wxString new_value = debug->GetMacroOutput(variable_object);
			if (new_value!=gdb_value) {
				gdb_value=new_value;
				if (generate_event /*&& is_frozen*/) GenerateEvent(&myDIEventHandler::OnDIValueChanged);
				return true;
			}
		}
		return false;
	}
	// solo llamar a estas funciones si GetDbiType()==DIT_VARIABLE_OBJECT
	bool IsSimpleType() { return num_children==0; } 
	bool IsCompound() { return num_children!=0 && !value_type.EndsWith("*"); }
	bool IsPointer() { return num_children==1 && value_type.EndsWith("*"); }
	bool IsClass() { return IsCompound() && !value_type.EndsWith("]"); }
	bool IsArray() { return IsCompound() && value_type.EndsWith("]"); }
	long GetThreadID() { return thread_id; }
	long GetFrameID() { return frame_id; }
	bool IsFromCurrentThread() { return !debug->debugging || debug->waiting || thread_id==debug->current_thread_id; }
	wxString GetHelperExpression() { return helper&&!helper->flags.Get(DIF_IS_INTERNAL_HELPER)?helper->expression:""; }
	wxString GetShortExpression() { return short_expression.Len()?short_expression:expression; }
	// some ramdon inspections related functions
	static bool TryToImproveExpression (const wxString &pattern, wxString type, wxString &new_expr, const wxString &expr);
	static bool TryToImproveExpression (wxString type, wxString &new_expr, const wxString &expr);
	static wxString RewriteExpressionForBreaking(wxString expr);
	
	static wxString GetUserStatusText(DEBUG_INSPECTION_MESSAGE type);
};

#endif 
