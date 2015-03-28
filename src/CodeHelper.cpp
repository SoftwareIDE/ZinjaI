#include "CodeHelper.h"
#include "mxSource.h"
#include "mxUtils.h"
#include <wx/string.h>
#include <wx/textfile.h>
#include "Parser.h"
#include "parserData.h"
#include <wx/ffile.h>
#include "ProjectManager.h"
#include "Autocoder.h"
#include <map>
#include <wx/choicdlg.h>
#include "mxMainWindow.h"
#include "mxMessageDialog.h"
#include "mxCalltip.h"
using namespace std;

MyAutocompList autocomp_list;

// comparacion con distancia de Levenshtein
static bool ShouldAddToAutocompFuzzy(const wxString &typed, int len, const wxString &candidate) {
	return mxUT::Levenshtein(typed.mb_str(),len,candidate.mb_str(),len) <= 1+len/5;
}

// comparacion con normal letra a letra no case sensitive, solo el comienzo
static bool ShouldAddToAutocompStart(const wxString &typed, int len, const wxString &candidate) {
	int l2=candidate.Len(); 
	if (l2<len) return false;
	for(int i=0; i<len; i++) 
		if (tolower(candidate[i])!=typed[i]) 
			return false;
	return true;
}

 // comparacion con normal letra a letra no case sensitive, cualquier parte
static bool ShouldAddToAutocompFind(const wxString &typed, int len, const wxString &candidate) {
	return len==0 || candidate.Lower().Contains(typed);
}

static bool (*ShouldAddToAutocomp)(const wxString &typed, int len, const wxString &candidate);


CodeHelper *code_helper = nullptr;

CodeHelper::CodeHelper(int ml, int mode) {
	
	SetAutocompletionMatchingMode(mode);
	
	min_len=ml;
	reserved_words.icon="$15";
	preproc_directives.icon="$16";
	doxygen_directives.icon="$17";

}

CodeHelper::~CodeHelper() {

}

bool CodeHelper::AutocompletePreprocesorDirective(mxSource *source, wxString typed) {
	return AutocompleteFromArray(source,preproc_directives,typed);
}

bool CodeHelper::AutocompleteDoxygen(mxSource *source, wxString typed) {
	return AutocompleteFromArray(source,doxygen_directives,typed);
}

bool CodeHelper::AutocompleteFromArray(mxSource *source, CodeHelperSpecialArray &words, wxString typed) {
	if (!words.keywords.GetCount()) return false;
	
	autocomp_list.Init();
	
	if (!typed.Len()) {
		for (unsigned int i=1;i<words.keywords.GetCount();i++)
			autocomp_list.Add(words.keywords[i],words.icon,words.help);
		source->ShowAutoComp(0,autocomp_list.GetResult());
		return true;
	}
	
	unsigned int l=typed.Len();
	typed.MakeLower();

	int j, ll = words.keywords.GetCount();
	for (j=0;j<ll;j++) {
		unsigned int i=0;
		while (i<l && (tolower(words.keywords[j][i])==typed[i]))
			i++;
		if (i==l) autocomp_list.Add(words.keywords[j],words.icon,words.help);
	}
	
	if (autocomp_list.Empty()) return false;
	source->ShowAutoComp(l,autocomp_list.GetResult(false));
	return true;	
}


// key es en realidad scope??
bool CodeHelper::AutocompleteScope(mxSource *source, wxString &key, wxString typed, bool consider_inherit, bool add_reserved_words/*, int max_str_dist*/) {
	UnTemplate(key);
	unsigned int len=typed.Len();
	typed.MakeLower();
	HashStringParserClass::iterator it=parser->h_classes.find(key);
	if (it!=parser->h_classes.end()) {
		autocomp_list.Init();
		pd_var *aux_var = it->second->first_attrib->next;
		while (aux_var) {
			if (!len||ShouldAddToAutocomp(typed,len,aux_var->name)) {
				if (aux_var->properties&PD_CONST_PRIVATE)
					autocomp_list.Add(aux_var->name,"$6",aux_var->full_proto);
				else if (aux_var->properties&PD_CONST_PROTECTED)
					autocomp_list.Add(aux_var->name,"$7",aux_var->full_proto);
				else if (aux_var->properties&PD_CONST_PUBLIC)
					autocomp_list.Add(aux_var->name,"$8",aux_var->full_proto);
				else
					autocomp_list.Add(aux_var->name,"$5",aux_var->full_proto);
			}
			aux_var = aux_var->next;
		}
		pd_func *aux_func = it->second->first_method->next;
		while (aux_func) {
			if (!len||ShouldAddToAutocomp(typed,len,aux_func->name)) {
				if (aux_func->properties&PD_CONST_PRIVATE)
					autocomp_list.Add(aux_func->name,"$10",aux_func->full_proto);
				else if (aux_func->properties&PD_CONST_PROTECTED)
					autocomp_list.Add(aux_func->name,"$11",aux_func->full_proto);
				else if (aux_func->properties&PD_CONST_PUBLIC)
					autocomp_list.Add(aux_func->name,"$12",aux_func->full_proto);
				else
					autocomp_list.Add(aux_func->name,"$9",aux_func->full_proto);
			}
			aux_func = aux_func->next;
		}
		
		if (consider_inherit) {
			
			// armar la lista de todos los ancestros
			wxArrayString fathers;
			pd_inherit *item=parser->first_inherit->next;
			int fcount=0,flen;
			while (item) {
				if (item->son==key)
					fathers.Add(item->father);
				item=item->next;
			}
			while ((flen=fathers.GetCount())>fcount) {
				item=parser->first_inherit->next;
				while (item) {
					for (int i=fcount;i<flen;i++)
						if (item->son==fathers[i]) {
							int j=fathers.GetCount();
							while (--j>=0)
								if (fathers[j]==item->father)
									break;
							if (j<0)
								fathers.Add(item->father);
						}
					item=item->next;
				}
				fcount=flen;
			}
			// agregar los atributos y metodos de los ancestros
			for (int a=0;a<flen;a++) {
				it=parser->h_classes.find(fathers[a]);
				if (it!=parser->h_classes.end()) {
					pd_var *aux_var = it->second->first_attrib->next;
					while (aux_var) {
						if (!len||ShouldAddToAutocomp(typed,len,aux_var->name)) {
							if (aux_var->properties&PD_CONST_PRIVATE)
								autocomp_list.Add(aux_var->name,"$6",aux_var->full_proto);
							else if (aux_var->properties&PD_CONST_PROTECTED)
								autocomp_list.Add(aux_var->name,"$7",aux_var->full_proto);
							else if (aux_var->properties&PD_CONST_PUBLIC)
								autocomp_list.Add(aux_var->name,"$8",aux_var->full_proto);
							else
								autocomp_list.Add(aux_var->name,"$5",aux_var->full_proto);
						}
						aux_var = aux_var->next;
					}
					pd_func *aux_func = it->second->first_method->next;
					while (aux_func) {
						if (!len||ShouldAddToAutocomp(typed,len,aux_func->name)) {
							if (aux_func->properties&PD_CONST_PRIVATE)
								autocomp_list.Add(aux_func->name,"$10",aux_func->full_proto);
							else if (aux_func->properties&PD_CONST_PROTECTED)
								autocomp_list.Add(aux_func->name,"$11",aux_func->full_proto);
							else if (aux_func->properties&PD_CONST_PUBLIC)
								autocomp_list.Add(aux_func->name,"$12",aux_func->full_proto);
							else
								autocomp_list.Add(aux_func->name,"$9",aux_func->full_proto);
						}
						aux_func = aux_func->next;
					}
				}
			}
			
		}
		// agregar las palabras resevadas si corresponde
		if (add_reserved_words) AddReservedWords(typed);
		// mostrar la lista final
		if (autocomp_list.Empty()) return false;
		source->ShowAutoComp(len,autocomp_list.GetResult());
		return true;
	}

	return false;
}

