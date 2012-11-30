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
#include "DebugManager.h"
#include "mxDrawClasses.h"
#include "Language.h"
#include "mxCompiler.h"
#include "CodeHelper.h"

Parser *parser;
//wxTreeCtrl* symbol_tree;

Parser::Parser (wxTreeCtrl *tree) {
	on_end = POE_NONE;
	wxString pd_aux;
	symbol_tree=tree;
	follow_includes=true;

	item_root = symbol_tree->AddRoot(_T("Simbolos encontrados"),0);
	item_macros = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_MACROS,"Macros y Typedefs"),0);
	item_globals = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_GLOBALS,"Variables Globales"),0);
	item_functions = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_FUNCTIONS,"Funciones"),0);
	item_classes = symbol_tree->AppendItem(item_root,LANG(SYMBOLS_CLASSES,"Clases"),0);

	symbol_tree->ExpandAll();
	
	first_function = new pd_func(wxTreeItemId(), NULL, pd_aux, pd_aux, pd_aux, NULL);
	first_class = new pd_class(NULL, NULL, 0, pd_aux, pd_aux, NULL);
	first_file = new pd_file(NULL,pd_aux,wxDateTime::Now(),NULL);
	first_macro = new pd_macro(NULL,NULL,0,pd_aux,NULL);
	first_global = new pd_var(wxTreeItemId(), NULL,NULL,0,pd_aux,pd_aux,pd_aux,NULL);
	first_inherit = new pd_inherit(NULL,pd_aux,pd_aux,0,NULL);
	
	last_file = first_file;
	last_class = first_class;
	last_function = first_function;
	last_global = first_global;
	last_macro = first_macro;
	
	working=false;
	should_stop=false;

}


void Parser::ParseProject(bool show_progress) {
//	main_window->SetStatusText(wxString(_T("Analizando fuentes...")));
	home=project->path;
	file_item *item;
	wxString str;
//	project->SaveAll(false);

	item=project->first_source;
	ML_ITERATE(item) {
		actions.insert(actions.end(),parserAction(project,DIR_PLUS_FILE(project->path,item->name)));
	}
	
//	if (!follow_includes) {
		item=project->first_header;
		ML_ITERATE(item) {
			actions.insert(actions.end(),parserAction(project,DIR_PLUS_FILE(project->path,item->name)));
		}
//	}

	source=NULL;
		
	if (project->use_wxfb && project->auto_wxfb) parser->OnEnd(POE_AUTOUPDATE_WXFB);

	if (!working) Parse(show_progress);
}


void Parser::ParseSource(mxSource *src, bool dontsave) {
	if (src->lexer!=wxSTC_LEX_CPP) return;
	if (project) {
		if (project->HasFile(src->source_filename))
			actions.insert(actions.end(),parserAction(src,dontsave));
	} else {
		actions.insert(actions.end(),parserAction(src,dontsave));
	}
	if (!working) Parse();
}

bool Parser::ParseNextSource(mxSource *src, bool dontsave) {

	if (main_window->notebook_sources->GetPageIndex(src)==wxNOT_FOUND) 
		return false;
	
	source=src;
	source->never_parsed=false;
	home=source->source_filename.GetPath();

	if (source->sin_titulo) {
		if (!dontsave)
			source->SaveTemp();
		ParseNextFile(source->temp_filename,source->temp_filename.GetFullPath());
	} else {
		if (source->GetModify() && !dontsave) {
			source->SaveTemp();
			ParseNextFile(source->temp_filename,source->source_filename.GetFullPath());
		} else {
			ParseNextFile(source->source_filename,source->source_filename.GetFullPath());
		}
	}
	return true;
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
	mxSource *src=main_window->OpenFile(the_one.GetFullPath(),!project);
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
	// si no esta abierto
//	int res = mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
//	if (res == mxMD_YES)
	mxSource *src=main_window->OpenFile(the_one.GetFullPath(),!project);
	if (src && source!=EXTERNAL_SOURCE) src->MarkError(line-1);
}



