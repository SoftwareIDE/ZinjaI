#ifndef MX_SOURCE_H
#define MX_SOURCE_H

#include <wx/stc/stc.h>
#include <wx/filename.h>
#include <wx/treebase.h>
#include "ConfigManager.h"

class er_source_register;

#define mxSOURCE_BOLD 1
#define mxSOURCE_ITALIC 2
#define mxSOURCE_UNDERL 4
#define mxSOURCE_HIDDEN 8

#define SRC_PARSING_ERROR 999

class project_file_item;
class SourceExtras;
class wxString;
class wxTreeItemId;
class DiffInfo;

enum MXS_MARKER {
//	mxSTC_MARK_CURRENT=0, 	///< resaltar la linea actual en el editor (fondo celeste)
	mxSTC_MARK_USER=0, 		///< resaltar lineas que elige el usuario (fondo rojo)
	mxSTC_MARK_BREAKPOINT,	///< marcar los puntos de interrupcion
	mxSTC_MARK_BAD_BREAKPOINT,	///< marcar los puntos de interrupcion
	mxSTC_MARK_EXECPOINT,	///< indicar donde esta actualmente la ejecucion
	mxSTC_MARK_FUNCCALL,	///< indicar donde quedo una funcion a la espera de retornar de una llamada a otra
	mxSTC_MARK_STOP,		///< indicar donde se interrumpio la ejecucion (por ej, por SegFault)
	mxSTC_MARK_DIFF_ADD, mxSTC_MARK_DIFF_CHANGE, mxSTC_MARK_DIFF_DEL, mxSTC_MARK_DIFF_NONE
};

enum ReadOnlyModeEnum { 
	ROM_NONE, ///< not readonly, can edit
	ROM_PROJECT, ///< setted as readonly in project_file_item props
	ROM_DEBUG, ///< currently debugging, and preferences say don't edit while debug
	ROM_PROJECT_AND_DEBUG, ///< currently debugging, and preferences say don't edit while debug, and also setted in project_file_item
	ROM_SPECIAL, ///< special file, such as full compiler output
	ROM_SPECIALS, ///< dummy item to split real states and special flags
	ROM_ADD_DEBUG, ///< not a real state, just used for telling SetReadOnlyMode "change to some debugging state"
	ROM_DEL_DEBUG, ///< not a real state, just used for telling SetReadOnlyMode "change to some non-debugging state"
	ROM_ADD_PROJECT, ///< not a real state, just used for telling SetReadOnlyMode "ProjectManager says you're read-only"
	ROM_DEL_PROJECT, ///< not a real state, just used for telling SetReadOnlyMode "ProjectManager says you're not read-only"
};

class mxSource;
#define MAX_NAVIGATION_HISTORY_LEN 100
class NavigationHistory {
	struct Location {
		wxString file;
		mxSource *src;
		int line;
		Location():src(NULL),line(0){}
	};
	Location locs[MAX_NAVIGATION_HISTORY_LEN];
	int hsize, hcur, hbase; // hcur y hsize son relativos a hbase
	mxSource *focus_source;
	bool jumping;
	void Goto(int i);
	void Add(mxSource *src, int line);
public:
	NavigationHistory():hsize(1),hcur(0),hbase(0),focus_source(NULL),jumping(false) {}
	void OnFocus(mxSource *src);
	void OnJump(mxSource *src, int current_line);
	void OnClose(mxSource *src);
	void Prev();
	void Next();
	void Reset() { hsize=1; hcur=0; hbase=0; focus_source=NULL; jumping=false; }
};

extern NavigationHistory navigation_history;

/**
* @brief Componente visual de edicion de texto para fuentes
**/
class mxSource: public wxStyledTextCtrl {

	class AutocompletionLocation {
		int pos, age;
		wxString key;
	public:
		bool IsSameLocation(int p, const wxString &k, int parser_age) {
			if (p==pos && k.StartsWith(key) && parser_age==age) return true;
			age=parser_age; key=k; pos=p; return false;
		}
		void Reset() { pos=-1; }
	};
	AutocompletionLocation last_failed_autocompletion;
	