wxString CodeHelper::GetAttribType(wxString &scope, wxString &key, int &dims) {
	HashStringParserClass::iterator it = parser->h_classes.find(scope);
	if (it!=parser->h_classes.end()) {
		pd_var *pd_aux = it->second->first_attrib->next;
		while (pd_aux) {
			if (pd_aux->name==key) {
				dims=0;
				wxString ret=pd_aux->type;
				char c;
				while ((c=ret.Last())=='*' || ret.Last()==']') {
					dims++;
					ret.RemoveLast();
					if (c==']')
						ret.RemoveLast();
				}
				while (ret.Last()=='&' || ret.Last()==' ')
					ret.RemoveLast();
				return UnMacro(ret);
			}
			pd_aux=pd_aux->next;
		}
		
		// buscar en los ancestros
		wxArrayString fathers;
		pd_inherit *item=parser->first_inherit->next;
		while (item) {
			if (item->son==scope) {
				HashStringParserClass::iterator it = parser->h_classes.find(item->father);
				if (it!=parser->h_classes.end()) {
					pd_var *pd_aux = it->second->first_attrib->next;
					while (pd_aux) {
						if (pd_aux->name==key) {
							dims=0;
							wxString ret=pd_aux->type;
							char c;
							while ((c=ret.Last())=='*' || ret.Last()==']') {
								dims++;
								ret.RemoveLast();
								if (c==']')
									ret.RemoveLast();
							}
							while (ret.Last()=='&' || ret.Last()==' ')
								ret.RemoveLast();
							return UnMacro(ret);
						}
						pd_aux=pd_aux->next;
					}
				}
			}				
			item=item->next;
		}
	}
	return "";
}

wxString CodeHelper::GetGlobalType(wxString &key, int &dims) {
	pd_var *pd_aux = parser->first_global->next;
	while (pd_aux) {
		if (pd_aux->name==key) {
			dims=0;
			wxString ret=pd_aux->type;
			char c;
			while ((c=ret.Last())=='*' || ret.Last()==']') {
				dims++;
				ret.RemoveLast();
				if (c==']')
					ret.RemoveLast();
			}
			while (ret.Last()=='&' || ret.Last()==' ')
				ret.RemoveLast();
			return UnMacro(ret);
		}
		pd_aux=pd_aux->next;
	}
	return "";
}


wxString ExtractIdentifierFromDeclaration(const wxString &decl) {
	int i=decl.Len(); 
	i--; if (i<0) return "";
	if (decl[i]==' ') {
		i--; if (i<0) return "";
	}
	while (i>0 && (decl[i]==']'||decl[i]==')') ) {
		int cor=0; char c=decl[--i];
		while (i>0 && (cor||(c!='['&&c!='('))) {
			if (c==']'||c==')') cor++;
			else if (c=='['||c=='(') cor--;
			c=decl[--i];
		}
		if (decl[i]=='('&&decl[i+1]=='*') { // puntero a funcion, o a arreglo
			i+=2; int i0=i; 
			char c=decl[i];
			while ( (c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c>='0'&&c<='9') || c=='_' ) c=decl[++i];
			return decl.Mid(i0,i-i0);
		}
		--i; if (i<0) return "";
		if (decl[i]==' ') {
			i--; if (i<0) return "";
		}
	}
	int i1=i; char c=decl[i]; while ( i>0 && ( (c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c>='0'&&c<='9') || c=='_' ) ) c=decl[--i];
	return decl.Mid(i+1,i1-i);
	
}

