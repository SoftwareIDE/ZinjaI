#ifndef PARSER_H
#define PARSER_H

#include <wx/aui/aui.h>
#include <wx/hashset.h>
#include <wx/treectrl.h>
#include <wx/filename.h>
#include <map>
#include <list>
#include <wx/process.h>
#include <wx/timer.h>
using namespace std;

#include "paf_defs.h"

class ProjectManager;
class mxMainWindow;
class mxSource;

class pd_file;
class pd_func;
class pd_class;
class pd_var;
class pd_macro;
class pd_inherit;

// hashes de los punteros a cosas por nombre para encontrarlas mas rapidos
#define HashStringParserFile std::map<wxString,pd_file*>
#define HashStringParserMacro std::map<wxString,pd_macro*>
#define HashStringParserClass std::map<wxString,pd_class*>
#define HashStringParserFunction std::map<wxString,pd_func*>
#define HashStringParserVariable std::map<wxString,pd_var*>

enum {
	mxSTI_DEFINE=2,
	mxSTI_FUNCTION,
	mxSTI_CLASS,
	mxSTI_ATTRIB,
	mxSTI_PRIVATE_ATTRIB,
	mxSTI_PROTECTED_ATTRIB,
	mxSTI_PUBLIC_ATTRIB,
	mxSTI_MEMBER,
	mxSTI_PRIVATE_MEMBER,
	mxSTI_PROTECTED_MEMBER,
	mxSTI_PUBLIC_MEMBER,
	mxSTI_NAMESPACE,
	mxSTI_GLOBAL,
	mxSTI_TYPEDEF,
	mxSTI_ENUM_CONST,
};

enum ParserActionEnum {
	PA_NULL,
	PA_PARSE_OPENED_SOURCE,
	PA_PARSE_PROJECT_FILE,
	PA_PARSE_SAVED_FILE,
	PA_DELETE_FILE,
	PA_CLEAR_ALL,
	PA_SET_HIDE_SYMBOLS,
};

class parserAction {
private:
	friend class Parser;
	ParserActionEnum action_type; 
	void *ptr; ///< pointer to the current project, or to an opened mxSource, depends on action_type value
	bool flag; ///< if action_type==PA_PARSE_OPENED_SOURCE, flag=false means save before parsing; if action_type==PA_PROJECT_FILE, flag=true means sets its pd_file::hide_symbols as true
	wxString str; ///< if action_type needs a filename, its here
	parserAction(ParserActionEnum _action, void *_ptr, bool _flag, wxString _str):action_type(_action),ptr(_ptr),flag(_flag),str(_str){}
public:
	static parserAction Null() { return parserAction(PA_NULL,NULL,false,""); }
	static parserAction ClearAll() { return parserAction(PA_CLEAR_ALL,NULL,false,""); }
	static parserAction ParseSource(mxSource *src, bool dont_save) { return parserAction(PA_PARSE_OPENED_SOURCE,src,dont_save,""); }
	static parserAction ParseProjectFile(ProjectManager *prj, wxString filename, bool hide_symbols) { return parserAction(PA_PARSE_PROJECT_FILE,prj,hide_symbols,filename); }
	static parserAction DeleteFile(wxString filename) { return parserAction(PA_DELETE_FILE,NULL,false,filename); }
	static parserAction ParseSavedFile(wxString filename) { return parserAction(PA_PARSE_SAVED_FILE,NULL,false,filename); }
	static parserAction SetHideSymbols(wxString filename, bool hide_symbols) { return parserAction(PA_SET_HIDE_SYMBOLS,NULL,hide_symbols,filename); }
};


class mxParserProcess;

class Parser {
	
	friend class mxParserProcess;

	mxParserProcess *process; ///< curren async process (launched by ParseNextFile, interrupting ParseSomething, and waiting to call ParseContinue on termination)

public:
	class OnEndAction {
		OnEndAction *next;
		friend class Parser;
	public:
		virtual void Do()=0;
		virtual ~OnEndAction(){}
	};
	void OnEnd(OnEndAction *what, bool run_now_if_not_working=false);
private:
	OnEndAction *on_end;
	
public:
	wxTreeCtrl* symbol_tree;
	HashStringParserFile h_files;
	HashStringParserMacro h_macros;
	HashStringParserClass h_classes;
	HashStringParserFunction h_functions;
	HashStringParserVariable h_globals;

	// estos representan los primeros nodos ficticios de las listas
	pd_func *first_function; ///< puntero al nodo ficticio que inicia la lista de funciones (luego siguen las del autocompletado)
	pd_class *first_class; ///< puntero al nodo ficticio que inicia la lista de clases (luego siguen las del autocompletado)
	pd_file *first_file; ///< puntero al nodo ficticio que inicia la lista de archivos (luego siguen los del autocompletado)
	pd_var *first_global; ///< puntero al nodo ficticio que inicia la lista de variables globales(luego siguen las del autocompletado)
	pd_macro *first_macro; ///< puntero al nodo ficticio que inicia la lista de macros (luego siguen las del autocompletado)
	pd_inherit *first_inherit; ///< puntero al nodo ficticio que inicia la lista de relaciones de herencia (luego siguen las del autocompletado)

