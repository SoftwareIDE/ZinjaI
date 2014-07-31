#include "Parser.h"
#include "ids.h"

#include "parserData.h"
#include "ConfigManager.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxMainWindow.h"
#include "ProjectManager.h"

#define PARSER_PARTE(i) s.Mid(p[i]+1,p[i+1]-p[i]-1)
#define PARSER_PARTE_NB(i) s.Mid(p[i]+2,p[i+1]-p[i]-3)
#include <wx/wx.h>
#include <wx/txtstrm.h>
#include "DebugManager.h"
#include "mxDrawClasses.h"
#include "Language.h"
#include "mxCompiler.h"
#include "CodeHelper.h"
#include "execution_workaround.h"

Parser *parser;

Parser::Parser (mxMainWindow *mainwin) {
	data_age=0;
	on_end = NULL;
	wxString pd_aux;
	symbol_tree=mainwin->symbols_tree.treeCtrl;
	follow_includes=true;

	item_root = symbol_tree->AddRoot(_T("Simbolos encontrados"),0);
	item_macros = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_MACROS,"Macros y Otros"),0);
	item_globals = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_GLOBALS,"Variables Globales"),0);
	item_functions = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_FUNCTIONS,"Funciones"),0);
	item_classes = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_CLASSES,"Clases"),0);

	symbol_tree->ExpandAll();
	
	first_function = new pd_func(wxTreeItemId(), NULL, pd_aux, pd_aux, pd_aux, NULL);
	first_class = new pd_class(NULL, NULL, 0, pd_aux, pd_aux, NULL);
	first_file = new pd_file(NULL,pd_aux,wxDateTime::Now(),NULL);
	first_macro = new pd_macro(NULL,NULL,0,pd_aux,0,NULL);
	first_global = new pd_var(wxTreeItemId(), NULL,NULL,0,pd_aux,pd_aux,pd_aux,0,NULL);
	first_inherit = new pd_inherit(NULL,pd_aux,pd_aux,0,NULL);
	
	last_file = first_file;
	last_class = first_class;
	last_function = first_function;
	last_global = first_global;
	last_macro = first_macro;
	
	working=false;
	should_stop=false;
	
	process_timer = new wxTimer(mainwin->GetEventHandler(),mxID_PARSER_PROCESS_TIMER);

}


void Parser::ParseProject(bool show_progress) {
//	main_window->SetStatusText(wxString(_T("Analizando fuentes...")));
	home=project->path;
	wxString str;
//	project->SaveAll(false);

	for(int i=0;i<2;i++) { 
		LocalListIterator<project_file_item*> item(i==0?&project->files_sources:&project->files_headers);
		while(item.IsValid()) {
			actions.insert(actions.end(),parserAction::ParseProjectFile(project,DIR_PLUS_FILE(project->path,item->name),item->hide_symbols));
			item.Next();
		}
	}
	source=NULL;
		
//	if (project->use_wxfb && project->auto_wxfb) parser->OnEnd(POE_AUTOUPDATE_WXFB);

	Parse(show_progress);
}


void Parser::ParseSource(mxSource *src, bool dontsave) {
	if (src->lexer!=wxSTC_LEX_CPP) return;
	project_file_item *item=project?project->HasFile(src->source_filename):NULL;
	if (project && (!item || item->where==FT_OTHER)) return; // if we are in project mode and the source is not attached to the project, don't parse
	actions.insert(actions.end(),parserAction::ParseSource(src,dontsave));
	Parse();
}

bool Parser::ParseNextSource(mxSource *src, bool dontsave) {

	if (main_window->notebook_sources->GetPageIndex(src)==wxNOT_FOUND) 
		return false;
	
	source=src;
	source->never_parsed=false;
	home=source->source_filename.GetPath();

	if (source->sin_titulo) {
		if (!dontsave) source->SaveTemp();
		return ParseNextFileStart(source->temp_filename,source->temp_filename.GetFullPath());
	} else {
		if (source->GetModify() && !dontsave) {
			source->SaveTemp();
			return ParseNextFileStart(source->temp_filename,source->source_filename.GetFullPath());
		} else {
			return ParseNextFileStart(source->source_filename,source->source_filename.GetFullPath());
		}
	}
	return false; // nunca llega a esta linea
}


