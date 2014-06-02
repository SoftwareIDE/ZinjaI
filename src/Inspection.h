#ifndef INSPECTION_H
#include <map>
#include <wx/string.h>
#include "DebugManager.h"
#include "mxUtils.h"
#include "SingleList.h"
using namespace std;

////! Información acerca de una inspección en el depurador

///< Tipo/estado de una inspeccion
enum DEBUG_INSPECTION_EXPRESSION_TYPE {
	DIET_VARIABLE_OBJECT, ///< la expresion tienen un variable object asociado
	DIET_GDB_COMMAND, ///< la expresion es en realidad una macro o comando para gdb
	DIET_PENDING, ///< la ultima operacion sobre la inspeccion no se ejecutó porque el depurador estaba ocupado, por lo que quedó encolada para cuando se detenga
	DIET_ERROR
};

///< Accion sobre una inspeccion cuyo compando para gdb no pudo ejecutarse cuando fue requerida porque el depurador no esta ejecutandos o pausado, y fue encolada para ejecurse luego
enum DEBUG_INSPECTION_PENDING_ACTION {
	DIPA_NULL,
	DIPA_CREATE,
	DIPA_DELETE,
	DIPA_EDIT_EXPRESSION
};


/**
* @brief Clase para gestionar todo lo relacionado a las inspecciones a nivel zinjai-gdb (no interfaz de usuario)
*
* Esta clase represanta en sus atributos regulares una inspeccion, que puede ser "consumida"
* por cualquier componente de la interfaz. La inspeccion encapsula la comunicacion con el depurador
* y la gestion de su estado interno en zinjai, para que las clases que la usan solo se preocupen
* de como mostrarla. Además tiene métodos staticos para gestionar el conjunto completo de inspecciones
* de forma de poner aca toda la comunicacion con gdb que venga que ver con inspecciones y que
* DebugManager solo llama un par de métodos y se desligue del tema.
**/
struct DebuggerInspection {
	
private:
	/// estructura para encolar acciones que se invoquen cuando el depurador esta ocupado y entonces deben ser ejecutadas mas adelante cuando se detenga
	struct DIPendingAction {
		DEBUG_INSPECTION_PENDING_ACTION action;
		DebuggerInspection *inspection;
		DIPendingAction(DEBUG_INSPECTION_PENDING_ACTION _action=DIPA_NULL, DebuggerInspection *_inspection=NULL):action(_action),inspection(_inspection) {}
	};
	
	/// lista de acciones en cola para ejecutarse cuando el depurador se detenga
	static SingleList<DIPendingAction> pending_actions;
	
	/// asocia los variable_objects (por nombre, como lo da gdb) a las instancias de esta clase que los representan internamente en ZinjaI
	static map<wxString,DebuggerInspection*> vo2di_map;
	
	/// busca una instancia de esta clase DebuggerInspection a partir del nombre de su variable-object en gdb
	static DebuggerInspection *GetFromVO(const wxString &variable_object) {
		map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(variable_object);
		if (it!=vo2di_map.end()) return it->second; else return NULL;
	}
	
	/**
	* @brief metodo a usar antes de intentar dialogar con gdb... 
	*
	* Si no se puede dialogar ahora (esta ejecutando), retorna falso y ademas 
	* encola el intento para que se intente nuevamente cuando se pueda (en UpdateAll)
	**/
	bool CanSendCommand(DEBUG_INSPECTION_PENDING_ACTION action) {
		if (debug->waiting) {
			pending_actions.Add(DIPendingAction(action,this));
			return false; // si no se esta depurando o si esta ocupado (deberia ser ejecutando), no se puede hacer, encolar el mensaje
		} else {
			return true;
		}
	}
		
	static void ProcessPendingCommands() {
		for(int i=0;i<pending_actions.GetSize();i++) { 
			DIPendingAction &pa=pending_actions[i];
			switch(pa.action) {
			case DIPA_CREATE:
				/// @todo: completar accion
				break;
			case DIPA_DELETE:
				/// @todo: completar accion
				break;
			case DIPA_EDIT_EXPRESSION:
				/// @todo: completar accion
				break;
			case DIPA_NULL: // no deberia llegar, es solo para que el analisis estatico no se queje que faltan opciones por considerar
				break;
			}
		}
	}
	
public:
//	static int date; ///< instante de tiempo actual en la depuración (cambia con cada paso en el depurador, para saber cuando fue la ultima vez que cambió una inspección)
	
