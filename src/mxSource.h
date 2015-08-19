#ifndef MX_SOURCE_H
#define MX_SOURCE_H

#include <wx/stc/stc.h>
#include <wx/filename.h>
#include <wx/treebase.h>
#include "ConfigManager.h"
//#include <wx/timer.h>
#include "Cpp11.h"
#include "raii.h"

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
class mxCalltip;
class mxInspectionBaloon;

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
		Location():src(nullptr),line(0){}
	};
	Location locs[MAX_NAVIGATION_HISTORY_LEN];
	int hsize, hcur, hbase; // hcur y hsize son relativos a hbase
	mxSource *focus_source;
	bool jumping;
	void Goto(int i);
	void Add(mxSource *src, int line);
	bool masked;
public:
	class MaskGuard { public: MaskGuard(); ~MaskGuard(); void UnmaskNow(); };
	NavigationHistory():hsize(1),hcur(0),hbase(0),focus_source(nullptr),jumping(false),masked(false) {}
	void OnFocus(mxSource *src);
	void OnJump(mxSource *src, int current_line);
	void OnClose(mxSource *src);
	void Prev();
	void Next();
	void Reset() { hsize=1; hcur=0; hbase=0; focus_source=nullptr; jumping=false; }
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
		AutocompletionLocation():pos(-1),age(-1) {}
		bool IsSameLocation(int p, const wxString &k, int parser_age) {
			if (p==pos && k.StartsWith(key) && parser_age==age) return true;
			age=parser_age; key=k; pos=p; return false;
		}
		void Reset() { pos=-1; }
	};
	AutocompletionLocation last_failed_autocompletion;
	
	friend class NavigationHistory;
	friend class LocalRefactory;
	int old_current_line; ///< for detecting jumps long enought to record in navigation_history
	