void Parser::Stop() {
//	if (!working) return;
	should_stop=true;
	actions.clear();
//	while (working) {
//		wxYield();
//	}
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


bool Parser::ParseNextFile(wxFileName filename, wxString HashName) {
	if (!compiler->IsCompiling())
		main_window->SetStatusText(wxString(LANG(PARSER_PARSING_FILE_PRE,"Analizando \""))<<HashName<<LANG(PARSER_PARSING_FILE_POST,"\"..."));
	pd_file *file;
	pd_class *aux_class=NULL;
	PD_REGISTER_FILE(file, HashName,filename.GetModificationTime());
	if (!filename.FileExists()) { file->Purge(); return false; }
	unsigned int props;
	wxString my_home=DIR_PLUS_FILE(home,filename.GetPath());
	wxArrayString output;
	wxExecute(wxString(config->Files.parser_command)<<_T(" \"")<<filename.GetFullPath()<<_T("\""), output, wxEXEC_SYNC|wxEXEC_NODISABLE);
	long int id, line;
	wxString s,name,aux,key,par,fullproto, fullpar;
	int p[15], ii, jj, kk;
	int dectype, postype, auxtype; // para las flags de private, public,protected, static y const en miembros
	// int ch_ini; bool ch_bool;
	for (int i=0, j=output.GetCount();i<j;i++) {
		s=output[i];
		if (s.GetChar(1)==';') {
			id = s.GetChar(0)-'0';
			p[0]=1;
		} else if (s.GetChar(2)==';') {
			id = (s.GetChar(0)-'0')*10+output[i].GetChar(1)-'0';
			p[0]=2;
		} else id=-1;
		if (id!=-1) {
			for (ii=p[0]+1, jj=s.Len(), kk=1;ii<jj;ii++) {
				if (s[ii]==';' || s[ii]==1) {
					p[kk++]=ii;
					if (kk==10) break;
				}
			}
			p[kk]=ii;
			switch (id) {
			case PAF_TYPE_DEF:
			case PAF_MACRO_DEF:
				name=PARSER_PARTE(0);
				if (id==PAF_MACRO_DEF) {
					aux=PARSER_PARTE_NB(5); // "-" indica que es typedef o macro de una linea (el parser da el valor, para UnMacro)
					key=PARSER_PARTE_NB(6); 
				} else { 
					aux="-"; // "-" indica que es typedef o macro de una linea (el parser da el valor, para UnMacro)
					key=PARSER_PARTE_NB(5);
				}
				PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
				PD_REGISTER_MACRO(file,line,name,key,aux,id==PAF_TYPE_DEF);
				break;
			case PAF_FUNC_DEF:
				key=PARSER_PARTE(0);
				if (key=="NULL") break;
				if (key[0]=='<') key=key.AfterLast('>');
				name=PARSER_PARTE_NB(5)+_T(" ")+key+_T("(")+PARSER_PARTE_NB(6)+_T(")");
				fullproto=PARSER_PARTE_NB(5)+_T(" ")+key+_T("(")+JoinNames(PARSER_PARTE_NB(6),PARSER_PARTE_NB(7))+_T(")");
				PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
				PD_REGISTER_FUNCTION_DEF(file,line,key,name,fullproto);
				break;
			case PAF_FUNC_DCL:
				key=PARSER_PARTE(0);
				if (key=="NULL") break;
				if (key[0]=='<') key=key.AfterLast('>');
				name=PARSER_PARTE_NB(5)+_T(" ")+key+_T("(")+PARSER_PARTE_NB(6)+_T(")");
				fullproto=PARSER_PARTE_NB(5)+_T(" ")+key+_T("(")+JoinNames(PARSER_PARTE_NB(6),PARSER_PARTE_NB(7))+_T(")");
				PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
				PD_REGISTER_FUNCTION_DEC(file,line,key,name,fullproto);
				break;
			case PAF_CLASS_DEF:
			case PAF_UNION_DEF:
				name=PARSER_PARTE(0);
				key=PARSER_PARTE_NB(6);
				PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
				PD_REGISTER_CLASS_DEF(aux_class,file,line,name,key,id==PAF_UNION_DEF);
				break;
			case PAF_GLOB_VAR_DEF:
				name=PARSER_PARTE(0);
				PARSER_PARTE(1).BeforeFirst('.').ToLong(&line);
				key=PARSER_PARTE_NB(5);
				if (key=="NULL") break;
				props=0;
				if (key.Left(9)==_T("volatile ")) {
					key=key.Mid(9);
					props=PD_CONST_VOLATILE;
				}
				if (!PARSER_PARTE_NB(6).Len()) {
					PD_REGISTER_GLOBAL(file,line,key,name,props);
				}
				break;
			case PAF_MBR_FUNC_DEF:
				aux=PARSER_PARTE(0);
				key=PARSER_PARTE_NB(6);
				props=0;
				PARSER_PARTE(2).ToLong(&line);
				name=PARSER_PARTE(1);
				par=PARSER_PARTE_NB(7);
				fullpar=JoinNames(par,PARSER_PARTE_NB(8));
				if (name[0]=='<') name=name.AfterLast('>');
				if (name[0]=='~')
					props|=PD_CONST_DESTRUCTOR;
				else if (name==aux)
					props=PD_CONST_CONSTRUCTOR;
				else if (key=="NULL") break;
				dectype=0; auxtype=1; postype=p[6]-1;
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
				PD_REGISTER_METHOD_DEF(aux_class,file,line,key,aux,name,par,fullpar,props);
				break;
			case PAF_MBR_FUNC_DCL:
				aux=PARSER_PARTE(0);
				key=PARSER_PARTE_NB(6);
				props=0;
				PARSER_PARTE(2).ToLong(&line);
				name=PARSER_PARTE(1);
				par=PARSER_PARTE_NB(7);
				fullpar=JoinNames(par,PARSER_PARTE_NB(8));
				if (name[0]=='<') name=name.AfterLast('>');
				if (name[0]=='~')
					props|=PD_CONST_DESTRUCTOR;
				else if (name==aux)
					props=PD_CONST_CONSTRUCTOR;
				else if (key=="NULL") break;
				dectype=0; auxtype=1; postype=p[6]-1;
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
				PD_REGISTER_METHOD_DEC(aux_class,file,line,key,aux,name,par,fullpar,props);
				break;
			case PAF_MBR_VAR_DEF:
				aux=PARSER_PARTE(0);
				name=PARSER_PARTE(1);
				PARSER_PARTE(2).ToLong(&line);
				key=PARSER_PARTE_NB(6);
				if (key=="NULL") break;
				props=0;
				if (key.Left(9)==_T("volatile ")) {
					key=key.Mid(9);
					props=PD_CONST_VOLATILE;
				}
				dectype=0; auxtype=1; postype=p[6]-1;
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
				PD_REGISTER_ATTRIB(aux_class,file,line,aux,key,name,props);
				break;
			case PAF_INCLUDE_DEF: 
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
			case PAF_CLASS_INHERIT:
				name=PARSER_PARTE(1);
				key=PARSER_PARTE(0);
				code_helper->UnMacro(name);
				code_helper->UnTemplate(name);
				switch (s[p[5]+3]) {
				case '4':
					PD_REGISTER_INHERIT(file,name,key,PD_CONST_PUBLIC);
					break;
				case '2':
					PD_REGISTER_INHERIT(file,name,key,PD_CONST_PROTECTED);
					break;
				case '1':
					PD_REGISTER_INHERIT(file,name,key,PD_CONST_PRIVATE);
					break;
				default:
					PD_REGISTER_INHERIT(file,name,key,0);
					break;
				}
				break;
			}
			
		}
	}
	file->Purge();
	
	return true;
	
}

void Parser::ParseDeleteFile(wxString fname) {
	PD_DELETE_FILE(fname);
}

bool Parser::RemoveSource(mxSource *source) {
	if (source->sin_titulo) {
		actions.insert(actions.end(),parserAction(source->temp_filename.GetFullPath(),'d'));
	} else {
		actions.insert(actions.end(),parserAction(source->source_filename.GetFullPath(),'d'));
	}
	if (!working) Parse();
	return true;
}

bool Parser::RemoveFile(wxString fname) {
	actions.insert(actions.end(),parserAction(fname,'d'));
	if (!working) Parse();
	return true;
}

bool Parser::RenameSource(mxSource *source, wxFileName newname) {
	if (source->sin_titulo) {
		PD_RENAME_FILE(source->temp_filename.GetFullPath(),newname.GetFullPath());
	} else {
		PD_RENAME_FILE(source->source_filename.GetFullPath(),newname.GetFullPath());
	}
	return true;
}

bool Parser::RenameFile(wxString oldname, wxString newname) {
	PD_RENAME_FILE(oldname,newname);
	return true;
}

bool Parser::Parse(bool show_progress) {
	
	working = true;
	int progress_total=0,progress_now=0;
	if (show_progress) {
		list<parserAction>::iterator it=actions.begin();
		while (it!=actions.end()) { ++progress_total; ++it; }
		if (progress_total>0) main_window->SetStatusProgress(0);
	}
	
	list<parserAction>::iterator it;
	parserAction pa;
//	symbol_tree->Freeze();
	while ((it=actions.begin())!=actions.end()) {
		pa=*it; actions.erase(it);
		if (debug->debugging || should_stop) {
			working=false;
//			symbol_tree->Thaw();
			if (show_progress) main_window->SetStatusProgress(-1);
			return false;
		}
		switch(pa.action) {
		case 'c':
			ParseNextCleanAll();
			break;
		case 'p':
			if (pa.ptr==project)
				ParseNextFile(pa.str,pa.str);
			break;
		case 'f':
			ParseNextFile(pa.str,pa.str);
			break;
		case 'd':
			ParseDeleteFile(pa.str);
		case 's':
			ParseNextSource((mxSource*)pa.ptr,pa.flag);
			break;
		}
		if (show_progress) { main_window->SetStatusProgress((100*(++progress_now))/progress_total); }
	}
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
	switch (on_end) {
	case POE_AUTOUPDATE_WXFB:
		if (project) project->WxfbAutoCheck();
		break;
	case POE_DRAWCLASSES:
		new mxDrawClasses;
		break;
	default:
		;
	}
	if (show_progress) main_window->SetStatusProgress(-1);
	on_end = POE_NONE;
	return true;
}

void Parser::CleanAll() {
	actions.clear();
	actions.insert(actions.end(),parserAction(0));
	if (!working) Parse();
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
	actions.insert(actions.end(),parserAction(fname.GetFullPath()));
	if (!working) Parse();
}

void Parser::OnEnd(parser_on_end what) {
	on_end=what;
}

void Parser::UnregisterSource(mxSource *src) {
	list<parserAction>::iterator it=actions.begin();
	while (it!=actions.end()) {
		if (it->action=='s' && !(it==actions.begin() && parser->working)) {
			if (src->sin_titulo)
				it->action='*';
			else {
				it->action='f';
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
