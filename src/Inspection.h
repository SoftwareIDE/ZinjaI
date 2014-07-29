#ifndef INSPECTION_H
#define INSPECTION_H
#include <map>
#include <wx/string.h>
#include "DebugManager.h"
#include "mxUtils.h"
#include "SingleList.h"
using namespace std;

////! Información acerca de una inspección en el depurador

///< Tipo/estado de una inspeccion
enum DEBUG_INSPECTION_EXPRESSION_TYPE {
	DIT_VARIABLE_OBJECT, ///< la expresion tienen un variable object asociado
	DIT_GDB_COMMAND, ///< la expresion es en realidad una macro o comando para gdb
	DIT_PENDING, ///< la ultima operacion sobre la inspeccion no se ejecutó porque el depurador estaba ocupado, por lo que quedó encolada para cuando se detenga
	DIT_ERROR
};

class DebuggerInspection;

///< class base que heredarán los componentes visuales para ser notificados de los cambios en el estado de las inspecciones
class myDIEventHandler {
public:
	virtual void OnDICreated(DebuggerInspection *di) {}
	virtual void OnDIError(DebuggerInspection *di) {}
	virtual void OnDIValueChanded(DebuggerInspection *di) {}
	virtual void OnDIOutOfScope(DebuggerInspection *di) {}
	virtual void OnDIInScope(DebuggerInspection *di) {}
	virtual void OnDINewType(DebuggerInspection *di) {}
};

class myDIGlobal {
public:
	virtual void OnDIDebugStart() {}
	virtual void OnDIFullUpdateBegin() {}
	virtual void OnDIFullUpdateEnd() {}
	virtual void OnDIDebugStop() {}
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
	
	/// estructura para encolar acciones que se invoquen cuando el depurador esta ocupado y entonces deben ser ejecutadas mas adelante cuando se detenga
	struct DIPendingAction {
		DebuggerInspection *inspection;
		pending_action action;
		DIPendingAction(DebuggerInspection *_inspection=NULL, pending_action _action=NULL):inspection(_inspection),action(_action) {}
	};
	
	/// lista de acciones en cola para ejecutarse cuando el depurador se detenga
	static SingleList<DIPendingAction> pending_actions;
	
	static void AddPendingAction(DebuggerInspection *inspection, pending_action action) {
		pending_actions.Add(DIPendingAction(inspection,action));
	}
	
	
	/**
	* @brief metodo a usar antes de intentar dialogar con gdb... 
	*
	* Si no se puede dialogar ahora (esta ejecutando), retorna falso y además
	* encola el intento para que se intente nuevamente cuando se pueda (en UpdateAll)
	**/
	bool TryToExec(pending_action action) {
		if (debug->waiting) {
			AddPendingAction(this,action);
			return false;
		} else {
			(this->*action)();
			return true;
		}
	}
		
	static void ProcessPendingCommands() {
		if (!debug->debugging || debug->waiting) { 
			DEBUG_INFO("ERROR: ProcessPendingCommands: !debug->debugging || debug->waiting");
		}
		for(int i=0;i<pending_actions.GetSize();i++) { 
			DIPendingAction &pa=pending_actions[i];
			if (pa.action) (pa.inspection->*pa.action)();
			else delete pa.inspection; // action=NULL signfica que hay que eliminar el objeto
		}
	}
	
	/// para notificar los cambios a los componentes de interfaz que utilizan estas inspecciones
	void GenerateEvent(void (myDIEventHandler::*foo)(DebuggerInspection*)) { 
		if (consumer) (consumer->*foo)(this);
	}
	
	
public:
	
	static void OnDebugStop() {
		ProcessPendingCommands(); // delete pending commands, they shoud not be run in next debugging session
		vo2di_map.clear();
	}
	
	static void OnDebugStart() {
		ProcessPendingCommands(); // commands for creating new variable objects should be here
		// re-create al vo-based inspections on next pause
		for(int i=0;i<all_inspections.GetSize();i++) { 
			DebuggerInspection *di = all_inspections[i];
			if (di->dit_type==DIT_GDB_COMMAND) continue;
			di->is_frameless=true;
			AddPendingAction(di,&DebuggerInspection::CreateVO);
		}
	}
	
	static void OnDebugPause() {
		ProcessPendingCommands();
		UpdateAll();
	}
		
