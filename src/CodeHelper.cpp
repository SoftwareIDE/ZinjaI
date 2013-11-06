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
				text<<_T("\n")<<comp_array[li=i];
		source->AutoCompShow(l,text);
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
			text<<_T("\n")<<comp_array[li=i];
	source->AutoCompShow(l,text);
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
					comp_array.Add(aux_var->name+_T("?6"));
				else if (aux_var->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_var->name+_T("?7"));
				else if (aux_var->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_var->name+_T("?8"));
				else if (aux_var->properties&(PD_CONST_ENUM_CONST|PD_CONST_ENUM))
					comp_array.Add(aux_var->name+_T("?19"));
				else 
					comp_array.Add(aux_var->name+_T("?5"));
			} else {
				if (aux_var->name.Len()>=l) {
					CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_var->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_var->name+_T("?6"));
						else if (aux_var->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_var->name+_T("?7"));
						else if (aux_var->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_var->name+_T("?8"));
						else
							comp_array.Add(aux_var->name+_T("?5"));
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
					comp_array.Add(aux_func->name+_T("?10"));
				else if (aux_func->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_func->name+_T("?11"));
				else if (aux_func->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_func->name+_T("?12"));
				else
					comp_array.Add(aux_func->name+_T("?9"));
			} else {
				if (aux_func->name.Len()>=l) {
					CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_func->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_func->name+_T("?10"));
						else if (aux_func->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_func->name+_T("?11"));
						else if (aux_func->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_func->name+_T("?12"));
						else
							comp_array.Add(aux_func->name+_T("?9"));
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
								comp_array.Add(aux_var->name+_T("?6"));
							else if (aux_var->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_var->name+_T("?7"));
							else if (aux_var->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_var->name+_T("?8"));
							else
								comp_array.Add(aux_var->name+_T("?5"));
						} else {
							if (aux_var->name.Len()>=l) {
								CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_var->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_var->name+_T("?6"));
									else if (aux_var->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_var->name+_T("?7"));
									else if (aux_var->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_var->name+_T("?8"));
									else
										comp_array.Add(aux_var->name+_T("?5"));
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
								comp_array.Add(aux_func->name+_T("?10"));
							else if (aux_func->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_func->name+_T("?11"));
							else if (aux_func->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_func->name+_T("?12"));
							else
								comp_array.Add(aux_func->name+_T("?9"));
						} else {
							if (aux_func->name.Len()>=l) {
								CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_func->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_func->name+_T("?10"));
									else if (aux_func->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_func->name+_T("?11"));
									else if (aux_func->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_func->name+_T("?12"));
									else
										comp_array.Add(aux_func->name+_T("?9"));
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
				text<<_T("\n")<<comp_array[li=i];
		source->AutoCompShow(l,text);
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
	return _T("");
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
	return _T("");
}