	/// Metodo que actualiza todas las inspecciones (consultando a gdb, se debe invocar desde DebugManager cuando hay una pausa/interrupción en la ejecución)
	static void UpdateAll() {
		
		ProcessPendingCommands();
		
		struct update {
			wxString name,value,in_scope,new_type,new_num_children;
		};
		
		wxString s = debug->SendCommand("-var-update --all-values *");
		unsigned int l=s.Len();
		for(unsigned int i=6;i<l-4;i++) { // empieza en 6 porque primero dice algo como "^done,...."
			if (s[i]=='n' && s[i+1]=='a' && s[i+2]=='m' && s[i+3]=='e' && s[i+4]=='=') {
				update u;
				while(true) {
					int p0=i; // posicion donde empieza el nombre del "campo"
					while (i<l && s[i]!=']' && s[i]!='}' && s[i]!='=') i++;
					int p1=i; // posicion del igual
					if (++i>=l || s[i]!='\"') break; 
					while (i<l && s[i]!='\"') { if (i=='\\') i++; i++; }
					if (i==l) break; 
					int p2=i; // posicion de la comilla que cierra
					wxString name=s.Mid(p0,p1-p0);
					if (name=="name") u.name=s.Mid(p1+2,p2-p1-2);
					else if (name=="value") u.value=s.Mid(p1+2,p2-p1-2);
					else if (name=="in_scope") u.in_scope=s.Mid(p1+2,p2-p1-2);
					else if (name=="new_type") u.new_type=s.Mid(p1+2,p2-p1-2);
					else if (name=="new_num_children") u.new_num_children=s.Mid(p1+2,p2-p1-2);
				}
				map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(u.name);
				if (it==vo2di_map.end()) {
					DEBUG_INFO("Inspection::UpdateAll vo name not found in vo2di_map");
					continue;
				}
//				DebuggerInspection &di=*(it->second);
				/// @todo: procesar u
			}
		}
		
//		while (p!=wxNOT_FOUND) {
//			int p2 = p+6;
//			while (ans[p2]!='\"')
//				p2++;
//			wxString name = ans.SubString(p+6,p2-1);
//			ans.Remove(0,p2);
//			p = ans.Find("in_scope=");
//			if (p!=wxNOT_FOUND && ans[p+10]=='f') {
//				for (unsigned int i=0;i<inspections.size();i++)
//					if (inspections[i].on_scope && inspections[i].name==name && inspections[i].is_vo) {
//						if (!inspections[i].freezed) {
//							inspection_grid->SetCellValue(i,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<<Fuera de Ambito>>>"));
//							inspection_grid->SetCellRenderer(i,IG_COL_VALUE,new wxGridCellStringRenderer());
//							if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
//						}
//						inspection_grid->SetCellValue(i,IG_COL_LEVEL,"");
//						inspection_grid->SetReadOnly(i,IG_COL_VALUE,true);
//						inspection_grid->SetReadOnly(i,IG_COL_FORMAT,true);
//						inspection_grid->SetReadOnly(i,IG_COL_WATCH,true);
//						inspections[i].on_scope=false;
//						if (inspections[i].is_vo) SendCommand("-var-delete ",inspections[i].name);
//						break;
//					}
//			} else {
//				for (unsigned int i=0;i<inspections.size();i++) {
//					if (!inspections[i].freezed && inspections[i].on_scope && inspections[i].is_vo && inspections[i].name==name) {
////					bool do_inspect=false; // ver si esta el scope en el backtrace y seleccionar el frame
////					if (inspections[i].frame!=my_frame) {
////						for (int j=0;j<last_backtrace_size;j++)
////							if (frames_addrs[j]==inspections[i].frame) {
////								do_inspect=true;
////								my_frame=frames_addrs[j];
////								inspection_grid->SetCellValue(i,IG_COL_LEVEL,frames_nums[j]);
////								break;
////							}
////					} else 
////						do_inspect=true;
////					if (!do_inspect) 
////						inspection_grid->SetCellValue(i,IG_COL_LEVEL,"Error");
////					else {
//						if (inspections[i].frame_num[0]!='E') {
//							inspection_grid->SetCellValue(i,IG_COL_VALUE,
//								GetValueFromAns( SendCommand("-var-evaluate-expression ", name) , "value" ,true,true ) );
//							if (config->Debug.select_modified_inspections)
//								inspection_grid->HightlightChange(i);
//						}
//						break;
//					}
//				}
//			}
//			p = ans.Find("name=");
//		}
		
		
	}
	
private:
	DEBUG_INSPECTION_EXPRESSION_TYPE DIET_type; /// si es una vo, una macro, o incorrecto
	wxString expression; ///< expresión que está siendo inspeccionada
	wxString variable_object; ///< si es variable object (der DIET_type) guarda el nombre de la vo; si es macro/comando gdb guarda el comando
	bool is_frameless; ///< si su valor está asociado a un frame/scope particular o no (en gdb se conocen como "floating" variable objects)
	// informacion "calculada"
	wxString value_type; ///< tipo de dato c/c++ del resultado de la expresión
	long num_childs; ///< si es una inspeccion compuesta, cantidad de "partes" (campos en un struct, elementos en un arreglo, etc)
	
//	int age; ///< indica cuando fue la última vez que se actualizó esta información
//	int bt_frame; ///< en que nivel del backtrace está actualmente
//	wxString pretty_value; ///< valor en versión "para mostrar" (puede no ser como lo da gdb, por ejemplo, los vo de structs no muestran sus campos)
	wxString gdb_value;///< valor tal como lo da gdb, sin alterar
//	bool on_scope; ///< indica si la expresión existe en el scope actual
//	DEBUG_INSPECTION_TYPE itype; ///< tipo de inspección, ver DEBUG_INSPECTION_TYPE
	