bool CodeHelper::AutocompleteGeneral(mxSource *source, wxString scope, wxString typed, wxString *args, int scope_start) {
	UnTemplate(typed); UnTemplate(scope);
	unsigned int len=typed.Len();
	typed.MakeLower();
	autocomp_list.Init();
	pd_var *aux_var = parser->first_global;
	while (aux_var) {
		if (ShouldAddToAutocomp(typed,len,aux_var->name)) {
			autocomp_list.Add(aux_var->name,(aux_var->properties&(PD_CONST_ENUM_CONST|PD_CONST_ENUM)?"$19":"$14"),aux_var->full_proto);
		}
		aux_var = aux_var->next;
	}

	pd_func *aux_func = parser->first_function;
	while (aux_func) {
		if (ShouldAddToAutocomp(typed,len,aux_func->name)) {
			autocomp_list.Add(aux_func->name,"$3",aux_func->full_proto);
		}
		aux_func = aux_func->next;
	}
	
	pd_macro *aux_macro = parser->first_macro;
	while (aux_macro) {
		if (ShouldAddToAutocomp(typed,len,aux_macro->name)) {
			autocomp_list.Add(aux_macro->name,(aux_macro->props&(PD_CONST_ENUM|PD_CONST_ENUM_CONST)?"$19":(aux_macro->props&PD_CONST_TYPEDEF?"$18":"$2")),aux_macro->proto);
		}
		aux_macro = aux_macro->next;
	}
	
	pd_class *aux_class = parser->first_class;
	while (aux_class) {
		if (ShouldAddToAutocomp(typed,len,aux_class->name)) {
			if (aux_class->file) {
				autocomp_list.Add(aux_class->name,"$4",wxString("class ")+aux_class->name);
			} else
				autocomp_list.Add(aux_class->name,"$13",wxString("class ")+aux_class->name);
		}
		aux_class = aux_class->next;
	}
	
	HashStringParserClass::iterator it=parser->h_classes.find(scope);
	if (it!=parser->h_classes.end()) {
		
		pd_var *aux_var = it->second->first_attrib->next;
		while (aux_var) {
			if (ShouldAddToAutocomp(typed,len,aux_var->name)) {
				if (aux_var->properties&PD_CONST_PRIVATE)
					autocomp_list.Add(aux_var->name,"$6",aux_var->full_proto);
				else if (aux_var->properties&PD_CONST_PROTECTED)
					autocomp_list.Add(aux_var->name,"$7",aux_var->full_proto);
				else if (aux_var->properties&PD_CONST_PUBLIC)
					autocomp_list.Add(aux_var->name,"$8",aux_var->full_proto);
				else
					autocomp_list.Add(aux_var->name,"$5",aux_var->full_proto);
			}
			aux_var = aux_var->next;
		}
		pd_func *aux_func = it->second->first_method->next;
		while (aux_func) {
			if (ShouldAddToAutocomp(typed,len,aux_func->name)) {
				if (aux_func->properties&PD_CONST_PRIVATE)
					autocomp_list.Add(aux_func->name,"$10",aux_func->full_proto);
				else if (aux_func->properties&PD_CONST_PROTECTED)
					autocomp_list.Add(aux_func->name,"$11",aux_func->full_proto);
				else if (aux_func->properties&PD_CONST_PUBLIC)
					autocomp_list.Add(aux_func->name,"$12",aux_func->full_proto);
				else
					autocomp_list.Add(aux_func->name,"$9",aux_func->full_proto);
			}
			aux_func = aux_func->next;
		}
		
//		if (consider_inherit) {
			
			// armar la lista de todos los ancestros
			wxArrayString fathers;
			pd_inherit *item=parser->first_inherit->next;
			int fcount=0,flen;
			while (item) {
				if (item->son==scope)
					fathers.Add(item->father);
				item=item->next;
			}
			while ((flen=fathers.GetCount())>fcount) {
				item=parser->first_inherit->next;
				while (item) {
					for (int i=fcount;i<flen;i++)
						if (item->son==fathers[i]) {
							int j=fathers.GetCount();
							while (--j>=0)
								if (fathers[j]==item->father)
									break;
							if (j<0)
								fathers.Add(item->father);
						}
					item=item->next;
				}
				fcount=flen;
			}
			// agregar los atributos y metodos de los ancestros
			for (int a=0;a<flen;a++) {
				it=parser->h_classes.find(fathers[a]);
				if (it!=parser->h_classes.end()) {
					pd_var *aux_var = it->second->first_attrib->next;
					while (aux_var) {
						if (ShouldAddToAutocomp(typed,len,aux_var->name)) {
							if (aux_var->properties&PD_CONST_PRIVATE)
								autocomp_list.Add(aux_var->name,"$6",aux_var->full_proto);
							else if (aux_var->properties&PD_CONST_PROTECTED)
								autocomp_list.Add(aux_var->name,"$7",aux_var->full_proto);
							else if (aux_var->properties&PD_CONST_PUBLIC)
								autocomp_list.Add(aux_var->name,"$8",aux_var->full_proto);
							else
								autocomp_list.Add(aux_var->name,"$5",aux_var->full_proto);
						}
						aux_var = aux_var->next;
					}
					pd_func *aux_func = it->second->first_method->next;
					while (aux_func) {
						if (ShouldAddToAutocomp(typed,len,aux_func->name)) {
							if (aux_func->properties&PD_CONST_PRIVATE)
								autocomp_list.Add(aux_func->name,"$10",aux_func->full_proto);
							else if (aux_func->properties&PD_CONST_PROTECTED)
								autocomp_list.Add(aux_func->name,"$11",aux_func->full_proto);
							else if (aux_func->properties&PD_CONST_PUBLIC)
								autocomp_list.Add(aux_func->name,"$12",aux_func->full_proto);
							else
								autocomp_list.Add(aux_func->name,"$9",aux_func->full_proto);
						}
						aux_func = aux_func->next;
					}
				}
			}
//		}
	}
	
	// agregar las palabras reservadas
	AddReservedWords(typed);
	
	// agregar los argumentos de la función/método actual
	if (args && args->Len()) {
		bool comillas=false, done=false; int parentesis=0;
		wxString &s=*args; int al=s.Len(), la=0; s[al-1]=',';
		for(int i=1;i<al;i++) {
			char c=s[i];
			if (c=='\''||c=='\"') comillas=!comillas;
			else {
				if (c=='('||c=='['||c=='{') parentesis++;
				else if (c==')'||c==']'||c=='}') parentesis--;
				else if (!parentesis) {
					if (c=='='||(c==','&&!done)) { // consideramos identificador lo que esté antes de una coma o de un igual (valores por defecto)... el done es para no considerar ambos en un mismo argumento
						wxString id=ExtractIdentifierFromDeclaration(s.Mid(la+1,i-la-1));
//						unsigned int CH_COMPARE(typed,id,i,l,max_str_dist);
						if (ShouldAddToAutocomp(typed,len,id)) autocomp_list.Add(id,"$20","");
					}
					if (c==',') { done=false; la=i+1; }
				}
			}
		}
	}
	
	// agregar otras cosas que puedan parecer variables locales
	if (scope_start!=-1) code_helper->AutocompleteLocals(source,typed,scope_start);
	
	// mostrar la lista final
	if (autocomp_list.Empty()) return false;
	source->ShowAutoComp(len,autocomp_list.GetResult());
	return true;
}

bool CodeHelper::AutocompleteAutocode(mxSource *source, wxString typed/*, int max_str_dist*/) {
	autocomp_list.Init(); int t=0, len=typed.Len();
	HashStringAutoCode::iterator it = autocoder->list.begin();
	while (it!=autocoder->list.end()) {
//		wxString &aux=it->first;
//		int CH_COMPARE(typed,aux,i,l,max_str_dist);
		if (ShouldAddToAutocomp(typed,len,it->first)) { ++t; autocomp_list.Add(it->first,"",""); }
		++it;
	}
	
	// mostrar la lista final
	if (autocomp_list.Empty()) return false;
	source->ShowAutoComp(len,autocomp_list.GetResult());
	return true;
}

bool CodeHelper::ShowFunctionCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope) {
	wxString result=GetCalltip(scope,key,onlyScope);
	if (result.Len()) source->ShowCallTip(p,p-key.Len(),result);
	return result.Len();
}
	

static void aux_AddToCalltip(int &t, wxString &text, const wxString &full_proto, bool only_type) {
	if (only_type) {
		if (t!=0 && !text.Len()) return; // esto indica que ya hubo problemas con los tipos
		wxString aux=full_proto;
		aux.Replace("static ","",true);
		aux.Replace("const ","",true);
		aux.Replace("constexpr ","",true);
		aux.Replace("virtual ","",true);
		aux.Replace("volatile ","",true);
		wxString type=aux.BeforeFirst(' ');
		aux.Replace(" ","",true);
		while (aux[type.Len()]=='*') { type<<"*"; }
		type.Replace("&","");
		if (t!=0 && aux!=text) type=""; 
		else text=type;
	} else {
		if (t++) text+="\n";
		text+=full_proto;
	}
}

