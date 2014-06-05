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
using namespace std;

// comparacion con distancia de Levenshtein
/*#define CH_COMPARE(typed,aux,i,l,max_str_dist)\*/
/*	if (max_str_dist) \*/
/*		i = utils->Levenshtein(typed.mb_str(),typed.Len(),aux.mb_str(),aux.Len())<=max_str_dist?l:0; \*/
/*	else { \*/
/*		i=0; \*/
/*		while (i<l && (amux[i]|32)==typed[i]) \*/
/*			i++; \*/
/*	}\*/
/*	if (i==l) cerr<<typed<<"  "<<aux<<"  "<<utils->Levenshtein(typed.mb_str(),l,aux.mb_str(),aux.Len())<<"   "<<max_str_dist<<endl; \*/

// comparacion con normal letra a letra no case sensitive
#define CH_COMPARE(typed,aux,i,l,max_str_dist)\
	i=0; \
	while (i<l && (aux[i]|32)==typed[i]) \
		i++; \
	

CodeHelper *code_helper;

CodeHelper::CodeHelper(int ml) {
	min_len=ml;

}

CodeHelper::~CodeHelper() {

}

bool CodeHelper::AutocompletePreprocesorDirective(mxSource *source, wxString typed) {
	return AutocompleteFromArray(source,preproc_directives,typed);
}

bool CodeHelper::AutocompleteDoxygen(mxSource *source, wxString typed) {
	return AutocompleteFromArray(source,doxygen_directives,typed);
}

bool CodeHelper::AutocompleteFromArray(mxSource *source, wxArrayString &words, wxString typed) {
	
	if (!words.GetCount()) return false;
	
	unsigned int t=0,i,l=typed.Len();
	for (i=0;i<l;i++)
		typed[i]=(typed[i]|32);
	if (!typed.Len()) {
		wxString text(words[0]);
		int li=0;
		for (unsigned int i=1;i<comp_array.GetCount();i++)
			if (comp_array[li]!=comp_array[i])
				text<<"\n"<<comp_array[li=i];
		source->ShowAutoComp(l,text);
	}

	wxArrayString comp_array;

	int j, ll = words.GetCount();
	for (j=0;j<ll;j++) {
		i=0;
		while (i<l && (words[j][i]|32)==typed[i])
			i++;
		if (i==l) {
			t++;
			comp_array.Add(words[j]);
		}
	}
	
	if (t==0) return false;
//	utils->SortArrayString(comp_array,0,t-1);
	wxString text(comp_array[0]);
	int li=0;
	for (unsigned int i=1;i<comp_array.GetCount();i++)
		if (comp_array[li]!=comp_array[i])
			text<<"\n"<<comp_array[li=i];
	source->ShowAutoComp(l,text);
	return true;	
}

