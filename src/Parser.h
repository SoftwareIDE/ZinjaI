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

struct parserAction {
	char action; // s=parseSource, c=clearAll, f=parseFile, d=deleteFile
	void *ptr;
	bool flag;
	wxString str;
	parserAction(){}
	parserAction(mxSource *src, bool dont_save) {
		ptr=src; flag=dont_save; action='s';
	}
	parserAction(int) {
		action='c';
	}
	parserAction(ProjectManager *prj, wxString fnm) {
		ptr=prj; str=fnm; action='p';
	}
	parserAction(wxString fnm, char aaction='f') {
		str=fnm; action=aaction;
	}
};


class mxParserProcess;

class Parser {

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

	list<parserAction> actions;
	void ParseSource(mxSource *src, bool dontsave=false);
	bool ParseNextSource(mxSource *src, bool dontsave=false);
	long ParseNextFileStart (wxFileName filename, wxString hashName);
	void ParseNextFileContinue(const wxString &s); ///< to be called when an asyn process from ParseNextFile have some output
	void ParseNextFileEnd(); ///< to be called when an asyn process from ParseNextFile ends
	void ParseDeleteFile (wxString file);
	void ParseFile(wxString filename);
	bool RenameSource(mxSource *src, wxFileName newename);
	bool RemoveSource(mxSource *src);
	bool RenameFile(wxString oldname, wxString newename);
	bool RemoveFile(wxString fname);
	void ParseProject(bool show_progress=false);
	void CleanAll();
	bool ParseNextCleanAll();
	
	void Parse(bool show_progress=false); ///< put the parser to work if it is not working yet (it is safe to call it while parser is working, will be ignored)
	void ParseSomething(bool first, bool arg_show_progress=false);

	void OnSelectSymbol (wxTreeEvent &event, wxAuiNotebook *notebook);
	void OnPopupMenu (wxTreeEvent &event, wxAuiNotebook *notebook);
	void OnGotoDec (wxAuiNotebook *notebook);
	void OnGotoDef (wxAuiNotebook *notebook);
	void UnregisterSource(mxSource *src);
	void Stop(bool clean_end=false);
	
	wxString JoinNames(wxString types, wxString names);
	
	/// Method for finding info for a file by its name
	pd_file *GetFile(wxString name);
	/// Method for finding info for a class by its name
	pd_class *GetClass(wxString name);
	/// Method for finding ancestor classes for a class child (start with father=NULL an call until returns false to get all direct ancestor in father->father)
	bool GetFather(wxString child, pd_inherit *&father);
	
	/// Parsea un archivo si ha sido modificado
	void ParseIfUpdated(wxFileName fname); 
	
	wxTimer *process_timer; /// timer for flushing input stream from process while its running to avoid process lock due to too much unprocessed output (happens on win32)
	void OnParserProcessTimer(); /// ask the current process to flush and parse its curren inputs
	
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