	friend class NavigationHistory;
	int old_current_line; ///< for detecting jumps long enought to record in navigation_history
	
public:
	struct MacroAction { 
		int msg; unsigned long wp; long lp; char data[2]; bool for_sci; char *extra;
		MacroAction(int _msg, unsigned long _wp, long _lp):msg(_msg),wp(_wp),lp(_lp),for_sci(true),extra(NULL) { 
			/// 2170 = replace selection, with a 16-bits keycode stored in the address pointed by lp
			/// 2001/2003 = add/insert text, wp has lenght/position, lp points to a null-terminated string
			/// 2002 = add styled text, wp has lenght, lp points to a some data buffer
			if (lp && msg==2170) { memcpy(data,(void*)lp,2); }
			else if (msg==2003||msg==2001) { 
				char *aux=(char*)_lp; int i=0; while (aux[i]!='\0') i++;
				extra=new char[i+1]; i=0; while (aux[i]!='\0') { extra[i]=aux[i]; i++; } extra[i]='\0';
			} else if (msg==2001) { 
				char *aux=(char*)_lp; extra=new char[wp]; for(unsigned int i=0;i<wp;i++) extra[i]=aux[i];
			}
		}
		MacroAction(int id=0):msg(id),wp(0),lp(0),for_sci(false),extra(NULL) { }
		MacroAction(const MacroAction &other) { this->extra=other.extra; const_cast<MacroAction*>(&other)->extra=NULL; }
		MacroAction &Get() { if (msg==2170) lp=(long)data; else if (msg>=2001&&msg<=2003) lp=(long)extra; return *this; }
		~MacroAction() { delete [] extra; }
	};
	void OnMacroAction(wxStyledTextEvent &evt);

	bool ignore_char_added; ///< algunas operaciones (pegar,autocode,etc) necesitan desactivar el evento
	
	int last_s1, last_s2; ///< para el resaltado de campos
	
	SourceExtras *m_extras; ///< breakpoints, highlighted lines, saved cursor positio (see m_owns_extras)
	bool m_owns_extras; ///< if true, this objtec is the owner of m_extras and should delete it on its destructor; if false, m_owns_extras points to an instance owned by ProjectManager
	void UpdateExtras(); ///< updates info in m_extras
	
	int calltip_brace;
	int lexer;
	bool first_view;
	bool never_parsed;
	bool false_calltip;
	er_source_register *er_register;
	
private:
	void SetSourceTime(wxDateTime stime);
	wxDateTime source_time; ///< para saber si alguien lo modifico desde afuera
	bool source_time_dont_ask, source_time_reload;
	
public:
	void SplitFrom(mxSource *orig);
	mxSource *next_source_with_same_file; // lista circular de mxSource que muestran el mismo archivo

private:
	bool ro_quejado;
	ReadOnlyModeEnum readonly_mode;
	void OnModifyOnRO(wxStyledTextEvent &event);
public:
	void SetReadOnlyMode(ReadOnlyModeEnum mode);

	void OnPainted(wxStyledTextEvent &event);
	
	void CheckForExternalModifications(); ///< checks if the file has changed, if it did, enqueue a call to ThereAreExternalModifications for the end of the main_window event loop
	void ThereAreExternalModifications(); ///< show the warning dialog or reload the file when we know it has changed
	mxSource *diff_brother;
	DiffInfo *first_diff_info,*last_diff_info;
	wxString page_text;
	void SetPageText(wxString ptext);
	void MakeUntitled(wxString ptext); // para abrir resultados de compilacion, salidas de gprof, y cosas asi
	void ShowBaloon(wxString text, int pos = -1);
	
	void SetColours(bool also_style=true);
	bool IsComment(int pos);
	bool IsEmptyLine(int l, bool ignore_comments=true, bool ignore_preproc=true);

	mxSource(wxWindow *parent, wxString ptext, project_file_item *fitem=NULL);
	
	~mxSource ();
	
	int InstructionBegin(int p);

	// edit
	void OnEditRedo (wxCommandEvent &event);
	void OnEditUndo (wxCommandEvent &event);
	void OnEditClear (wxCommandEvent &event);
	void OnEditCut (wxCommandEvent &event);
	void OnEditCopy (wxCommandEvent &event);
	void OnEditPaste (wxCommandEvent &event);
	void OnEditDuplicateLines (wxCommandEvent &event);
	void OnEditDeleteLines (wxCommandEvent &event);
	void OnEditToggleLinesUp (wxCommandEvent &event);
	void OnEditToggleLinesDown (wxCommandEvent &event);
	void OnEditForceAutoComplete (wxCommandEvent &event);
	void OnEditAutoCompleteAutocode (wxCommandEvent &event);
	// selection
	void OnEditSelectAll (wxCommandEvent &event);
//	void OnEditSelectLine (wxCommandEvent &event);
	void OnEditMarkLines (wxCommandEvent &event);
//	void OnEditUnMarkLines (wxCommandEvent &event);
	void OnEditGotoMark (wxCommandEvent &event);
	// c++ specific
	void OnComment (wxCommandEvent &event);
	void OnUncomment (wxCommandEvent &event);
	void OnBraceMatch (wxCommandEvent &event);
	void OnIndentSelection (wxCommandEvent &event);
	// view
	void OnFoldToggle (wxCommandEvent &event);
	void SetFolded(int level, bool folded);
	// stc
	void OnMarginClick (wxStyledTextEvent &event);
	void OnCharAdded  (wxStyledTextEvent &event);
	void OnUpdateUI (wxStyledTextEvent &event);
	void OnKillFocus (wxFocusEvent &event);
	void OnSetFocus (wxFocusEvent &event);
	
	void OnHighLightWord(wxCommandEvent &event);
	void OnDoubleClick(wxStyledTextEvent &event);
	