	/// Metodo que actualiza todas las inspecciones (consultando a gdb, se debe invocar desde DebugManager cuando hay una pausa/interrupción en la ejecución)
	static void UpdateAll() {
		
		// struct para guardar los campos que interesan de cada vo actualizada
		struct update { wxString name,value,in_scope,new_type,new_num_children; };
		
		// consulta cuales vo cambiaron
		wxString s = debug->SendCommand("-var-update --all-values *");
		unsigned int l=s.Len();
		for(unsigned int i=6;i<l-4;i++) { // empieza en 6 porque primero dice algo como "^done,...."
			if (s[i]=='n' && s[i+1]=='a' && s[i+2]=='m' && s[i+3]=='e' && s[i+4]=='=') { // por cada "name=" empieza un vo...
				// busca los campos del vo
				update u;
				while(true) {
					int p0=i; // posicion donde empieza el nombre del "campo"
					while (i<l && s[i]!=']' && s[i]!='}' && s[i]!='=') i++;
					int p1=i; // posicion del igual
					if (++i>=l || s[i]!='\"') break; 
					while (++i<l && s[i]!='\"') { if (i=='\\') i++; }
					int p2=i; // posicion de la comilla que cierra
					wxString name=s.Mid(p0,p1-p0);
					if (name=="name") u.name=s.Mid(p1+2,p2-p1-2);
					else if (name=="value") u.value=s.Mid(p1+2,p2-p1-2);
					else if (name=="in_scope") u.in_scope=s.Mid(p1+2,p2-p1-2);
					else if (name=="new_type") u.new_type=s.Mid(p1+2,p2-p1-2);
					else if (name=="new_num_children") u.new_num_children=s.Mid(p1+2,p2-p1-2);
					while (++i<l && s[i]!=',' && s[i]!=']' && s[i]!='}'); 
					if (i==l || s[i]==']' || s[i]=='}') break; else i++;
//					while (++i<l && s[i]==' '); // pareciera no ser necesario
				}
				// busca el DebuggerInspection para el vo (por su nombre gdb)
				map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(u.name);
				if (it==vo2di_map.end()) {
					DEBUG_INFO("ERROR: Inspection::UpdateAll: it==vo2di_map.end()");
					continue;
				}
				// actualiza el estado del DebuggerInspection y notifica a la interfaz mediante consumer
				DebuggerInspection &di=*(it->second);
				if (u.in_scope=="true") {
					di.gdb_value=u.value;
					if (!u.new_num_children.IsEmpty()) { 
						u.new_num_children.ToLong(&di.num_children);
						if (!u.new_type.IsEmpty()) di.value_type=u.new_type; 
						di.GenerateEvent(&myDIEventHandler::OnDINewType);
					} else if (!u.new_type.IsEmpty()) { 
						di.value_type=u.new_type; di.GenerateEvent(&myDIEventHandler::OnDINewType);
					} else if (!di.is_in_scope) { 
						di.is_in_scope=true; di.GenerateEvent(&myDIEventHandler::OnDIInScope);
					} else {
						di.GenerateEvent(&myDIEventHandler::OnDIValueChanded);
					}
				} else if (di.is_in_scope) {
					di.is_in_scope=false;
					di.GenerateEvent(&myDIEventHandler::OnDIOutOfScope);
				}
			}
		}
	}
	
	
private:
	DEBUG_INSPECTION_EXPRESSION_TYPE dit_type; ///< si es una vo, una macro, todavía no se creo, o fallo la creacion
	// inf definida por el usuario/consumidor de la inspeccion
	wxString expression; ///< expresión que está siendo inspeccionada 
	wxString variable_object; ///< si es variable object (ver dit_type) guarda el nombre de la vo, sino el comando gdb que se evalua
	bool is_frameless; ///< si su valor está asociado a un frame/scope particular o no (en gdb se conocen como "floating" variable objects)
	bool is_in_scope; ///< autoexplicativo (solo para vo, los comandos gdb siempre tendran true)
	bool is_frozen; ///< autoexplicativo (solo para vo, los comandos gdb siempre tendran true)
	myDIEventHandler *consumer; ///< puntero a quien utiliza esta inspección, para notificarle los cambios
	// informacion "calculada"
	wxString value_type; ///< tipo de dato c/c++ del resultado de la expresión
	long num_children; ///< si es una inspeccion compuesta, cantidad de "partes" (campos en un struct, elementos en un arreglo, etc)
	wxString gdb_value; ///< valor tal como lo da gdb, sin alterar
	
//	int age; ///< indica cuando fue la última vez que se actualizó esta información
//	int bt_frame; ///< en que nivel del backtrace está actualmente
//	wxString pretty_value; ///< valor en versión "para mostrar" (puede no ser como lo da gdb, por ejemplo, los vo de structs no muestran sus campos)
	
	void VODelete() {
		debug->SendCommand("-var-delete - ",variable_object);
	}
	
	void VOCreate() {
		wxString cmd = "-var-create - ";
		if (is_frameless) cmd<<"@ "; else cmd<<"* ";
		wxString ans = debug->SendCommand(cmd,mxUT::EscapeString(expression,true));
		if (ans.Left(5)!="^done") { dit_type=DIT_ERROR; return; } 
		else dit_type = DIT_VARIABLE_OBJECT;
		variable_object = debug->GetValueFromAns(ans,"name",true);
		value_type = debug->GetValueFromAns(ans,"type",true);
		gdb_value = debug->GetValueFromAns(ans,"value",true);
		debug->GetValueFromAns(ans,"numchild",true).ToLong(&num_children);
		vo2di_map[variable_object] = this;
	}
	
	void VOSetFrozen() {
		debug->SendCommand(wxString("-var-set-frozen ")+variable_object,is_frozen?1:0);
	}
	
	void VOEvaluate() {
		wxString ans = debug->SendCommand(wxString("-var-evaluate-expression "),variable_object);
		gdb_value = debug->GetValueFromAns(ans,"value",true,true);
	}
	
