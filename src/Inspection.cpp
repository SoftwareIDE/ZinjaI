#include "Inspection.h"
#include "ConfigManager.h"
#include "Language.h"

DebuggerInspection::vo2di_type DebuggerInspection::vo2di_map;

SingleList<DebuggerInspection::DIPendingAction> DebuggerInspection::pending_actions;

SingleList<DebuggerInspection*> DebuggerInspection::all_inspections;

SingleList<myDIGlobalEventHandler*> DebuggerInspection::global_consumers;

#ifdef _INSPECTION_LOG
	int DebuggerInspection::CallLogger::lev=0;
	wxFFile inspection_log_file(_INSPECTION_LOG,"w+");
#endif

static wxString &RemoveEscapeChar(wxString &s) {
	int i=0, l=s.Len();
	while (i<l) {
		if (s[i]=='\\') { s.erase(i,1); l--; } 
		i++;
	}
	return s;
}

void DebuggerInspection::UpdateAllVO() {
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
				while (++i<l && s[i]!='\"') { if (s[i]=='\\') i++; }
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
				DEBUG_INFO("ERROR: Inspection::UpdateAllVO: it==vo2di_map.end()");
				continue;
			}
			// actualiza el estado del DebuggerInspection y notifica a la interfaz mediante consumer
			DebuggerInspection &di=*(it->second);
			bool new_scope=false,new_type=false, in_scope=u.in_scope=="true";
			if (!u.new_num_children.IsEmpty()) { u.new_num_children.ToLong(&di.num_children); new_type=true; } 
			if (!u.new_type.IsEmpty()) { di.value_type=u.new_type; new_type=true; }
			if (di.flags.Get(DIF_IN_SCOPE)!=in_scope) { di.flags.Set(DIF_IN_SCOPE,in_scope); new_scope=true; }
			if (in_scope && (new_type || !di.helper)) di.gdb_value=RemoveEscapeChar(u.value);
			
			if (di.dit_type==DIT_VARIABLE_OBJECT) {
				if (in_scope) {
					if (new_type||new_scope) di.SetupChildInspection();
					if (new_scope) di.GenerateEvent(&myDIEventHandler::OnDIInScope);
					else if (new_type) di.GenerateEvent(&myDIEventHandler::OnDINewType);
					else di.GenerateEvent(&myDIEventHandler::OnDIValueChanged);
				} else {
					if (new_scope) {
						if (di.helper) di.DeleteHelper();
						di.GenerateEvent(&myDIEventHandler::OnDIOutOfScope);
					}
				}
//			} else /*if (di.dit_type==DIT_HELPER_VO)*/ { // la condicion comentada se cumple siempre, no hay otro tipo de inspeccion que use vo mas que estas dos
//				if (in_scope && !new_type && !new_scope) { // los eventos de cambio de tipo y/o scope se lanzan en la inspeccion padre
//					di.MakeEvaluationExpressionForParent(); // cambia el puntero que usa
//					di.parent->UpdateValue();
//					di.parent->GenerateEvent(&myDIEventHandler::OnDIOutOfScope);
//				}
			}
		}
	}
}

bool DebuggerInspection::Break(SingleList<DebuggerInspection*> &children, bool skip_visibility_groups, bool recursive_on_inheritance, bool set_full_expressions) {
	__debug_log_method__;
	if (!debug->CanTalkToGDB()) return false;
	
	// auxiliares para escribir la expresión final a partir de la expresión base y los nombre de los hijos
	wxString base_exp = set_full_expressions?RewriteExpressionForBreaking(expression):"";
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
			if (di->helper) di->DeleteHelper();
		}
	}
	ProcessPendingActions(); // delete pending commands, they shoud not be run in next debugging session
	vo2di_map.clear();
	for(int i=0;i<global_consumers.GetSize();i++) global_consumers[i]->OnDebugStop();
}