bool CodeHelper::AutocompleteScope(mxSource *source, wxString &key, wxString typed, bool consider_inherit, bool add_reserved_words, int max_str_dist) {
	UnTemplate(key);
	unsigned int i,l=typed.Len();
	for (i=0;i<l;i++)
		typed[i]=(typed[i]|32);
	if (!key.Len()) return false;
	HashStringParserClass::iterator it=parser->h_classes.find(key);
	if (it!=parser->h_classes.end()) {
		int t=0;
		wxArrayString comp_array;
		pd_var *aux_var = it->second->first_attrib->next;
		while (aux_var) {
			if (!l) {
				t++;
				if (aux_var->properties&PD_CONST_PRIVATE)
					comp_array.Add(aux_var->name+"?6");
				else if (aux_var->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_var->name+"?7");
				else if (aux_var->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_var->name+"?8");
				else if (aux_var->properties&(PD_CONST_ENUM_CONST|PD_CONST_ENUM))
					comp_array.Add(aux_var->name+"?19");
				else 
					comp_array.Add(aux_var->name+"?5");
			} else {
				if (aux_var->name.Len()>=l) {
					CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_var->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_var->name+"?6");
						else if (aux_var->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_var->name+"?7");
						else if (aux_var->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_var->name+"?8");
						else
							comp_array.Add(aux_var->name+"?5");
					}
				}
			}
			aux_var = aux_var->next;
		}
		pd_func *aux_func = it->second->first_method->next;
		while (aux_func) {
			if (!l) {
				t++;
				if (aux_func->properties&PD_CONST_PRIVATE)
					comp_array.Add(aux_func->name+"?10");
				else if (aux_func->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_func->name+"?11");
				else if (aux_func->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_func->name+"?12");
				else
					comp_array.Add(aux_func->name+"?9");
			} else {
				if (aux_func->name.Len()>=l) {
					CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_func->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_func->name+"?10");
						else if (aux_func->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_func->name+"?11");
						else if (aux_func->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_func->name+"?12");
						else
							comp_array.Add(aux_func->name+"?9");
					}
				}
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
						if (!l) {
							t++;
							if (aux_var->properties&PD_CONST_PRIVATE)
								comp_array.Add(aux_var->name+"?6");
							else if (aux_var->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_var->name+"?7");
							else if (aux_var->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_var->name+"?8");
							else
								comp_array.Add(aux_var->name+"?5");
						} else {
							if (aux_var->name.Len()>=l) {
								CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_var->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_var->name+"?6");
									else if (aux_var->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_var->name+"?7");
									else if (aux_var->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_var->name+"?8");
									else
										comp_array.Add(aux_var->name+"?5");
								}
							}
						}
						aux_var = aux_var->next;
					}
					pd_func *aux_func = it->second->first_method->next;
					while (aux_func) {
						if (!l) {
							t++;
							if (aux_func->properties&PD_CONST_PRIVATE)
								comp_array.Add(aux_func->name+"?10");
							else if (aux_func->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_func->name+"?11");
							else if (aux_func->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_func->name+"?12");
							else
								comp_array.Add(aux_func->name+"?9");
						} else {
							if (aux_func->name.Len()>=l) {
								CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_func->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_func->name+"?10");
									else if (aux_func->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_func->name+"?11");
									else if (aux_func->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_func->name+"?12");
									else
										comp_array.Add(aux_func->name+"?9");
								}
							}
						}
						aux_func = aux_func->next;
					}
				}
			}
			
		}
		// agregar las palabras resevadas si corresponde
		if (add_reserved_words)
			t+=AddReservedWords(comp_array,typed);
		// mostrar la lista final
		if (t==0) return false;
		utils->SortArrayString(comp_array,0,t-1);
		wxString text(comp_array[0]);
		int li=0;
		for (unsigned int i=1;i<comp_array.GetCount();i++)
			if (comp_array[li]!=comp_array[i])
				text<<"\n"<<comp_array[li=i];
		source->ShowAutoComp(l,text);
		//comp_array.Clear();
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

bool CodeHelper::AutocompleteGeneral(mxSource *source, wxString scope, wxString typed, wxString *args, int max_str_dist) {
	UnTemplate(typed); UnTemplate(scope);
	int t=0;
	unsigned int i,l=typed.Len();
	for (i=0;i<l;i++) typed[i]=(typed[i]|32);
	wxArrayString comp_array;
	pd_var *aux_var = parser->first_global;
	while (aux_var) {
		if ( aux_var->name.Len()>=l) {
			CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
			if (i==l) {
				t++;
				comp_array.Add(aux_var->name+(aux_var->properties&(PD_CONST_ENUM_CONST|PD_CONST_ENUM)?"?19":"?14"));
			}
		}
		aux_var = aux_var->next;
	}

	pd_func *aux_func = parser->first_function;
	while (aux_func) {
		if ( aux_func->name.Len()>=l) {
			CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
			if (i==l) {
				t++;
				comp_array.Add(aux_func->name+"?3");
			}
		}
		aux_func = aux_func->next;
	}
	
	pd_macro *aux_macro = parser->first_macro;
	while (aux_macro) {
		if ( aux_macro->name.Len()>=l) {
			CH_COMPARE(typed,aux_macro->name,i,l,max_str_dist);
			if (i==l) {
				t++;
				comp_array.Add(aux_macro->name+(aux_macro->props&(PD_CONST_ENUM|PD_CONST_ENUM_CONST)?"?19":(aux_macro->props&PD_CONST_TYPEDEF?"?18":"?2")));
			}
		}
		aux_macro = aux_macro->next;
	}
	
	pd_class *aux_class = parser->first_class;
	while (aux_class) {
		if ( aux_class->name.Len()>=l) {
			CH_COMPARE(typed,aux_class->name,i,l,max_str_dist);
			if (i==l) {
				t++;
				if (aux_class->file) {
					comp_array.Add(aux_class->name+"?4");
				} else
					comp_array.Add(aux_class->name+"?13");
			}
		}
		aux_class = aux_class->next;
	}
	
	HashStringParserClass::iterator it=parser->h_classes.find(scope);
	if (it!=parser->h_classes.end()) {
		
		
		pd_var *aux_var = it->second->first_attrib->next;
		while (aux_var) {
			if (!l) {
				t++;
				if (aux_var->properties&PD_CONST_PRIVATE)
					comp_array.Add(aux_var->name+"?6");
				else if (aux_var->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_var->name+"?7");
				else if (aux_var->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_var->name+"?8");
				else
					comp_array.Add(aux_var->name+"?5");
			} else {
				if (aux_var->name.Len()>=l) {
					CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_var->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_var->name+"?6");
						else if (aux_var->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_var->name+"?7");
						else if (aux_var->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_var->name+"?8");
						else
							comp_array.Add(aux_var->name+"?5");
					}
				}
			}
			aux_var = aux_var->next;
		}
		pd_func *aux_func = it->second->first_method->next;
		while (aux_func) {
			if (!l) {
				t++;
				if (aux_func->properties&PD_CONST_PRIVATE)
					comp_array.Add(aux_func->name+"?10");
				else if (aux_func->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_func->name+"?11");
				else if (aux_func->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_func->name+"?12");
				else
					comp_array.Add(aux_func->name+"?9");
			} else {
				if (aux_func->name.Len()>=l) {
					CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_func->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_func->name+"?10");
						else if (aux_func->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_func->name+"?11");
						else if (aux_func->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_func->name+"?12");
						else
							comp_array.Add(aux_func->name+"?9");
					}
				}
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
						if (!l) {
							t++;
							if (aux_var->properties&PD_CONST_PRIVATE)
								comp_array.Add(aux_var->name+"?6");
							else if (aux_var->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_var->name+"?7");
							else if (aux_var->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_var->name+"?8");
							else
								comp_array.Add(aux_var->name+"?5");
						} else {
							if (aux_var->name.Len()>=l) {
								CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_var->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_var->name+"?6");
									else if (aux_var->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_var->name+"?7");
									else if (aux_var->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_var->name+"?8");
									else
										comp_array.Add(aux_var->name+"?5");
								}
							}
						}
						aux_var = aux_var->next;
					}
					pd_func *aux_func = it->second->first_method->next;
					while (aux_func) {
						if (!l) {
							t++;
							if (aux_func->properties&PD_CONST_PRIVATE)
								comp_array.Add(aux_func->name+"?10");
							else if (aux_func->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_func->name+"?11");
							else if (aux_func->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_func->name+"?12");
							else
								comp_array.Add(aux_func->name+"?9");
						} else {
							if (aux_func->name.Len()>=l) {
								CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_func->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_func->name+"?10");
									else if (aux_func->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_func->name+"?11");
									else if (aux_func->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_func->name+"?12");
									else
										comp_array.Add(aux_func->name+"?9");
								}
							}
						}
						aux_func = aux_func->next;
					}
				}
			}