	bool VOAssign(const wxString &new_value) {
		wxString ans = debug->SendCommand(wxString("-var-assign ")<<variable_object<<" "<<mxUT::EscapeString(new_value,true));
		return ans.StartsWith("^done");
	}
	
	
	void CreateVO() {
		VOCreate();
		if (dit_type==DIT_ERROR) GenerateEvent(&myDIEventHandler::OnDIError);
		else { VOEvaluate(); is_in_scope = true; GenerateEvent(&myDIEventHandler::OnDICreated); }
	}
	
	/// las instancias se construyen solo a través de Create
	DebuggerInspection(DEBUG_INSPECTION_EXPRESSION_TYPE type, const wxString &expr, bool frameless, myDIEventHandler *event_handler=NULL) 
		: dit_type(type),expression(expr),is_frameless(frameless), is_frozen(false), consumer(event_handler) { };
	/// las instancias se destruyen a través de Destroy, esto evita que alguien de afuera le quiera hacer delete
	~DebuggerInspection() {}; 
		
	DebuggerInspection(const DebuggerInspection &); ///< esta clase no es copiable
	void operator=(const DebuggerInspection &); ///< esta clase no es copiable
	
public:

	/**
	* @brief Crea una instancia a partir de una expresion
	*
	* @para init_now si es falso, no se inicializa (no se registra en gdb la variable_object).
	*   			 se debe llamar inmediatamente a Init con la instancia... esto es para que
	*                el que la crea pueda conocer el puntero antes de recibir los eventos
	**/ 
	static DebuggerInspection *Create(const wxString &expr, bool frameless, myDIEventHandler *event_handler=NULL, bool init_now=true) {
		bool is_cmd = expr.size()&&expr[0]=='>';
		DebuggerInspection *di = new DebuggerInspection(is_cmd?DIT_GDB_COMMAND:DIT_PENDING,expr,frameless,event_handler);
		all_inspections.Add(di);
		if (init_now) di->Init();
		return di;
	}
	
	void FirstManualEvaluation() {
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
		if (dit_type==DIT_PENDING) { // si es variable-object y estamos en depuracion, crearla en gdb
			return (!debug->debugging || TryToExec(&DebuggerInspection::CreateVO)); 
		} else if (dit_type==DIT_GDB_COMMAND) {
			variable_object = expression.Mid(1);
			return (!debug->debugging || TryToExec(&DebuggerInspection::FirstManualEvaluation));
		}
		return true;
	}
	
	void Destroy() {
		if (dit_type==DIT_VARIABLE_OBJECT) { // si tenia una vo asociada....
			// eliminarla en gdb
			if (debug->debugging) TryToExec(&DebuggerInspection::VODelete);
			// eliminarla del mapa
			map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(variable_object);
			if (it!=vo2di_map.end()) vo2di_map.erase(it); // el if siempre debería dar true
			else { DEBUG_INFO("ERROR: Inspection::Destroy: it==vo2di_map.end()"); }
		}
		all_inspections.Remove(all_inspections.Find(this));
		AddPendingAction(this,NULL);
	}
	
	bool ModifyValue(const wxString &new_value) {
		if (dit_type!=DIT_VARIABLE_OBJECT) return false;
		if (!debug->debugging || debug->waiting) return false;
		return VOAssign(new_value);
	}
	
	void Freeze() {
		is_frozen=true;
		if (dit_type==DIT_VARIABLE_OBJECT && debug->debugging) 
			TryToExec(&DebuggerInspection::VOSetFrozen);
	}
	
	void UnFreeze() {
		is_frozen=false;
		if (dit_type==DIT_VARIABLE_OBJECT && debug->debugging) 
			TryToExec(&DebuggerInspection::VOSetFrozen);
	}
	
	DEBUG_INSPECTION_EXPRESSION_TYPE GetDbiType() { return dit_type; }
	wxString GetExpression() { return expression; }
	wxString GetValue() { return gdb_value; }
	wxString GetValueType() { return value_type; }
	bool IsFrameless() { return is_frameless; }
	bool IsInScope() { return is_in_scope; }
	bool IsFrozen() { return is_frozen; }
	bool RequiresManualUpdate() { return dit_type==DIT_GDB_COMMAND; }
	bool UpdateValue(bool generate_event=true) { // solo para cuando RequiresManualUpdate()==true
		if (!debug->debugging || debug->waiting) return false;
		if (dit_type==DIT_GDB_COMMAND) {
			wxString new_value = debug->GetMacroOutput(variable_object);
			if (new_value!=gdb_value) {
				gdb_value=new_value;
				if (generate_event) GenerateEvent(&myDIEventHandler::OnDIValueChanded);
				return true;
			}
		}
		return false;
	}
	// solo llamar a estas funciones si GetDbiType()==DIT_VARIABLE_OBJECT
	bool IsSimpleType() { return num_children==0; } 
	bool IsClass() { return num_children!=0 && gdb_value.Len() && gdb_value[0]=='{'; }
	bool IsArray() { return num_children!=0 && gdb_value.Len() && gdb_value[0]!='{'; }
	
};

#endif