wxString CodeHelper::GetCalltip(wxString scope, wxString key, bool onlyScope, bool only_type) {
	UnTemplate(key); UnTemplate(scope);
	int t=0;
	wxString text;
	
	if (!only_type) { // no usefull for determining return type for autocompletion
		HashStringParserMacro::iterator it = parser->h_macros.find(key);
		if (it!=parser->h_macros.end()) {
			t=1;
			text=it->second->proto;
		}
	}
	
	pd_func *aux_func;
	
	if (!onlyScope || scope.Len()==0) {
		aux_func=parser->first_function->next;
		while (aux_func) {
			if (aux_func->name==key) {
				aux_AddToCalltip(t,text,aux_func->full_proto,only_type);
			}
			aux_func = aux_func->next;
		}
		pd_class *aux_class = parser->first_class->next;
		while (aux_class) {
			if (aux_class->name==key) {
				if (only_type) {
					aux_AddToCalltip(t,text,key+" "+key,only_type);
				} else {
					aux_func=aux_class->first_method->next;
					while (aux_func) {
						if (aux_func->name==key) {
							aux_AddToCalltip(t,text,aux_func->full_proto,only_type);
						}
						aux_func = aux_func->next;
					}
				}
			}
			aux_class = aux_class->next;
		}
	}

	if (scope.Len()!=0) {
		HashStringParserClass::iterator it=parser->h_classes.find(scope);
		if (it!=parser->h_classes.end()) {
			aux_func=it->second->first_method->next;
			while (aux_func) {
				if (aux_func->name==key) {
					aux_AddToCalltip(t,text,aux_func->full_proto,only_type);
				}
				aux_func = aux_func->next;
			}
		}
		
		// buscar en los ancestros
		wxArrayString fathers;
		pd_inherit *item=parser->first_inherit->next;
		int fcount=0,flen;
		while (item) {
			if (item->son==scope)
				fathers.Add(item->father);
			item=item->next;
		}
		while ((flen=fathers.GetCount())>fcount) {
			item=parser->first_inherit->next;
			while (item) {
				for (int i=fcount;i<flen;i++)
					if (item->son==fathers[i]) {
						int j=fathers.GetCount();
						while (--j>=0)
							if (fathers[j]==item->father)
								break;
						if (j<0)
							fathers.Add(item->father);
					}
				item=item->next;
			}
			fcount=flen;
		}
		// agregar los atributos y metodos de los ancestros
		for (int a=0;a<flen;a++) {
			it=parser->h_classes.find(fathers[a]);
			if (it!=parser->h_classes.end()) {
				aux_func=it->second->first_method->next;
				while (aux_func) {
					if (aux_func->name==key) 
						aux_AddToCalltip(t,text,aux_func->full_proto,only_type);
					aux_func = aux_func->next;
				}
			}
		}
	}
	return text;
}

void CodeHelper::ResetStdData() {
	
	actual_indexes.Clear();
	reserved_words.keywords.Clear();
	doxygen_directives.keywords.Clear();
	preproc_directives.keywords.Clear();
	
	pd_inherit *inherit=parser->first_inherit, *aux_inherit;
	while (inherit->next && !inherit->next->prev) {
		aux_inherit = inherit->next;
		inherit->next = aux_inherit->next;
		delete aux_inherit;
	}
	
	pd_file *file=parser->first_file, *aux_file;
	while (file->next && !file->next->prev) {
		aux_file = file->next;
		file->next = aux_file->next;
		HashStringParserFile::iterator it = parser->h_files.find(aux_file->name);
		if (it!=parser->h_files.end())
			parser->h_files.erase(it);
		delete aux_file;
	}
	
	pd_func *func=parser->first_function, *aux_func;
	while (func->next && !func->next->prev) {
		aux_func = func->next;
		func->next = aux_func->next;
		HashStringParserFunction::iterator it = parser->h_functions.find(aux_func->proto);
		if (it!=parser->h_functions.end())
			parser->h_functions.erase(it);
		delete aux_func;
	}

	pd_var *var=parser->first_global, *aux_var;
	while (var->next && !var->next->prev) {
		aux_var = var->next;
		var->next = aux_var->next;
		HashStringParserVariable::iterator it = parser->h_globals.find(aux_var->name);
		if (it!=parser->h_globals.end())
			parser->h_globals.erase(it);
		delete aux_var;
	}
	
	pd_macro *macro=parser->first_macro, *aux_macro;
	while (macro->next && !macro->next->prev) {
		aux_macro = macro->next;
		macro->next = aux_macro->next;
		HashStringParserMacro::iterator it = parser->h_macros.find(aux_macro->name);
		if (it!=parser->h_macros.end())
			parser->h_macros.erase(it);
		delete aux_macro;
	}
	
	pd_class *aclass=parser->first_class, *aux_class;
	while (aclass->next && !aclass->next->prev) {
		aux_class = aclass->next;
		aclass->next = aux_class->next;
		HashStringParserClass::iterator it = parser->h_classes.find(aux_class->name);
		if (it!=parser->h_classes.end())
			parser->h_classes.erase(it);
		
		pd_func *aux_func = aclass->first_method, *tmp_func;
		while (aux_func->next) {
			tmp_func = aux_func;
			aux_func = tmp_func->next;
			delete tmp_func;
		}
		
		pd_var *aux_var = aclass->first_attrib, *tmp_var;
		while (aux_var->next) {
			tmp_var = aux_var;
			aux_var = tmp_var->next;
			delete tmp_var;
		}
	
		delete aux_class;
	}
		
}