//		}
	}
	
	// agregar las palabras reservadas
	t+=AddReservedWords(comp_array,typed);
	
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
						unsigned int CH_COMPARE(typed,id,i,l,max_str_dist);
						if (i==l) { comp_array.Add(id+"?20"); t++; }
					}
					if (c==',') { done=false; la=i+1; }
				}
			}
		}
	}
	
	
	// mostrar la lista final
	if (t==0) return false;
	utils->SortArrayString(comp_array,0,t-1);
	wxString text(comp_array[0]);
	int li=0;
	for (unsigned int i=1;i<comp_array.GetCount();i++)
		if (comp_array[li]!=comp_array[i])
			text<<"\n"<<comp_array[li=i];
	source->ShowAutoComp(l,text);
	return true;
}

bool CodeHelper::AutocompleteAutocode(mxSource *source, wxString typed, int max_str_dist) {
	wxArrayString comp_array; int t=0, l=typed.Len();
	HashStringAutoCode::iterator it = autocoder->list.begin();
	while (it!=autocoder->list.end()) {
		wxString &aux=it->first;
		int CH_COMPARE(typed,aux,i,l,max_str_dist);
		if (i==l) { ++t; comp_array.Add(it->first); }
		++it;
	}
	
	// mostrar la lista final
	if (t==0) return false;
	utils->SortArrayString(comp_array,0,t-1);
	wxString text(comp_array[0]);
	int li=0;
	for (unsigned int i=1;i<comp_array.GetCount();i++)
		if (comp_array[li]!=comp_array[i])
			text<<"\n"<<comp_array[li=i];
	source->ShowAutoComp(l,text);
	return true;
}

bool CodeHelper::ShowCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope) {
	wxString result=GetCalltip(scope,key,onlyScope);
	if (result.Len()) {
		source->ShowCallTip(p-key.Len(),result);
		source->calltip_brace=p;
	}
	return result.Len();
}
	

static void aux_AddToCalltip(int &t, wxString &text, const wxString &full_proto, bool only_type) {
	if (only_type) {
		if (t!=0 && !text.Len()) return; // esto indica que ya hubo problemas con los tipos
		wxString aux=full_proto;
		aux.Replace("static ","",true);
		aux.Replace("const ","",true);
		aux.Replace("constexpr ","",true);
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
					if (aux_func->name==key) {
						aux_AddToCalltip(t,text,aux_func->full_proto,only_type);
					}
					aux_func = aux_func->next;
				}
			}
		}
	}
	return text;
}