	void DeleteVO() {
		if (DIET_type==DIET_VARIABLE_OBJECT) { // si era vo, eliminarla del mapa
			map<wxString,DebuggerInspection*>::iterator it=vo2di_map.find(variable_object);
			if (it!=vo2di_map.end()) vo2di_map.erase(it); // el if siempre debería dar verdadero
		}
	}
	
	void SetCommand(const wxString &expr, bool framless=false) {
		DIET_type=DIET_GDB_COMMAND; variable_object=expr.Mid(1);
	}
	
	void SetVO(const wxString &expr, bool frameless=false) {
		DIET_type=DIET_VARIABLE_OBJECT;
		expression=expr; is_frameless=frameless;
		wxString cmd="-var-create - ";
		if (frameless) cmd<<"@ "; else cmd<<"* ";
		cmd<<utils->EscapeString(expr,true);
		wxString ans=debug->SendCommand(cmd);
		
		if (ans.Left(5)!="^done") { DIET_type=DIET_ERROR; return; }
		variable_object = debug->GetValueFromAns(ans,"name",true);
		value_type = debug->GetValueFromAns(ans,"type",true);
		gdb_value = debug->GetValueFromAns(ans,"value",true);
		debug->GetValueFromAns(ans,"numchild",true).ToLong(&num_childs);
	}
	
public:
	DebuggerInspection(const wxString &expr, bool frameless=false) {
		if (expr.size() && expr[0]=='>') SetCommand(expr,frameless);
		else SetVO(expr,frameless);
	}
	
	DEBUG_INSPECTION_EXPRESSION_TYPE GetDbiType() { return DIET_type; }
	wxString GetExpression() { return expression; }
	wxString GetValue() { return gdb_value; }
	wxString GetValueType() { return value_type; }
//	DEBUG_INSPECTION_TYPE GetInspectionType() { return itype; }
	bool IsFrameless() { return is_frameless; }

	~DebuggerInspection() { 
		for(int i=pending_actions.GetSize()-1;i>=0;i--)
			if (pending_actions[i].inspection==this) pending_actions.Remove(i);
		DeleteVO();
	}
	
};

#endif