	void OnKeyDown(wxKeyEvent &evt);

	int GetMarginForThisX(int x);
	void OnClick(wxMouseEvent &evt);
	void OnPopupMenu(wxMouseEvent &evt);
	void OnPopupMenuInside(wxMouseEvent &evt);
	void OnPopupMenuMargin(wxMouseEvent &evt);
	void OnToolTipTime(wxStyledTextEvent &event);
	void OnToolTipTimeOut(wxStyledTextEvent &event);
	void OnSavePointReached(wxStyledTextEvent &event);
	void OnSavePointLeft(wxStyledTextEvent &event);
	
	// manejo de archivos
	void LoadSourceConfig();
	void SetLineNumbers();
	bool LoadFile (const wxFileName &filename);
	void Reload();
	bool MySaveFile (const wxString &filename);
	bool SaveSource ();
	bool SaveTemp ();
	bool SaveTemp (wxString fname);
	bool SaveSource (const wxFileName &filename);
	void GotoPos(int pos);

	void MoveCursorTo (long pos, bool focus=false);

private:
	///* auxiliar function for the public SetStyle
	void SetStyle(int idx, const wxChar *fontName, int fontSize, const wxColour &foreground, const wxColour &background, int fontStyle);
public:
	void SetStyle(bool color);
	void SetStyle(int lexer);
	void SetModify(bool modif=true);
	void MarkError(int line, bool focus=true);
	void SelectError(int indic, int p1, int p2);
	void Indent(int min, int max);
	DiffInfo *MarkDiffs(int from, int to, MXS_MARKER marker, wxString extra="");
	void SetDiffBrother(mxSource *source);
	void ApplyDiffChange();
	void DiscardDiffChange();
	void ShowDiffChange();
	void GotoDiffChange(bool forward);
	
	bool AddInclude(wxString header);
	wxString FindTypeOf(wxString &key, int &pos); // en pos retorna los asteriscos y en key el scope de la funcion
	wxString FindTypeOf(int pos, int &dims, bool first_call=true);
	wxString FindScope(int pos, wxString *args=NULL, bool full_scope=false);
	wxString WhereAmI();
	
	void ShowCallTip(int p, wxString str, bool fix_pos=true);
private:
	void AutoCompShow(int p, const wxString &s) {} // solo para evitar llamarla sin querer
public:
	void ShowAutoComp(int p, const wxString &s) { last_failed_autocompletion.Reset(); wxStyledTextCtrl::AutoCompShow(p,s); }

	wxFileName working_folder;
	wxFileName source_filename;
private:
	wxFileName binary_filename; ///< nombre del ejecutable, solo para el modo programa simple
public:
	wxFileName GetBinaryFileName(); ///< en modo programa simple devuelve binary_filename, en modo proyecto arma la ruta completa del .o
	
	wxFileName temp_filename;

	wxString exec_args;
	bool sin_titulo, cpp_or_just_c; // para un programa simple sin_titulo, la segunda bandera indica si compilar con g++ (true) o gcc (false), si tiene título se usa la extensión
	bool IsCppOrJustC();
	
	void RemoveComments();
	void AlignComments(int c);

	wxTreeItemId treeId;

	cfgSource config_source;
	cfgRunning config_running;
	wxString GetCompilerOptions(bool parsed=true);
	void SetCompilerOptions(const wxString &compiler_options);

	bool ApplyAutotext();
	
	wxString GetPath(bool for_user=false);
	wxString GetFullPath();
	wxString GetFileName(bool with_extension=true);
	wxString SaveSourceForSomeTool();
	void JumpToCurrentSymbolDefinition();
	
	wxString GetCurrentKeyword(int pos=-1);
	
private:
	
	int brace_1,brace_2;
	void MyBraceHighLight(int b1=wxSTC_INVALID_POSITION, int b2=wxSTC_INVALID_POSITION);
	
	DECLARE_EVENT_TABLE();
};


struct DiffInfo {
	DiffInfo *brother; // para saber de que cambio es, e identificar el mismo en el hermano
	mxSource *src;
	int *handles, *bhandles, len, marker;
	wxString extra;
	DiffInfo *prev,*next;
	DiffInfo(mxSource *s, int *h, int l, int m, wxString e):
		brother(NULL),src(s),handles(h),bhandles(NULL),len(l),marker(m),extra(e) {
			prev=src->last_diff_info;
			next=NULL;
			if (prev)
				prev->next=this;
			else
				src->first_diff_info=this;
			src->last_diff_info=this;
		}
	~DiffInfo() {
		delete []handles;
		delete []bhandles;
		if (src->first_diff_info==this) {
			src->first_diff_info=next;
			if (next) next->prev=NULL;
		} else {
			prev->next=next;
			if (next) next->prev=prev;
		}
		if (this==src->last_diff_info)
			src->last_diff_info=prev;
	}
};


#endif