void Parser::OnSelectSymbol (wxTreeEvent &event, wxAuiNotebook *notebook) {
	wxTreeItemId item = event.GetItem();
	wxTreeItemId parent = symbol_tree->GetItemParent(item);
	if (parent!=item_root) {
		wxString key=symbol_tree->GetItemText(item);
		if (parent==item_macros) { // macro
			HashStringParserMacro::iterator it = h_macros.find(key);
			if (it!=h_macros.end() && it->second->file) {
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
				OnGotoDef(main_window->notebook_sources);
			}
		} else if (parent==item_classes) { // class
			HashStringParserClass::iterator it = h_classes.find(key);
			if (it!=h_classes.end() && it->second->file) {
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
				OnGotoDef(main_window->notebook_sources);
			}
		} else if (parent==item_globals) { // global
			HashStringParserVariable::iterator it = h_globals.find(key);
			if (it!=h_globals.end() && it->second->file) {
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
				OnGotoDef(main_window->notebook_sources);
			}
		} else if (parent==item_functions) { // function
			HashStringParserFunction::iterator it = h_functions.find(key);
			if (it!=h_functions.end()) {
				if (it->second->file_def) {
					popup_file_def = it->second->file_def->name;
					popup_line_def = it->second->line_def;
					OnGotoDef(main_window->notebook_sources);
				} else if (it->second->file_dec) {
					popup_file_dec = it->second->file_dec->name;
					popup_line_dec = it->second->line_dec;
					OnGotoDec(main_window->notebook_sources);
				}
			}
		} else {
			wxString key2=symbol_tree->GetItemText(parent);
			HashStringParserClass::iterator it2 = h_classes.find(key2);
			if (it2!=h_classes.end()) {
				HashStringParserVariable::iterator it = it2->second->h_attribs.find(key);
				if (it!=it2->second->h_attribs.end() && it->second->file) {
					popup_file_def = it->second->file->name;
					popup_line_def = it->second->line;
					OnGotoDef(main_window->notebook_sources);
				} else { // function
					HashStringParserFunction::iterator it = it2->second->h_methods.find(key);
					if (it!=it2->second->h_methods.end()) {
						if (it->second->file_def) {
							popup_file_def = it->second->file_def->name;
							popup_line_def = it->second->line_def;
							OnGotoDef(main_window->notebook_sources);
						} else if (it->second->file_dec) {
							popup_file_dec = it->second->file_dec->name;
							popup_line_dec = it->second->line_dec;
							OnGotoDec(main_window->notebook_sources);
						}
					}
				}
			}
		}
	}
}