bool CodeHelper::AutocompleteGeneral(mxSource *source, wxString scope, wxString typed, int max_str_dist) {
	UnTemplate(typed); UnTemplate(scope);
	int t=0;
	unsigned int i,l=typed.Len();
	for (i=0;i<l;i++)
		typed[i]=(typed[i]|32);
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
				comp_array.Add(aux_func->name+_T("?3"));
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
					comp_array.Add(aux_class->name+_T("?4"));
				} else
					comp_array.Add(aux_class->name+_T("?13"));
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
					comp_array.Add(aux_var->name+_T("?6"));
				else if (aux_var->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_var->name+_T("?7"));
				else if (aux_var->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_var->name+_T("?8"));
				else
					comp_array.Add(aux_var->name+_T("?5"));
			} else {
				if (aux_var->name.Len()>=l) {
					CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_var->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_var->name+_T("?6"));
						else if (aux_var->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_var->name+_T("?7"));
						else if (aux_var->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_var->name+_T("?8"));
						else
							comp_array.Add(aux_var->name+_T("?5"));
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
					comp_array.Add(aux_func->name+_T("?10"));
				else if (aux_func->properties&PD_CONST_PROTECTED)
					comp_array.Add(aux_func->name+_T("?11"));
				else if (aux_func->properties&PD_CONST_PUBLIC)
					comp_array.Add(aux_func->name+_T("?12"));
				else
					comp_array.Add(aux_func->name+_T("?9"));
			} else {
				if (aux_func->name.Len()>=l) {
					CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
					if (i==l) {
						t++;
						if (aux_func->properties&PD_CONST_PRIVATE)
							comp_array.Add(aux_func->name+_T("?10"));
						else if (aux_func->properties&PD_CONST_PROTECTED)
							comp_array.Add(aux_func->name+_T("?11"));
						else if (aux_func->properties&PD_CONST_PUBLIC)
							comp_array.Add(aux_func->name+_T("?12"));
						else
							comp_array.Add(aux_func->name+_T("?9"));
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
								comp_array.Add(aux_var->name+_T("?6"));
							else if (aux_var->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_var->name+_T("?7"));
							else if (aux_var->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_var->name+_T("?8"));
							else
								comp_array.Add(aux_var->name+_T("?5"));
						} else {
							if (aux_var->name.Len()>=l) {
								CH_COMPARE(typed,aux_var->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_var->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_var->name+_T("?6"));
									else if (aux_var->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_var->name+_T("?7"));
									else if (aux_var->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_var->name+_T("?8"));
									else
										comp_array.Add(aux_var->name+_T("?5"));
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
								comp_array.Add(aux_func->name+_T("?10"));
							else if (aux_func->properties&PD_CONST_PROTECTED)
								comp_array.Add(aux_func->name+_T("?11"));
							else if (aux_func->properties&PD_CONST_PUBLIC)
								comp_array.Add(aux_func->name+_T("?12"));
							else
								comp_array.Add(aux_func->name+_T("?9"));
						} else {
							if (aux_func->name.Len()>=l) {
								CH_COMPARE(typed,aux_func->name,i,l,max_str_dist);
								if (i==l) {
									t++;
									if (aux_func->properties&PD_CONST_PRIVATE)
										comp_array.Add(aux_func->name+_T("?10"));
									else if (aux_func->properties&PD_CONST_PROTECTED)
										comp_array.Add(aux_func->name+_T("?11"));
									else if (aux_func->properties&PD_CONST_PUBLIC)
										comp_array.Add(aux_func->name+_T("?12"));
									else
										comp_array.Add(aux_func->name+_T("?9"));
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
	
	// mostrar la lista final
	if (t==0) return false;
	utils->SortArrayString(comp_array,0,t-1);
	wxString text(comp_array[0]);
	int li=0;
	for (unsigned int i=1;i<comp_array.GetCount();i++)
		if (comp_array[li]!=comp_array[i])
			text<<_T("\n")<<comp_array[li=i];
	source->AutoCompShow(l,text);
	return true;
}

bool CodeHelper::AutocompleteAutocode(mxSource *source, wxString typed, int max_str_dist) {
	wxArrayString comp_array; int t=0, l=typed.Len(), i;
	HashStringAutoCode::iterator it = autocoder->list.begin();
	while (it!=autocoder->list.end()) {
		wxString &aux=it->first;
		CH_COMPARE(typed,aux,i,l,max_str_dist);
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
			text<<_T("\n")<<comp_array[li=i];
	source->AutoCompShow(l,text);
	return true;
}

bool CodeHelper::ShowCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope) {
	UnTemplate(key); UnTemplate(scope);
	int t=0;
	wxString text;
	
	HashStringParserMacro::iterator it = parser->h_macros.find(key);
	if (it!=parser->h_macros.end()) {
		t=1;
		text=it->second->proto;
	}
	
	pd_func *aux_func;
	
	if (!onlyScope || scope.Len()==0) {
		aux_func=parser->first_function->next;
		while (aux_func) {
			if (aux_func->name==key) {
				if (t++)
					text+=_T("\n");
				text+=aux_func->full_proto;
			}
			aux_func = aux_func->next;
		}
		pd_class *aux_class = parser->first_class->next;
		while (aux_class) {
			if (aux_class->name==key) {
				aux_func=aux_class->first_method->next;
				while (aux_func) {
					if (aux_func->name==key) {
						if (t++)
							text+=_T("\n");
						text+=aux_func->full_proto;
					}
					aux_func = aux_func->next;
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
					if (t++)
						text+=_T("\n");
					text+=aux_func->full_proto;
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
						if (t++)
							text+=_T("\n");
						text+=aux_func->full_proto;
					}
					aux_func = aux_func->next;
				}
			}
		}
	}
	if (t!=0) {
		source->ShowCallTip(p-key.Len(),text);
	}
	source->calltip_brace=p;
	return t!=0;
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
			doxygen_directives.Add(str.Mid(1)+_T("?17"));
		} else if (str.Len() && str[0]=='#') {
			preproc_directives.Add(str.Mid(1)+_T("?16"));
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
				if ( (pos=str.Find(' '))==wxNOT_FOUND || str.StartsWith("enum ")) { // macro o enum
					CH_REGISTER_MACRO(aux_file,str,PD_CONST_MACRO_CONST);
				} else if (str.StartsWith("enum ")) { // macro o enum
					wxString name = str.AfterLast(' ');
					wxString type = str.BeforeLast(' ');
					int props=PD_CONST_ENUM;
					if (type.StartsWith("enum const ")) {
						type=type.Mid(11); if (!type.Len()) type="anonymous";
						props=PD_CONST_ENUM_CONST;
					}
					CH_REGISTER_MACRO(aux_file,str,props);
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
					CH_REGISTER_MACRO_P(aux_file,name,str);
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
						type=type.Mid(11); if (!type.Len()) type="anonymous";
						props=PD_CONST_ENUM_CONST;
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
				while ( ! ((c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9')) )
					pos--;
				while ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') )
					pos--;
				name=name.Mid(pos+1);
				// scope
				pos-=2;
				while ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') )
					pos--;
				while (str[pos]==' ')
					pos--;
				// tipo
				if (name==aux_class->name) {
					CH_REGISTER_METHOD(aux_file, aux_class, name, name, str);				
				} else {
					wxString type = str.Mid(0,pos+1);
					while (str[pos]==' ' || str[pos]=='*' || str[pos]=='&')
						pos--;
					while ( pos>0 && ( (c=str[pos])=='_' || ((c|32)>='a' && (c|32)<='z') || (c>='0' && c<='9') ) )
						pos--;
					if (pos!=0)
						pos++;
					type=type.Mid(pos);
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
							return wxString(_T("\""))+fn.GetFullName()+_T("\"");
					}
				}
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString(_T("\""))+fn.GetFullPath()+_T("\"");
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
				return wxString(_T("\""))+fn.GetFullPath()+_T("\"");
			} else if (aux_func->file_def) {
				wxFileName fn(aux_func->file_def->name);
				if (path.Len())
					fn.MakeRelativeTo(path);
				return wxString(_T("\""))+fn.GetFullPath()+_T("\"");
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
				return wxString(_T("\""))+fn.GetFullPath()+_T("\"");
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
				return wxString(_T("\""))+fn.GetFullPath()+_T("\"");
			}
		}
		aux_macro = aux_macro->next;
	}
	return _T("");
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
				return wxString(_T("#include \""))+fn.GetFullPath()+_T("\"");
			}
		}
		aux_class = aux_class->next;
	}
	return _T("");
}

wxString CodeHelper::UnMacro(wxString name) {
	HashStringParserMacro::iterator it = parser->h_macros.find(name);
	if (it!=parser->h_macros.end() && it->second->cont.Len())
		return it->second->cont;
	return name;
}

wxString CodeHelper::UnMacro(wxString name, int &dims) {
	HashStringParserMacro::iterator it = parser->h_macros.find(name);
	if (it!=parser->h_macros.end() && it->second->cont.Len()) {
		name = it->second->cont;
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
					text+=_T("\n");
				text+=aux_func->full_proto;
			}
			aux_func = aux_func->next;
		}
	}
		
	if (t!=0)
		source->ShowCallTip(p-name.Len(),text);
	source->calltip_brace=p;
	return true;
	
}