	// estos no representan los ultimos nodos totales, sino los ultimos de las cosas estandar,
	// a partir de estos son todos los generados por los fuentes del usuario
	pd_func *last_function; ///< puntero a la ultima funcion estandar de la lista, luego siguen las del usuario
	pd_class *last_class; ///< puntero a la ultima clase estandar de la lista, luego siguen las del usuario
	pd_file *last_file; ///< puntero al ultimo archivo estandar de la lista, luego siguen los del usuario
	pd_var *last_global; ///< puntero a la ultima variable global estandar de la lista, luego siguen las del usuario
	pd_macro *last_macro; ///< puntero a la ultima macro estandar de la lista, luego siguen las del usuario
	
	bool should_stop; ///< bandera para indicar que debe detenerse (se levanta con el metodo Stop, lo hace el main antes de salir)
	
	wxString popup_file_dec;
	int popup_line_dec;
	wxString popup_file_def;
	int popup_line_def;
	
	//inherit_data *first_inherit;
	//parser_filename *first_file;
	wxString home;
	
	//wxTreeCtrl *tree;
	mxSource *source;

	wxTreeItemId item_root;
	wxTreeItemId item_functions;
	wxTreeItemId item_classes;
	wxTreeItemId item_macros;
	wxTreeItemId item_globals;
	
	bool working;
	bool follow_includes;
	bool sort_items;

	Parser (mxMainWindow *mainwin);
	~Parser();

	/// put the parser to work if it is not working yet (it is safe to call it while parser is working, will be ignored)
	void Parse(bool show_progress=false); 
	/// enque a parsing task for an openned mxSource
	void ParseSource(mxSource *src, bool dontsave=false); 
	/// enque a parsing task for a saved source file
	void ParseFile(wxString filename); 
	/// set hide_symbols flag in the give file's pd_class (now if possible, if not put into queue)
	void SetHideSymbols(wxString filename, bool hide_symbols); 
	///
	bool RemoveFile(wxString fname);
	/// Parsea un archivo si ha sido modificado
	void ParseIfUpdated(wxFileName fname); 
	/// Removes information from a single mxSource's file (called when closing it on simple program mode)
	void UnregisterSource(mxSource *src);
	/// Clean all parser's data
	void CleanAll();
	/// stops parser if working, and cleans all external task setted to be run after parsing (for instance, displaying mxWxfbInheriter)
	void Stop(bool clean_end=false); 
	/// 
	bool RenameFile(wxString oldname, wxString newename);
	
	
	
	/// Method for finding info for a file by its name
	pd_file *GetFile(wxString name);
	/// Method for finding info for a class by its name
	pd_class *GetClass(wxString name);
	/// Method for finding ancestor classes for a class child (start with father=NULL an call until returns false to get all direct ancestor in father->father)
	bool GetFather(wxString child, pd_inherit *&father);
	
	
	/// event to be fired from main_window, launched by the symbol_tree's popup menu
	void OnGotoDec (wxAuiNotebook *notebook);
	/// event to be fired from main_window, launched by the symbol_tree's popup menu
	void OnGotoDef (wxAuiNotebook *notebook);
	/// event to be fired from main_window, launched by the symbol_tree's popup menu
	void OnSelectSymbol (wxTreeEvent &event, wxAuiNotebook *notebook);
	/// event to be fired from main_window, launched by the symbol_tree's popup menu
	void OnPopupMenu (wxTreeEvent &event, wxAuiNotebook *notebook);
	
	/// enque all project's files to be parsed, and puts the parser too work
	void ParseProject(bool show_progress=false);
	

	/// helper event launched from main_window (the class that owns the timer and receives its event), ask the current process to flush and parse its curren inputs
	void OnParserProcessTimer(); 
	
	
private:
	list<parserAction> actions;
	bool ParseNextSource(mxSource *src, bool dontsave=false);
	long ParseNextFileStart (wxFileName filename, wxString hashName, bool hide_symbols=false);
	void ParseNextFileContinue(const wxString &s); ///< to be called when an asyn process from ParseNextFile have some output
	void ParseNextFileEnd(); ///< to be called when an async process from ParseNextFile ends
	void ParseDeleteFile (wxString file);
	bool ParseNextCleanAll();
	bool ParseSetHideSymbols(wxString filename, bool hide_symbols);
	
	
	void ParseSomething(bool first, bool arg_show_progress=false);

	
	wxString JoinNames(wxString types, wxString names);
	
	
	wxTimer *process_timer; /// timer for flushing input stream from process while its running to avoid process lock due to too much unprocessed output (happens on win32)
	
};

extern Parser *parser;

class mxParserProcess:public wxProcess {
public:
	pd_file *file; pd_class *aux_class;
	mxParserProcess():wxProcess(wxPROCESS_REDIRECT),file(NULL),aux_class(NULL){ parser->process_timer->Start(100,false); }
	void OnTerminate(int pid, int status);
	void ParseOutput();
};


#endif