void DebuggerInspection::OnDebugStart() {
	__debug_log_static_method__;
	ProcessPendingActions(); // commands for creating new variable objects should be here
	// re-create al vo-based inspections on next pause
	for(int i=0;i<all_inspections.GetSize();i++) { 
		if (all_inspections[i]->dit_type!=DIT_GDB_COMMAND) {
			all_inspections[i]->flags.Set(DIF_FRAMELESS);
			AddPendingAction(all_inspections[i],&DebuggerInspection::CreateVO,true,true);
		}
	}
	for(int i=0;i<global_consumers.GetSize();i++) global_consumers[i]->OnDebugStart();
}

void DebuggerInspection::RecreateAllFramelessInspections(const wxString &expression) {
	__debug_log_static_method__;
	for(int i=0;i<all_inspections.GetSize();i++) {
		DebuggerInspection *di = all_inspections[i];
		if (!di->IsFrameless() || di->dit_type==DIT_GDB_COMMAND || di->expression!=expression) continue;
		if (di->parent) di->RemoveParentLink();
		if (di->variable_object.Len()) AddPendingAction(di,&DebuggerInspection::VODelete,true,true);
		AddPendingAction(di,&DebuggerInspection::CreateVO,true,true);
	}
}

void DebuggerInspection::OnDebugPause() {
	__debug_log_static_method__;
	ProcessPendingActions();
	for(int i=0;i<global_consumers.GetSize();i++) global_consumers[i]->OnDebugPausePre();
	UpdateAllManual();
	UpdateAllVO();
	for(int i=0;i<global_consumers.GetSize();i++) global_consumers[i]->OnDebugPausePost();
}


void myCompoundHelperDIEH::OnDIValueChanged (DebuggerInspection * di) {
	di->MakeEvaluationExpressionForParent(helper_parent);
	helper_parent->UpdateValue(true);
}

void myUserHelperDIEH::OnDIValueChanged (DebuggerInspection * di) {
	helper_parent->gdb_value=di->gdb_value;
	/*if (!helper_parent->is_frozen) */helper_parent->GenerateEvent(&myDIEventHandler::OnDIValueChanged);	
}

/**
* @brief intenta hacer el match entre el tipo de una inspección y un patrón configurado en la lista de mejoras automáticas
*
* Ejemplo: si pattern="std::list<${TYPE}, std::allocator<${TYPE}> >", type="std::list<int, std::alocator<int> >"
* new_expr=">plist ${EXP} ${TYPE}" y expr="lista1", la funcion modificara new_expr a ">plist lista1 int" 
* y retornara true.
*
* @param pattern    el patron de tipo con el que se intenta matchear la inspeccion real
* @param type       el tipo de la inspeccion real, como es arrojado por gdb
* @param new_expre  el patron de expresion mejorada, argumento de entrada/salida
* @param expr       la expresion real original
* @retval 
**/
bool DebuggerInspection::TryToImproveExpression (const wxString &pattern, wxString type, wxString &new_expr, const wxString &expr) {
	if (type.StartsWith("const ")) type=type.Mid(6);
	if (type.EndsWith(" &")) type.RemoveLast().RemoveLast();
	if (!pattern.Contains("${")) {
		if (pattern==type) {
			new_expr.Replace("${EXP}",expr,true);
			return true;
		}
	} else {
		map<wxString,wxString> the_map;
		int lp=pattern.Len(), ip=0;
		int lt=type.Len(), it=0;
		while (ip<lp && it<lt) {
			if (pattern[ip]=='$' && pattern[ip+1]=='{') {
				// buscar donde termina la variable en pattern
				int pos_ip0=ip; ip+=2;
				while (ip<lp && pattern[ip]!='}') ip++;
				if (ip==lp) return false;
				wxString vname = pattern.Mid(pos_ip0,ip-pos_ip0+1);
				// ver que sigue en pattern para buscarlo en type
				int ip0=++ip;
				while (ip<lp && (pattern[ip]!='$'||pattern[ip+1]!='{')) ip++;
				wxString subpat = pattern.Mid(ip0,ip-ip0);
				int lev=0, lsp=subpat.Len(), it0=it++;
				while (it<lt && (lev!=0 || type.Mid(it,lsp)!=subpat)) {
					if (type[it]=='{' || type[it]=='<' || type[it]=='(') lev++;
					else if (type[it]=='}' || type[it]=='>' || type[it]==')') lev--;
					it++;
				}
				if ((lev!=0 || type.Mid(it,lsp)!=subpat)) return false;
				// guardar el match, y dejar los dos contadores listos para el proximo
				wxString vvalue = type.Mid(it0,it-it0);
				the_map[vname] = vvalue;
				it+=lsp;
			} else 
				if (pattern[ip++]!=type[it++]) return false;
		}
		if (ip==lp&&it==lt) {
			map<wxString,wxString>::iterator it1=the_map.begin(), it2=the_map.end();
			while(it1!=it2) {
				new_expr.Replace(it1->first,it1->second,true);
				it1++;
			}
			new_expr.Replace("${EXP}",expr,true);
			return true;
		}
	}
	return false;
}

