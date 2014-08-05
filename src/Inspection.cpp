#include "Inspection.h"

DebuggerInspection::vo2di_type DebuggerInspection::vo2di_map;

SingleList<DebuggerInspection::DIPendingAction> DebuggerInspection::pending_actions;

SingleList<DebuggerInspection*> DebuggerInspection::all_inspections;

int DebuggerInspection::CallLogger::lev=0;

void DebuggerInspection::UpdateAll ( ) {
	__debug_log_static_method__;
	if (!debug->debugging||debug->waiting) return;
	// struct para guardar los campos que interesan de cada vo actualizada
	struct update { wxString name,value,in_scope,new_type,new_num_children; };
	
	// consulta cuales vo cambiaron
	wxString s = debug->SendCommand("-var-update --all-values *");
	for(unsigned int i=6,l=s.Len();i<l-4;i++) { // empieza en 6 porque primero dice algo como "^done,...."
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
			bool new_scope=false,new_type=false, in_scope=u.in_scope=="true";
			if (!u.new_num_children.IsEmpty()) { u.new_num_children.ToLong(&di.num_children); new_type=true; } 
			if (!u.new_type.IsEmpty()) { di.value_type=u.new_type; new_type=true; }
			if (di.is_in_scope!=in_scope) { di.is_in_scope=in_scope; new_scope=true; }
			if (in_scope && (new_type || !di.helper)) di.gdb_value=u.value;
			
			if (di.dit_type==DIT_VARIABLE_OBJECT) {
				if (in_scope) {
					if (new_type||new_scope) di.SetupChildInspection();
					if (new_scope) di.GenerateEvent(&myDIEventHandler::OnDIInScope);
					else if (new_type) di.GenerateEvent(&myDIEventHandler::OnDINewType);
					else di.GenerateEvent(&myDIEventHandler::OnDIValueChanged);
				} else {
					if (new_scope) {
						if (di.helper) di.helper->Destroy();
						di.GenerateEvent(&myDIEventHandler::OnDIOutOfScope);
					}
				}
			} else /*if (di.dit_type==DIT_HELPER_VO)*/ { // la condicion comentada se cumple siempre, no hay otro tipo de inspeccion que use vo mas que estas dos
				if (in_scope && !new_type && !new_scope) { // los eventos de cambio de tipo y/o scope se lanzan en la inspeccion padre
					di.MakeEvaluationExpressionForParent(); // cambia el puntero que usa
					di.parent->UpdateValue();
					di.parent->GenerateEvent(&myDIEventHandler::OnDIOutOfScope);
				}
			}
		}
	}
}

bool DebuggerInspection::Break(SingleList<DebuggerInspection*> &children, bool skip_visibility_groups, bool recursive_on_inheritance, bool set_full_expressions) {
	__debug_log_method__;
	if (!debug->CanTalkToGDB()) return false;
	
	// auxiliares para escribir la expresión final a partir de la expresión base y los nombre de los hijos
	wxString base_exp = set_full_expressions?debug->RewriteExpressionForBreaking(expression):"";
	bool is_array = IsArray(); wxString base_pre = is_array?"[":".", base_post=is_array?"]":"";
	
	// crear y obtener vos hijos
	wxString s = debug->SendCommand("-var-list-children ",variable_object);
	struct child { wxString name,exp,type; long num_children; child():num_children(0){} };
	for(unsigned int i=6,l=s.Len();i<l-4;i++) { // empieza en 6 porque primero dice algo como "^done,...."
		if (s[i]=='n' && s[i+1]=='a' && s[i+2]=='m' && s[i+3]=='e' && s[i+4]=='=') { // por cada "child=" empieza un hijo...
			// busca los campos del hijo
			child c;
			while(true) {
				int p0=i; // posicion donde empieza el nombre del "campo"
				while (i<l && s[i]!=']' && s[i]!='}' && s[i]!='=') i++;
				int p1=i; // posicion del igual
				if (++i>=l || s[i]!='\"') break; 
				while (++i<l && s[i]!='\"') { if (s[i]=='\\') i++; }
				int p2=i++; // posicion de la comilla que cierra
				wxString name=s.Mid(p0,p1-p0);
				if (name=="name") c.name=s.Mid(p1+2,p2-p1-2);
				else if (name=="exp") c.exp=s.Mid(p1+2,p2-p1-2);
				else if (name=="type") c.type=s.Mid(p1+2,p2-p1-2);
				else if (name=="numchild") s.Mid(p1+2,p2-p1-2).ToLong(&c.num_children);
				while (i<l && s[i]!=',' && s[i]!=']' && s[i]!='}') 
					i++;
				if (i==l || s[i-1]==']' || s[i]=='}') break; else i++;
			}
			// por cada hijo, crea una nueva inspección dependiente
			DebuggerInspection *di = new DebuggerInspection(this,c.name,c.exp,c.type,c.num_children);
			di_children++;
			all_inspections.Add(di);
			vo2di_map[c.name] = di;
			// ver si hay que romper este tambien (porque es clase baso, o grupo de visibilidad)
			bool break_again = (recursive_on_inheritance && c.type==c.exp) ||
				(skip_visibility_groups && ((c.exp=="public"||c.exp=="private"||c.exp=="protected")&&c.type==""));
			if (break_again) di->expression=base_exp; else di->expression=base_exp+base_pre+di->expression+base_post;
			if (break_again) {
				di->Break(children,skip_visibility_groups,recursive_on_inheritance,set_full_expressions);
				di->Destroy();
			} else { 
				if (!di->SetupChildInspection()) di->VOEvaluate();
				children.Add(di);
			}
		}
	}
	return true;
}


void DebuggerInspection::OnDebugStop() {
	__debug_log_static_method__;
	// delete helpers and ghost inspections by unlinking its children
	for(int i=0;i<all_inspections.GetSize();i++) {
		DebuggerInspection *di = all_inspections[i];
		if (di->dit_type==DIT_VARIABLE_OBJECT) {
			if (di->parent) di->RemoveParentLink();
			if (di->helper) di->helper->Destroy();
		}
	}
	ProcessPendingActions(); // delete pending commands, they shoud not be run in next debugging session
	vo2di_map.clear();
}

void DebuggerInspection::OnDebugStart() {
	__debug_log_static_method__;
	ProcessPendingActions(); // commands for creating new variable objects should be here
	// re-create al vo-based inspections on next pause
	for(int i=0;i<all_inspections.GetSize();i++) { 
		if (all_inspections[i]->dit_type!=DIT_GDB_COMMAND) {
			all_inspections[i]->is_frameless=true;
			AddPendingAction(all_inspections[i],&DebuggerInspection::CreateVO,true,true);
		}
	}
}

void DebuggerInspection::RecreateAllFramelessInspections() {
	__debug_log_static_method__;
	for(int i=0;i<all_inspections.GetSize();i++) {
		DebuggerInspection *di = all_inspections[i];
		if (!di->is_frameless || di->dit_type==DIT_GDB_COMMAND || di->dit_type==DIT_HELPER_VO) continue;
		if (di->parent) di->RemoveParentLink();
		if (di->variable_object.Len()) AddPendingAction(di,&DebuggerInspection::VODelete,true,true);
		AddPendingAction(di,&DebuggerInspection::CreateVO,true,true);
	}
}

void DebuggerInspection::OnDebugPause() {
	__debug_log_static_method__;
	ProcessPendingActions();
	UpdateAll();
}