public:
	struct MacroAction { 
		int msg; unsigned long wp; long lp; char data[2]; bool for_sci; char *extra;
		MacroAction(int _msg, unsigned long _wp, long _lp):msg(_msg),wp(_wp),lp(_lp),for_sci(true),extra(nullptr) { 
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
		MacroAction(int id=0):msg(id),wp(0),lp(0),for_sci(false),extra(nullptr) { }
		MacroAction &Get() { if (msg==2170) lp=(long)data; else if (msg>=2001&&msg<=2003) lp=(long)extra; return *this; }
		~MacroAction() { delete [] extra; }
	};
	void OnMacroAction(wxStyledTextEvent &evt);

	bool ignore_char_added; ///< algunas operaciones (pegar,autocode,etc) necesitan desactivar el evento
	
	SourceExtras *m_extras; ///< breakpoints, highlighted lines, saved cursor positio (see m_owns_extras)
	bool m_owns_extras; ///< if true, this objtec is the owner of m_extras and should delete it on its destructor; if false, m_owns_extras points to an instance owned by ProjectManager
	void UpdateExtras(); ///< updates info in m_extras
	
	int lexer;
	bool first_view, never_parsed;
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
	DiffInfo *first_diff_info, *last_diff_info;
	wxString page_text;
	void SetPageText(wxString ptext);
	void MakeUntitled(wxString ptext); // para abrir resultados de compilacion, salidas de gprof, y cosas asi
	
	void SetColours(bool also_style=true);
	bool IsComment(int pos);
	bool IsEmptyLine(int l, bool ignore_comments=true, bool ignore_preproc=true);

	mxSource(wxWindow *parent, wxString ptext, project_file_item *fitem=nullptr);
	
	~mxSource ();
	
	int InstructionBegin(int p);

	// edit
	void OnEditMakeLowerCase (wxCommandEvent &event);
	void OnEditMakeUpperCase (wxCommandEvent &event);
	void OnEditRedo (wxCommandEvent &event);
	void OnEditUndo (wxCommandEvent &event);
//	void OnEditClear (wxCommandEvent &event);
	void OnEditCut (wxCommandEvent &event);
	void OnEditCopy (wxCommandEvent &event);
	void OnEditPaste (wxCommandEvent &event);
	void GetSelectedLinesRange(int &lmin, int &lmax);
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
	bool GetCurrentScopeLimits(int pos, int &pmin, int &pmax, bool only_curly_braces=false);
	void OnBraceMatch (wxCommandEvent &event);
	void OnIndentSelection (wxCommandEvent &event);
	// view
	void OnFoldToggle (wxCommandEvent &event);
	void SetFolded(int level, bool folded);
	// stc
	void OnMarginClick (wxStyledTextEvent &event);
	void OnCharAdded  (wxStyledTextEvent &event);
	void OnUpdateUI (wxStyledTextEvent &event);
	void UpdateCalltipArgHighlight(int current_pos);
	void OnKillFocus (wxFocusEvent &event);
	void OnSetFocus (wxFocusEvent &event);

private:
	wxString highlithed_word;
public:
	void OnHighLightWord(wxCommandEvent &event);
	void OnFindKeyword(wxCommandEvent &event);
	void OnDoubleClick(wxStyledTextEvent &event);
	static bool IsKeywordChar(char c);
	
	void OnKeyDown(wxKeyEvent &evt);

	int GetMarginForThisX(int x);
	void OnClick(wxMouseEvent &evt);
	void OnMouseWheel(wxMouseEvent & event);
	void OnPopupMenu(wxMouseEvent &evt);
	void OnPopupMenuInside(wxMouseEvent &evt, bool fix_current_pos=true);
	void OnPopupMenuMargin(wxMouseEvent &evt);
private:
	void PopulatePopupMenuCodeTools(wxMenu &menu);
public:
	void PopupMenuCodeTools();
	void OnToolTipTime(wxStyledTextEvent &event);
	void OnToolTipTimeOut(wxStyledTextEvent &event);
	void OnSavePointReached(wxStyledTextEvent &event);
	void OnSavePointLeft(wxStyledTextEvent &event);
	
	// manejo de archivos
	void LoadSourceConfig();
	void SetLineNumbers();
	bool LoadFile (const wxFileName &filename);
	void Reload(); ///< only reload the code from its source-file (or temp file if its sin_titulo)
	void UserReload(); ///< reloads code, previously asking for unsaved changes, and also setting the reload time_stamp
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
	int GetStatementStartPos(int pos, bool skip_coma=false, bool skip_white=true, bool first_stop=false); ///< given an absolute text position, finds where that statements starts (skipping comments and indentation at the begginning unless skip_whites=false)
	wxString FindTypeOfByKey(wxString &key, int &pos, bool include_template_spec=false); // en pos retorna los asteriscos y en key el scope de la funcion
	wxString FindTypeOfByPos(int pos, int &dims, bool include_template_spec=false, bool first_call=true);
	wxString FindScope(int pos, wxString *args=nullptr, bool full_scope=false, int *scope_start=nullptr, bool local_start=false);
	wxString WhereAmI();
	
	
private:
	// solo protipos (son los que provee wxStyledTextCtrl), para que produzcan undefined reference si intento usar alguno en lugar de los propios
	void CallTipShow(int pos, const wxString& definition);
	void CallTipCancel();
	bool CallTipActive();
	void AutoCompShow(int lenEntered, const wxString& itemList);
	void AutoCompCancel();
	bool AutoCompActive();
	
private:
	friend class CodeHelper;
	mxInspectionBaloon *inspection_baloon;
	enum MXS_CALLTIP_MODE { MXS_NULL, MXS_CALLTIP, MXS_INSPECTION, MXS_BALOON, MXS_AUTOCOMP };
	MXS_CALLTIP_MODE calltip_mode;
	void SetCalltipMode(MXS_CALLTIP_MODE new_mode) { if (new_mode!=calltip_mode) HideCalltip(); calltip_mode=new_mode; }
	mxCalltip *calltip; ///< frame used (and reuse) for custom calltip (null until first use, created on ShowCalltip)
	int calltip_brace, calltip_line;
	wxArrayString autocomp_help_text;
	void OnAutocompSelection(wxStyledTextEvent &event);
	
	/// para evitar que al mostrarse el autocompletado o el calltip (y recibir asi el foco) el evento kill_focus del source los cierre inmediatamente
	class FocusHelper {
		wxTimer timer;
		bool mask;
	public:
		FocusHelper(mxSource *src) : timer(src->GetEventHandler(),wxID_ANY),mask(false) {}
		void Mask() { mask=true; timer.Start(150,true); } ///< masks next killfocus event
		void Unmask() { mask=false; } ///< unmask killfocus event
		bool KillIsMasked() { return mask; } ///< to query if killfocus event is currently masked
		bool IsThisYourTimer(const wxTimer *t) { return t==&timer; } ///< to query in mxSource::OnTimer it the current event if for this timer
	} focus_helper;
	
	class AutocompHelper {
		wxTimer timer;
		int x,y; /// ¿screen? coordinates for autocompletion menu
		int base_pos; ///< position in code where autocompletion keyword starts
		int user_pos; ///< position in code where user invoked autocompletion (base_pos+keyword.Len())
	public:
		AutocompHelper(mxSource *src) : timer(src->GetEventHandler(),wxID_ANY),x(-1),y(-1),base_pos(-1),user_pos(-1) {}
		void Start(int bp, int up, int _x, int _y) { /// up=-1 for filtered results
			base_pos=bp; if (up!=-1) user_pos=up; x=_x; y=_y; timer.Start(250,true);
		}
		void Restart() { timer.Start(250,true); } ///< to be called when an existing autocompletion list changes its selection
		bool IsThisYourTimer(const wxTimer *t) { return t==&timer; } ///< to query in mxSource::OnTimer it the current event if for this timer
		int GetX() { return x; }
		int GetY() { return y; }
		int GetBasePos() { return base_pos; }
		int GetUserPos() { return user_pos; }
	} autocomp_helper;
	
	void OnTimer(wxTimerEvent &event);
public:
	void HideInspection();
	void ShowInspection(const wxPoint &pos, const wxString &exp, const wxString & val);
	void HideCalltip();
	void ShowBaloon(wxString str, int p = -1);
	void ShowCallTip(int brace_pos, int calltip_pos, const wxString &s);
	void ShowAutoComp(int p, const wxString &s, bool is_filter=false);

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
	
	
	/// En lugar de llamar a BeginUndoAction y EndUndoAction, crear una instancia de esta clase, se encarga de todo y se asegura de que nunca se hagan dos Begin anidados o falte un End
	class UndoActionGuard {
		mxSource *src;
	public:
		UndoActionGuard(mxSource *source, bool begin=true) : src(source) { if (begin) Begin(); }
		void Begin() { if (src->undo_action_guard==nullptr) src->BeginUndoAction(this); }
		void End() { if (src->undo_action_guard==this) src->EndUndoAction(this); }
		~UndoActionGuard() { End(); }
	};
	
	NullInitializedPtr<UndoActionGuard> undo_action_guard; ///< si estamos en una "undo action" apunta al UndoActionGuard que hizo se hizo el BeginUndoAction, sino es null
	
private:
	
	void BeginUndoAction(); ///< solo prototipo, para evitar llamar al del wxStyledTextCtrl sin querer, ver UndoActionGuard a cambio
	void EndUndoAction(); ///< solo prototipo, para evitar llamar al del wxStyledTextCtrl sin querer, ver UndoActionGuard a cambio
	void BeginUndoAction(UndoActionGuard *guard) { ///< para ser invocado desde el UndoActionGuard
		wxStyledTextCtrl::BeginUndoAction();
		undo_action_guard = guard;
	}
	void EndUndoAction(UndoActionGuard *guard) { ///< para ser invocado desde el UndoActionGuard
		wxStyledTextCtrl::EndUndoAction();
		undo_action_guard = nullptr;
	}
	
	int brace_1, brace_2;
	void MyBraceHighLight(int b1=wxSTC_INVALID_POSITION, int b2=wxSTC_INVALID_POSITION);
	
	
private:
	struct MultiSel {
		
		struct EditPos { int line, offset; };
		vector<EditPos> positions;
		GenericAction *on_end;
		void Reset() { on_end=nullptr; is_on=false; was_rect_select=false; keep_highlight=false; positions.clear(); }
		void AddPos(mxSource *src, int line, int position) { 
			EditPos e; e.line = line; 
			e.offset = position-src->PositionFromLine(line); 
			positions.push_back(e); 
		}
		
		bool is_on; ///< si estamos o no editando "rectangularmente"
		bool was_rect_select; ///< para saber al aplicar un cambio si habia en el estado anterior una seleccion rectangular (porque en ese caso se modicaron todas las lineas)
		bool keep_highlight; ///< si hay que marcar la nueva edicion como highlighted word
		int line, offset_beg, offset_end; ///< dentro de la linea, en que posicion estamos editando (posicion en la que empezaria ref_str, y cuanto mide)
//		int line_from, line_to; ///< cuales lineas estamos editando "rectangularmente" (desde line_from hasta line_to, incluidos ambos extremos)
		wxString ref_str; ///< string de la primer linea de la seleccion rectangular, para comparar y ver como cambio y hacer lo mismo en las otras
		MultiSel():is_on(false){}
		void SetEditRegion(mxSource *src, int line, int pbeg, int pend);
		void Begin(mxSource *src, bool was_rect_select, bool keep_highlight, bool notify=true, GenericAction *aon_end=nullptr);
		void End(mxSource *src);
		operator bool() { return is_on; }
		
	} multi_sel;
	void OnClickUp(wxMouseEvent &evt);
	void OnEditRectangularEdition(wxCommandEvent &evt);
	void OnEditHighLightedWordEdition(wxCommandEvent &evt);
	void InitRectEdit(bool keep_rect_select);
	void ApplyRectEdit();
	
	// helper functions for autocomp parsing
	template<int N> bool TextRangeWas(int pos_end, const char (&word)[N]);
	template<int N> bool TextRangeIs(int pos_start, const char (&word)[N]);
	template<int N> bool TextRangeIs(int pos_start, int pos_end, const char (&word)[N]);
	void CopyIndentation(int line_to_indent, int pos_to_get_amount, bool increase=false) {
		SetLineIndentation(line_to_indent,GetLineIndentation(LineFromPosition(pos_to_get_amount))+(increase?config_source.tabWidth:0));
	}
public:
	int SkipTemplateSpec(int pos_start, int pos_max=0);
	/// sabiendo donde termina una lista de argumentos del template (pos_start, '>'), 
	/// encuentra donde empiza (retorna la pos *antes* del '<')
	int SkipTemplateSpecBack(int pos_start); 
	
	DECLARE_EVENT_TABLE();
public:
	
	// helper functions for refactory operations
	/// assuming pos (only input arg) is in a the name of function in a function call, 
	/// returns that name, the actual arguments (args), and the expected return type
	bool GetCurrentCall (wxString &ftype, wxString &fname, wxArrayString &args, int pos);
	
};


struct DiffInfo {
	DiffInfo *brother; // para saber de que cambio es, e identificar el mismo en el hermano
	mxSource *src;
	int *handles, *bhandles, len, marker;
	wxString extra;
	DiffInfo *prev,*next;
	DiffInfo(mxSource *s, int *h, int l, int m, wxString e):
		brother(nullptr),src(s),handles(h),bhandles(nullptr),len(l),marker(m),extra(e) {
			prev=src->last_diff_info;
			next=nullptr;
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
			if (next) next->prev=nullptr;
		} else {
			prev->next=next;
			if (next) next->prev=prev;
		}
		if (this==src->last_diff_info)
			src->last_diff_info=prev;
	}
};


#endif

