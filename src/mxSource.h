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

/**
* @brief Componente visual de edicion de texto para fuentes
**/
class mxSource: public wxStyledTextCtrl {

public:
	struct MacroAction { 
		int msg; int wp=0; int lp=0;
		MacroAction(int _msg=0, int _wp=0, int _lp=0):msg(_msg),wp(_wp),lp(_lp){}
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
	void OnEditSelectLine (wxCommandEvent &event);
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

	void OnClick(wxMouseEvent &evt);
	void OnPopupMenu(wxMouseEvent &evt);
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
	DiffInfo *MarkDiffs(int from, int to, MXS_MARKER marker, wxString extra=_T(""));
	void SetDiffBrother(mxSource *source);
	void ApplyDiffChange();
	void DiscardDiffChange();
	void ShowDiffChange();
	void GotoDiffChange(bool forward);
	
	bool AddInclude(wxString header);
	wxString FindTypeOf(wxString &key, int &pos); // en pos retorna los asteriscos y en key el scope de la funcion
	wxString FindTypeOf(int pos, int &dims, bool first_call=true);
	wxString FindScope(int pos);
	wxString WhereAmI();
	
	void ShowCallTip(int p, wxString str, bool fix_pos=true);

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
	wxString GetParsedCompilerOptions();

	bool ApplyAutotext();
	
	wxString GetPath(bool for_user=false);
	wxString GetFullPath();
	wxString GetFileName(bool with_extension=true);
	wxString SaveSourceForSomeTool();
	
private:
	
	// margin variables
	int m_LineNrID;
	int m_FoldingID;
	int m_DividerID;
	
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