void Parser::OnPopupMenu (wxTreeEvent &event, wxAuiNotebook *notebook) {
	wxTreeItemId item = event.GetItem();
	wxTreeItemId parent = symbol_tree->GetItemParent(item);
	wxMenu menu(""/*LANG(SYMBOLS_POPUP_TITLE,"<Visor de Simbolos>")*/);
	if (parent!=item_root) {
		wxString key=symbol_tree->GetItemText(item);
		
		if (parent==item_macros) { // macro
			HashStringParserMacro::iterator it = h_macros.find(key);
			if (it!=h_macros.end() && it->second->file) {
				menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
			}
		} else if (parent==item_classes) { // class
			HashStringParserClass::iterator it = h_classes.find(key);
			if (it!=h_classes.end() && it->second->file) {
				menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
			}
		} else if (parent==item_globals) { // global
			HashStringParserVariable::iterator it = h_globals.find(key);
			if (it!=h_globals.end() && it->second->file) {
				menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
				popup_file_def = it->second->file->name;
				popup_line_def = it->second->line;
			}
		} else if (parent==item_functions) { // function
			HashStringParserFunction::iterator it = h_functions.find(key);
			if (it!=h_functions.end()) {
				if (it->second->file_dec) {
					menu.Append(mxID_SYMBOL_POPUP_DEC, LANG(SYMBOLS_POPUP_GOTO_DEC,"Ir a De&claracion"));
					popup_file_dec = it->second->file_dec->name;
					popup_line_dec = it->second->line_dec;
				}
				if (it->second->file_def) {
					menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
					popup_file_def = it->second->file_def->name;
					popup_line_def = it->second->line_def;
				}
			}
		} else {
			wxString key2=symbol_tree->GetItemText(parent);
			HashStringParserClass::iterator it2 = h_classes.find(key2);
			if (it2!=h_classes.end()) {
				HashStringParserVariable::iterator it = it2->second->h_attribs.find(key);
				if (it!=it2->second->h_attribs.end() && it->second->file) {
					menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
					popup_file_def = it->second->file->name;
					popup_line_def = it->second->line;
				} else { // function
					HashStringParserFunction::iterator it = it2->second->h_methods.find(key);
					if (it!=it2->second->h_methods.end()) {
						if (it->second->file_dec) {
							menu.Append(mxID_SYMBOL_POPUP_DEC, LANG(SYMBOLS_POPUP_GOTO_DEC,"Ir a De&claracion"));
							popup_file_dec = it->second->file_dec->name;
							popup_line_dec = it->second->line_dec;
						}
						if (it->second->file_def) {
							menu.Append(mxID_SYMBOL_POPUP_DEF, LANG(SYMBOLS_POPUP_GOTO_DEF,"Ir a De&finicion"));
							popup_file_def = it->second->file_def->name;
							popup_line_def = it->second->line_def;
						}
					}
				}
			}
		}
		menu.AppendSeparator();
	}
	menu.Append(mxID_VIEW_UPDATE_SYMBOLS, LANG(SYMBOLS_POPUP_UPDATE_TREE,"&Actualizar arbol"));
	menu.Append(mxID_SYMBOL_GENERATE_CACHE, LANG(SYMBOLS_POPUP_GENERATE_INDEX,"&Generar indice para autocompletado..."));
	menu.Append(mxID_EDIT_GOTO_FUNCTION, LANG(MENUITEM_EDIT_FIND,"&Buscar..."));
	
//	menu.AppendCheckItem(mxID_SYMBOL_POPUP_INCLUDES, _T("&Seguir includes"))->Check(follow_includes);
	
	symbol_tree->PopupMenu( &menu );	
}

void Parser::OnGotoDec(wxAuiNotebook *notebook) {
	wxFileName the_one = popup_file_dec;
	int line = popup_line_dec;
	for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++) {
		mxSource *src=((mxSource*)(main_window->notebook_sources->GetPage(i)));
		if ((!src->sin_titulo && src->source_filename==the_one) || (src->temp_filename==the_one && src==parser->source)) {
			main_window->notebook_sources->SetSelection(i);
			((mxSource*)main_window->notebook_sources->GetPage(i))->MarkError(line-1);
			return;
		}
	}
	// si no esta abierto
//	int res = mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxYES_NO|mxMD_QUESTION).ShowModal();
//	if (res == mxMD_YES)
	mxSource *src=main_window->OpenFile(the_one.GetFullPath());
	if (src && source!=EXTERNAL_SOURCE) src->MarkError(line-1);
}

void Parser::OnGotoDef(wxAuiNotebook *notebook) {
	wxFileName the_one = popup_file_def;
	int line = popup_line_def;
	for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++) {
		mxSource *src = ((mxSource*)(main_window->notebook_sources->GetPage(i)));
		if ((!src->sin_titulo && src->source_filename==the_one) || (src->temp_filename==the_one && src==parser->source)) {
			main_window->notebook_sources->SetSelection(i);
			((mxSource*)main_window->notebook_sources->GetPage(i))->MarkError(line-1);
			return;
		}
	}
	mxSource *src=main_window->OpenFile(the_one.GetFullPath());
	if (src && source!=EXTERNAL_SOURCE) src->MarkError(line-1);
}