bool DebuggerInspection::TryToImproveExpression (wxString type, wxString &new_expr, const wxString &expr) {
	wxArrayString &from=config->Debug.inspection_improving_template_from;
	if (!config->Debug.improve_inspections_by_type) return false;
	if (type.EndsWith(" &")) { type.RemoveLast(); type.RemoveLast(); }
	if (type.StartsWith("const ")) { type=type.Mid(6); }
	for(unsigned int i=0, n=from.GetCount(); i<n; i++) {
		new_expr = config->Debug.inspection_improving_template_to[i];
		if (TryToImproveExpression(from[i],type,new_expr,expr))
			return true;
	}
	return false;
}

wxString DebuggerInspection::GetUserStatusText (DEBUG_INSPECTION_MESSAGE type) {
	switch(type) {
	case DIMSG_PENDING:
		return LANG(INSPECTION_MSG_PENDING,"<<< evaluación pendiente >>>");
	case DIMSG_OUT_OF_SCOPE:
		return LANG(INSPECTION_MSG_PENDING_OUT_OF_SCOPE,"<<< fuera de ámbito >>>");
	case DIMSG_ERROR:
		return LANG(INSPECTION_MSG_ERROR,"<<< error >>>");
	}
	return "";
}

myDIGlobalEventHandler::myDIGlobalEventHandler ( ) { 
	DebuggerInspection::global_consumers.Add(this); 
	registered=true;
}

myDIGlobalEventHandler::~myDIGlobalEventHandler ( ) { 
	UnRegister();
}

void myDIGlobalEventHandler::UnRegister ( ) {
	if (!registered) return;
	int pos = DebuggerInspection::global_consumers.Find(this); 
	if (pos!=DebuggerInspection::global_consumers.NotFound())
		DebuggerInspection::global_consumers.Remove(pos);
	registered=false;
}

void DebuggerInspection::UpdateAllManual ( ) {
	__debug_log_static_method__;
	for(int i=0;i<all_inspections.GetSize();i++) { 
		if (all_inspections[i]->RequiresManualUpdate())
			all_inspections[i]->UpdateValue(true);
	}
}

wxString DebuggerInspection::RewriteExpressionForBreaking(wxString main_expr) {
	int i=0,l=main_expr.Len();
	bool comillas=false;
	int parentesis=0; bool first_level0_parentesis=true;
	while (i<l) { 	// agregar parentesis si la expresion no es simple
		char c=main_expr[i];
		if (c=='\'') { 
			if (main_expr[++i]=='\\') i++;
		} else if (c=='\"') {
			comillas=!comillas;
		} else if (!comillas) {
			if (first_level0_parentesis && c=='(') {
				first_level0_parentesis=false;
				parentesis++;
			} else if (parentesis) {
				if (c==')') parentesis--;
			} else {
				if (c=='[') {
					int l=1; i++;
					while (l) {
						c=main_expr[i];
						if (c=='\'') { if (main_expr[++i]=='\\') i++; }
						else if (c=='\"') comillas=!comillas;
						else if (c=='[') l++;
						else if (c==']') l--;
						i++;
					}
				} else if (c=='@') {
					break;
				} else if ( ! ( (c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c>='0'&&c<='9') || c=='_' || c=='.') ) {
					main_expr.Prepend(wxChar('('));
					main_expr.Append(wxChar(')'));
					break;
				}
			}
		}
		i++;
	}
	return main_expr;
}