void CodeHelper::ResetStdData() {
	
	actual_indexes.Clear();
	reserved_words.Clear();
	doxygen_directives.Clear();
	preproc_directives.Clear();
	
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
	wxString filepath=utils->WichOne(index,"autocomp",true);
	if (!filepath.Len()) return false;
	wxTextFile fil(filepath);
	actual_indexes.Add(index);
	fil.Open();
	wxString header;
	unsigned int tabs;
	int pos;
	pd_file *aux_file=NULL;
	pd_class *aux_class=NULL;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (!str.Len()) 
			continue;
		while (str.Last()==';' || str.Last()==' ')
			str.RemoveLast();
		tabs=0;
		while (tabs+1<str.Len() && str[tabs]=='\t')
			tabs++;
		if (tabs+1>=str.Len()) 
			continue;
		str=str.Mid(tabs);
		if (str.Len() && str[0]=='\\') {
			reserved_words.Add(str.Mid(1));
		} else if (str.Len() && str[0]=='@') {
			doxygen_directives.Add(str.Mid(1)+"?17");
		} else if (str.Len() && str[0]=='#') {
			preproc_directives.Add(str.Mid(1)+"?16");
		} else if (tabs==0) { // es el nombre de la cabecera
			header=str;
			CH_REGISTER_FILE(aux_file, header);
		} else if (tabs==1) {
			if (str.Mid(0,6)==_("class ")) { // es una clase
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
				while ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') )
					pos--;
				name=name.Mid(pos+1);
				while (str[pos]==' ')
					pos--;
				wxString type = str.Mid(0,pos+1);
				while (str[pos]==' ' || str[pos]=='*' || str[pos]=='&')
					pos--;
				while ( pos>0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
					pos--;
				if (pos!=0)
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
					char c=type.Last();
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
	
	utils->SortArrayString(preproc_directives);
	utils->SortArrayString(doxygen_directives);
	
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
					utils->Split(project->active_configuration->headers_dirs,header_dirs_array,true,false);
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
		
	if (t!=0)
		source->ShowCallTip(p-name.Len(),text);
	source->calltip_brace=p;
	return t!=0;
	
}

int CodeHelper::AddReservedWords(wxArrayString &comp_list, wxString &typed, int max_str_dist) {
	int j, c = 0, l = typed.Len(), ll = reserved_words.GetCount();
	for (j=0;j<ll;j++) {
		int CH_COMPARE(typed,reserved_words[j],i,l,max_str_dist);
		if (i==l) {
			c++;
			comp_list.Add(reserved_words[j]+"?15");
		}
	}
	return c;
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
			if (PD_UNREF(pd_var,ref)->space==NULL) {
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
			if (PD_UNREF(pd_func,ref)->space==NULL)
				idx.Write(wxString("\t")<<PD_UNREF(pd_func,ref)->proto<<"\n");
			ref = ref->next;
		}
		ref = fil->first_func_def->next;
		while (ref) {
			if (PD_UNREF(pd_func,ref)->space==NULL && PD_UNREF(pd_func,ref)->file_dec==NULL)
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
	utils->Split(indexes,new_list,true,false); // cortar indices
	utils->Purgue(new_list,true); // ordenar y eliminar repetidos
	actual_indexes.Clear();
	if (!utils->Compare(actual_indexes,new_list)) { // comparar
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
	utils->Split(indexes,new_list,true,false);
	for (unsigned int i=0;i<new_list.GetCount();i++) {
#if defined(_WIN32) || defined(__WIN32__)
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

void CodeHelper::TryToSuggestTemplateSolutionForLinkingErrors (const wxArrayString &full_output, bool for_running) {
	
	static bool dont_check=false; if (dont_check) return;

	mxSource *source=main_window->GetCurrentSource();
	if (!source) return;
	
	static bool template_map_filled=false;
	static map<wxString,int> the_map; // asocia include con elementos de temp_names y temp_args;
	static wxArrayString temp_names; // nombres de las plantillas
	static wxArrayString temp_args; // argumentos de las plantillas
	
	if (!template_map_filled) {
		template_map_filled=true; wxArrayString templates_list;
		utils->GetFilesFromBothDirs(templates_list,"templates");
		for(unsigned int i=0;i<templates_list.GetCount();i++) {
			wxString name=templates_list[i], options, includes;
			wxString filename = utils->WichOne(name,"templates",true);
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
					wxArrayString ainc; utils->Split(includes,ainc,true,true);
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
	if (for_running) main_window->OnRunRun(evt);
	else main_window->OnRunCompile(evt);
}