int CodeHelper::AddReservedWords(wxArrayString &comp_list, wxString &typed, int max_str_dist) {
	int i, j, c = 0, l = typed.Len(), ll = reserved_words.GetCount();
	for (j=0;j<ll;j++) {
		CH_COMPARE(typed,reserved_words[j],i,l,max_str_dist);
		if (i==l) {
			c++;
			comp_list.Add(reserved_words[j]+_T("?15"));
		}
	}
	return c;
}

bool CodeHelper::GenerateCacheFile(wxString path, wxString filename) {
	
	wxFFile idx(DIR_PLUS_FILE(DIR_PLUS_FILE(config->home_dir,"autocomp"),filename),_T("w+"));
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
					idx.Write(wxString(_T("\tenum const "))<<PD_UNREF(pd_var,ref)->proto<<_T("\n"));
				else if (PD_UNREF(pd_var,ref)->properties==PD_CONST_CONST)
					idx.Write(wxString(_T("\tconst "))<<PD_UNREF(pd_var,ref)->proto<<_T("\n"));
				else
					idx.Write(wxString(_T("\t"))<<PD_UNREF(pd_var,ref)->proto<<_T("\n"));
			}
			ref = ref->next;
		}
		ref = fil->first_func_dec->next;
		while (ref) {
			if (PD_UNREF(pd_func,ref)->space==NULL)
				idx.Write(wxString(_T("\t"))<<PD_UNREF(pd_func,ref)->proto<<_T("\n"));
			ref = ref->next;
		}
		ref = fil->first_func_def->next;
		while (ref) {
			if (PD_UNREF(pd_func,ref)->space==NULL && PD_UNREF(pd_func,ref)->file_dec==NULL)
				idx.Write(wxString(_T("\t"))<<PD_UNREF(pd_func,ref)->proto<<_T("\n"));
			ref = ref->next;
		}
		ref = fil->first_macro->next;
		while (ref) {
			idx.Write(wxString(_T("\t"))<<PD_UNREF(pd_macro,ref)->proto<<_T("\n"));
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
			idx.Write(wxString(_T("\tclass "))<<cls->name<<herencias<<_T("\n"));
			pd_var *var = cls->first_attrib->next;
			while (var) {
				if (var->properties&PD_CONST_PUBLIC)
					idx.Write(wxString(_T("\t\t"))<<var->proto<<_T("\n"));
				else if (var->properties&PD_CONST_ENUM_CONST)
					idx.Write(wxString(_T("\t\tenum const "))<<var->proto<<_T("\n"));
				var = var->next;
			}
			pd_func *met = cls->first_method->next;
			while (met) {
				if (met->properties&PD_CONST_PUBLIC)
					idx.Write(wxString(_T("\t\t"))<<met->proto<<_T("\n"));
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