void Parser::Stop(bool clean_end) {
	if (clean_end) 
		while (on_end) {
			OnEndAction *aux=on_end->next;
			delete on_end;
			on_end=aux;
		}
	should_stop=true;
	actions.clear();
}

Parser::~Parser() {
	Stop();
}

wxString Parser::JoinNames(wxString types, wxString names) {
	int l1=types.Len(), l2=names.Len(),i1=0,i2=0,p1=0,p2=0;
	wxString res;
	while (true) {
		while (i1<l1 && types[i1]!=',' && types[i1]!='[') i1++;
		while (i2<l2 && names[i2]!=',') i2++;
		res<<types.Mid(p1,i1-p1)<<" "<<names(p2,i2-p2);
		if(i1<l1 && types[i1]!=',') {
			p1=i1;
			while (i1<l1 && types[i1]!=',') i1++;
			res<<types.Mid(p1,i1-p1);
		}
		if (i1<l1) res<<", "; else break;
		p2=i2+1; p1=i1+1;
		i1=p1; i2=p2;
	}
	return res;
}


long Parser::ParseNextFileStart(wxFileName filename, wxString HashName, bool hide_symbols) {
	if (!filename.FileExists()) return false;
	if (!compiler->IsCompiling())
		main_window->SetStatusText(wxString(LANG(PARSER_PARSING_FILE,"Analizando"))<<" \""<<HashName<<"\"...");
	process=new mxParserProcess;
	PD_REGISTER_FILE(process->file, HashName,filename.GetModificationTime());
	process->file->hide_symbols=hide_symbols;
	return wxExecute(wxString(config->Files.parser_command)<<" \""<<filename.GetFullPath()<<"\"",wxEXEC_ASYNC,process);
}

