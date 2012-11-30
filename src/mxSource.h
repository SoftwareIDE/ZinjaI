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

class wxString;
class wxTreeItemId;
class BreakPointInfo;
class DiffInfo;

enum MXS_MARKER {
	mxSTC_MARK_CURRENT=0, 	///< resaltar la linea actual en el editor (fondo celeste)
	mxSTC_MARK_USER, 		///< resaltar lineas que elige el usuario (fondo rojo)
	mxSTC_MARK_BREAKPOINT,	///< marcar los puntos de interrupcion
	mxSTC_MARK_BAD_BREAKPOINT,	///< marcar los puntos de interrupcion
	mxSTC_MARK_EXECPOINT,	///< indicar donde esta actualmente la ejecucion
	mxSTC_MARK_FUNCCALL,	///< indicar donde quedo una funcion a la espera de retornar de una llamada a otra
	mxSTC_MARK_STOP,		///< indicar donde se interrumpio la ejecucion (por ej, por SegFault)
	mxSTC_MARK_DIFF_ADD, mxSTC_MARK_DIFF_CHANGE, mxSTC_MARK_DIFF_DEL, mxSTC_MARK_DIFF_NONE
};

/**
* @brief Componente visual de edicion de texto para fuentes
**/
class mxSource: public wxStyledTextCtrl {

public:

	bool ignore_char_added; ///< algunas operaciones (pegar,autocode,etc) necesitan desactivar el evento
	
	int last_s1, last_s2; ///< para el resaltado de campos
	
	BreakPointInfo **breaklist; ///< lista de info de breakpoints 
	bool own_breaks; ///< para saber si la info de los breakpoints es resposabilidad propia o del proyecto
	int calltip_brace;
	int current_line;
	int current_marker;
	int lexer;
	bool first_view;
	bool never_parsed;
	bool false_calltip;
	bool debug_time;
	er_source_register *er_register;
	
private:
	void SetSourceTime(wxDateTime stime);
	wxDateTime source_time; ///< para saber si alguien lo modifico desde afuera
	bool source_time_dont_ask, source_time_reload;
	
public:
	void SplitFrom(mxSource *orig);
	mxSource *next_source_with_same_file; // lista circular de mxSource que muestran el mismo archivo

	bool ro_quejado;
	void OnModifyOnRO(wxStyledTextEvent &event);
	
	void CheckForExternalModifications();
	mxSource *diff_brother;
	DiffInfo *first_diff_info,*last_diff_info;
	wxString page_text;
	void SetDebugTime(bool setted);
	void SetPageText(wxString ptext);
	void MakeUntitled(wxString ptext); // para abrir resultados de compilacion, salidas de gprof, y cosas asi
	void ShowBaloon(wxString text, int pos = -1);
	
	void SetColours(bool also_style=true);
	bool IsComment(int pos);
	bool IsEmptyLine(int l, bool ignore_comments=true, bool ignore_preproc=true);

	mxSource (wxWindow *parent, wxString ptext, wxWindowID id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxSUNKEN_BORDER|wxVSCROLL
		);
	
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
	
	void OnKeyDown(wxKeyEvent &evt);
	void OnCharHook (wxStyledTextEvent &evt);

	void OnClick(wxMouseEvent &evt);
	void OnPopupMenu(wxMouseEvent &evt);
	void OnToolTipTime(wxStyledTextEvent &event);
	void OnToolTipTimeOut(wxStyledTextEvent &event);
	void OnSavePointReached(wxStyledTextEvent &event);
	void OnSavePointLeft(wxStyledTextEvent &event);
	void OnCalltipClick(wxStyledTextEvent &event);
	void OnStartDrag(wxStyledTextEvent &event);
	void EndDrag();
		
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
	wxFileName binary_filename;
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
	int m_LineNrMargin;
	int m_FoldingID;
	int m_FoldingMargin;
	int m_DividerID;
	
	DECLARE_EVENT_TABLE()
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