bool CodeHelper::LoadData(wxString index) {
	wxString filepath=mxUT::WichOne(index,"autocomp",true);
	if (!filepath.Len()) return false;
	wxTextFile fil(filepath);
	actual_indexes.Add(index);
	fil.Open();
	wxString header;
	unsigned int tabs;
	int pos;
	pd_file *aux_file=nullptr;
	pd_class *aux_class=nullptr;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (!str.Len()) 
			continue;
		while (!str.IsEmpty() && (str.Last()==';' || str.Last()==' '))
			str.RemoveLast();
		tabs=0;
		while (tabs+1<str.Len() && str[tabs]=='\t')
			tabs++;
		if (tabs+1>=str.Len()) 
			continue;
		str=str.Mid(tabs);
		if (str.Len() && str[0]=='\\') {
			reserved_words.keywords.Add(str.Mid(1));
		} else if (str.Len() && str[0]=='@') {
			doxygen_directives.keywords.Add(str.Mid(1));
		} else if (str.Len() && str[0]=='#') {
			preproc_directives.keywords.Add(str.Mid(1));
		} else if (tabs==0) { // es el nombre de la cabecera
			header=str;
			CH_REGISTER_FILE(aux_file, header);
		} else if (tabs==1) {
			if (str.Mid(0,6)==_("class ")) { // es una clase
				for (int i=0, l=str.Len(); i<l; i++) {
					if (str[i]=='<') {
						int i0=i++, templ_lev=1; 
						do {
							if (str[i]=='>') templ_lev--;
							else if (str[i]=='<') templ_lev++;
						} while (++i<l && templ_lev);
						if (templ_lev==0) { str=str.Mid(0,i0)+str.Mid(i); l=str.Len(); }
					}
				}
				str=str.Mid(6);
				if ( (pos=str.Find(' '))==wxNOT_FOUND) { // si no hay herencia
					CH_REGISTER_CLASS(aux_file, aux_class, str);
				} else { // si hay herencia
					wxString classname = str.Mid(0,pos), label, father;
					CH_REGISTER_CLASS(aux_file, aux_class, classname);
					str=str.Mid(pos+1);
					while (true) { // incorporar info de herencia
						label = str.BeforeFirst(' ');
						father = str.AfterFirst(' ');
						if ( (pos=father.Find(' '))==wxNOT_FOUND) {
							if (label==_T("private")) { 
								CH_REGISTER_INHERIT(classname, PD_CONST_PRIVATE, father);
							} else if (label==_T("protected")) {
								CH_REGISTER_INHERIT(classname, PD_CONST_PROTECTED, father);
							} else {
								CH_REGISTER_INHERIT(classname, PD_CONST_PUBLIC, father);
							}
							break;
						} else {
							str=father.Mid(pos+1);
							father=father.Mid(0,pos);
							if (label==_T("private")) { 
								CH_REGISTER_INHERIT(classname, PD_CONST_PRIVATE, father);
							} else if (label==_T("protected")) {
								CH_REGISTER_INHERIT(classname, PD_CONST_PROTECTED, father);
							} else {
								CH_REGISTER_INHERIT(classname, PD_CONST_PUBLIC, father);
							}
						}
					}
				}
			} else if ( (pos=str.Find('('))==wxNOT_FOUND) {
				if ( (pos=str.Find(' '))==wxNOT_FOUND /*|| str.StartsWith("enum ")*/) { // macro o enum
					CH_REGISTER_MACRO(aux_file,str,PD_CONST_MACRO_CONST);
				} else if (str.StartsWith("enum ")) { // macro o enum
					int props=PD_CONST_ENUM;
					wxString type = str.BeforeLast(' ');
					if (type.StartsWith("enum const ")) {
						props=PD_CONST_ENUM_CONST;
						type=type.Mid(11); if (!type.Len()) type="anonymous";
					}
					wxString name = str.AfterLast(' ');
					CH_REGISTER_MACRO_P(aux_file,name,type,props);
				} else { // variable
					wxString name = str.AfterLast(' ');
					wxString type = str.BeforeLast(' ');
					int props=0;
					if (type.StartsWith("const ")) props&=PD_CONST_CONST;
					if (type.Find(' ')!=wxNOT_FOUND) type = type.AfterLast(' ');
					CH_REGISTER_GLOBAL(aux_file,type,name,str,props);
				}
			} else { //  funcion
				pos--;
				while (str[pos]==' ') {
					pos--;
				}
				wxString name=str.Mid(0,pos+1);
				char c;
				while ( ! ((c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9')) )
					pos--;
				while (pos>=0 && ((c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ))
					pos--;
				name=name.Mid(pos+1);
				while (pos>=0 && str[pos]==' ')
					pos--;
				wxString type = str.Mid(0,pos+1);
				while (pos>=0 && (str[pos]==' ' || str[pos]=='*' || str[pos]=='&') )
					pos--;
				while (pos>=0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ))
					pos--;
//				if (pos!=0)
					pos++;
				type=type.Mid(pos);
				if (type.Len()) {
					CH_REGISTER_FUNCTION(aux_file,type,name,str);
				} else {
					CH_REGISTER_MACRO_P(aux_file,name,str,PD_CONST_MACRO_FUNC);
				}
			}
		} else if (tabs==2) { // es un metodo o atributo
			if ( (pos=str.Find('('))==wxNOT_FOUND) { // atributo
				if (str.Find(':')==wxNOT_FOUND) {
					wxString name = str.AfterLast(' ');
					wxString type = str.BeforeLast(' ');
					int props=PD_CONST_PUBLIC;
					if (type=="enum"){
						props=PD_CONST_ENUM;
					} else if (type.StartsWith("enum const ")) {
						props=PD_CONST_ENUM_CONST;
						type=type.Mid(11); if (!type.Len()) type="anonymous";
					}
					char c = type.IsEmpty()?0:type.Last();
					int dims=0;
					while (c=='&' || c=='*' || c==' ') {
						if (c=='*')
							dims++;
						type.RemoveLast();
						c=type.Last();
					}
					if (type.Find(' ')!=wxNOT_FOUND)
						type = type.AfterLast(' ');
					for (int i=0;i<dims;i++)
						type<<'*';
					CH_REGISTER_ATTRIB(aux_file,aux_class,type,name,str,props);
				} else {
					wxString name = str.AfterLast(':');
					wxString type = str.BeforeLast(' ');
					if (type.Find(' ')!=wxNOT_FOUND)
						type = type.AfterLast(' ');
					CH_REGISTER_ATTRIB(aux_file,aux_class,type,name,str,PD_CONST_PUBLIC);
				}
			} else { // metodo
				pos--;
				while (str[pos]==' ') {
					pos--;
				}
				wxString name=str.Mid(0,pos+1);
				char c;
				// nombre
				while ( pos>=0 && ! ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
					pos--;
				while ( pos>=0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
					pos--;
				name=name.Mid(pos+1);
				// scope
				if (pos==-1) { 
					str=aux_class->name+"::"+str; 
				} else {
					if (c!=':') { str=str.Mid(0,pos+1)+aux_class->name+"::"+str.Mid(pos+1); }
					else pos-=2;
				}
				while ( pos>=0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
					pos--;
				while (pos>=0 && str[pos]==' ')
					pos--;
				// tipo
				if (name==aux_class->name) {
					CH_REGISTER_METHOD(aux_file, aux_class, name, name, str);				
				} else {
					wxString type = pos<0?"":str.Mid(0,pos+1);
					while (pos>=0 && (str[pos]==' ' || str[pos]=='*' || str[pos]=='&'))
						pos--;
					while (pos>=0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
						pos--;
					if (pos>0) { pos++; type=type.Mid(pos); }
					CH_REGISTER_METHOD(aux_file, aux_class, type, name, str);				
				}
			}
		}
	}
	
	mxUT::SortArrayString(preproc_directives.keywords);
	mxUT::SortArrayString(doxygen_directives.keywords);
	
	parser->last_file=parser->first_file;
	if (parser->last_file->next) {
		while (parser->last_file->next && !parser->last_file->next->prev) {
			parser->last_file = parser->last_file->next;
		}
	}

	parser->last_class=parser->first_class;
	if (parser->last_class->next) {
		while (parser->last_class->next && !parser->last_class->next->prev) {
			parser->last_class = parser->last_class->next;
		}
	}

	parser->last_macro=parser->first_macro;
	if (parser->last_macro->next) {
		while (parser->last_macro->next && !parser->last_macro->next->prev)
			parser->last_macro = parser->last_macro->next;
	}

		parser->last_function=parser->first_function;
	if (parser->last_function->next) {
		while (parser->last_function->next && !parser->last_function->next->prev)
			parser->last_function = parser->last_function->next;
	}

	parser->last_global=parser->first_global;
	if (parser->last_global->next) {
		while (parser->last_global->next && !parser->last_global->next->prev)
			parser->last_global = parser->last_global->next;
	}
	return true;
}

wxString CodeHelper::GetInclude(wxString path, wxString key) {
	pd_class *aux_class = parser->first_class->next;
	while (aux_class) {
		if (aux_class->name==key) {
			if (!aux_class->prev)
				return aux_class->file->name;
			else if (aux_class->file) {
				wxFileName fn(aux_class->file->name);
				if (project) {
					wxArrayString header_dirs_array;
					mxUT::Split(project->active_configuration->headers_dirs,header_dirs_array,true,false);
					unsigned int i;
					for (i=0;i<header_dirs_array.GetCount();i++) {
						if (wxFileName(DIR_PLUS_FILE(DIR_PLUS_FILE(project->path,header_dirs_array[i]),fn.GetFullName()))==fn)
							return wxString("\"")+fn.GetFullName()+"\"";
					}
				}
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString("\"")+fn.GetFullPath()+"\"";
			}
		}
		aux_class = aux_class->next;
	}
	pd_func *aux_func = parser->first_function->next;
	while (aux_func) {
		if (aux_func->name==key) {
			if (!aux_func->prev) {
				return aux_func->file_dec->name;
			} else if (aux_func->file_dec) {
				wxFileName fn(aux_func->file_dec->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString("\"")+fn.GetFullPath()+"\"";
			} else if (aux_func->file_def) {
				wxFileName fn(aux_func->file_def->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString("\"")+fn.GetFullPath()+"\"";
			}
		}
		aux_func = aux_func->next;
	}
	pd_var *aux_var = parser->first_global->next;
	while (aux_var) {
		if (aux_var->name==key) {
			if (!aux_var->prev) {
				return aux_var->file->name;
			} else {
				wxFileName fn(aux_var->file->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString("\"")+fn.GetFullPath()+"\"";
			}
		}
		aux_var = aux_var->next;
	}
	pd_macro *aux_macro = parser->first_macro->next;
	while (aux_macro) {
		if (aux_macro->name==key) {
			if (!aux_macro->prev) {
				return aux_macro->file->name;
			} else {
				wxFileName fn(aux_macro->file->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString("\"")+fn.GetFullPath()+"\"";
			}
		}
		aux_macro = aux_macro->next;
	}
	return "";
}

wxString CodeHelper::GetIncludeForClass(wxString path, wxString key) {
	pd_class *aux_class = parser->first_class->next;
	while (aux_class) {
		if (aux_class->name==key) {
			if (!aux_class->prev)
				return wxString(_T("#include "))+aux_class->file->name;
			else if (aux_class->file) {
				wxFileName fn(aux_class->file->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString(_T("#include \""))+fn.GetFullPath()+"\"";
			}
		}
		aux_class = aux_class->next;
	}
	return "";
}

wxString CodeHelper::UnMacro(wxString name) {
	HashStringParserMacro::iterator it = parser->h_macros.find(name);
	if (it!=parser->h_macros.end() && it->second->cont.Len()) {
		name=it->second->cont;
		int p=name.Find("//");
		if (p!=wxNOT_FOUND) name=name.Mid(0,p);
		name.Trim(true); name.Trim(false);
		return name;
	}
	return name;
}

wxString CodeHelper::UnMacro(wxString name, int &dims) {
	wxString res=UnMacro(name); if (res!=name) {
		name=res;
		int i=name.Len()-1, c=0;
		while (name[i]=='*') { i--; c++; }
		if (c) name=name.Mid(0,i+1);
		dims+=c;
		i=0; c=0;
		while (name[i]=='*') { i++; c++; }
		if (c) name=name.Mid(i);
		dims-=c;
	}
	return name;
}

void CodeHelper::UnTemplate(wxString &var) {
	if (var.IsEmpty()) return;
	int i = var.Len()-1;
	if (var[i]=='>') {
		int p=var.Find('<');
		if (p!=wxNOT_FOUND)
			var=var.Mid(0,p);
	} else {
		while (i && var[i]=='*') 
			i--;
		if (var[i]=='>') {
			int p=var.Find('<');
			if (p!=wxNOT_FOUND)
				var=var.Mid(0,p)+var.Mid(i+1);
		}
	}
	int p=var.Find("::");
	if (p!=wxNOT_FOUND) var.Remove(0,p+2);
}

bool CodeHelper::ShowConstructorCalltip(int p, mxSource *source, wxString name) {
	UnTemplate(name);
	int t=0;
	wxString text;
	
	HashStringParserMacro::iterator itm = parser->h_macros.find(name);
	if (itm!=parser->h_macros.end()) {
		t=1;
		text=itm->second->proto;
	}
	
	pd_func *aux_func;
	
	HashStringParserClass::iterator it=parser->h_classes.find(name);
	if (it!=parser->h_classes.end()) {
		aux_func=it->second->first_method->next;
		while (aux_func) {
			if (aux_func->name==name && aux_func->proto[0]!='~') {
				if (t++)
					text+="\n";
				text+=aux_func->full_proto;
			}
			aux_func = aux_func->next;
		}
	}
		
	if (t!=0) source->ShowCallTip(p,p-name.Len(),text);
	return t!=0;
	
}

void CodeHelper::AddReservedWords(wxString &typed/*, int max_str_dist*/) {
	int len = typed.Len(), ll = reserved_words.keywords.GetCount();
	for (int j=0;j<ll;j++) {
//		int CH_COMPARE(typed,reserved_words.keywords[j],i,l,max_str_dist);
		if (ShouldAddToAutocomp(typed,len,reserved_words.keywords[j])) autocomp_list.Add(reserved_words.keywords[j],reserved_words.icon,reserved_words.help);
	}
}

bool CodeHelper::GenerateAutocompletionIndex(wxString path, wxString filename) {
	wxString autocomp_dir = DIR_PLUS_FILE(config->home_dir,"autocomp");
	if (!wxFileName::DirExists(autocomp_dir)) wxFileName::Mkdir(autocomp_dir);
	wxFFile idx(DIR_PLUS_FILE(autocomp_dir,filename),_T("w+"));
	if (!idx.IsOpened()) 
		return false;
	
	pd_file *fil = parser->last_file->next;
	while (fil) {
		wxFileName fn(fil->name);
		fn.MakeRelativeTo(path);
		idx.Write(wxString(_T("<"))<<fn.GetFullPath()<<_T(">\n"));
		pd_ref *ref = fil->first_global->next;
		while (ref) {
			if (PD_UNREF(pd_var,ref)->space==nullptr) {
				if (PD_UNREF(pd_var,ref)->properties==PD_CONST_ENUM_CONST)
					idx.Write(wxString(_T("\tenum const "))<<PD_UNREF(pd_var,ref)->proto<<"\n");
				else if (PD_UNREF(pd_var,ref)->properties==PD_CONST_CONST)
					idx.Write(wxString(_T("\tconst "))<<PD_UNREF(pd_var,ref)->proto<<"\n");
				else
					idx.Write(wxString("\t")<<PD_UNREF(pd_var,ref)->proto<<"\n");
			}
			ref = ref->next;
		}
		ref = fil->first_func_dec->next;
		while (ref) {
			if (PD_UNREF(pd_func,ref)->space==nullptr)
				idx.Write(wxString("\t")<<PD_UNREF(pd_func,ref)->proto<<"\n");
			ref = ref->next;
		}
		ref = fil->first_func_def->next;
		while (ref) {
			if (PD_UNREF(pd_func,ref)->space==nullptr && PD_UNREF(pd_func,ref)->file_dec==nullptr)
				idx.Write(wxString("\t")<<PD_UNREF(pd_func,ref)->proto<<"\n");
			ref = ref->next;
		}
		ref = fil->first_macro->next;
		while (ref) {
			idx.Write(wxString("\t")<<PD_UNREF(pd_macro,ref)->proto<<"\n");
			ref = ref->next;
		}
		ref = fil->first_class->next;
		while (ref) {
			pd_class *cls = PD_UNREF(pd_class,ref);
			wxString herencias;
			pd_inherit *item=parser->first_inherit->next;
			while (item) {
				if (item->son==cls->name) {
					if (item->properties&PD_CONST_PROTECTED)
						herencias<<" protected "<<item->father;
					else if (item->properties&PD_CONST_PRIVATE)
						herencias<<" private "<<item->father;
					else
						herencias<<" public "<<item->father;
				}
				item=item->next;
			}
			idx.Write(wxString(_T("\tclass "))<<cls->name<<herencias<<"\n");
			pd_var *var = cls->first_attrib->next;
			while (var) {
				if (var->properties&PD_CONST_PUBLIC)
					idx.Write(wxString(_T("\t\t"))<<var->proto<<"\n");
				else if (var->properties&PD_CONST_ENUM_CONST)
					idx.Write(wxString(_T("\t\tenum const "))<<var->proto<<"\n");
				var = var->next;
			}
			pd_func *met = cls->first_method->next;
			while (met) {
				if (met->properties&PD_CONST_PUBLIC)
					idx.Write(wxString(_T("\t\t"))<<met->proto<<"\n");
				met = met->next;
			}
			ref = ref->next;
		}
		fil = fil->next;
	}
	idx.Close();

	return true;
}

/**
* Recarga los indices de autocompletado solo si la nueva lista es diferente a la
* actual. Se encarga además de evitar repetir elementos en la lista si viniensen
* dados dos veces en el argumento. La lista actual esta guardada en actual_indexes
* @param indexes cadena con la nueva lista de indices
**/
void CodeHelper::ReloadIndexes(wxString indexes) {
	wxArrayString new_list;
	mxUT::Split(indexes,new_list,true,false); // cortar indices
	mxUT::Purgue(new_list,true); // ordenar y eliminar repetidos
	actual_indexes.Clear();
	if (!mxUT::Compare(actual_indexes,new_list)) { // comparar
		ResetStdData(); // borrar la lista actual
		for (unsigned int i=0;i<new_list.GetCount();i++)
			LoadData(new_list[i]); // cargar datos y agregar a la lista
	}
}

/**
* Agrega indices de autocompletado. Se encarga además de evitar repetir elementos 
* en la lista si viniensen dados dos veces, o indices ya cargados en el argumento. 
* La lista actual esta guardada en actual_indexes.
* @param indexes cadena con la lista de indices a agregar
**/
void CodeHelper::AppendIndexes(wxString indexes) {
	wxArrayString new_list;
	mxUT::Split(indexes,new_list,true,false);
	for (unsigned int i=0;i<new_list.GetCount();i++) {
#ifdef __WIN32__
		if (actual_indexes.Index(new_list[i],false)==wxNOT_FOUND)
#else
		if (actual_indexes.Index(new_list[i],true)==wxNOT_FOUND)
#endif
			LoadData(new_list[i]);
	}
	actual_indexes.Sort();
}

#define EN_COMPOUT_UNDEFINED_REFERENCE ": undefined reference to "
#define EN_COMPOUT_NO_SUCH_FILE_OR_DIRECTORY ": No such file or directory"

/**
* @brief on_end Si encuentra una solucion y el usuario la acepta, le transfiere 
*               esta accion al nuevo proceso de compilacion (y setea el argumento
*               en nullptr para indicarlo), sino lo deja como esta, no lo ejecuta, y
*               sigue siendo responsabilidad de quien llamo
**/
void CodeHelper::TryToSuggestTemplateSolutionForLinkingErrors (const wxArrayString &full_output, GenericAction *&on_end) {
	
	static bool dont_check=false; if (dont_check) return;

	mxSource *source=main_window->GetCurrentSource();
	if (!source) return;
	
	static bool template_map_filled=false;
	static map<wxString,int> the_map; // asocia include con elementos de temp_names y temp_args;
	static wxArrayString temp_names; // nombres de las plantillas
	static wxArrayString temp_args; // argumentos de las plantillas
	
	if (!template_map_filled) {
		template_map_filled=true; wxArrayString templates_list;
		mxUT::GetFilesFromBothDirs(templates_list,"templates");
		for(unsigned int i=0;i<templates_list.GetCount();i++) {
			wxString name=templates_list[i], options, includes;
			wxString filename = mxUT::WichOne(name,"templates",true);
			wxTextFile file(filename);
			file.Open();
			if (file.IsOpened()) { 
				// buscar si tiene nombre, argumentos e includes
				wxString line = file.GetFirstLine();
				while (!file.Eof() && line.Left(7)==_T("// !Z! ")) {
					if (line.Left(15)==_T("// !Z! Options:")) {
						options = line.Mid(15).Trim(false).Trim(true);
					}
					if (line.Left(12)==_T("// !Z! Name:")) {
						name = line.Mid(12).Trim(false).Trim(true);
					}
					if (line.Left(16)==_T("// !Z! Includes:")) {
						includes = line.Mid(16).Trim(false).Trim(true);
					}
					line = file.GetNextLine();
				}
				file.Close();
				if (options.Len() && includes.Len()) {
					wxArrayString ainc; mxUT::Split(includes,ainc,true,true);
					temp_names.Add(name);
					temp_args.Add(options);
					for(unsigned int j=0;j<ainc.Count();j++) {
						wxString key=ainc[j];
						while (the_map.find(key)!=the_map.end()) key+="#";
						the_map[key]=temp_names.GetCount()-1;
					}
				}
			}
		}
	}	

	if (the_map.empty()) return;
		
	map<wxString,wxString> candidatos; // opciones
	
	for(unsigned int i=0;i<full_output.GetCount();i++) { 
		
		wxString &l=full_output[i], keyword; 
		
		// simbolos que no encuentra el linker
		int p=l.Find(EN_COMPOUT_UNDEFINED_REFERENCE);
		if (p!=wxNOT_FOUND) {
			keyword=l.Mid(p+wxString(EN_COMPOUT_UNDEFINED_REFERENCE).Len()+1);
			unsigned int j=0; 
			while (j<keyword.Len()) {
				if (keyword[j]=='\'' || keyword[j]=='`') {
					keyword=keyword.Mid(0,j);
					break;
				}
				j++;
			}
			if (!keyword.Len()) continue;
			keyword=GetInclude("",keyword);
			if (!keyword.Len()) continue;
		} else {
		
			// includes que no encuentra el linker
			p=l.Find(EN_COMPOUT_NO_SUCH_FILE_OR_DIRECTORY);
			if (p!=wxNOT_FOUND) {
				keyword=l.Mid(0,p);
				unsigned int j=p-1; 
				while (j>0 && keyword[j]!=':') j--;
				if (keyword[j+1]==' ') j++;
				keyword=keyword.Mid(j+1);
				if (!keyword.Len()) continue;
				keyword=wxString("<")<<keyword<<">";
			} else 
				if (!keyword.Len()) continue;
		}
		// buscar en las plantillas
		map<wxString,int>::iterator it=the_map.find(keyword);
		if (it==the_map.end()) continue;
		while (it!=the_map.end()) {
			candidatos[temp_names[it->second]]=temp_args[it->second];
			keyword+="#";
			it=the_map.find(keyword);
		}
	}
	
	if (candidatos.empty()) return;
	wxArrayString vals;
	map<wxString,wxString>::iterator it=candidatos.begin(), it2=candidatos.end();
	while (it!=it2) {
		it->second.Replace("${DEFAULT}",config->GetDefaultCompilerOptions(source->IsCppOrJustC()),true);
		if (it->second==source->GetCompilerOptions(false)) return; // si ya se habia aplicado una, no volver a preguntar, probablemente sea otro el problema
		vals.Add(it->first);
		++it;
	}
	int ans=mxMessageDialog(main_window,("Los errores de compilación/enlazado podrían deberse a la falta de argumentos\nde compilación adecuados para las bibliotecas que utiliza.\n¿Desea que ZinjaI modifique automáticamente los argumentos en base a una plantilla?"),("Errores de compilacion/enlazado"),mxMD_YES_NO|mxMD_QUESTION,("No volver a mostrar este mensaje"),false).ShowModal();
	if (ans&mxMD_NO) {
		if (ans&mxMD_CHECKED) dont_check=false;
		return;
	}
	wxString cual=wxGetSingleChoice("Seleccione una plantilla","Parametros extra para el compilador",vals,main_window);
	if (cual.Len()==0) return;
	source->SetCompilerOptions(candidatos[cual]);
	wxCommandEvent evt;
	main_window->CompileSource(true,fms_move(on_end)); // null caller's compile_and_run_struct_single to avoid double-delete
}

void MyAutocompList::Add(const wxString &keyword, const wxString &icon, const wxString &help) {
	if (keyword.Len()>max_len) max_len=keyword.Len();
	if (icon.Len()) keywords.Add(keyword+icon); else keywords.Add(keyword);
	if (help.Len()) helps.Add(keywords.Last()+"\n"+help);
//	count++;
}

const wxString &MyAutocompList::GetResult (bool sort, int n) {
	if (n==-1) n=keywords.GetCount();
	if (sort) mxUT::SortArrayString(keywords,0,n-1);
	wxString text(keywords[0]);
	for (int li=0,i=1;i<n;i++)
		if (keywords[li]!=keywords[i])
			text<<"\n"<<keywords[li=i];
	return result=text;
}

wxString MyAutocompList::GetHelp (int sel) {
	if (sel==-1) return "";
	wxString text(keywords[0]); int c=0;
	int li=0;
	for (int i=1;c!=sel && i<int(keywords.GetCount());i++)
		if (keywords[li]!=keywords[i]) { li=i; c++; }				
	wxString key = keywords[li]+"\n", help;
	for (int i=0;i<int(helps.GetCount());i++) {
		if (helps[i].StartsWith(key)) {
			if (help.Len()) help<<"\n";
			help<<helps[i].AfterFirst('\n');
		}
	}
	return help;
}

void CodeHelper::Initialize ( ) {
	code_helper = new CodeHelper(config->Help.min_len_for_completion,config->Source.autoCompletion);
}

void CodeHelper::SetAutocompletionMatchingMode (int mode) {
	switch (mode) {
	case 2: ShouldAddToAutocomp = ShouldAddToAutocompFind; break;
	case 3: ShouldAddToAutocomp = ShouldAddToAutocompFuzzy; break;
	default: ShouldAddToAutocomp = ShouldAddToAutocompStart; break;
	}
}

CodeHelper::RAIAutocompModeChanger::RAIAutocompModeChanger () {
}

CodeHelper::RAIAutocompModeChanger::~RAIAutocompModeChanger ( ) {
	code_helper->SetAutocompletionMatchingMode(config->Source.autoCompletion);
}

void CodeHelper::RAIAutocompModeChanger::Change (int mode) {
	code_helper->SetAutocompletionMatchingMode(mode);
}

void CodeHelper::FilterAutocomp (mxSource *source, const wxString & key, bool force_show) {
	wxString old_res = autocomp_list.GetLastResult();
	const wxString &new_res = autocomp_list.GetFiltered(key);
	if (!force_show && new_res==old_res) return;
	if (new_res.IsEmpty()) {
		source->HideCalltip();
	}
	else source->ShowAutoComp(key.Len(),new_res,true);
}

wxString MyAutocompList::GetFiltered (wxString typed) {
	int len=typed.Len(), n=keywords.GetCount(), i=0; max_len=0; typed.MakeLower();
	while(i<n) { 
		if (!ShouldAddToAutocomp(typed,len,keywords[i])) {
			swap(keywords[i],keywords[--n]);
		} else {
			if (keywords[i].Len()>max_len) max_len=keywords[i].Len();
			i++;
		}
	}
	if (!n) return "";
	return GetResult(true,n);
}

bool auxAutocompLocalsPreChar(char c) {
	return c==' '||c=='\t'||c==','||c=='*'||c=='&'||c=='\n'||c=='\r';
}

void CodeHelper::AutocompleteLocals (mxSource * source, wxString typed, int scope_start) {
	typed.MakeLower();
	int p_to = source->GetCurrentPos()-typed.Len(), p=scope_start;
	p = source->FindText(p,p_to,"{",0);
	while(p!=wxSTC_INVALID_POSITION && p<p_to) {
		if (p==0 || auxAutocompLocalsPreChar(source->GetCharAt(p-1))) { 
			int s = source->GetStyleAt(p);
			if (s==wxSTC_C_IDENTIFIER||s==wxSTC_C_GLOBALCLASS) { 
				wxString word = source->GetTextRange(p,source->WordEndPosition(p,true));
				if (!autocomp_list.Contains(word)) autocomp_list.Add(word,"$21","");
			}
		}
		p = source->FindText(p+typed.Len(),p_to,typed,0);
	}
}