void Parser::ParseNextFileContinue(const wxString &s) {
	
	// toda linea de cbrowser empieza con un entero indicando que es, de 1 o 2 digitos, seguido por punto y coma
	int id, p[15];
	if (s.GetChar(1)==';') {
		id = s.GetChar(0)-'0';
		p[0]=1;
	} else if (s.GetChar(2)==';') {
		id = (s.GetChar(0)-'0')*10+s.GetChar(1)-'0';
		p[0]=2;
	} else return;
	// "cortar" las partes de la linea, separadas por ';' o 1. En p[i] se guardan las posiciones de estos caracteres de separacion
	int ii, jj, kk;
	for (ii=p[0]+1, jj=s.Len(), kk=1;ii<jj;ii++) {
		if (s[ii]==';' || s[ii]==1) {
			p[kk++]=ii;
			if (kk==10) break;
		}
	}
	p[kk]=ii;
	
	long int line;
	// segun el id, ver que significa
	switch (id) {
			
		case PAF_ENUM_DEF: {
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			wxString scope=PARSER_PARTE_NB(5), name=PARSER_PARTE(0), type="enum";
			if (scope=="") {
				PD_REGISTER_MACRO(process->file,line,name,type,name,PD_CONST_ENUM);
//				PD_REGISTER_GLOBAL(process->file,line,type,name,PD_CONST_ENUM);
			} else {
				PD_REGISTER_ATTRIB(process->aux_class,process->file,line,scope,type,name,PD_CONST_ENUM);
			}
			break;
		}
			
		case PAF_ENUM_CONST_DEF: {
			PARSER_PARTE(2).BeforeFirst('.').ToLong(&line);
			wxString scope=PARSER_PARTE(0), name=PARSER_PARTE(1), type=PARSER_PARTE_NB(6);
			if (scope=="-") {
				PD_REGISTER_MACRO(process->file,line,name,type,name,PD_CONST_ENUM_CONST);
			} else {
				PD_REGISTER_ATTRIB(process->aux_class,process->file,line,scope,type,name,PD_CONST_ENUM_CONST);
			}
			break;
		}
			
		case PAF_TYPE_DEF:
		case PAF_MACRO_DEF: {
			wxString name=PARSER_PARTE(0),aux,key;
			int props=PD_CONST_MACRO_CONST;
			if (id==PAF_MACRO_DEF) {
				aux=PARSER_PARTE_NB(5); // "-" indica que es typedef o macro de una linea (el parser da el valor, para UnMacro)
				if (aux!="-") props=PD_CONST_MACRO_FUNC;
				key=PARSER_PARTE_NB(6); 
			} else { 
				props=PD_CONST_TYPEDEF;
				aux="-"; // "-" indica que es typedef o macro de una linea (el parser da el valor, para UnMacro)
				key=PARSER_PARTE_NB(5);
			}
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			PD_REGISTER_MACRO(process->file,line,name,key,aux,props);
			break;
		}
			
		case PAF_FUNC_DEF: {
			wxString key=PARSER_PARTE(0);
			if (key=="NULL") break;
			if (key[0]=='<') key=key.AfterLast('>');
			wxString name=PARSER_PARTE_NB(5)+" "+key+"("+PARSER_PARTE_NB(6)+")";
			wxString fullproto=PARSER_PARTE_NB(5)+" "+key+"("+JoinNames(PARSER_PARTE_NB(6),PARSER_PARTE_NB(7))+")";
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			PD_REGISTER_FUNCTION_DEF(process->file,line,key,name,fullproto);
			break;
		}
			
		case PAF_FUNC_DCL: {
			wxString key=PARSER_PARTE(0);
			if (key=="NULL") break;
			if (key[0]=='<') key=key.AfterLast('>');
			wxString name=PARSER_PARTE_NB(5)+" "+key+"("+PARSER_PARTE_NB(6)+")";
			wxString fullproto=PARSER_PARTE_NB(5)+" "+key+"("+JoinNames(PARSER_PARTE_NB(6),PARSER_PARTE_NB(7))+")";
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			PD_REGISTER_FUNCTION_DEC(process->file,line,key,name,fullproto);
			break;
		}
			
		case PAF_CLASS_DEF:
		case PAF_UNION_DEF: {
			wxString name=PARSER_PARTE(0);
			wxString key=PARSER_PARTE_NB(6);
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			PD_REGISTER_CLASS_DEF(process->aux_class,process->file,line,name,key,id==PAF_UNION_DEF);
			break;
		}
			
		case PAF_CONS_DEF: 
		case PAF_GLOB_VAR_DEF: {
			wxString name=PARSER_PARTE(0);
			PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
			wxString key=PARSER_PARTE_NB(5);
			if (key=="NULL") break;
			unsigned int props=0;
			if (key.Left(9)==_T("volatile ")) {
				key=key.Mid(9);
				props=PD_CONST_VOLATILE;
			}
			if (id==PAF_CONS_DEF) props=props|PD_CONST_CONST;
			if (!PARSER_PARTE_NB(6).Len()) {
				PD_REGISTER_GLOBAL(process->file,line,key,name,props);
			}
			break;
		}
			
		case PAF_MBR_FUNC_DEF: {
			wxString aux=PARSER_PARTE(0);
			wxString key=PARSER_PARTE_NB(6);
			unsigned int props=0;
			PARSER_PARTE(2).ToLong(&line);
			wxString name=PARSER_PARTE(1);
			wxString par=PARSER_PARTE_NB(7);
			wxString fullpar=JoinNames(par,PARSER_PARTE_NB(8));
			if (name[0]=='<') name=name.AfterLast('>');
			if (name[0]=='~')
				props|=PD_CONST_DESTRUCTOR;
			else if (name==aux)
				props=PD_CONST_CONSTRUCTOR;
			else if (key=="NULL") break;
			int dectype=0, auxtype=1, postype=p[6]-1;
			while (s[postype]!='x'&&s[postype]!=1) {
				if (s[postype]>='a')
					dectype+=(s[postype]-'a'+10)*auxtype; 
				else
					dectype+=(s[postype]-'0')*auxtype; 
				postype--;
				auxtype*=16;
			}
			if (dectype&PAF_VIRTUAL) { props|=PD_CONST_VIRTUAL; key.Replace("virtual ","",false); }
			if (dectype&PAF_PRIVATE) props|=PD_CONST_PRIVATE;
			else if (dectype&PAF_PROTECTED) props|=PD_CONST_PROTECTED;
			else props|=PD_CONST_PUBLIC;
			if (dectype&PAF_STATIC) props|=PD_CONST_STATIC;
			if (dectype&PAF_CONST_OR_VOLATILE) props|=PD_CONST_CONST;
			PD_REGISTER_METHOD_DEF(process->aux_class,process->file,line,key,aux,name,par,fullpar,props);
			break;
		}
			
		case PAF_MBR_FUNC_DCL: {
			wxString aux=PARSER_PARTE(0);
			wxString key=PARSER_PARTE_NB(6);
			unsigned int props=0;
			PARSER_PARTE(2).ToLong(&line);
			wxString name=PARSER_PARTE(1);
			wxString par=PARSER_PARTE_NB(7);
			wxString fullpar=JoinNames(par,PARSER_PARTE_NB(8));
			if (name[0]=='<') name=name.AfterLast('>');
			if (name[0]=='~')
				props|=PD_CONST_DESTRUCTOR;
			else if (name==aux)
				props=PD_CONST_CONSTRUCTOR;
			else if (key=="NULL") break;
			int dectype=0, auxtype=1, postype=p[6]-1;
			while (s[postype]!='x'&&s[postype]!=1) {
				if (s[postype]>='a')
					dectype+=(s[postype]-'a'+10)*auxtype; 
				else
					dectype+=(s[postype]-'0')*auxtype; 
				postype--;
				auxtype*=16;
			}
			if (dectype&PAF_VIRTUAL) { props|=PD_CONST_VIRTUAL; key.Replace("virtual ","",false); }
			if (dectype&PAF_VIRTUAL_PURE) props|=PD_CONST_VIRTUAL_PURE;
			if (dectype&PAF_PRIVATE) props|=PD_CONST_PRIVATE;
			else if (dectype&PAF_PROTECTED) props|=PD_CONST_PROTECTED;
			else props|=PD_CONST_PUBLIC;
			if (dectype&PAF_STATIC) props|=PD_CONST_STATIC;
			if (dectype&PAF_CONST_OR_VOLATILE) props|=PD_CONST_CONST;
			PD_REGISTER_METHOD_DEC(process->aux_class,process->file,line,key,aux,name,par,fullpar,props);
			break;
		}
			
		case PAF_MBR_VAR_DEF: {
			wxString aux=PARSER_PARTE(0);
			wxString name=PARSER_PARTE(1);
			PARSER_PARTE(2).ToLong(&line);
			wxString key=PARSER_PARTE_NB(6);
			if (key=="NULL") break;
			unsigned int props=0;
			if (key.Left(9)==_T("volatile ")) {
				key=key.Mid(9);
				props=PD_CONST_VOLATILE;
			}
			int dectype=0, auxtype=1, postype=p[6]-1;
			while (s[postype]!='x'&&s[postype]!=1) {
				if (s[postype]>='a')
					dectype+=(s[postype]-'a'+10)*auxtype; 
				else
					dectype+=(s[postype]-'0')*auxtype; 
				postype--;
				auxtype*=16;
			}
			if (dectype&PAF_PRIVATE) props|=PD_CONST_PRIVATE;
			else if (dectype&PAF_PROTECTED) props|=PD_CONST_PROTECTED;
			else props|=PD_CONST_PUBLIC;
			if (dectype&PAF_STATIC) props|=PD_CONST_STATIC;
			if (dectype&PAF_CONST_OR_VOLATILE) props|=PD_CONST_VOLATILE;
			PD_REGISTER_ATTRIB(process->aux_class,process->file,line,aux,key,name,props);
			break;
		}
	
		case PAF_INCLUDE_DEF: {
	//				if (follow_includes) {
	//					wxFileName inc_file(DIR_PLUS_FILE(my_home,PARSER_PARTE(0)));
	//					if (inc_file.FileExists()) {
	//						parser_filename *it=first_file;
	//						bool found=first_file->name==inc_file;
	//						ML_ITERATE(it)
	//							if (found=(it->name==inc_file)) 
	//							break;
	//						if (!found)
	//							ParseNextFile(it->next=new parser_filename(inc_file));
	//					}
	//				}
			break;
		}
	
		case PAF_CLASS_INHERIT: {
			wxString name=PARSER_PARTE(1);
			wxString key=PARSER_PARTE(0);
			code_helper->UnMacro(name);
			code_helper->UnTemplate(name);
			switch (s[p[5]+3]) {
			case '4':
				PD_REGISTER_INHERIT(process->file,name,key,PD_CONST_PUBLIC);
				break;
			case '2':
				PD_REGISTER_INHERIT(process->file,name,key,PD_CONST_PROTECTED);
				break;
			case '1':
				PD_REGISTER_INHERIT(process->file,name,key,PD_CONST_PRIVATE);
				break;
			default:
				PD_REGISTER_INHERIT(process->file,name,key,0);
				break;
			}
			break;
		}
	}	
}

void Parser::ParseNextFileEnd() { ///< to be called when an asyn process from ParseNextFile ends
	process->file->Purge();
	delete process; process=NULL;
	ParseSomething(false);
}

void Parser::ParseDeleteFile(wxString fname) {
	PD_DELETE_FILE(fname);
}

bool Parser::RemoveFile(wxString fname) {
	actions.insert(actions.end(),parserAction::DeleteFile(fname));
	Parse();
	return true;
}

bool Parser::RenameFile(wxString oldname, wxString newname) {
	PD_RENAME_FILE(oldname,newname);
	return true;
}

void Parser::Parse(bool show_progress) {
	if (!working) ParseSomething(true,show_progress);
}

/// if first==true, then consider arg_show_progress, else consider last valid arg_show_progress
void Parser::ParseSomething(bool first, bool arg_show_progress) {
	
	working = true;
	
	static bool show_progress=false;
	static int progress_total=0;
	if (first) {
		show_progress=arg_show_progress;
		if (show_progress) {
			list<parserAction>::iterator it=actions.begin();
			while (it!=actions.end()) { ++progress_total; ++it; }
			if (progress_total>0) main_window->SetStatusProgress(0);
		}
	}
	
	if (!actions.empty()) {
		int progress_now=0;
		bool async=false; // si un paso lanza un proceso asincrono, esta bandera lo indica, para cortar el loop y esperar el evento terminate de ese proceso
		do {
			data_age++;
			list<parserAction>::iterator it=actions.begin();
			parserAction pa=*it; actions.erase(it);
			if (debug->IsDebugging() || should_stop) {
				working=false;
	//			symbol_tree->Thaw();
				if (show_progress) main_window->SetStatusProgress(-1);
				return;
			}
			switch(pa.action_type) {
			case PA_CLEAR_ALL:
				ParseNextCleanAll();
				break;
			case PA_PARSE_PROJECT_FILE:
				if (pa.ptr==project) { 
					if (ParseNextFileStart(pa.str,pa.str,pa.flag)) async=true;
				}
				break;
			case PA_PARSE_SAVED_FILE:
				if (ParseNextFileStart(pa.str,pa.str)) async=true;
				break;
			case PA_DELETE_FILE:
				ParseDeleteFile(pa.str);
				break; // este break faltaba, era adrede o error?
			case PA_SET_HIDE_SYMBOLS:
				ParseSetHideSymbols(pa.str,pa.flag);
				break; // este break faltaba, era adrede o error?
			case PA_PARSE_OPENED_SOURCE:
				if (ParseNextSource((mxSource*)pa.ptr,pa.flag)) async=true;
				break;
			default:;
			}
			if (show_progress) { main_window->SetStatusProgress((100*(++progress_now))/progress_total); }
		} while (!async && !actions.empty());
		if (!async) ParseSomething(false,false);
	} else {
		working = false;
		symbol_tree->SortChildren(item_classes);
		symbol_tree->SortChildren(item_globals);
		symbol_tree->SortChildren(item_macros);
		symbol_tree->SortChildren(item_functions);
	//	symbol_tree->Expand(item_functions);
	//	symbol_tree->Expand(item_globals);
	//	symbol_tree->Expand(item_macros);
	//	symbol_tree->Expand(item_classes);
	//	symbol_tree->Thaw();
		if (main_window && !compiler->IsCompiling())
			main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
		// se va a operar sobre una "copia" de on_end, para evitar que una de estas acciones 
		// se introduzca a sí misma en esta lista y genere un loop infinito (pasaba al crear
		// ventanas hijas al volver el foco desde wxfb)
		OnEndAction *aux_oe = on_end; on_end=NULL;
		while (aux_oe) {
			OnEndAction *aux_i=aux_oe;
			aux_oe=aux_oe->next;
			aux_i->Do();
			delete aux_i;
		}
		if (show_progress) main_window->SetStatusProgress(-1);
	}
	
}

void Parser::CleanAll() {
	actions.clear();
	actions.insert(actions.end(),parserAction::ClearAll());
	Parse();
}

bool Parser::ParseNextCleanAll() {
	pd_file *item = last_file->next, *aux;
	while (item) {
		aux = item->next;
		if (item->prev) 
			PD_DELETE_FILE(item->name);
		item=aux;
	}
	last_file->next=NULL;
	last_class->next=NULL;
	last_global->next=NULL;
	last_macro->next=NULL;
	last_function->next=NULL;
	return true;
}

void Parser::ParseFile(wxString filename) {
	wxFileName fname(filename);
	fname.Normalize();
	actions.insert(actions.end(),parserAction::ParseSavedFile(fname.GetFullPath()));
	Parse();
}

void Parser::OnEnd(OnEndAction *what, bool run_now_if_not_working) {
	if (run_now_if_not_working && !parser->working) {
		what->Do();
		delete what;
	} else {
		what->next=on_end;
		on_end=what;
	}
}

void Parser::UnregisterSource(mxSource *src) {
	list<parserAction>::iterator it=actions.begin();
	while (it!=actions.end()) {
		if (it->action_type==PA_PARSE_OPENED_SOURCE && !(it==actions.begin() && parser->working)) {
			if (src->sin_titulo)
				it->action_type=PA_NULL;
			else {
				it->action_type=PA_PARSE_SAVED_FILE;
				it->str=src->source_filename.GetFullPath();
			}
		}
		++it;
	}
}

pd_file *Parser::GetFile(wxString name) {
	HashStringParserFile::iterator it = h_files.find(name);
	if (it!=h_files.end())
		return it->second;
	else 
		return NULL;
}

pd_class *Parser::GetClass(wxString name) {
	HashStringParserClass::iterator it = h_classes.find(name);
	if (it!=h_classes.end())
		return it->second;
	else 
		return NULL;
}

void Parser::ParseIfUpdated(wxFileName fname) {
	if (fname.FileExists()) {
		pd_file *pdf = parser->GetFile(fname.GetFullPath());
		if (pdf && pdf->time!=fname.GetModificationTime())
			parser->ParseFile(fname.GetFullPath());
	}
}

bool Parser::GetFather(wxString child, pd_inherit *&father) {
	if (!father) father=first_inherit; else father=father->next;
	while (father && father->son!=child) father=father->next;
	return father;
}

void mxParserProcess::OnTerminate (int pid, int status) {
	parser->process_timer->Stop();
	if (parser->should_stop) return;
	ParseOutput();
	parser->ParseNextFileEnd();
}

void mxParserProcess::ParseOutput ( ) {
	wxTextInputStream input(*GetInputStream());
	while ( IsInputAvailable() ) {
		parser->ParseNextFileContinue(input.ReadLine());
	}
}

void Parser::OnParserProcessTimer() {
	process->ParseOutput();
}

bool Parser::ParseSetHideSymbols (wxString filename, bool hide_symbols) {
	pd_file *file = GetFile(filename);
	if (file) file->hide_symbols=hide_symbols;
	return file;
}

void Parser::SetHideSymbols(wxString filename, bool hide_symbols) {
	if (!ParseSetHideSymbols(filename,hide_symbols))
		actions.insert(actions.end(),parserAction::SetHideSymbols(filename,hide_symbols));
}

