#include <iostream>
#include <wx/encconv.h>
#include <wx/file.h>
#include "mxGCovSideBar.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "CodeHelper.h" 
#include "mxDropTarget.h" 
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "DebugManager.h"
#include "mxArt.h" 
#include "mxBreakOptions.h"
#include "Parser.h"
#include "mxGotoFunctionDialog.h"
#include "mxApplication.h"
#include "Language.h"
#include "mxStatusBar.h"
#include "mxCompiler.h"
#include "Autocoder.h"
#include "mxColoursEditor.h"
#include "error_recovery.h"
#include "mxCalltip.h"
#include "MenusAndToolsConfig.h"
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionBaloon.h"
#include <wx/utils.h>
using namespace std;


NavigationHistory navigation_history;

void NavigationHistory::OnClose(mxSource *src) {
	for(int i=0;i<hsize;i++) { 
		int j=(hbase+i)%MAX_NAVIGATION_HISTORY_LEN;
		if (locs[j].src==src) {
			if (src->sin_titulo) locs[j].file.Clear();
			else locs[j].file=src->GetFullPath(); 
			locs[j].src=nullptr;
		}
	}
	if (focus_source==src) focus_source=nullptr;
}

void NavigationHistory::Goto(int i) {
	jumping=true;
	Location &loc=locs[i%MAX_NAVIGATION_HISTORY_LEN];
	if (!loc.src) {
		if (!loc.file.Len()) return;
		loc.src=main_window->OpenFile(loc.file);
	} else if (focus_source!=loc.src) {
		for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++)
			if (main_window->notebook_sources->GetPage(i)==loc.src) {
				main_window->notebook_sources->SetSelection(i);
				break;
			}
	}
	if (loc.src) {
		loc.src->GotoPos(loc.src->GetLineIndentPosition(loc.line));
		loc.src->SetFocus();
	}
	jumping=false;
}


void NavigationHistory::OnFocus(mxSource *src) {
	if (src==focus_source) return;
	focus_source=src; Add(src,src->GetCurrentLine());
}

void NavigationHistory::OnJump(mxSource *src, int current_line) {
	const int min_long_jump_len=10;
	if (!jumping) {
		if (
			current_line>=src->old_current_line+min_long_jump_len
			||
			current_line<=src->old_current_line-min_long_jump_len
		) {
			Add(src,current_line);
		} else {
			Location &old_loc=locs[(hbase+hcur)%MAX_NAVIGATION_HISTORY_LEN];
			old_loc.line=current_line;
		}
	}
	src->old_current_line=current_line;
}

void NavigationHistory::Add(mxSource *src, int line) {
	Location &old_loc=locs[(hbase+hcur)%MAX_NAVIGATION_HISTORY_LEN];
	if (old_loc.src==src&&old_loc.line==line) return;
	if (hsize<MAX_NAVIGATION_HISTORY_LEN) hsize=(++hcur)+1;
	else hbase=(hbase+1)%MAX_NAVIGATION_HISTORY_LEN;
	Location &new_loc=locs[(hbase+hcur)%MAX_NAVIGATION_HISTORY_LEN];
	new_loc.src=src; new_loc.line=line;
}

void NavigationHistory::Prev() {
	if (hcur==0) return;
	Goto(hbase+(--hcur));
}

void NavigationHistory::Next() {
	if (hcur+1==hsize) return; // no hay historial para adelante
	Goto(hbase+(++hcur));
}

#define II_BACK(p,a) while(p>0 && (a)) p--;
#define II_BACK_NC(p,a) while(a) p--;
#define II_FRONT(p,a) while(p<l && (a)) p++;
#define II_FRONT_NC(p,a) while(a) p++;
#define II_IS_2(p,c1,c2) ((c=GetCharAt(p))==c1 || c==c2)
#define II_IS_3(p,c1,c2,c3) ((c=GetCharAt(p))==c1 || c==c2 || c==c3)
#define II_IS_4(p,c1,c2,c3,c4) ((c=GetCharAt(p))==c1 || c==c2 || c==c3 || c==c4)
#define II_IS_5(p,c1,c2,c3,c4,c5) ((c=GetCharAt(p))==c1 || c==c2 || c==c3 || c==c4 || c==c5)
#define II_IS_6(p,c1,c2,c3,c4,c5,c6) ((c=GetCharAt(p))==c1 || c==c2 || c==c3 || c==c4 || c==c5 || c==c6)
#define II_IS_COMMENT(p) ((s=GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)
#define II_SHOULD_IGNORE(p) ((s=GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRING || s==wxSTC_C_STRINGEOL || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)
#define II_IS_NOTHING_4(p) (II_IS_4(p,' ','\t','\r','\n') || II_SHOULD_IGNORE(p))
#define II_IS_NOTHING_2(p) (II_IS_2(p,' ','\t') || II_SHOULD_IGNORE(p)) 
#define II_IS_KEYWORD_CHAR(c) ( ( (c|32)>='a' && (c|32)<='z' ) || (c>='0' && c<='9') || c=='_' )

#define STYLE_IS_CONSTANT(s) (s==wxSTC_C_STRING || s==wxSTC_C_STRINGEOL || s==wxSTC_C_CHARACTER || s==wxSTC_C_REGEX || s==wxSTC_C_NUMBER)
#define STYLE_IS_COMMENT(s) (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)

const wxChar* mxSourceWords1 =
	"and asm auto break case catch class const const_cast "
	"continue default delete do dynamic_cast else enum explicit "
	"export extern false for friend if goto inline "
	"mutable namespace new not operator or private protected public "
	"reinterpret_cast return sizeof static_cast "
	"struct switch template this throw true try typedef typeid "
	"typename union using virtual while xor "
	"auto constexpr decltype static_assert final override noexcept nullptr"; // c++ 2011
const wxChar* mxSourceWords2 =
	"bool char const double float int long mutable register "
	"short signed static unsigned void volatile wchar_t";
const wxChar* mxSourceWords3 =
	"a addindex addtogroup anchor arg attention author b brief bug c "
	"class code date def defgroup deprecated dontinclude e em endcode "
	"endhtmlonly endif endlatexonly endlink endverbatim enum example "
	"exception f$ f[ f] file fn hideinitializer htmlinclude "
	"htmlonly if image include ingroup internal invariant interface "
	"latexonly li line link mainpage n name namespace nosubgrouping note "
	"overload p page par param post pre ref relates remarks return "
	"retval sa section see showinitializer since skip skipline struct "
	"subsection test throw todo tparam typedef union until var verbatim "
	"verbinclude version warning weakgroup $ @ \"\" & < > # { }";


enum Margins { MARGIN_LINENUM=0, MARGIN_BREAKS, MARGIN_FOLD, MARGIN_NULL };

BEGIN_EVENT_TABLE (mxSource, wxStyledTextCtrl)
	// edit
//	EVT_MENU (mxID_EDIT_CLEAR, mxSource::OnEditClear)
	EVT_MENU (mxID_EDIT_CUT, mxSource::OnEditCut)
	EVT_MENU (mxID_EDIT_COPY, mxSource::OnEditCopy)
	EVT_MENU (mxID_EDIT_PASTE, mxSource::OnEditPaste)
	EVT_MENU (mxID_EDIT_SELECT_ALL, mxSource::OnEditSelectAll)
	EVT_MENU (mxID_EDIT_REDO, mxSource::OnEditRedo)
	EVT_MENU (mxID_EDIT_UNDO, mxSource::OnEditUndo)
	EVT_MENU (mxID_EDIT_DUPLICATE_LINES, mxSource::OnEditDuplicateLines)
	EVT_MENU (mxID_EDIT_DELETE_LINES, mxSource::OnEditDeleteLines)
	EVT_MENU (mxID_EDIT_MARK_LINES, mxSource::OnEditMarkLines)
	EVT_MENU (mxID_EDIT_GOTO_MARK, mxSource::OnEditGotoMark)
	EVT_MENU (mxID_EDIT_FORCE_AUTOCOMPLETE, mxSource::OnEditForceAutoComplete)
	EVT_MENU (mxID_EDIT_RECTANGULAR_EDITION, mxSource::OnEditRectangularEdition)
	EVT_MENU (mxID_EDIT_AUTOCODE_AUTOCOMPLETE, mxSource::OnEditAutoCompleteAutocode)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_UP, mxSource::OnEditToggleLinesUp)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_DOWN, mxSource::OnEditToggleLinesDown)
	EVT_MENU (mxID_EDIT_COMMENT, mxSource::OnComment)
	EVT_MENU (mxID_EDIT_UNCOMMENT, mxSource::OnUncomment)
	EVT_MENU (mxID_EDIT_BRACEMATCH, mxSource::OnBraceMatch)
	EVT_MENU (mxID_EDIT_INDENT, mxSource::OnIndentSelection)
	EVT_MENU (mxID_EDIT_HIGHLIGHT_WORD, mxSource::OnHighLightWord)
	EVT_MENU (mxID_EDIT_MAKE_LOWERCASE, mxSource::OnEditMakeLowerCase)
	EVT_MENU (mxID_EDIT_MAKE_UPPERCASE, mxSource::OnEditMakeUpperCase)
	// view
	EVT_MENU (mxID_FOLDTOGGLE, mxSource::OnFoldToggle)
	EVT_SET_FOCUS (mxSource::OnSetFocus)
	EVT_KILL_FOCUS (mxSource::OnKillFocus)
	EVT_STC_MARGINCLICK (wxID_ANY, mxSource::OnMarginClick)
	// Stc
	EVT_STC_CHARADDED (wxID_ANY, mxSource::OnCharAdded)
	EVT_STC_UPDATEUI (wxID_ANY, mxSource::OnUpdateUI)
	EVT_STC_DWELLSTART (wxID_ANY, mxSource::OnToolTipTime)
	EVT_STC_DWELLEND (wxID_ANY, mxSource::OnToolTipTimeOut)
	EVT_STC_SAVEPOINTREACHED(wxID_ANY, mxSource::OnSavePointReached)
	EVT_STC_SAVEPOINTLEFT(wxID_ANY, mxSource::OnSavePointLeft)
	EVT_LEFT_DOWN(mxSource::OnClick)
	EVT_LEFT_UP(mxSource::OnClickUp)
	EVT_RIGHT_DOWN(mxSource::OnPopupMenu)
	EVT_STC_ROMODIFYATTEMPT (wxID_ANY, mxSource::OnModifyOnRO)
	EVT_STC_DOUBLECLICK (wxID_ANY, mxSource::OnDoubleClick)
	
	EVT_STC_PAINTED(wxID_ANY, mxSource::OnPainted)
	
	EVT_KEY_DOWN(mxSource::OnKeyDown)
	EVT_STC_MACRORECORD(wxID_ANY,mxSource::OnMacroAction)
	EVT_STC_AUTOCOMP_SELECTION(wxID_ANY,mxSource::OnAutocompSelection)
	
	EVT_TIMER(wxID_ANY,mxSource::OnTimer)
	EVT_MOUSEWHEEL(mxSource::OnMouseWheel)
	
END_EVENT_TABLE()

mxSource::mxSource (wxWindow *parent, wxString ptext, project_file_item *fitem) 
	: wxStyledTextCtrl (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL)
	, focus_helper(this), autocomp_helper(this)
{

	// LC_CTYPE and LANG env vars are altered in the launcher, so this is commented now
	// with this patch, text shows ok, but files can be saved with utf8 encoding and then it's shown differently when oppened somewhere else with the same zinjai
//#ifndef __WIN32__
//	// esto evita problemas en los ubuntus en castellano donde al agregar acentos, ñs y esas cosas, se desfaza el cursor, o al borrar se borra mal
//	if (wxLocale::GetSystemEncoding()==wxFONTENCODING_UTF8) 
//		SetCodePage(wxSTC_CP_UTF8);
//#endif
	
//	AutoCompSetDropRestOfWord(true); // esto se torna muy molesto en muchos casos (por ejemplo, intentar agregar unsigned antes de int), mejor no usar
	
	calltip = nullptr;	calltip_mode = MXS_NULL; inspection_baloon = nullptr;
	
	old_current_line=-1000;
	
	brace_1=-1; brace_2=-1;
	
	next_source_with_same_file=this;
	
	source_time_dont_ask = false; source_time_reload = false;
	diff_brother=nullptr; first_diff_info=last_diff_info=nullptr;
	
	readonly_mode = ROM_NONE;
	
	LoadSourceConfig();
	
	if (fitem) {
		m_extras = &fitem->extras;
		m_owns_extras=false;
		treeId=fitem->item;
		if (fitem->read_only) SetReadOnlyMode(ROM_ADD_PROJECT);
	} else {
		m_owns_extras=true;
		m_extras = new SourceExtras();
	}
	
	ignore_char_added=false;
	
	page_text = ptext;
	
	RegisterImage(2,*(bitmaps->parser.icon02_define));
	RegisterImage(3,*(bitmaps->parser.icon03_func));
	RegisterImage(4,*(bitmaps->parser.icon04_class));
	RegisterImage(5,*(bitmaps->parser.icon05_att_unk));
	RegisterImage(6,*(bitmaps->parser.icon06_att_pri));
	RegisterImage(7,*(bitmaps->parser.icon07_att_pro));
	RegisterImage(8,*(bitmaps->parser.icon08_att_pub));
	RegisterImage(9,*(bitmaps->parser.icon09_mem_unk));
	RegisterImage(10,*(bitmaps->parser.icon10_mem_pri));
	RegisterImage(11,*(bitmaps->parser.icon11_mem_pro));
	RegisterImage(12,*(bitmaps->parser.icon12_mem_pub));
	RegisterImage(13,*(bitmaps->parser.icon13_none));
	RegisterImage(14,*(bitmaps->parser.icon14_global_var));
	RegisterImage(15,*(bitmaps->parser.icon15_res_word));
	RegisterImage(16,*(bitmaps->parser.icon16_preproc));
	RegisterImage(17,*(bitmaps->parser.icon17_doxygen));
	RegisterImage(18,*(bitmaps->parser.icon18_typedef));
	RegisterImage(19,*(bitmaps->parser.icon19_enum_const));
	RegisterImage(20,*(bitmaps->parser.icon20_argument));
	RegisterImage(21,*(bitmaps->parser.icon21_local));
	
	lexer = wxSTC_LEX_CPP;
	
	config_running = config->Running;
	
	source_filename = wxEmptyString;
	temp_filename = DIR_PLUS_FILE(config->temp_dir,"sin_titulo.cpp");
	binary_filename = DIR_PLUS_FILE(config->temp_dir,"sin_titulo")+_T(BINARY_EXTENSION);
	working_folder = wxFileName::GetHomeDir();

	sin_titulo = true;
	cpp_or_just_c = true;
	never_parsed = true;
	first_view = true;
//	current_line = 0; current_marker = -1;
	
//	SetViewEOL (false); // no mostrar fin de lineas
//	SetIndentationGuides (true); 
//	SetViewWhiteSpace(config_source.whiteSpace?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
	SetEdgeColumn (80);
	SetEdgeMode (true?wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);
//	SetViewWhiteSpace (wxSTC_WS_INVISIBLE);
	SetOvertype (config_source.overType);
	config_source.wrapMode = config->Init.wrap_mode==2||(config->Init.wrap_mode==1&&!config_source.syntaxEnable);
	SetWrapMode (config_source.wrapMode?wxSTC_WRAP_WORD: wxSTC_WRAP_NONE);
	SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END|wxSTC_WRAPVISUALFLAG_START);
	
	wxFont font (config->Styles.font_size, wxMODERN, wxNORMAL, wxNORMAL);
	StyleSetFont (wxSTC_STYLE_DEFAULT, font);

	SetKeyWords (0, mxSourceWords1);
	SetKeyWords (1, mxSourceWords2);
	SetKeyWords (2, mxSourceWords3);

	//SetCaretLineBackground("Z LIGHT BLUE");
	//SetCaretLineVisible(true);

	// numeros de linea
//	SetMarginWidth (MARGIN_LINENUM, TextWidth (wxSTC_STYLE_LINENUMBER, " XXX"));

	// set margin as unused
	SetMarginType (MARGIN_BREAKS, wxSTC_MARGIN_SYMBOL);
	SetMarginWidth (MARGIN_BREAKS, 16);
	SetMarginSensitive (MARGIN_BREAKS, true);

	// folding
	SetMarginType (MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
	SetMarginMask (MARGIN_FOLD, wxSTC_MASK_FOLDERS);
	SetMarginWidth (MARGIN_FOLD, 0);
	SetMarginSensitive (MARGIN_FOLD, false);
	// folding enable
	if (config_source.foldEnable) {
		SetMarginWidth (MARGIN_FOLD, true? 15: 0);
		SetMarginSensitive (MARGIN_FOLD, 1);
		SetProperty("fold", true?"1":"0");
		SetProperty("fold.comment",true?"1":"0");
		SetProperty("fold.compact",true?"1":"0");
		SetProperty("fold.preprocessor",true?"1":"0");
		SetProperty("fold.html.preprocessor",true?"1":"0");
		SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
	}

	// set visibility
	SetVisiblePolicy (wxSTC_VISIBLE_STRICT|wxSTC_VISIBLE_SLOP, 1);
	SetXCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
	SetYCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
//	SetVisiblePolicy(wxSTC_CARET_SLOP,50);
//	SetXCaretPolicy(wxSTC_CARET_SLOP,50);
//	SetYCaretPolicy(wxSTC_CARET_SLOP,50);
		
	SetEndAtLastLine(!config->Init.autohide_panels);

	AutoCompSetSeparator('\n');
	AutoCompSetIgnoreCase(true);
	AutoCompSetAutoHide(false);
	AutoCompSetTypeSeparator('$');
	
	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetStyle(1,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground (0, 0x0000ff);
	IndicatorSetForeground (1, 0x550055);
	
	// lineas que no se compilan
	IndicatorSetStyle(2,wxSTC_INDIC_STRIKE);
	IndicatorSetForeground (2, 0x005555);

	if (config_source.syntaxEnable) {
		lexer=wxSTC_LEX_CPP;
		SetStyle(true);
	}
	
	SetColours(false);
	
	SetLayoutCache (wxSTC_CACHE_PAGE);
	
	SetDropTarget(new mxDropTarget(this));
	UsePopUp(false);
#ifndef __WIN32__
	// el default para BufferedDraw es true, pero por alguna razón en algun momento
	// lo puse en false, tal vez por velocidad o problemas de refresco
	// en gtk parece no cambiar nada, pero en windows genera un flickering molesto
	// (para verlo solo basta pararse al comienzo de una linea con codigo y esperar)
	SetBufferedDraw(false); 
#endif
//	SetTwoPhaseDraw (false);
	SetMouseDwellTime(1000); // mis tooltips bizarros (con showbaloon = calltip)
	
	if (debug->IsDebugging() && !config->Debug.allow_edition) SetReadOnlyMode(ROM_ADD_DEBUG);

	er_register_source(this);
	
}


mxSource::~mxSource () {
	HideCalltip(); if (calltip) { calltip->Destroy(); calltip=nullptr; }
	
	bool only_view=next_source_with_same_file==this; // there can be more than one view of the same source
	
	if (diff_brother) diff_brother->SetDiffBrother(nullptr); diff_brother=nullptr;
	while (first_diff_info) delete first_diff_info;
	
	navigation_history.OnClose(this);
	parser->UnregisterSource(this);
	debug->UnregisterSource(this);
	if (main_window) main_window->UnregisterSource(this);
	er_unregister_source(this);
	if (main_window) {
		if (compiler->last_compiled==this) compiler->last_compiled=nullptr;
		if (compiler->last_runned==this) compiler->last_runned=nullptr;
	}
	
	if (!only_view) {
		mxSource *iter=this;
		while (iter->next_source_with_same_file!=this)
			iter=iter->next_source_with_same_file;
		iter->next_source_with_same_file=next_source_with_same_file;
		m_extras->ChangeSource(next_source_with_same_file);
		if (m_owns_extras) next_source_with_same_file->m_owns_extras=true;
	} else {
		// si no es un fuente de un proyecto, tiene la resposabilidad de liberar la memoria de los breakpoints
		m_extras->ChangeSource(nullptr);
		if (m_owns_extras) delete m_extras; /// @todo: esto puede traer problemas en combinacion con el split
	}
	
}

void mxSource::OnEditDeleteLines (wxCommandEvent &event) {
	int ss,se;
	int min=LineFromPosition(ss=GetSelectionStart());
	int max=LineFromPosition(se=GetSelectionEnd());
	if (max==min) {
		LineDelete();
		if (LineFromPosition(ss)!=min)
			GotoPos(GetLineEndPosition(min));
		else
			GotoPos(ss);
	} else {
		if (min>max) { int aux=min; min=max; max=aux; /*aux=ss; */ss=se; /*se=aux;*/}
		if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
		GotoPos(ss);
		UndoActionGuard undo_action(this);
		for (int i=min;i<=max;i++)
			LineDelete();
		if (LineFromPosition(ss)!=min) 
			GotoPos(GetLineEndPosition(min));
		else 
			GotoPos(ss);
	}
}

void mxSource::OnEditGotoMark (wxCommandEvent &event) {
	int cl=GetCurrentLine(), lc=GetLineCount();
	int i=cl+1;
	while (!(MarkerGet(i)&(1<<mxSTC_MARK_USER)) && i<lc)
		i++;
	if (i<lc) {
		GotoPos(GetLineIndentPosition(i));
		EnsureVisibleEnforcePolicy(GetCurrentLine());
		return;
	}
	i=0;
	while (!(MarkerGet(i)&(1<<mxSTC_MARK_USER)) && i<cl)
		i++;
	if (i<cl) {
		GotoPos(GetLineIndentPosition(i));
		EnsureVisibleEnforcePolicy(GetCurrentLine());
		return;
	}
}

void mxSource::OnEditMarkLines (wxCommandEvent &event) {
	int min=LineFromPosition(GetSelectionStart());
	int max=LineFromPosition(GetSelectionEnd());
	if (max==min) {
		if (MarkerGet(min)&(1<<mxSTC_MARK_USER))
			MarkerDelete(min,mxSTC_MARK_USER);
		else
			MarkerAdd(min,mxSTC_MARK_USER);
	} else {
			if (min>max) { int aux=min; min=max; max=aux; }
			if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
			int mark=true;
			for (int i=min;i<=max;i++)
				if (MarkerGet(i)&(1<<mxSTC_MARK_USER)) {
					mark=false;
					break;
				}
			for (int i=min;i<=max;i++)
				if (mark)
					MarkerAdd(i,mxSTC_MARK_USER);
				else
					MarkerDelete(i,mxSTC_MARK_USER);
	}
	Refresh();
}

void mxSource::OnEditDuplicateLines (wxCommandEvent &event) {
	int ss,se;
	int min=LineFromPosition(ss=GetSelectionStart());
	int max=LineFromPosition(se=GetSelectionEnd());
	UndoActionGuard undo_action(this);
	if (max==min) {
		LineDuplicate();
	} else {
		if (min>max) { 
			int aux=min; 
			min=max; 
			max=aux;
		}
		if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
		wxString text;
		for (int i=min;i<=max;i++)
			text+=GetLine(i);
		InsertText(PositionFromLine(max+1),text);
		SetSelection(ss,se);
	}
}

void mxSource::OnEditRedo (wxCommandEvent &event) {
    if (!CanRedo()) return;
    Redo ();
	EnsureVisibleEnforcePolicy(GetCurrentLine());
}

void mxSource::OnEditUndo (wxCommandEvent &event) {
    if (!CanUndo()) return;
    Undo ();
	EnsureVisibleEnforcePolicy(GetCurrentLine());
}

//void mxSource::OnEditClear (wxCommandEvent &event) {
//    if (GetReadOnly()) return;
//    Clear ();
//}

void mxSource::OnEditCut (wxCommandEvent &event) {
//    if (GetReadOnly() || (GetSelectionEnd()-GetSelectionStart() <= 0)) return;
//	int ss = GetSelectionStart(), se = GetSelectionEnd();
//	if (se==ss) return;
//	if (se<ss) { int aux=ss; ss=se; se=aux; }
//	wxTextDataObject data;
//	if (wxTheClipboard->Open()) {
//		wxTheClipboard->SetData( new wxTextDataObject(GetTextRange(ss,se)) );
//		wxTheClipboard->Close();
//	}
//	ReplaceSelection("");
	Cut();
}

void mxSource::OnEditCopy (wxCommandEvent &event) {
//	int ss = GetSelectionStart(), se = GetSelectionEnd();
//    if (se==ss) return;
//	if (se<ss) { int aux=ss; ss=se; se=aux; }
//	wxTextDataObject data;
//	if (wxTheClipboard->Open()) {
//		wxTheClipboard->SetData( new wxTextDataObject(GetTextRange(ss,se)) );
//		wxTheClipboard->Close();
//	}
    Copy ();
}

void mxSource::OnEditPaste (wxCommandEvent &event) {
	if (config_source.indentPaste && config_source.syntaxEnable) {
		wxString str = mxUT::GetClipboardText();
		if (str.IsEmpty()) return;
		UndoActionGuard undo_action(this);
		// borrar la seleccion previa
		if (GetSelectionEnd()-GetSelectionStart()!=0)
			DeleteBack();
		// insertar el nuevo texto
		int cp = GetCurrentPos();
		InsertText(cp,str);
		ignore_char_added=false;
		// indentar el nuevo texto
		Colourise(cp,cp+str.Len());
		int l1 = LineFromPosition(cp);
		int l2 = LineFromPosition(cp+str.Len());
		int pos = PositionFromLine(l1);
		char c;
		int p=cp-1;
		while (p>=pos && II_IS_2(p,' ','\t'))
			p--;
		if (p!=pos && p>=pos) 
			l1++;
//		p=cp+data.GetTextLength();
//		if ((unsigned int)PositionFromLine(l2)==cp+str.Len())
//			l2--;
		//			pos=GetLineEndPosition(l2);
		//			while (p<=pos && II_IS_2(p,' ','\t'))
		//				p++;
		//			if (p<=pos && c!='\n' && c!='\r')
		//				l2--;
		// indentar y acomodar el cursor
		cp+=str.Len();
		if (l2>=l1) {
			c=LineFromPosition(cp);
			pos=cp-GetLineIndentPosition(c);
			if (pos<0) pos=0;
			Indent(l1,l2);
			pos+=GetLineIndentPosition(c);
			SetSelection(pos,pos);
		} else
			SetSelection(cp,cp);
		// is that the problem?
		HideCalltip();
	} else {
		if (CanPaste()) Paste();
	}
}


void mxSource::OnBraceMatch (wxCommandEvent &event) {
	int min = GetCurrentPos();
	char c=GetCharAt(min);
	if (c!='(' && c!=')' && c!='{' && c!='}' && c!='[' && c!=']') {
		min--;  c=GetCharAt(min);
	}
	if (c!='(' && c!=')' && c!='{' && c!='}' && c!='[' && c!=']') {
		int omin=min-1;
		while (min>=0 && (c=GetCharAt(min))!='(' && c!='{'&& c!='[') {
			if ( (c==']' || c==')' || c=='}') && omin!=min )
				if ((min=BraceMatch(min))==wxSTC_INVALID_POSITION)
					return;
			min--;
		}
		if (min<0) return;
	}
	int max = BraceMatch (min);
	if (max<0 && min>0) max = BraceMatch(--min);
	if (max>0/* && max<min*/) {
//		int aux=min; 
//		min=max; 
//		max=aux;
//	}
//	if (max > min) 
		BraceHighlight (min+1, max);
		if (max > min) 
			SetSelection (min, max+1);
		else
			SetSelection (min+1, max);
	} else {
		BraceBadLight (min);
	}
}

void mxSource::OnFoldToggle (wxCommandEvent &event) {
    ToggleFold (GetFoldParent(GetCurrentLine()));
}

void mxSource::OnMarginClick (wxStyledTextEvent &event) {
	
    if (event.GetMargin() == MARGIN_FOLD) { // margen del folding
        int lineClick = LineFromPosition (event.GetPosition());
        int levelClick = GetFoldLevel (lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0) {
            ToggleFold (lineClick);
        }
		
	} else { // margen de los puntos de interrupcion
		
		// buscar si habia un breakpoint en esa linea
		int l = LineFromPosition (event.GetPosition());
		BreakPointInfo *bpi=m_extras->FindBreakpointFromLine(this,l);
		
		// si apretó shift o ctrl (por alguna razon en linux solo me anda shift) mostrar el cuadro de opciones
		if (event.GetModifiers()&wxSTC_SCMOD_SHIFT || event.GetModifiers()&wxSTC_SCMOD_CTRL) {
			if (!debug->IsDebugging() || debug->CanTalkToGDB()) {
				if (!bpi) { // si no habia, lo crea
					bpi=new BreakPointInfo(this,l);
					if (debug->IsDebugging()) debug->SetBreakPoint(bpi);
					else bpi->SetStatus(BPS_UNKNOWN);
				}
				new mxBreakOptions(bpi); // muestra el dialogo de opciones del bp
			}
		
		// si hay que sacar el breakpoint
		} else if (bpi) { 
			if (debug->IsDebugging()) debug->DeleteBreakPoint(bpi); // debug se encarga de hacer el delete y eso se lleva el marker en el destructor
			else delete bpi;
		
		// si hay que poner un breakpoint nuevo
		} else { 
			if (IsEmptyLine(l)) { // no dejar poner un pto de interrupción en una línea que no tiene código
				mxMessageDialog(main_window,LANG(DEBUG_NO_BREAKPOINT_ON_EMPTY_LINE,""
					"Los puntos de interrupcion no pueden colocarse en lineas vacias\n"
					" que contengan solo comentarios o directivas de preprocesador"),
					LANG(DEBUG_BREAKPOINT_CAPTION,"Puntos de Interrupcion"),mxMD_INFO|mxMD_OK).ShowModal();
				return;
			}
			bpi=new BreakPointInfo(this,l);
			if (debug->IsDebugging()) debug->LiveSetBreakPoint(bpi); // esta llamada cambia el estado del bpi y eso pone la marca en el margen
			else bpi->SetStatus(BPS_SETTED);
		}
		
	}
}

//void mxSource::OnEditSelectLine (wxCommandEvent &event) {
//	int lineStart = PositionFromLine (GetCurrentLine());
//	int lineEnd = PositionFromLine (GetCurrentLine() + 1);
//	SetSelection (lineStart, lineEnd);
// }

void mxSource::OnEditSelectAll (wxCommandEvent &event) {
	SetSelection (0, GetTextLength ());
}

struct auxMarkersConserver {
	struct amc_aux {
		bool um; // user mark
		BreakPointInfo *bpi;
		void Get(mxSource *s, int l) { 
			um = s->MarkerGet(l)&(1<<mxSTC_MARK_USER);
			if (um) s->MarkerDelete(l,mxSTC_MARK_USER);
			bpi = s->m_extras->FindBreakpointFromLine(s,l);
			if (bpi) {
				s->MarkerDeleteHandle(bpi->marker_handle);
				bpi->marker_handle=-1;
			}
		}
		void Set(mxSource *s, int l) { 
			if (um) s->MarkerAdd(l,mxSTC_MARK_USER);
			if (bpi) { bpi->line_number=l; bpi->SetMarker(); }
		}
	};
	amc_aux v[1000];
	bool u;
	int m,n; 
	mxSource *s;
	auxMarkersConserver(mxSource *src, int min, int max, bool up) {
		u=up; s=src; n = max-min+2; m = min-1;
		if (n+1>1000) { n=0; return; }
		for(int i=0;i<=n;i++) v[i].Get(s,m+i);
	}
	~auxMarkersConserver() {
		if (u) {
			for(int i=1;i<n;i++) v[i].Set(s,m+i-1);
			v[0].Set(s,m+n-1); v[n].Set(s,m+n);
		} else {
			v[0].Set(s,m); v[n].Set(s,m+1);
			for(int i=1;i<n;i++) v[i].Set(s,m+i+1);
		}
	}
};

void mxSource::OnEditToggleLinesUp (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	if (min>0) {
		UndoActionGuard undo_action(this);
		wxString line = GetLine(min-1);
		if (max==GetLineCount()-1) AppendText("\n");
		auxMarkersConserver aux_mc(this,min,max,true);
		SetTargetStart(PositionFromLine(max+1));
		SetTargetEnd(PositionFromLine(max+1));
		ReplaceTarget(line);
		SetTargetStart(PositionFromLine(min-1));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget("");
		EnsureVisibleEnforcePolicy(min);
	}
}

void mxSource::OnEditToggleLinesDown (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (PositionFromLine(min)==ss) ss=-1;
	if (PositionFromLine(max)==se) se=-1;
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	if (max+1<GetLineCount()) {
		auxMarkersConserver aux_mc(this,min,max,false);
		UndoActionGuard undo_action(this);
		wxString line = GetLine(max+1);
		SetTargetStart(GetLineEndPosition(max));
		SetTargetEnd(GetLineEndPosition(max+1));
		ReplaceTarget("");
		SetTargetStart(PositionFromLine(min));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(line);
		if (ss==-1) SetSelectionStart(PositionFromLine(min+1));
		if (se==-1) SetSelectionStart(PositionFromLine(min+1));
		EnsureVisibleEnforcePolicy(max);
	}	
}

void mxSource::OnComment (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min==max && se!=ss) {
		ReplaceSelection(wxString("/*")<<GetSelectedText()<<"*/");
		SetSelection(ss,se+4);
		return;
	}
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	UndoActionGuard undo_action(this);
	if (cpp_or_just_c) {
		for (int i=min;i<=max;i++) {
			//if (GetLine(i).Left(2)!="//") {
			SetTargetStart(PositionFromLine(i));
			SetTargetEnd(PositionFromLine(i));
			ReplaceTarget("//");
		}	
	} else {
		for (int i=min;i<=max;i++) {
			int lp=PositionFromLine(i), ll=GetLineEndPosition(i); 
			while (true) {
				int s; char c; // para los II_*
				int l0=lp;
				while (lp<ll && II_IS_2(lp,' ','\t')) lp++;
				if (lp==ll) break;
				if (II_IS_COMMENT(lp)) {
					while (lp<ll && II_IS_COMMENT(lp)) lp++;
					if (lp==ll) break;
				} else {
					int le=lp;
					while (le<ll && !II_IS_COMMENT(le)) le++;
					SetTargetStart(le); SetTargetEnd(le); ReplaceTarget("*/");
					SetTargetStart(l0); SetTargetEnd(l0); ReplaceTarget("/*");
					lp=le+4; ll+=4;
				}
			}
		}
	}
}

void mxSource::OnUncomment (wxCommandEvent &event) {
	int ss = GetSelectionStart();
	int min=LineFromPosition(ss);
	
	int max=LineFromPosition(GetSelectionEnd());
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	UndoActionGuard undo_action(this);
	bool did_something=false;
	for (int i=min;i<=max;i++) {
		int s, p=GetLineIndentPosition(i); char c=GetCharAt(p+1);; // s y c se reutilizan para II_*
		if (II_IS_COMMENT(p)) {
			bool remove_asterisco_barra=false, add_barra_asterisco=false;
			if (GetCharAt(p)=='/' && (c=='/' || c=='*')) {
				SetTargetStart(p);
				SetTargetEnd(p+2);
				ReplaceTarget("");
				remove_asterisco_barra = c=='*';
			} else if (i>0) {
				p=GetLineEndPosition(i-1);
				SetTargetStart(p);
				SetTargetEnd(p);
				ReplaceTarget("*/");
				p+=2;
				remove_asterisco_barra=add_barra_asterisco=true;
			}
			if (remove_asterisco_barra||add_barra_asterisco) {
				int p2=p,pl=GetLineEndPosition(i);
				while (p2<pl && !(GetCharAt(p2)=='*' && GetCharAt(p2+1)=='/') ) p2++;
				if (p2!=pl) {
					SetTargetStart(p2);
					SetTargetEnd(p2+2);
					ReplaceTarget("");
					Colourise(p,p2);
				} else if (add_barra_asterisco && i+1<GetLineCount()) {
					p=PositionFromLine(i+1);
					SetTargetStart(p);
					SetTargetEnd(p);
					ReplaceTarget("/*");
				}
				did_something=true; // can we just return now?
			} 
		}
	}
	
	// si no hizo nada por linea, recupera el funcionamiento original para comentarios con /* y */ dentro de una linea
	if (!did_something && GetStyleAt(ss)==wxSTC_C_COMMENT && GetLine(min).Left((GetLineIndentPosition(min))-PositionFromLine(min)+2).Right(2)!="//") {
		int se=ss, l=GetLength();
		while (ss>0 && (GetCharAt(ss)!='/' || GetCharAt(ss+1)!='*') )
			ss--;
		if (!se) 
			se++;
		while (se<l && (GetCharAt(se-1)!='*' || GetCharAt(se)!='/') )
			se++;
		if (GetCharAt(se)=='/' && GetCharAt(se-1)=='*') {
			SetTargetStart(se-1);
			SetTargetEnd(se+1);
			ReplaceTarget("");
		}
		if (GetCharAt(ss)=='/' && GetCharAt(ss+1)=='*') {
			SetTargetStart(ss);
			SetTargetEnd(ss+2);
			ReplaceTarget("");
		}
		SetSelection(ss,se-3);
		return;
	}
	
}


bool mxSource::LoadFile (const wxFileName &filename) {
	wxString ext = filename.GetExt().MakeLower();
	if (mxUT::ExtensionIsCpp(ext) || mxUT::ExtensionIsH(ext)) {
		SetStyle(wxSTC_LEX_CPP);
	} else if (ext=="htm" || ext=="html") {
		SetStyle(wxSTC_LEX_HTML);
	} else if (ext=="sh") {
		SetStyle(wxSTC_LEX_BASH);
	} else if (ext=="xml") {
		SetStyle(wxSTC_LEX_XML);
	} else if (filename.GetName().MakeUpper()=="MAKEFILE") {
		SetStyle(wxSTC_LEX_MAKEFILE);
	} else {
		SetStyle(wxSTC_LEX_NULL);
	}
	
	source_filename = filename;
	source_time = source_filename.GetModificationTime();
	cpp_or_just_c = source_filename.GetExt().Lower()!="c";
	working_folder = filename.GetPath();
	if (project)
		binary_filename="binary_filename_not_seted";
	else
		binary_filename=source_filename.GetPathWithSep()+source_filename.GetName()+_T(BINARY_EXTENSION);
	SetReadOnly(false); // para que wxStyledTextCtrl::LoadFile pueda modificarlo
	ClearAll ();
	sin_titulo=!wxStyledTextCtrl::LoadFile(source_filename.GetFullPath());
	EmptyUndoBuffer();
	if (readonly_mode!=ROM_NONE) SetReadOnly(true);
	config_source.wrapMode = config->Init.wrap_mode==2||(config->Init.wrap_mode==1&&(lexer!=wxSTC_LEX_CPP||!config_source.syntaxEnable));
	SetWrapMode (config_source.wrapMode?wxSTC_WRAP_WORD: wxSTC_WRAP_NONE);
	SetLineNumbers();
	return true;
}

bool mxSource::SaveSource() {
	int lse = GetEndStyled();
	StartStyling(0,wxSTC_INDICS_MASK);
	SetStyling(GetLength(),0);
	StartStyling(lse,0x1F);
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!="")
		AppendText("\n");
	sin_titulo = false;
	bool ret=MySaveFile(source_filename.GetFullPath());
	SetSourceTime(source_filename.GetModificationTime());
	if (source_filename==config->Files.autocodes_file||(project&&source_filename==DIR_PLUS_FILE(project->path,project->autocodes_file)))
		autocoder->Reset(project?project->autocodes_file:"");
	return ret;
}

bool mxSource::SaveTemp (wxString fname) {
	bool mod = GetModify();
	bool ret=MySaveFile(fname);
	SetModify(mod);
	return ret; 
}

bool mxSource::SaveTemp () {
	bool mod = GetModify();
	int lse = GetEndStyled();
	StartStyling(0,wxSTC_INDICS_MASK);
	SetStyling(GetLength(),0);
	StartStyling(lse,0x1F);
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!="")
		AppendText("\n");
	if (sin_titulo)
		binary_filename=temp_filename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR)+temp_filename.GetName()+_T(BINARY_EXTENSION);
	bool ret=MySaveFile(temp_filename.GetFullPath());
	SetModify(mod);
	return ret; 
}

bool mxSource::SaveSource (const wxFileName &filename) {
	int lse = GetEndStyled();
	StartStyling(0,wxSTC_INDICS_MASK);
	SetStyling(GetLength(),0);
	StartStyling(lse,0x1F);
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!="")
		AppendText("\n");
	if (MySaveFile(filename.GetFullPath())) {
		source_filename = filename;
		working_folder = filename.GetPath();
		cpp_or_just_c = source_filename.GetExt().Lower()!="c";
		if (project)
			binary_filename=source_filename.GetPathWithSep()+source_filename.GetName()+".o";
		else
			binary_filename=source_filename.GetPathWithSep()+source_filename.GetName()+_T(BINARY_EXTENSION);
		sin_titulo = false;
		bool ret=MySaveFile(source_filename.GetFullPath());
		SetSourceTime(source_filename.GetModificationTime());
		return ret;
	}
	return false;
}


void mxSource::OnUpdateUI (wxStyledTextEvent &event) {
	if (calltip_mode==MXS_AUTOCOMP && calltip && !wxStyledTextCtrl::AutoCompActive()) HideCalltip();
	int cl=GetCurrentLine();
	if (first_view) {
		ScrollToColumn(0);
		first_view=false;
	} else {
		if (calltip_mode==MXS_BALOON) HideCalltip();
		else if (calltip_mode==MXS_CALLTIP) {
			if (GetCurrentLine()!=calltip_line) HideCalltip();
			else {
				int cp=GetCurrentPos();
				if (cp<=calltip_brace) { 
					HideCalltip();
				} else {
					int p=calltip_brace+1, cur_arg=0, par=0, l=cp/*+1*/; // s y l son para II_*
					while (p<l) {
						int s;
						II_FRONT(p, II_SHOULD_IGNORE(p));
						char c=GetCharAt(p++);
						if (c==',' && par==0) { cur_arg++; }
						else if (c=='(' || c=='[') { par++; }
						else if (c==')' || c=='}') { par--; }
					}
					calltip->SetArg(cur_arg);
				}
			}
		}
	}
	navigation_history.OnJump(this,cl);
	if (!config_source.lineNumber) main_window->status_bar->SetStatusText(wxString("Lin ")<<cl<<" - Col "<<GetCurrentPos()-PositionFromLine(cl),1);
	
	
	// tomado de pseint, para resaltar las partes a completar
//	if (GetStyleAt(p)&wxSTC_INDIC2_MASK) {
//		int p2=p;
//		while (GetStyleAt(p2)&wxSTC_INDIC2_MASK)
//			p2++;
//		while (GetStyleAt(p)&wxSTC_INDIC2_MASK)
//			p--;
//		int s1=GetAnchor(), s2=GetCurrentPos();
//		if (s1==s2) {
//			if (s1==p+1 && last_s1==p+1 && last_s2==p2) {
//				SetSelection(p,p);
//			} else {
//				SetAnchor(p+1);
//				SetCurrentPos(p2);
//			}
//		} else if (s1>s2) {
//			if (s1<p2) SetAnchor(p2);
//			if (s2>p+1) SetCurrentPos(p+1);
//		} else {
//			if (s2<p2) SetCurrentPos(p2);
//			if (s1>p+1) SetAnchor(p+1);
//		}
//		last_s1=GetSelectionStart(); last_s2=GetSelectionEnd();
//	}
	if (rect_sel) ApplyRectEdit();
}


void mxSource::OnCharAdded (wxStyledTextEvent &event) {
	if (ignore_char_added) return;
	char chr = (char)event.GetKey();
	if (calltip_mode==MXS_AUTOCOMP) {
		if (!II_IS_KEYWORD_CHAR(chr)) HideCalltip();
		else if (config_source.autocompFilters) 
			code_helper->FilterAutocomp(this,GetTextRange(autocomp_helper.GetBasePos(),GetCurrentPos()));
	}
	if (config_source.autocloseStuff) {
		int pos=GetCurrentPos();
		if ((chr==']'||chr==')'|| chr=='\''||chr=='}'||chr=='\"') && GetCharAt(pos)==chr) {
			SetTargetStart(pos);
			SetTargetEnd(pos+1);
			ReplaceTarget(""); 
			return;
		}
		if (chr=='<' && GetStyleAt(pos)==wxSTC_C_PREPROCESSOR && pos>GetLineIndentPosition(LineFromPosition(pos))+8&&GetTextRange(GetLineIndentPosition(LineFromPosition(pos)),GetLineIndentPosition(LineFromPosition(pos))+8)=="#include") { 
			InsertText(pos++,">" );
		} else if (chr=='(') { 
			InsertText(pos++,")" );
		} else if (chr=='{') { 
			InsertText(pos++,"}" );
		} else if (chr=='[') { 
			InsertText(pos++,"]" );
		} else if (chr=='\"') { 
			InsertText(pos++,"\"" );
		} else if (chr=='\'') { 
			InsertText(pos++,"\'" );
		} 
	}
	if (config_source.smartIndent && config_source.syntaxEnable) {
		if (chr=='e') {
			// si es una 'e', vemos si decia 'else' solo en esa linea y contamos cuantos ifs para atras empezo e identamos igual que el if correspondiente
			int e=GetCurrentPos(), p=PositionFromLine(GetCurrentLine());
			while (p<e && (GetCharAt(p)==' ' || GetCharAt(p)=='\t')) 
				p++;
			if (p+4==e && GetTextRange(p,e)=="else") {
				char c; int s;
				p=PositionFromLine(GetCurrentLine())-1;
				while (p>=0 && ((c=GetCharAt(p))==' ' || c=='\t'  || c=='\r'  || c=='\n' || (s=GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR))
					p--;
				if (p>0) 
					if (c=='}') {
						p=BraceMatch(p);
						if (p==wxSTC_INVALID_POSITION)
								return;
						p--;
						while (p>=0 && ((c=GetCharAt(p))==' ' || c=='\t'  || c=='\r'  || c=='\n' || (s=GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR))
							p--;
					}
					bool baux; e=0;
					while (p>0 && ((baux=!(GetCharAt(p)=='f' && GetCharAt(p-1)=='i' && GetStyleAt(p)==wxSTC_C_WORD)) || e!=0) ) {
						if (!baux)
							e--; 
						else if (p>2 && GetCharAt(p-2)=='l' && GetCharAt(p-1)=='s' && GetCharAt(p)=='e' && GetCharAt(p-3)=='e' && GetStyleAt(p)==wxSTC_C_WORD)
							 e++;
						p--;
					}
					if (p>0)
						SetLineIndentation(GetCurrentLine(),GetLineIndentation(LineFromPosition(p)));
			}
		} else if (chr==':') {
			// si son dos puntos y decia public:, private:, protected:,  default: o case algo: quitamos uno al identado
			int s,e=GetCurrentPos(), p=PositionFromLine(GetCurrentLine());
			Colourise(e-1,e);
			while (p<e && (GetCharAt(p)==' ' || GetCharAt(p)=='\t')) 
				p++;
			if ( (p+7==e && GetTextRange(p,e)=="public:") ||
				(p+8==e && GetTextRange(p,e)=="private:") ||
				(p+10==e && GetTextRange(p,e)=="protected:") ||
				(p+8==e && GetTextRange(p,e)=="default:") ||
				(p+6<e && GetTextRange(p,p+5)=="case " && (s=GetStyleAt(e))!=wxSTC_C_CHARACTER && s!=wxSTC_C_STRING && s!=wxSTC_C_STRINGEOL) 
				)  {
					char c; e=p-1;
					while (e>0 && (c=GetCharAt(e))!='{') {
						if (c=='}' && !II_SHOULD_IGNORE(e)) 
							if ((e=BraceMatch(e))==wxSTC_INVALID_POSITION) {
								e=GetLineIndentation(p=GetCurrentLine())-config_source.tabWidth;
								SetLineIndentation(p,e<0?0:e);
							}
						e--;
					}
					SetLineIndentation(GetCurrentLine(),GetLineIndentation(LineFromPosition(e)));
			}
		} else if (chr=='#') {
			int p=GetCurrentPos()-2, l=GetCurrentLine();
			int e=PositionFromLine(l);
			char c;
			while (e<=p && ((c=GetCharAt(p))==' ' || c=='\t'))
				p--;
			if (e>p)
				SetLineIndentation(GetCurrentLine(),0);
//		} else if (chr=='#') {
//			int p=GetCurrentPos()-1;
//			Colourise(p,p+1);
//			if (GetStyleAt(p)==wxSTC_C_PREPROCESSOR)
//				SetLineIndentation(GetCurrentLine(),0);
		} else if (chr=='{') {
			int p=GetCurrentPos()-2, l=GetCurrentLine();
			int e=PositionFromLine(l);
			char c;
			while (e<=p && ((c=GetCharAt(p))==' ' || c=='\t'))
				p--;
			if (e>p)  {
				char c; int s;
				II_BACK(e,II_IS_4(e,' ','\t','\r','\n') || II_SHOULD_IGNORE(e));
					if (c==')' && (e=BraceMatch(e))!=wxSTC_INVALID_POSITION) {
					II_BACK(e,II_IS_4(e,' ','\t','\r','\n') || II_SHOULD_IGNORE(e));
					SetLineIndentation(l,GetLineIndentation(LineFromPosition(e)));
				}
			}
		} else if (chr=='}') {
			// si es una llave que cierra y esta sola, la colocamos a la altura de la que abre
			int p=GetCurrentPos(), s=PositionFromLine(GetCurrentLine());
			p-=2;
			while (p>=s && (GetCharAt(p)==' ' || GetCharAt(p)=='\t')) 
				p--;
			if (p<s) {
				p=GetCurrentPos()-1;
				Colourise(p,p+1);
				p=BraceMatch(p);
				if (p!=wxSTC_INVALID_POSITION)
					SetLineIndentation(GetCurrentLine(),GetLineIndentation(LineFromPosition(p)));
				event.Skip();
			}
			return;
		} else if (chr == '\n') { // si es un enter, es un viaje!!!
			int s, cl=GetCurrentLine();
			int p = GetCurrentPos();
			char c;
			II_FRONT_NC(p,II_IS_2(p,' ','\t') );
			if (GetCharAt(p)=='/' && GetCharAt(p+1)=='/' && GetCharAt(p+2)=='\t') { // si era un comentario, de los que van a la izquierda, dejarlo a la izquierda
				SetLineIndentation(GetCurrentLine(),0);
				return;
			}
			if (GetCharAt(p)=='}') { // si estaba justo antes de la llave que cierra, no agregar un nivel al indentado, mantener
				wxStyledTextEvent evt;
				int op=p; p=BraceMatch(p);
				if (p!=wxSTC_INVALID_POSITION) {
					SetLineIndentation(cl,GetLineIndentation(cl-1));
//				if (cl>0){
//					p = PositionFromLine(cl-1);
//					II_FRONT_NC (p,II_IS_2(p,' ','\t'));
//					if (c=='\r' || c=='\n')  // si agregar un nivel al indentado de la linea anterior
//						SetLineIndentation(cl-1,GetLineIndentation(cl)+config_source.tabWidth);
//				}
					if (LineFromPosition(p)==cl-1) { // si estaban las dos llaves juntas, agregar un enter mas para quedar escribiendo entre medio
						while (++p<op&&(II_IS_NOTHING_4(p)));
						if (p==op) {
							InsertText(PositionFromLine(cl),"\n");
							SetLineIndentation(cl,GetLineIndentation(cl-1)+config_source.tabWidth);
						}
					}
				}
				wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
				return;
			}
			p=GetLineEndPosition(cl-1)-1;
			bool flag;
			II_BACK(p,II_IS_2(p,' ','\t') );
			if (config_source.bracketInsertion && c=='{') { // si estaba en la llave que abre, agregar si se debe la que cierra
				if (cl==GetLineCount()-1)
					AppendText("\n");
				int e=p+1, l=GetLength();
				II_FRONT(e,II_IS_4(e,' ','\t','\r','\n') || II_SHOULD_IGNORE(e));
				if (LineFromPosition(e)==cl) {
					if (c=='}') {
						SetLineIndentation(cl,GetLineIndentation(cl-1));
						wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
						return;
					} else {
						if (c!='}' && LineFromPosition(e)==cl) {
							if ( (e=BraceMatch(p))==wxSTC_INVALID_POSITION || GetLineIndentation(LineFromPosition(e))<GetLineIndentation(cl-1)){
								InsertText(PositionFromLine(cl+1),"}\n");
								SetLineIndentation(cl+1,GetLineIndentation(cl-1));
								// sacar la llave si ya estaba al final de la linea, porque la acabamos de agregar abajo
								int pf = GetLineEndPosition(cl)-1; int opf=pf+1;
								II_BACK(pf,II_SHOULD_IGNORE(pf)||II_IS_2(pf,' ','\t'));
								if (GetCharAt(pf)=='}') {
									pf--;
									II_BACK(pf,II_SHOULD_IGNORE(pf)||II_IS_2(pf,' ','\t'));
									SetTargetStart(pf+1);
									SetTargetEnd(opf);
									ReplaceTarget("");
								}
							}
							SetLineIndentation(cl,GetLineIndentation(cl-1)+config_source.tabWidth);
							wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
							return;
						}
					}
				} else if (e==l || GetLineIndentation(LineFromPosition(e))<GetLineIndentation(cl-1) || ( c!='}' && GetLineIndentation(LineFromPosition(e))==GetLineIndentation(cl-1)  && GetTextRange(e,e+7)!="public:" && GetTextRange(e,e+8)!="private:" && GetTextRange(e,e+10)!="protected:" && GetTextRange(e,e+5)!="case " ) ) {
					e=GetLineIndentPosition(cl-1);
					if (GetTextRange(e,e+8)=="template" && GetStyleAt(e)==wxSTC_C_WORD) {
						e+=8;
						II_FRONT(e,II_IS_2(e,' ','\t') || II_SHOULD_IGNORE(e));
						if (c=='<' && (p=BraceMatch(e))!=wxSTC_INVALID_POSITION) {
							e=p+1;
							II_FRONT(e,II_IS_2(e,' ','\t') || II_SHOULD_IGNORE(e));
						}
					}
					if ( (GetTextRange(e,e+5)=="class" || GetTextRange(e,e+6)=="struct") && GetStyleAt(e)==wxSTC_C_WORD)
						InsertText(PositionFromLine(cl),"\n};");
					else
						InsertText(PositionFromLine(cl),"\n}");
					SetLineIndentation(cl+1,GetLineIndentation(cl-1));
					SetLineIndentation(cl,GetLineIndentation(cl-1)+config_source.tabWidth);
					wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
					return;
				}
			}
			p = GetCurrentPos()-1;

			II_BACK(p, II_IS_4(p,' ','\t','\r','\n') || II_SHOULD_IGNORE(p) );
			if (p==0) { // si se llego al principio del documento no hay nada contra lo que indentar
				SetLineIndentation(cl,GetLineIndentation(0));
				wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
				return;
			}
			
			if (c=='>') { // si era una declaracion de template, mantener el indentado
				int e = BraceMatch(p);
				if (e!=wxSTC_INVALID_POSITION) {
					e--;
					II_BACK(e,II_IS_2(e,' ','\t') || II_SHOULD_IGNORE(e));
					if (e>8 && GetStyleAt(e)==wxSTC_C_WORD && c=='e' && GetTextRange(e-7,e)=="templat") {
						SetLineIndentation(cl,GetLineIndentation(LineFromPosition(e)));
						wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
						return;
					}
				}
			}
			
			if (c=='{' /* || c==':' */ || (p>3 && s==wxSTC_C_WORD && GetCharAt(p-3)=='e' && GetCharAt(p-2)=='l' && GetCharAt(p-1)=='s' && GetCharAt(p)=='e') ) { // si abre llave, incrementar uno en el identado
				int e=p+1, l=GetLength();
				II_FRONT(e,II_IS_4(e,' ','\t','\r','\n') || II_SHOULD_IGNORE(e));
				if (c=='}' && cl==LineFromPosition(e) && cl-1!=LineFromPosition(p)) {
					SetLineIndentation(cl-1,GetLineIndentation(LineFromPosition(p))+config_source.tabWidth);
					SetLineIndentation(cl,GetLineIndentation(LineFromPosition(p)));
				} else
					SetLineIndentation(cl,GetLineIndentation(LineFromPosition(p))+config_source.tabWidth);
				wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
				return;
			}
			bool was_instruction=(c==';' || c=='}');
			// buscar el comienzo de la instruccion
			int l=p, cpm1=GetCurrentPos()-1;
			if (GetCharAt(p)!='}') p--;
			flag=true;
			while (flag) { 
				// retroceder hasta el comienzo de la expresion
				while (p>0 && ( ( !II_IS_2(p,';','{')/* && !(c==':' && GetCharAt(p-1)!=':' && GetCharAt(p+1)!=':')*/ ) || II_SHOULD_IGNORE(p) ) ) {
					// saltar bloques
					if ((c==')' || c=='}') && s!=wxSTC_C_CHARACTER && !II_SHOULD_IGNORE(p)) {
						if ( (p=BraceMatch(p))==wxSTC_INVALID_POSITION ) {
							if (cl!=0)
								SetLineIndentation(cl,GetLineIndentation(cl-1));
							wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
							return;
						} else
							p--;
					} else 
						p--;
				}
				p++;
				II_FRONT_NC(p , p<cpm1 && (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) ) );
				l=p;
				// para que siga si encuentra un else
				if ((flag=(s==wxSTC_C_WORD && GetCharAt(p)=='e' && GetCharAt(p+1)=='l' && GetCharAt(p+2)=='s' && GetCharAt(p+3)=='e'))) {
					p--;
					II_BACK(p,II_IS_4(p,' ','\t','\n','\r') || II_SHOULD_IGNORE(p));
					if (GetCharAt(p)!='}') p--;
				}
			}
			if (was_instruction) {
				if (s==wxSTC_C_WORD && ( (c=='p' && (GetTextRange(l+1,l+6)=="ublic" || GetTextRange(l+1,l+7)=="rivate" || GetTextRange(l+1,l+9)=="rotected") ) || (c=='c' && GetTextRange(l+1,l+4)=="ase") || (c=='d' && GetTextRange(l+1,l+7)=="efault") ) ) 
					SetLineIndentation(cl,GetLineIndentation(LineFromPosition(p))+config_source.tabWidth);
				else
					SetLineIndentation(cl,GetLineIndentation(LineFromPosition(p)));
			} else {
				// si era if/for/while/do/else, volver hasta la instruccion posta
				while (p<cpm1 && GetStyleAt(p)==wxSTC_C_WORD && ( (flag=(
					((c=GetCharAt(p))=='w' && GetCharAt(p+1)=='h' && GetCharAt(p+2)=='i' && GetCharAt(p+3)=='l' && GetCharAt(p+4)=='e' && GetStyleAt(p+5)!=wxSTC_C_WORD)
					|| (c=='f' && GetCharAt(p+1)=='o' && GetCharAt(p+2)=='r' && GetStyleAt(p+3)!=wxSTC_C_WORD)
					|| (c=='i' && GetCharAt(p+1)=='f' && GetStyleAt(p+2)!=wxSTC_C_WORD) ) )
					|| (c=='d' && GetCharAt(p+1)=='o' && GetStyleAt(p+2)!=wxSTC_C_WORD)
					|| (c=='e' && GetCharAt(p+1)=='l' && GetCharAt(p+2)=='s' && GetCharAt(p+3)=='e' && GetStyleAt(p+4)!=wxSTC_C_WORD)
					) ) {
						flag=true;
						l=p;
						// avanzar hasta la linea/condicion
						II_FRONT_NC(p , p<cpm1 && ( II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) || s==wxSTC_C_WORD) );
						// si va una condicion, saltearla
						if ( flag && (s=BraceMatch(p))==wxSTC_INVALID_POSITION ) {
							SetLineIndentation(cl,LineFromPosition(p)+config_source.tabWidth);
						} else if (flag)
							p=s+1;
						II_FRONT_NC(p , p<cpm1 && (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) );
					}
				if (l!=0) {
					SetLineIndentation(cl,GetLineIndentation(LineFromPosition(l))+config_source.tabWidth);
				}
			}
			wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
			return;
		}
		
	} else if (chr=='\n') {
		int currentLine = GetCurrentLine();
		int lineInd = 0;
		if (currentLine > 0)
			lineInd = GetLineIndentation(currentLine - 1);
		if (lineInd == 0) return;
		SetLineIndentation (currentLine, lineInd);
		wxStyledTextCtrl::GotoPos(GetLineIndentPosition(currentLine));
	}
	if (calltip_mode==MXS_CALLTIP && chr==')') {
		int p=GetCurrentPos()-1;
		Colourise(p,p+1);
		if (BraceMatch(p)==calltip_brace) HideCalltip();
	} else if ( ((calltip_mode!=MXS_AUTOCOMP || chr=='.') && config_source.autoCompletion) || ((chr==',' || chr=='(') && config_source.callTips) ) {
		if (chr==':') {
			int p=GetCurrentPos();
			if (p>1 && GetCharAt(p-2)==':') {
				p=p-3;
				int s; char c;
				II_BACK(p,II_IS_NOTHING_2(p));
				if ((c=GetCharAt(p))=='>') {
					int lc=1;
					while (p-- && lc) {
						c=GetCharAt(p);
						if (c=='>' && !II_IS_NOTHING_4(p)) lc++;
						else if (c=='<' && !II_IS_NOTHING_4(p)) lc--;
					}
					II_BACK(p,II_IS_NOTHING_2(p));
					if (p<0) p=0;
				}
				wxString key=GetTextRange(WordStartPosition(p,true),WordEndPosition(p,true));
				if (key.Len()!=0)
					code_helper->AutocompleteScope(this,key,"",false,false);
			}
		} else if (chr=='>') {
			int p=GetCurrentPos()-2;
			if (GetCharAt(p)=='-') {
				int dims;
				wxString type = FindTypeOf(p-1,dims);
				if (dims==SRC_PARSING_ERROR) {
					if (type.Len()!=0)
						ShowBaloon(type);
				} else if (dims==1)
					code_helper->AutocompleteScope(this,type,"",true,false);
				else if (type.Len()!=0 && dims==0)
					ShowBaloon(LANG(SOURCE_TIP_NO_DEREFERENCE,"Tip: Probablemente no deba desreferenciar este objeto."));
			}
		} else if (chr=='.') {
			bool shouldComp=true;
			char c;
			int p=GetCurrentPos()-2;
			if ((c=GetCharAt(p))>='0' && c<='9') {
				while ( (c=GetCharAt(p))>='0' && c<='9' ) {
					p--;
				}
				shouldComp = (c=='_' || ( (c|32)>='a' && (c|32)<='z' ) );
			}
			if (shouldComp) {
				int dims;
				wxString type = FindTypeOf(p,dims);
				if (dims==SRC_PARSING_ERROR) {
					if (type.Len()!=0)
						ShowBaloon(type);
				} else	if (dims==0)
					code_helper->AutocompleteScope(this,type,"",true,false);
				else if (type.Len()!=0 && dims>0)
					ShowBaloon(LANG(SOURCE_TIP_DO_DEREFERENCE,"Tip: Probablemente deba desreferenciar este objeto."));
			}
		} else {
			int s,e=GetCurrentPos()-1,ctp;
			char c;
			if (chr=='(') {
				ctp=e;
				e--;
				II_BACK(e,II_IS_NOTHING_4(e));
			} else if (chr==',') {
				int p=e;
				c=GetCharAt(--p);
				while (true) {
					II_BACK(p, !II_IS_5(p,';','}','{','(',')') || II_SHOULD_IGNORE(p));
					if (c=='(') {
						e=p-1;
						II_BACK(e,II_IS_NOTHING_4(e));
						if (calltip_brace==p && calltip_mode==MXS_CALLTIP) {
							return;
						}
						ctp=p;
						chr='(';
						break;
					} else if (c==')') {
						if ( (p=BraceMatch(p))==wxSTC_INVALID_POSITION )
							break;
						p--;
					} else
						break;
				}
			} else if (chr=='@' || chr=='\\') {
				s=GetStyleAt(e-1);
				if (s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTLINEDOC) {
					code_helper->AutocompleteDoxygen(this);
					return;
				}
			} else if (chr=='#') {
				int s=GetCurrentLine();
				s=PositionFromLine(s)+GetLineIndentPosition(LineFromPosition(s));
				if (s==e) {
					code_helper->AutocompletePreprocesorDirective(this);
					return;
				}
			}
			int p=WordStartPosition(e,true); int pos_key=p; // pos_key guarda la posición donde comienza la palabra a autocompletar
			if (e-p+1>=config->Help.min_len_for_completion || chr==',' || chr=='(') {
				wxString key = GetTextRange(p,e+1);
				if (last_failed_autocompletion.IsSameLocation(pos_key,key,parser->data_age)) return; // no intentar autocompletar nuevamente si ya se intentó un caracter atrás
				s=GetStyleAt(p-1);
				if (p && s==wxSTC_C_PREPROCESSOR && GetCharAt(p-1)=='#') {
					code_helper->AutocompletePreprocesorDirective(this,key);
					return;
				}
				if (p && (s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR) && (GetCharAt(p-1)=='@' || GetCharAt(p-1)=='\\')) {
					code_helper->AutocompleteDoxygen(this,key);
					return;
				}
				int dims;
				p--;
				II_BACK(p,II_IS_NOTHING_4(p));
				if (c=='.') {
					wxString type = FindTypeOf(p-1,dims);
					if (dims==0) {
						if (chr=='(' && config_source.callTips)
							code_helper->ShowFunctionCalltip(ctp,this,type,key);
						else if ( II_IS_KEYWORD_CHAR(chr) )
							code_helper->AutocompleteScope(this,type,key,true,false);
					}
				} else if (c=='>' && GetCharAt(p-1)=='-') {
					p--;
					wxString type = FindTypeOf(p-1,dims);
					if (dims==1) {
						if (chr=='(' && config_source.callTips)
							code_helper->ShowFunctionCalltip(ctp,this,type,key);
						else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
							code_helper->AutocompleteScope(this,type,key,true,false);
					}
				} else if (c==':' && GetCharAt(p-1)==':') {
					p-=2;
					II_BACK(p,II_IS_NOTHING_4(p));
					wxString type = GetTextRange(WordStartPosition(p,true),p+1);
					if (chr=='(' && config_source.callTips)
						code_helper->ShowFunctionCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				} else {
					if (chr=='(' && config_source.callTips) {
						if (!code_helper->ShowFunctionCalltip(ctp,this,FindScope(GetCurrentPos()),key,false)) {
							// mostrar calltips para constructores
							p=ctp-1;
							bool f;
							while ( p>0 && ( !II_IS_3(p,'(','{',';') || (f=II_SHOULD_IGNORE(p))) ) {
								if (!f && c==')' && (p=BraceMatch(p))==wxSTC_INVALID_POSITION)
									return;
								p--;
							}
							p++;
							II_FRONT_NC(p,II_IS_NOTHING_4(p) || II_SHOULD_IGNORE(p) || (s==wxSTC_C_WORD));
							int p1=p;
							II_FRONT_NC(p,(c=GetCharAt(p))=='_' || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') );
							if (!code_helper->ShowConstructorCalltip(ctp,this,GetTextRange(p1,p))) {
								if (!code_helper->ShowConstructorCalltip(ctp,this,key)) {
									// mostrar sobrecarga del operador()
									wxString type=FindTypeOf(key,p1);
									if (type.Len()) {
										code_helper->ShowFunctionCalltip(ctp,this,type,"operator()",true);
									}
								}
							}
						}
					} else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') ) {
						wxString args; int scope_start;
						wxString scope=FindScope(GetCurrentPos(),&args,false,&scope_start);
						code_helper->AutocompleteGeneral(this,scope,key,&args,scope_start);
					}
				}
			}
		}
	}
}

void mxSource::SetModify (bool modif) {
	if (modif) {
//		SetTargetStart(0); 
//		SetTargetEnd(1);
//		ReplaceTarget(GetTextRange(0,1));
		int p=GetLength()?GetSelectionStart()-1:0;
		if (GetLength()&&p<1) p=1;
		SetTargetStart(p); 
		SetTargetEnd(p);
		ReplaceTarget(" ");
		SetTargetStart(p); 
		SetTargetEnd(p+1);
		ReplaceTarget("");
	} else 
		SetSavePoint();
}

void mxSource::MarkError (int line, bool focus) {
	int p=PositionFromLine(line);
	while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
		p++;
	MoveCursorTo(p,focus);
	EnsureVisibleEnforcePolicy(line);
	if (config->Init.autohide_panels) {
		int l0a=GetFirstVisibleLine();
		int l0b=VisibleFromDocLine(line);
		int l0=DocLineFromVisible(l0b)-(l0b-l0a);
		int nl=LinesOnScreen();
		if (line>l0+nl/2) {
			LineScroll(0,line-l0-nl/2+1);
		}
	}
//	main_window->notebook_sources->SetFocus();
//	this->SetFocus();
}

void mxSource::MoveCursorTo(long pos, bool focus) {
	SetSelection(pos,pos);
	SetCurrentPos(pos);
	if (focus) SetFocus();
}

void mxSource::OnIndentSelection(wxCommandEvent &event) {
	main_window->SetStatusText(wxString("Indentando..."));
	int min = LineFromPosition(GetSelectionStart());
	int max = LineFromPosition(GetSelectionEnd());
//  int pfl = PositionFromLine(min);
//  int dind = ss-GetLineIndentPosition(min);
	Indent(min,max);
	if (min!=max)
		SetSelection(PositionFromLine(min),GetLineEndPosition(max));
//  else {
//    int sel = GetLineIndentPosition(min)+dind;
//    if (sel<pfl) sel=pfl;
//    SetSelection(sel,sel);
//  }
	main_window->SetStatusText(wxString(LANG(GENERAL_READY,"Listo")));
}

/**
* Corrige el indentado de las lineas min a max, incluyendo ambos extremos, 
* La indentacion de cada linea se determina segun la linea anterior, por lo
* que todo el bloque dependera de la linea min-1. Para que funciones 
* correctamente debe estar actualizado el coloreado (llamar a Colourize)
**/
void mxSource::Indent(int min, int max) {
	int ps1=GetSelectionStart() ,ps2=GetSelectionStart();
	int ls1=LineFromPosition(ps1), ls2=LineFromPosition(ps2);
	int ds1=ps1-GetLineIndentPosition(ls1), ds2=ps2-GetLineIndentPosition(ls2);
	if (ds1<0) ds1=0; if (ds2<0) ds2=0;
	// para evitar que al llamar a charadd se autocomplete o cierre algo
	bool old_autocomp = config_source.autoCompletion;
	config_source.autoCompletion=0;
	bool old_autoclose=config_source.autocloseStuff;
	config_source.autocloseStuff=false; 
	bool old_autotext=config_source.autotextEnabled;
	config_source.autotextEnabled=false; 
	UndoActionGuard undo_action(this);
	bool o_bracketInsertion = config_source.bracketInsertion;
	config_source.bracketInsertion = false;
	bool o_syntaxEnable = config_source.syntaxEnable, o_smartIndent = config_source.smartIndent;
	config_source.syntaxEnable = config_source.smartIndent = true;
	if (!o_syntaxEnable) {
		SetLexer (wxSTC_LEX_CPP);
		Colourise(0,GetLength());
	}
	if (min>max) { int aux=min; min=max; max=aux; }
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	wxStyledTextEvent evt, evt_n;
	evt_n.SetKey('\n');
	if (min==0)
		SetLineIndentation(0,0);
	for (int i=(min==0?1:min);i<=max;i++) {
		int s; char c; // para los II_*
		SetLineIndentation(i,0);
		int p=PositionFromLine(i);
		SetCurrentPos(p);
		OnCharAdded(evt_n);
		p=PositionFromLine(i);
		II_FRONT_NC(p, II_IS_2(p,' ','\t'));
		if (c=='#' && !II_IS_COMMENT(p)) {
			SetLineIndentation(i,0);
		} else if (c=='}' && !II_SHOULD_IGNORE(p)) {
			evt.SetKey('}');
			SetCurrentPos(p+1);
			OnCharAdded(evt);
		} else if (c=='{' && !II_SHOULD_IGNORE(p)) {
			evt.SetKey('{');
			SetCurrentPos(p+1);
			OnCharAdded(evt);
		} else if(c=='e' && GetStyleAt(p)==wxSTC_C_WORD && GetCharAt(p+1)=='l' && GetCharAt(p+2)=='s' && GetCharAt(p+3)=='e') {
			evt.SetKey('e');
			SetCurrentPos(p+4);
			OnCharAdded(evt);
		} else if (c=='p' && GetTextRange(p,p+8)=="private:" && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+8);
			OnCharAdded(evt);
		} else if (c=='p' && GetTextRange(p,p+7)=="public:" && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+7);
			OnCharAdded(evt);
		} else if (c=='p' && GetTextRange(p,p+10)=="protected:" && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+10);
			OnCharAdded(evt);
		} else if (c=='d' && GetTextRange(p,p+8)=="default:" && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+8);
			OnCharAdded(evt);
		} else if (c=='c' && GetTextRange(p,p+5)=="case " && !II_SHOULD_IGNORE(p)) {
			int a=GetLineEndPosition(i);
			p+=5;
			II_FRONT_NC(p,p<=a && (GetCharAt(p)!=':' || II_SHOULD_IGNORE(p)));
			if (p<=a) {
				evt.SetKey(':');
				SetCurrentPos(p+1);
				OnCharAdded(evt);
			}
		}
	}
		
	if (!o_syntaxEnable)
		SetStyle(false);
	else
		Colourise(0,GetLength());
	config_source.smartIndent = o_smartIndent;
	config_source.bracketInsertion = o_bracketInsertion;
  config_source.autocloseStuff=old_autoclose;
  config_source.autoCompletion=old_autocomp;
  config_source.autotextEnabled=old_autotext;
  SetSelectionStart(GetLineIndentPosition(ls1)+ds1);
  SetSelectionEnd(GetLineIndentPosition(ls2)+ds2);
}

void mxSource::SetFolded(int level, bool folded) {
	int ss=GetSelectionStart(), se=GetSelectionEnd();
	if (ss==se) {
		ss=0; 
		se=GetLength()-1;
	}
	if (ss>se)
		Colourise(se,ss);
	else
		Colourise(ss,se);
	if (level)
		level+=1023;
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min>max) { int aux=min; min=max; max=aux; }
	if (!(max>min && PositionFromLine(max)==se)) max++;
	for (int cl, line=min;line<=max;line++) {
		cl=(GetFoldLevel(line)&wxSTC_FOLDLEVELNUMBERMASK);
		if ((!level || cl==level) && (GetFoldLevel(line+1)&wxSTC_FOLDLEVELNUMBERMASK)>cl)
			if (GetFoldExpanded(line)==folded) {
				ToggleFold(line);
			}
	}
	EnsureVisibleEnforcePolicy(GetCurrentLine());
}

int mxSource::InstructionBegin(int p) {
	return GetLineIndentPosition(LineFromPosition(p));
}

void mxSource::SetStyle(int idx, const wxChar *fontName, int fontSize, const wxColour &foreground, const wxColour &background, int fontStyle){
	StyleSetFontAttr(idx,fontSize,fontName,fontStyle&mxSOURCE_BOLD,fontStyle&mxSOURCE_ITALIC,fontStyle&mxSOURCE_UNDERL);
	StyleSetForeground (idx, foreground);
	StyleSetBackground (idx, background);
	StyleSetVisible (idx,!(fontStyle&mxSOURCE_HIDDEN));
}

#define AUXSetStyle(who,name) SetStyle(wxSTC_##who##_##name,config->Styles.font_name,config->Styles.font_size,ctheme->name##_FORE,ctheme->name##_BACK,(ctheme->name##_BOLD?mxSOURCE_BOLD:0)|(ctheme->name##_ITALIC?mxSOURCE_ITALIC:0)); // default
#define AUXSetStyle3(who,name,real) SetStyle(wxSTC_##who##_##name,config->Styles.font_name,config->Styles.font_size,ctheme->real##_FORE,ctheme->real##_BACK,(ctheme->real##_BOLD?mxSOURCE_BOLD:0)|(ctheme->real##_ITALIC?mxSOURCE_ITALIC:0)); // default
void mxSource::SetStyle(bool color) {
	if ((config_source.syntaxEnable=color)) {
		SetLexer(lexer);
		AUXSetStyle(STYLE,BRACELIGHT); 
		AUXSetStyle(STYLE,BRACEBAD); 
		switch (lexer) {
		case wxSTC_LEX_CPP:
			config_source.callTips = config->Source.callTips;
			config_source.autoCompletion = config->Source.autoCompletion;
//			config_source.stdCalltips = config->Source.stdCalltips;
//			config_source.stdCompletion = config->Source.stdCompletion;
//			config_source.parserCalltips = config->Source.parserCalltips;
//			config_source.parserCompletion = config->Source.parserCompletion;
			config_source.smartIndent = config->Source.smartIndent;
			config_source.indentPaste = config->Source.indentPaste;
	
			AUXSetStyle(C,DEFAULT); // default
			AUXSetStyle(C,COMMENT); // comment
			AUXSetStyle(C,COMMENTLINE); // comment line
			AUXSetStyle(C,COMMENTDOC); // comment doc
			AUXSetStyle(C,NUMBER); // number
			AUXSetStyle(C,WORD); // keywords
			AUXSetStyle(C,STRING); // string
			AUXSetStyle(C,CHARACTER); // character
			AUXSetStyle3(C,UUID,DEFAULT); // uuid 
			AUXSetStyle(C,PREPROCESSOR); // preprocessor
			AUXSetStyle(C,OPERATOR); // operator 
			AUXSetStyle(C,IDENTIFIER); // identifier 
			AUXSetStyle(C,STRINGEOL); // string eol
			AUXSetStyle3(C,VERBATIM,STRINGEOL); // default verbatim
			AUXSetStyle3(C,REGEX,STRINGEOL); // regexp  
			AUXSetStyle(C,COMMENTLINEDOC); // special comment 
			AUXSetStyle(C,WORD2); // extra words
			AUXSetStyle(C,COMMENTDOCKEYWORD); // doxy keywords
			AUXSetStyle(C,COMMENTDOCKEYWORDERROR); // keywords errors
			AUXSetStyle(C,GLOBALCLASS); // keywords errors
			break;
		case wxSTC_LEX_HTML: case wxSTC_LEX_XML:
//			config_source.stdCalltips=config_source.stdCompletion=config_source.parserCalltips=config_source.parserCompletion=config_source.smartIndent=config_source.indentPaste=false;
			config_source.callTips = config_source.smartIndent = config_source.indentPaste = false;
			config_source.autoCompletion = 0;
			SetProperty ("fold.html","0");
			SetProperty ("fold.html.preprocessor", "0");
			AUXSetStyle(H,DEFAULT); // keywords
			AUXSetStyle3(H,TAG,WORD); // keywords
			AUXSetStyle3(H,TAGUNKNOWN,WORD); // keywords
			AUXSetStyle3(H,ATTRIBUTE,COMMENTDOC); // comment doc
			AUXSetStyle3(H,ATTRIBUTEUNKNOWN,COMMENTDOC); // comment doc
			AUXSetStyle(H,COMMENT); // comment doc
			AUXSetStyle3(H,SINGLESTRING,STRING); // string
			AUXSetStyle3(H,DOUBLESTRING,STRING); // string
			AUXSetStyle(H,NUMBER); // number
			AUXSetStyle3(H,VALUE,PREPROCESSOR); // preprocessor
			AUXSetStyle3(H,SCRIPT,PREPROCESSOR); // preprocessor
			AUXSetStyle3(H,ENTITY,WORD2); // &nbsp;
			break;
		case wxSTC_LEX_MAKEFILE:
//			config_source.stdCalltips=config_source.stdCompletion=config_source.parserCalltips=config_source.parserCompletion=config_source.smartIndent=config_source.indentPaste=false;
			config_source.callTips = config_source.smartIndent = config_source.indentPaste = false;
			config_source.autoCompletion = 0;
			AUXSetStyle(MAKE,DEFAULT); // default
			AUXSetStyle3(MAKE,IDENTIFIER,WORD); // keywords
			AUXSetStyle3(MAKE,COMMENT,COMMENT); // comment doc
			AUXSetStyle(MAKE,PREPROCESSOR); // preprocessor
			AUXSetStyle3(MAKE,OPERATOR,OPERATOR); // operator 
			AUXSetStyle3(MAKE,TARGET,OPERATOR); // string
			AUXSetStyle3(MAKE,IDEOL,NUMBER); // string
			break;
		case wxSTC_LEX_BASH:
//			config_source.stdCalltips=config_source.stdCompletion=config_source.parserCalltips=config_source.parserCompletion=config_source.smartIndent=config_source.indentPaste=false;
			config_source.callTips=config_source.smartIndent=config_source.indentPaste=false;
			config_source.autoCompletion=0;
			AUXSetStyle(SH,DEFAULT);
			AUXSetStyle3(SH,ERROR,COMMENTDOCKEYWORD);
			AUXSetStyle(SH,COMMENTLINE);
			AUXSetStyle(SH,NUMBER);
			AUXSetStyle(SH,WORD);
			AUXSetStyle(SH,STRING);
			AUXSetStyle3(SH,CHARACTER,STRING);
			AUXSetStyle(SH,OPERATOR);
			AUXSetStyle(SH,IDENTIFIER);
			AUXSetStyle3(SH,SCALAR,NUMBER);
			AUXSetStyle3(SH,PARAM,WORD2);
			AUXSetStyle3(SH,BACKTICKS,PREPROCESSOR);
			AUXSetStyle3(SH,HERE_DELIM,OPERATOR);
			AUXSetStyle3(SH,HERE_Q,OPERATOR);
			break;
		}
		Colourise(0,GetLength());
	} else {
		StyleClearAll();
		SetLexer (wxSTC_LEX_NULL);
		config_source.syntaxEnable=false;
		StyleSetForeground (wxSTC_STYLE_DEFAULT, ctheme->DEFAULT_FORE);
		StyleSetBackground (wxSTC_STYLE_DEFAULT, ctheme->DEFAULT_BACK);
	}
}


void mxSource::SetStyle(int a_lexer) {
	if (a_lexer!=lexer) {
		if (a_lexer==wxSTC_LEX_NULL) {
			config_source.syntaxEnable=false;
			SetStyle(false);
		} else {
			config_source.syntaxEnable=true;
			lexer=a_lexer;
			SetStyle(true);
		}
	}
}

void mxSource::SelectError(int indic, int p1, int p2) {
	if (p1>=GetLength()||p2>GetLength()) return;
	if (p1==p2) {
		p2=WordEndPosition(p1,true);
		while (p2>p1 && (GetCharAt(p2-1)==' ' || GetCharAt(p2-1)=='\t')) p2--;
		if (p2==p1) { GotoPos(p1); return; }
	}
	int lse = GetEndStyled();
//	StartStyling(0,wxSTC_INDICS_MASK);
//	SetStyling(GetLength(),0);
	StartStyling(p1,wxSTC_INDICS_MASK);
	if (indic==1)
		SetStyling(p2-p1,wxSTC_INDIC1_MASK);
	else
		SetStyling(p2-p1,wxSTC_INDIC0_MASK);
	GotoPos(p1);
	SetSelection(p1,p2);
	StartStyling(lse,0x1F);
}

bool mxSource::AddInclude(wxString header) {
	
	UndoActionGuard undo_action(this);

//	bool using_namespace_std_present=false;
	bool header_present=false;
	
	int lp = GetCurrentPos() , cl = GetCurrentLine();
	wxString oHeader=header;
#ifdef __WIN32__
	header=header.MakeLower();
#endif
	wxString str;
	int s,p,p1,p2,lta=0, flag=true, comment;
	int uncomment_line=0;
	for (int i=0;i<cl;i++) {
		p=GetLineIndentPosition(i);
		int le=GetLineEndPosition(i);
		if (GetTextRange(p,p+8)=="#include" || GetTextRange(p,p+10)=="//#include" || GetTextRange(p,p+11)=="// #include") {
			if ( ( comment = (GetTextRange(p,p+2)=="//") ) ) {
				p+=2;
				if (GetCharAt(p)==' ')
					p++;
			}
			if (flag) 
				lta=i+1;
			p+=8;
			while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
				p++;
			p1=p++;
			while ((GetCharAt(p)!='"' || GetCharAt(p)!='>') && p<le)
				p++;
			p2=p;
			str=GetTextRange(p1,p2);
#ifdef __WIN32__
			str=str.MakeLower();
#endif
			if (str==header) {
				if (comment) 
					uncomment_line=i+1;
				else
					header_present=true;
			}
		} else if (le==p || II_SHOULD_IGNORE(p)) {
			if (lta==0 && flag)
				lta=1;
			else if (flag && GetCharAt(p)=='#') // para que saltee defines e ifdefs
				lta=i+1;
//		} else if (GetTextRange(p,p+5)=="using") {
//			if (flag) {
//				lta=i;
//				flag=false;
//			}
//			p+=5;
//			while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
//				p++;
//			if (GetTextRange(p,p+9)=="namespace") {
//				p+=9;
//				while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
//					p++;
//				if (GetTextRange(p,p+3)=="std" && (GetCharAt(p+3)==';' || GetCharAt(p+3)==' ' || GetCharAt(p+3)=='\t' || GetCharAt(p+3)=='/'))
//					using_namespace_std_present=true;
//			}
		} else if (!II_SHOULD_IGNORE(p)/* && GetTextRange(p,p+7)!="#define"*/)
			flag=false;
	}
	if (!header_present) {
		if (uncomment_line) {
			p = PositionFromLine(uncomment_line-1);
			SetTargetStart(p);
			while (GetCharAt(p)!='#')
				p++;
			SetTargetEnd(p);
			ReplaceTarget("");
		} else {
			p = PositionFromLine(lta);
			wxString line = wxString("#include ")+oHeader+"\n";
			if (p<=lp)
				lp+=line.Len();
			InsertText(p,line);
			if (config_source.syntaxEnable)
				Colourise(p,p+line.Len());
			GotoPos(lp);
			lta++;
		}
	}
//	if (!using_namespace_std_present && header.Find(".")==wxNOT_FOUND && header.Last()!='\"') {
//		p = PositionFromLine(lta);
//		wxString line = wxString("using namespace std;\n");
//		if (p<=lp)
//			lp+=line.Len();
//		InsertText(p,line);
//		if (config_source.syntaxEnable)
//			Colourise(p,p+line.Len());
//		GotoPos(lp);
//	}
	
//	wxYield(); // sin esto no se ve el calltip (posiblemente un problema con el evento OnUpdateUI
	if (!header_present /*|| (!using_namespace_std_present && header.Last()!='\"' && header.Right(3)!=_(".h>"))*/) {
		int lse = GetEndStyled();
		StartStyling(0,wxSTC_INDICS_MASK);
		SetStyling(GetLength(),0);
		StartStyling(lse,0x1F);
		if (!header_present) {
			if (uncomment_line)
				ShowBaloon(LANG2(SOURCE_UNCOMMENTED_FOR_HEADER,"Descomentada linea <{1}>: \"#include <{2}>\".",wxString()<<uncomment_line,oHeader));
			else
				ShowBaloon(LANG1(SOURCE_ADDED_HEADER,"Cabecera agregada: <{1}>.",oHeader));
//		} else {
//			ShowBaloon(LANG(SOURCE_ADDED_USING_NAMESPACE_STD,"Agregado \"using namespace std;\""));
		}
		return true;
	} else {
		ShowBaloon(wxString(LANG(SOURCE_HEADER_WAS_ALREADY,"Sin cambios, ya estaba la cabecera "))<<oHeader);
		return false;
	}
	
}

void mxSource::OnPopupMenu(wxMouseEvent &evt) {
	if (GetMarginForThisX(evt.GetX())==MARGIN_NULL) OnPopupMenuInside(evt); 
	else OnPopupMenuMargin(evt);
}
	
void mxSource::OnPopupMenuMargin(wxMouseEvent &evt) {
	
	int x=10,p; for(int i=0;i<MARGIN_NULL;i++) { x+=GetMarginWidth(i); }
	int l=LineFromPosition( p=PositionFromPointClose(x,evt.GetY()) );
	if (GetCurrentLine()!=l) GotoPos(PositionFromLine(l));
	wxMenu menu("");
	
	BreakPointInfo *bpi=m_extras->FindBreakpointFromLine(this,l);
	if (bpi) {
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnDEBUG,mxID_DEBUG_TOGGLE_BREAKPOINT), LANG(SOURCE_POPUP_REMOVE_BREAKPOINT,"Quitar breakpoint"));
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_DEBUG_ENABLE_DISABLE_BREAKPOINT),LANG(SOURCE_POPUP_ENABLE_BREAKPOINT,"Habilitar breakpoint"))->Check(bpi->enabled);
	} else if (!IsEmptyLine(l))
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnDEBUG,mxID_DEBUG_TOGGLE_BREAKPOINT), LANG(SOURCE_POPUP_INSERT_BREAKPOINT,"Insertar breakpoint"));
	if (!debug->IsDebugging()||debug->CanTalkToGDB())
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnDEBUG,mxID_DEBUG_BREAKPOINT_OPTIONS));
	menu.AppendSeparator();
	int s=GetStyleAt(p);
	if (MarkerGet(l)&(1<<mxSTC_MARK_USER))
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_MARK_LINES), LANG(SOURCE_POPUP_REMOVE_HIGHLIGHT,"Quitar resaltado"));
	else 
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_MARK_LINES), LANG(SOURCE_POPUP_HIGHLIGHT_LINES,"Resaltar linea"));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_INDENT));
	if (STYLE_IS_COMMENT(s)) mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_UNCOMMENT));
	else mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_COMMENT));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_DUPLICATE_LINES));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_DELETE_LINES));
	if (l>1) mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_TOGGLE_LINES_UP));
	if (l+1<GetLineCount()) mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_TOGGLE_LINES_DOWN));
	
	main_window->PopupMenu(&menu);
	
}

void mxSource::OnPopupMenuInside(wxMouseEvent &evt) {
	
	// mover el cursor a la posición del click (a menos que haya una selección y se clickeó dentro)
	int p1=GetSelectionStart();
	int p2=GetSelectionEnd();
	if (p1==p2) {
		int p = PositionFromPointClose(evt.GetX(),evt.GetY());
		if (p!=wxSTC_INVALID_POSITION)
			GotoPos(p);
	}

	wxMenu menu("");
	
	int p=GetCurrentPos(); int s=GetStyleAt(p);
	wxString key=GetCurrentKeyword(p);
	if (key.Len()!=0) {
		if (!key[0]!='#') mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_SOURCE_GOTO_DEFINITION));
		if (!STYLE_IS_COMMENT(s) && !STYLE_IS_CONSTANT(s)) mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_HELP_CODE),LANG1(SOURCE_POPUP_HELP_ON,"Ayuda sobre \"<{1}>\"...",key));
		if (s==wxSTC_C_IDENTIFIER) mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_INSERT_HEADER),LANG1(SOURCE_POPUP_INSERT_INCLUDE,"Insertar #incl&ude correspondiente a \"<{1}>\"",key));
		if (s==wxSTC_C_IDENTIFIER) menu.Append(mxID_EDIT_HIGHLIGHT_WORD, LANG1(SOURCE_POPUP_HIGHLIGHT_WORD,"Resaltar identificador \"<{1}>\"",key));
	}
	
	if (s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_STRING) {
		if (p1==p2) {
			int pos=GetCurrentPos();
			p1=WordStartPosition(pos,true);
			p2=WordEndPosition(pos,true);
			while (GetCharAt(p1-1)=='.' || GetCharAt(p1-1)=='/' || GetCharAt(p1-1)=='\\' || GetCharAt(p1-1)==':')
				p1=WordStartPosition(p1-1,true);
			while (GetCharAt(p2)=='.' || GetCharAt(p2)=='/' || GetCharAt(p2)=='\\' || GetCharAt(p2)==':')
				p2=WordEndPosition(p2+1,true);
		}
		wxFileName the_one (sin_titulo?GetTextRange(p1,p2):DIR_PLUS_FILE(source_filename.GetPath(),GetTextRange(p1,p2)));
		if (wxFileName::FileExists(the_one.GetFullPath()))
			mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_FILE_OPEN_SELECTED),LANG1(SOURCE_POPUP_OPEN_SELECTED,"&Abrir \"<{1}>\"",GetTextRange(p1,p2)));
	}
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_WHERE_AM_I));
	menu.AppendSeparator();
	
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_UNDO));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_REDO));
	menu.AppendSeparator();
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_CUT));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_COPY));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_PASTE));
	menu.AppendSeparator();
	if (STYLE_IS_COMMENT(s)) mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_UNCOMMENT));
	else mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_COMMENT));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_INDENT));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_BRACEMATCH));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnEDIT,mxID_EDIT_SELECT_ALL));
	
	main_window->PopupMenu(&menu);
	
}

/**
* Averigua el tipo de una variable dentro de un contexto. 
* Primero busca en el código del archivo hacia atras y mira lo que parezca
* una declaración de variable. Si piensa que está en alguna función tambien
* mira los argumentos y anota el scope si la función parece el método de una
* clase, para que al final si no encontró nada le pueda preguntar al parser
* por esa clase
**/
wxString mxSource::FindTypeOf(wxString &key, int &pos) {
	
	int dims=0;
	bool notSpace=true;
	
	wxString ret="",space="";
	
	int p_to,p_from;
	p_from = pos;
	int p,s;
	int p1,p2;
	char c;
	bool aux_bool;
	
	int p_ocur=wxSTC_INVALID_POSITION;
	int p_type=wxSTC_INVALID_POSITION;
	
	int e=pos-1;
	II_BACK(e,II_IS_NOTHING_4(e));
	if (c=='.') {
		e--;
		II_BACK(e,II_IS_NOTHING_4(e));
		int ddims=0;
		while (c==']') {
			ddims++;
			e=BraceMatch(e);
			if (e==wxSTC_INVALID_POSITION) {
				pos=SRC_PARSING_ERROR;
				return "";
			}
			e--; II_BACK(e,II_IS_NOTHING_4(e));
		}
		int dims=WordStartPosition(e,true);
		wxString space = GetTextRange(dims,e+1);
		wxString type = FindTypeOf(space,dims);
		dims-=ddims;
		if (dims==0) {
			pos=-1;
			type = code_helper->GetAttribType(type,key,pos);
			key = space;
			return type;
		}
	} else if (c=='>' && e!=0 && GetCharAt(e-1)=='-') {
		e-=2;
		II_BACK(e,II_IS_NOTHING_4(e));
		wxString space=FindTypeOf(e,dims);
		if (space.Len()) { // codigo nuevo, usar el otro FindTypeOf
			pos=dims-1; // pos es argumento de entrada(posicion) y salida(dimension)
			wxString type=code_helper->GetAttribType(space,key,pos);
			key=type; // key es argumento de entrada (nombre de var a buscar) y salida (scope)
			return type;
		} else {// fin codigo nuevo, empieza codigo viejo, si no funca que siga como antes (todo: analizar si vale la pena o si con el nuevo ya reemplaza todo)
			int dims=WordStartPosition(e,true);
			wxString space = GetTextRange(dims,e+1);
			wxString type = FindTypeOf(space,dims);
			if (dims==1) {
				pos=-1;
				type = code_helper->GetAttribType(type,key,pos);
				key = space;
				return type;
			}
		}
	} else if (c==':' && e!=0 && GetCharAt(e-1)==':') {
		e-=2;
		II_BACK(e,II_IS_NOTHING_4(e));
		int dims=WordStartPosition(e,true);
		wxString type = GetTextRange(dims,e+1);
		if (dims==0) {
			dims=-1;
			type = code_helper->GetAttribType(type,key,dims);
			key = "";
			return type;
		}
	}
	
	while (true) {
		
		c=GetCharAt(p_from);
		
		if (c==')') { // ver si era una función/método
			// avanzar despues del paréntesis y ver si abre una llave
			p=p_from+1; int l=GetLength();
			II_FRONT(p,II_IS_NOTHING_4(p))
			// saltear el const
			if (p+5<l&&GetCharAt(p)=='c'&&GetCharAt(p+1)=='o'&&GetCharAt(p+2)=='n'&&GetCharAt(p+3)=='s'&&GetCharAt(p+4)=='t'&&(GetCharAt(p+5)=='{'||II_IS_NOTHING_4(p+5))) {
				p+=5; II_FRONT(p,II_IS_NOTHING_4(p))
			}
			
			if (c!='{') {
				p_from = BraceMatch(p_from);
				if (p_from==wxSTC_INVALID_POSITION)
					break;
			} else {
				do {
					p_to = BraceMatch(p_from);
					if (p_to==wxSTC_INVALID_POSITION)
						break;
					p=p_to-1;
					while (p>0 && II_IS_4(p,' ','\t','\n','\r'))
						p--;
					// ver que no sea un costructor de atributo o padre (fix 29/09)
					int p2=p;
					while  ( p>0 &&  ( (c>='a' && c<='z') ||  (c>='0' && c<='9') ||  (c>='A' && c<='Z') || c=='_' || II_SHOULD_IGNORE(p) ) )
						c=GetCharAt(--p);
					while (p>0 && II_IS_4(p,' ','\t','\n','\r'))
						p--;
					if (p==0 || ( c!=',' && (c!=':' || GetCharAt(p-1)==':') ) ) { p=p2; break; }
					p--;
					while (p>0 && II_IS_4(p,' ','\t','\n','\r'))
						p--;
					p_from=p;
				} while (true);
				if (p_to==wxSTC_INVALID_POSITION)
					break;
				
				if (GetStyleAt(p)!=wxSTC_C_WORD) { // si estamos en el prototipo de la funcion
					p=FindText(p_from,p_to,key,wxSTC_FIND_WHOLEWORD|wxSTC_FIND_MATCHCASE);
					if (p!=wxSTC_INVALID_POSITION) { // si es un parametro
//						p=p_to;
						p_to=p;
						while (p_to>0 && !II_IS_2(p_to,',','(')) {
							p_to--;
						}
						p_type=p_to+1;
						p_ocur=p;
						// contar las dimensiones (asteriscos)
						p--;
						while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
							p--;
						}
						if (c=='&') c=GetCharAt(--p); // alias de tipo puntero (foo *&var)
						while (c=='*') {
							p--;
							dims++;
							while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
								p--;
							}
						}
						break;
					}
					// buscar el scope de la funcion actual
					p=p_to-1;
					while (p>0 && ( II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) ) ) {
						p--;
					}
					c=GetCharAt(p);
					while  ( p>0 && ( (c>='a' && c<='z') || (c>='0' && c<='9') || (c>='A' && c<='Z') || c=='_' || II_SHOULD_IGNORE(p) ) ) {
						c=GetCharAt(--p);
					}
					if (p>0&&c=='~') c=GetCharAt(--p); // por si era un destructor
					while (p>0 && ( II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) ) ) {
						p--;
					}
					if (c==':' && GetCharAt(p-1)==':') {
						p-=2;
						while (p>0 && ( II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) ) ) {
							p--;
						}
						p2=p+1;
						while  ( p>0 &&  ( (c>='a' && c<='z') || (c>='0' && c<='9') || (c>='A' && c<='Z') || c=='_' /*|| II_SHOULD_IGNORE(p)*/ ) ) {
							c=GetCharAt(--p);
						}
						p1=p+1;
						if (notSpace) {
							space=GetTextRange(p1,p2);
							notSpace=false;
						}
					}
				} else {
					p_from=p;
				}
			}
			
		} else	if (c=='}') { // saltear todo el contenido de ese bloque
			p_from = BraceMatch(p_from);
			if (p_from==wxSTC_INVALID_POSITION)
				break;
			p_from--;
			// ver si era una función, y en ese caso saltear tambien el prototipo
			II_BACK(p_from,II_IS_NOTHING_4(p_from));
			// saltear el const
			if (p_from>4&&GetCharAt(p_from-4)=='c'&&GetCharAt(p_from-3)=='o'&&GetCharAt(p_from-2)=='n'&&GetCharAt(p_from-1)=='s'&&GetCharAt(p_from)=='t'&&(GetCharAt(p_from-5)==')'||II_IS_NOTHING_4(p_from-5))) {
				p_from-=5; II_BACK(p_from,II_IS_NOTHING_4(p_from));
			}			
			if (c==')') {
				p_from = BraceMatch(p_from);
				if (p_from==wxSTC_INVALID_POSITION)
					break;
			}
		}
		
//		p_llave_a = FindText(p_from-1,0,"{");
//		while (p_llave_a!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_a))
//			p_llave_a = FindText(p_llave_c-1,0,"}");
		int p_llave_c = FindText(p_from-1,0,"}");
		while (p_llave_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_c))
			p_llave_c = FindText(p_llave_c-1,0,"}");
		int p_par_c = FindText(p_from-1,0,")");
		while (p_par_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_par_c))
			p_par_c = FindText(p_par_c-1,0,")");

		
//		if (notSpace && p_llave_a!=wxSTC_INVALID_POSITION && (p_llave_c==wxSTC_INVALID_POSITION || p_llave_a>p_llave_c) ) { // ver si es una declaracion de clase para encontrar el scope
//			p = p_llave_a-1;
//			II_BACK(p,II_IS_NOTHING_4(p));
//			p_llave_a=p;
//			if ( !II_SHOULD_IGNORE(p) && II_IS_KEYWORD_CHAR(c) ) {
//				p--;
//				II_BACK (p, !II_SHOULD_IGNORE(p) && II_IS_KEYWORD_CHAR(c));
//				bool cont=true;
////				while (cont) {
//					if (c=='s' && p>=5 && GetTextRange(p-5,p)=="clas" ) {
//						notSpace=false;
//						space=GetTextRange(WordStartPosition(p_llave_a,true),p_llave_a+1);
//					}
//	//			}
//			}
//		}
		
		if ( p_llave_c!=wxSTC_INVALID_POSITION && p_par_c!=wxSTC_INVALID_POSITION ) {
			if (p_par_c>p_llave_c)
				p_to = p_par_c;
			else
				p_to = p_llave_c;
		} else if (p_par_c!=wxSTC_INVALID_POSITION) {
			p_to = p_par_c;
		} else if (p_llave_c!=wxSTC_INVALID_POSITION) {
			p_to = p_llave_c;
		} else
			p_to = 0;
		
		p_ocur = p = FindText(p_from, p_to, key, wxSTC_FIND_WHOLEWORD|wxSTC_FIND_MATCHCASE);
		if (p!=wxSTC_INVALID_POSITION) { // si se encuentra la palabra
			// retroceder al comienzo de la instruccion
			p_to=p_ocur; aux_bool=false;
			while (p>0 && ( !II_IS_5(p,'}','{','(',';',':') || (aux_bool=II_SHOULD_IGNORE(p)) ) ) {
				if (!aux_bool && c==')') {
					p = BraceMatch(p);
					if (p==wxSTC_INVALID_POSITION)
						break;
				}
				p--;
			}
			// saltar el indentado
			p++;
			while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p) )
				p++;
			// avanzar el hipotetico nombre de la clase (va a quedar entre p1 y p2)
			int p3;
			do {
				p1=p;
				while ( (c>='a' && c<='z') ||  (c>='A' && c<='Z') ||  (c>='0' && c<='9') || c=='_' || II_SHOULD_IGNORE(p) ) {
					p++;
					c=GetCharAt(p);
				}
				p3=p2=p;
				// avanzar espacios en blanco
				while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
					p++;
				}
			} while (GetTextRange(p1,p2)=="const" || GetTextRange(p1,p2)=="volatile" || GetTextRange(p1,p2)=="static" || GetTextRange(p1,p2)=="extern");
			// saltar los parametros del template si es template (mejorar: aqui no se tienen en cuenta comentarios y literales)
			int tplt_deep = 0;
			if (c=='<' && GetCharAt(p+1)!='<') {
				tplt_deep = 1; p++;
				while (p<p_ocur && tplt_deep!=0) {
					c = GetCharAt(p);
					if (c=='>')
						tplt_deep--;
					else if (c=='<')
						tplt_deep++;
					p++;
				}
				p3=p;
			}
			if (tplt_deep==0) {
				// ver si lo que sigue tiene cara de nombres de variable para la declaracion ¿?
				while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
					p++;
				}
				if ( (c>='a' && c<='z') || (c>='0' && c<='9') || (c>='A' && c<='Z') || c=='_' || c=='&' || c=='*' || II_SHOULD_IGNORE(p) ) {
					dims=0;
					if (p1!=p2 
						&& !(p2-p1==4 && GetCharAt(p1)=='e' && GetCharAt(p1+1)=='l' && GetCharAt(p1+2)=='s' && GetCharAt(p1+3)=='e')
						&& !(p2-p1==6 && GetCharAt(p1)=='d' && GetCharAt(p1+1)=='e' && GetCharAt(p1+2)=='l' && GetCharAt(p1+3)=='e' && GetCharAt(p1+4)=='t' && GetCharAt(p1+5)=='e')
						) {
						p=p_ocur-1;
						while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
							p--;
						}
						if (c=='&') c=GetCharAt(--p); // por si es algo como "string *&p=..."
						while (c=='*') {
							p--;
							dims++;
							while (II_IS_4(p,' ','\n','\t','\r') || II_SHOULD_IGNORE(p)) {
								p--;
							}
						}
						if (c==',' || p+1==p3 || c=='&') {
							p_type=p1;
							break;
						}
					}
				}
			}
		}
		
		if (p_to==0)
			break;
		else
			p_from=p_to;
		
	}
	
	if (p_type!=wxSTC_INVALID_POSITION) {
		
		while (II_IS_4(p_type,' ','\t','\r','\n') || II_SHOULD_IGNORE(p_type)) {
			p_type++;
		}
		p2=WordEndPosition(p_type,true);
		ret=GetTextRange(p_type,p2);
		while ( !(ret!="unsigned" && ret!="const" && ret!="signed" && ret!="extern" && ret!="volatile" && ret!="long" && ret!="static" && (ret!="public" || GetCharAt(p2)!=':') && (ret!="protected" || GetCharAt(p2)!=':') && (ret!="private" || GetCharAt(p2)!=':') ) ) {
			p_type=p2;
			while (II_IS_4(p_type,' ','\t','\r','\n') || II_SHOULD_IGNORE(p_type)) {
				p_type++;
			}
			p2=WordEndPosition(p_type,true);
			ret=GetTextRange(p_type,p2);
		}
//		while (II_IS_4(p2,' ','\t','\r','\n') || II_SHOULD_IGNORE(p2)) {
//			p2++;
//		}
//		while(GetCharAt(p2)=='*') {
//			dims++;
//			p2++;
//			while (II_IS_4(p2,' ','\t','\r','\n') || II_SHOULD_IGNORE(p2)) {
//				p2++;
//			}
//		}

		p_ocur+=key.Len();
		while (II_IS_4(p_ocur,' ','\t','\r','\n') || II_SHOULD_IGNORE(p_ocur)) {
			p_ocur++;
		}
		while (GetCharAt(p_ocur)=='[') {
			dims++;
			p_ocur = BraceMatch(p_ocur);
			if (p_ocur==wxSTC_INVALID_POSITION)
				return ret;
			p_ocur++;
			while (II_IS_4(p_ocur,' ','\t','\r','\n') || II_SHOULD_IGNORE(p_ocur)) {
				p_ocur++;
			}
		}
		pos=dims;
	} else {
		if (key=="this") {
			if ( (ret=space).Len()!=0) 
				dims=1;
		} else {
			wxString ans;
			if ( (space!="" && (ans=code_helper->GetAttribType(space,key,pos))!="") || (ans=code_helper->GetGlobalType(key,pos))!="" )
				ret=ans;
			else
				pos=SRC_PARSING_ERROR;
		}
	}
	key=space;
	return code_helper->UnMacro(ret,pos);
}

void mxSource::ShowBaloon(wxString str, int p) {
	if (p==-1) p = GetCurrentPos();
//	int cp = GetCurrentPos(); 
//	if (p==-1) p = cp;
//	// evitar que tape el cursor (reveer, parece solo tener sentido si se muestra en una posicion que no es la actual, cuando pasa esto?)
//	int cl = LineFromPosition(cp);
//	int l = LineFromPosition(p);
//	if (l!=cl) {
//		int dp = p-PositionFromLine(l);
//		p = PositionFromLine(cl)+dp;
//		if (LineFromPosition(p)!=cl) 
//			p = GetLineEndPosition(cl);
//	}
	
	int x1 = PointFromPosition(p).x;
	int x2 = PointFromPosition(p+1).x;
	char c = GetCharAt(p);
	if (x1!=x2 && c!='\n' && c!='\t' && c!='\r') {
		int l=str.Len(), ll = (GetSize().GetWidth()-x1)/(x2-x1)-1;
		if (ll>5) {
			int ac=0;
			for (int i = 0;i<l; i++) {
				if (str[i]=='\n') ac=0;
				else {
					ac++;
					if (ac>ll) {
						int j=i;
						while (j && ( str[j]!=' ' && str[j]!=',' && str[j]!='(' && str[j]!=')' ) ) j--;
						if (j && !(j>2 && str[j]==' ' && str[j-1]==' ' && str[j-2]==' ' && str[j-3]=='\n') ) {
							i=j;
							str = str.SubString(0,j)+"\n   "+str.Mid(j+1);
							l+=4;
						}
					}
				}
			}
		}
	}
	
	// mostrar utilizando el mecanismo de calltip de scintilla
	SetCalltipMode(MXS_BALOON);
	wxStyledTextCtrl::CallTipShow(p,str);
	// para que queríamos un yield aca???
	DEBUG_INFO("wxYield:in  mxSource::ShowBaloon");
	wxYield();
	DEBUG_INFO("wxYield:out mxSource::ShowBaloon");
}
	
	

wxString mxSource::FindTypeOf(int p,int &dims, bool first_call) {
	if (first_call)
		dims=0;
	int s; char c;
	// buscar donde comienza la expresion
	II_BACK(p,II_IS_NOTHING_4(p));
	
	while ( p>=0 && ( II_IS_NOTHING_2(p) || c==']' ) )  {
		if (c==']') {
			dims--;
			p = BraceMatch(p);
			if (p==wxSTC_INVALID_POSITION) {
				dims=SRC_PARSING_ERROR;
				return "La cantidad de corchetes no concuerda.";
			}
		}
		p--;
	}
	if (GetCharAt(p-3)=='t' && GetCharAt(p-2)=='h' && GetCharAt(p-1)=='i' && GetCharAt(p)=='s' && WordStartPosition(p-3,true)==p-3) {
		wxString scope = FindScope(p-3);
		if (scope.Len()) {
			dims++;
			return scope;
		}
	}
	if (c==')') { // si hay un parentesis puede que ya esté todo resuelto (cast)
		p = BraceMatch(p);
		if (p==wxSTC_INVALID_POSITION) {
			dims=SRC_PARSING_ERROR;
			return "Error: Se cerraron parentesis demas.";
		}
		int p2 = p-1;
		II_BACK(p2,II_IS_NOTHING_4(p2));
		if (c=='>') { // si habia un cast estilo c++
			// buscar hasta donde llegan los signos mayor y menor
			int cont=1; p=--p2;
			while (p2>0 && (c!='<' || cont!=0) ) {
				c=GetCharAt(p2);
				if (c=='>')
					cont++;
				else if (c=='<') 
					cont--;
				p2--;
			}
			if (c=='<' && cont==0) { // si se delimito correctamente
				// contar los asteriscos para informar las dimensiones
				p2++;
				while (p>p2 && ( II_IS_NOTHING_2(p) || c=='*') ) {
					if (c=='*')
						dims++;
					p--;
				}
				return code_helper->UnMacro(GetTextRange(p2+1,p+1),dims);
			}
		} else { 
			// puede ser un cast al estilo c: ejemplo ((lala)booga) y estabamos parados en el primer (, el tipo seria lala
			// o una llamada a metodo/funcion/constructor: ejemplo foo(bar) y estamos en el (
			// ver primero si es la segunda opcion, mirando que hay justo antes del (
			int p_par=p;
			p--; II_BACK(p, II_IS_NOTHING_4(p)); // ir a donde termina el nombre de la funcion
			int p_fin=p+1;
			// ir a donde empieza el nombre de la funcion
			p=WordStartPosition(p,true); c=GetCharAt(p);
			if (p<p_fin && (c<'0'||c>'9')) { // si puede ser un nombre de funcion o metodo
				int p0_name=p; wxString func_name=GetTextRange(p,p_fin), scope;
				--p; II_BACK(p,II_IS_NOTHING_4(p));
				if (GetCharAt(p)=='.') {
					--p; II_BACK(p,II_IS_NOTHING_4(p));
					scope=FindTypeOf(p,dims);
				} else if (p&& GetCharAt(p)==':' && GetCharAt(p-1)==':') {
					p-=2; II_BACK(p,II_IS_NOTHING_4(p));
					scope=GetTextRange(WordStartPosition(p,true),p+1);
				} else if (GetCharAt(p)=='>') {
					p-=2; II_BACK(p,II_IS_NOTHING_4(p));
					scope=FindTypeOf(p,dims); dims--;
				} else {
					scope = FindScope(p+1);
				}
				wxString type=code_helper->GetCalltip(scope,func_name,false,true);
				if (!type.Len()) { // sera sobrecarga del operator() ?
					type=FindTypeOf(func_name,p0_name);
					type=code_helper->GetCalltip(type,"operator()",true,true);
				}
				if (type.Len()) {
					while(type.Last()=='*') { dims++; type.RemoveLast(); }
					return type;
				}
			}
			
			// si no paso nada con metodo/funcion, probar si era cast
			p=p_par+1;
			II_FRONT_NC(p, II_IS_NOTHING_4(p));
			if (c=='(') {
				int p2=BraceMatch(p);
				p2++;
				II_FRONT_NC(p2,II_IS_NOTHING_4(p2));
				if (c=='(' || ((c|32)>='a' && (c|32)<='z') || c=='_' ) {
					int p2=BraceMatch(p)-1;
					while (p2>p && ( II_IS_NOTHING_2(p2) || c=='*') ) {
						if (c=='*')
							dims++;
						p2--;
					}
					return code_helper->UnMacro(GetTextRange(p+1,p2+1),dims);
				}
			}
				
		}
		
	} else { // si la cosa no estaba entre parentesis, podemos encontrarnos con indices entre corchetes o el nombre
		II_BACK(p, (II_IS_NOTHING_2(p) || ( ( (c|32)>='a' && (c|32)<='z' ) || (c>='0' && c<='9') || c=='_' )) && s!=wxSTC_C_WORD );
		p++;
	}
	if (GetCharAt(p)=='(')
		p++;
	while (II_IS_2(p,'&','*') || II_IS_NOTHING_4(p)) {
		if (!II_IS_NOTHING_4(p)) {
			if (c=='*')
				dims--;
			else if (c=='&')
				dims++;
		}
		p++;
	}
	if (c=='(') {
		p=BraceMatch(p);
		if (p==wxSTC_INVALID_POSITION) {
			dims=SRC_PARSING_ERROR;
			return "";
		}
		wxString ans = FindTypeOf(p,dims,false);
		return ans;
	} else {
		if ( ( (c|32)>='a' && (c|32)<='z' ) || c=='_' ) {
			s=p;
			wxString key=GetTextRange(p,WordEndPosition(p,true));
			wxString ans=FindTypeOf(key,s);
			if (ans.Len() && dims<0 && s==0) {
				wxString type=code_helper->GetCalltip(ans,"operator[]",true,true);				
				if (type.Len()) { 
					while(type.Last()=='*') { dims++; type.RemoveLast(); }
					dims++; ans=type;
				}
			}
			if (s==SRC_PARSING_ERROR) {
				dims=SRC_PARSING_ERROR;
				return "";
			}
			dims+=s;
			return ans;
		}
	}
	dims=SRC_PARSING_ERROR;
	return "";
}

/**
* @brief determina en qué función o clase estámos actualmente
* 
* Se usa para saber el scope con el que consultar al índice de autocompletado,
* y para mostrar el contexto (WhereAmI). El primer uso va con full_scope en falso
* para que retorna nada más que el nombre del scope (Ej: "mi_clase"), mientras que
* el segundo va con full_scope en true para que incluya el nombre del método
* (Ej: "mi_clase::mi_metodo") o el tipo de definicion (Ej: "class mi_clase").
* Los argumentos se retornan por separado porque se usan en los dos casos: en el
* primero para obtener los identificadores para usarlos al autocompletar, en el 
* segundo para mostrarlos como están. Se retornan como posiciones del código
* que son las de los paréntesis. Se retornan así y no como wxString para poder
* analizarlos en el código y utilizar así el coloreado como ayuda. Si no hay
* argumentos args no se modifica, por lo que debería entrar con valores
* inválidos (como {-1,-1}) para saber desde afuera si el valor de retorno es real.
**/ 
wxString mxSource::FindScope(int pos, wxString *args, bool full_scope, int *scope_start) {
	if (scope_start) *scope_start=0;
	wxString scope, type;
	int l=pos,s;
	int first_p=-1; // guarda el primer scope, para buscar argumentos si es funcion
	char c;
	while (true) {
		int p_llave_a = FindText(pos,0,"{");
		while (p_llave_a!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_a))
			p_llave_a = FindText(p_llave_a-1,0,"{");
		int p_llave_c = FindText(pos,0,"}");
		while (p_llave_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_c))
			p_llave_c = FindText(p_llave_c-1,0,"}");
		if (p_llave_c==wxSTC_INVALID_POSITION && p_llave_a==wxSTC_INVALID_POSITION) {
			break;
		} else if (p_llave_c!=wxSTC_INVALID_POSITION && (p_llave_a==wxSTC_INVALID_POSITION || p_llave_c>p_llave_a) ) {
			pos=BraceMatch(p_llave_c);
			if (pos==wxSTC_INVALID_POSITION)
				break;
			else
				pos--;
		} else if (p_llave_a!=wxSTC_INVALID_POSITION && (p_llave_c==wxSTC_INVALID_POSITION || p_llave_c<p_llave_a) ) {
			int p=pos=p_llave_a-1;
			II_BACK(p,II_IS_NOTHING_4(p));
			if (p>4&&GetCharAt(p-4)=='c'&&GetCharAt(p-3)=='o'&&GetCharAt(p-2)=='n'&&GetCharAt(p-1)=='s'&&GetCharAt(p)=='t'&&(GetCharAt(p-5)==')'||II_IS_NOTHING_4(p-5))) {
				p-=5; II_BACK(p,II_IS_NOTHING_4(p)); // saltear ("const")
			}
			if (c==')') { // puede ser funcion
				int op=p; // p can be first_p
				p=BraceMatch(p);
				// sigue como antes
				if (p!=wxSTC_INVALID_POSITION) {
					p--;
					II_BACK(p,II_IS_NOTHING_4(p));
					op=p; first_p=p+1;
					p=WordStartPosition(p,true)-1;
					if (scope_start) *scope_start=p;
					if (full_scope) scope=GetTextRange(p+1,op+1); // nombre de un método?
					II_BACK(p,II_IS_NOTHING_4(p));
					// el "GetCharAt(p)==','" se agrego el 29/09 para los constructores en constructores
					if (GetCharAt(p)==':' || GetCharAt(p)==',' || (p && GetCharAt(p)=='~' && GetCharAt(p-1)==':')) {
						if (GetCharAt(p)=='~') { p--; if (full_scope) scope=wxString("~")+scope; } // agregado para arreglar el scope de un destructor
						if (GetCharAt(p-1)==':') {
							p-=2;
							II_BACK(p,II_IS_NOTHING_4(p));
							wxString aux = code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1)); // nombre de la clase?
							if (scope_start) *scope_start=p;
							if (full_scope) scope=aux+"::"+scope; else { scope=aux; break; }
						} else { // puede ser constructor
							p = FindText(p,0,"::");
							if (p!=wxSTC_INVALID_POSITION) {
								int e=p+2;
								p--;
								II_BACK(p,II_IS_NOTHING_4(p));
								II_FRONT_NC(e,II_IS_NOTHING_4(e));
								if (GetTextRange(WordStartPosition(p,true),p+1)==GetTextRange(e,WordEndPosition(e,true))) {
									wxString aux=code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1)); // nombre de la clase?
									if (scope_start) *scope_start=p;
									if (full_scope) scope=aux+"::"+aux; else { scope=aux; break; }
								}
							}
						}
					}
				}
			} else { // puede ser clase o struct
				II_BACK(p,II_IS_NOTHING_4(p) || !II_IS_6(p,'{','}',':',';',')','('));
				p++;
				II_FRONT(p,II_IS_NOTHING_4(p));
				if (GetStyleAt(p)==wxSTC_C_WORD) {
					bool some=false;
					if (GetTextRange(p,p+6)=="struct")
					{ if (!type.Len()) type="struct"; p+=6; some=true; }
					else if (GetTextRange(p,p+5)=="class")
					{ if (!type.Len()) type="class"; p+=5; some=true; }
					else if (GetTextRange(p,p+p)=="namespace")
					{ if (!type.Len()) type="namespace"; p+=9; some=true; }
					if (some) {
						II_FRONT(p,II_IS_NOTHING_4(p));
						wxString aux=code_helper->UnMacro(GetTextRange(p,WordEndPosition(p,true)));
						if (scope_start) *scope_start=p;
						if (full_scope) scope=aux+"::"+scope; else { scope=scope; break; }
					}
				}
			}
		}
	}
	if (full_scope && scope.EndsWith("::")) { // si no es metodo ni funcion
		scope=scope.Mid(0,scope.Len()-2);
		scope=type+" "+scope;
	} else if (first_p!=-1 && args) { // sino agregar argumentos
		int p=first_p;
		II_FRONT(p,II_IS_NOTHING_4(p));
		if (GetCharAt(p)=='(') {
			int p2=BraceMatch(p);
			if (p2!=wxSTC_INVALID_POSITION) {
				// los argumentos se agregan medio formateados (sin dobles espacios ni saltos de linea, y sin comentarios)
				bool prev_was_space;
				for(int i=p;i<=p2;i++) {
					if (!II_IS_COMMENT(i)) {
						char c=GetCharAt(i);
						if (c==' '||c=='\t'||c=='\n'||c=='\r') {
							if (!prev_was_space) { (*args)<<' '; prev_was_space=true; }
						} else { 
							prev_was_space=false; (*args)<<c;
						}
					}
				}
			}
		}
	}
	return scope;
}

void mxSource::OnToolTipTimeOut (wxStyledTextEvent &event) {
	HideCalltip();
}

void mxSource::OnToolTipTime (wxStyledTextEvent &event) {
//	if (!FindFocus() || main_window->notebook_sources->GetSelection()!=main_window->notebook_sources->GetPageIndex(this)) // no mostrar tooltips si no se tiene el foco
//	if (FindFocus()!=this) // no mostrar tooltips si no se tiene el foco
//		return;
	
	// no mostrar tooltips si no es la pestaña del notebook seleccionada, o el foco no esta en esta ventana
	if (!main_window->IsActive() || main_window->focus_source!=this) return; 
	
	// no mostrar si el mouse no está dentro del area del fuente
	wxRect psrc = GetScreenRect();
	wxPoint pmouse = wxGetMousePosition()-psrc.GetTopLeft();
	if (pmouse.x<0||pmouse.y<0) return;
	if (pmouse.x>=psrc.GetWidth()||pmouse.y>=psrc.GetHeight()) return;
	
	
	// si esta en un margen....
	int p = event.GetPosition();
	if (p==-1) {
		int x=event.GetX(), y=event.GetY();
		if (GetMarginForThisX(x)==MARGIN_BREAKS) {
			x=10; for(int i=0;i<MARGIN_NULL;i++) { x+=GetMarginWidth(i); }
			int l=LineFromPosition( PositionFromPointClose(x,y) );
			BreakPointInfo *bpi=m_extras->FindBreakpointFromLine(this,l);
			if (bpi && bpi->annotation.Len()) ShowBaloon(bpi->annotation,PositionFromLine(l));
		}
		return;
	}
	
	// si no esta depurando, buscar el tipo de dato del simbolo y mostrarlo
	if (!debug->IsDebugging()) {
		if (!config_source.toolTips)
			return;
		int s;
		if (II_SHOULD_IGNORE(p)) 
			return;
		int e = WordEndPosition(p,true);
		s = WordStartPosition(p,true);
		if (s!=e) {
			wxString key = GetTextRange(s,e);
			// buscar en la funcion/metodo
			wxString bkey=key, type = FindTypeOf(e-1,s);
			if ( s!=SRC_PARSING_ERROR && type.Len() ) {
				while (s>0) {
					type<<_("*");
					s--;
				}
				ShowBaloon ( bkey +": "+type , p );
			} else { // buscar el scope y averiguar si es algo de la clase
				type = FindScope(s);
				if (type.Len()) {
					type = code_helper->GetAttribType(type,bkey,s);
					if (!type.Len())
						type=code_helper->GetGlobalType(bkey,s);
				} else {
					type=code_helper->GetGlobalType(bkey,s);
				}
				if (type.Len()) {
					while (s>0) {
						type<<_("*");
						s--;
					}
					ShowBaloon ( bkey +": "+type , p );
				}
			}
		}
	} else 
	// si se esta depurando, evaluar la inspeccion	
	if (debug->IsPaused()) {
		int e = GetSelectionEnd();
		int s = GetSelectionStart();
		if (e==s || p<s || p>e) {
			e = WordEndPosition(p,true);
			s = WordStartPosition(p,true);
		}
		if (s!=e) {
			while ( s>2 && (GetTextRange(s-1,s)=="." || GetTextRange(s-2,s)=="->" || GetTextRange(s-2,s)=="::")) {
				int s2=s-1; if (GetTextRange(s-1,s)!=".") s2--;
				int s3 = WordStartPosition(s2,true);
				if (s3<s2) s=s3; else break; // que significaria el caso del break?? (sin el break podria ser loop infinito)
			}
			wxString key = GetTextRange(s,e);
			wxString ans = debug->InspectExpression(key);
			if (ans.Len()) {
				wxRect r=GetScreenRect();
				int x=event.GetX()+r.GetLeft(),y=event.GetY()+r.GetTop();
				ShowInspection(wxPoint(x,y),key,ans);
//				ShowBaloon ( key +": "+ ans , p ); // old method
			}
		}
	}
}

void mxSource::MakeUntitled(wxString ptext) {
	SetPageText(ptext);
	sin_titulo=true;
}

void mxSource::SetPageText(wxString ptext) {
	main_window->notebook_sources->SetPageText(main_window->notebook_sources->GetPageIndex(this),page_text=ptext);
}

void mxSource::OnSavePointReached (wxStyledTextEvent &event) {
	main_window->notebook_sources->SetPageText(main_window->notebook_sources->GetPageIndex(this),page_text);
}

void mxSource::OnSavePointLeft (wxStyledTextEvent &event) {
	main_window->notebook_sources->SetPageText(main_window->notebook_sources->GetPageIndex(this),page_text+"*");
}

void mxSource::OnKillFocus(wxFocusEvent &event) {
	if (!focus_helper.KillIsMasked()) HideCalltip(); 
	event.Skip();
}

void mxSource::OnSetFocus(wxFocusEvent &event) {
	ro_quejado=false;
	if (main_window) {
		if (main_window->focus_source!=this) {
			navigation_history.OnFocus(this);
			main_window->focus_source=this;
		}
	}
	event.Skip();
	if (!sin_titulo) CheckForExternalModifications();
}

void mxSource::OnEditAutoCompleteAutocode(wxCommandEvent &evt) {
	HideCalltip();
	int p=GetCurrentPos();
	int ws=WordStartPosition(p,true);
	code_helper->AutocompleteAutocode(this,GetTextRange(ws,p));
}


//#ifndef _ZINJAI_DEBUG
void mxSource::OnEditForceAutoComplete(wxCommandEvent &evt) {
	int p=GetCurrentPos();
	char chr = p>0?GetCharAt(p-1):' ';

//	CodeHelper::RAIAutocompModeChanger rai_acmc;
//	if (calltip_mode==MXS_AUTOCOMP && p==AutoCompPosStart()) rai_acmc.Change(2);
	
	HideCalltip();
	
	int s=GetStyleAt(p-1);
	if (s==wxSTC_C_PREPROCESSOR) {
		int ws=WordStartPosition(p,true);
		if (chr=='#')
			code_helper->AutocompletePreprocesorDirective(this);
		else if (GetCharAt(ws-1)=='#')
			code_helper->AutocompletePreprocesorDirective(this,GetTextRange(ws,p));
		return;
	} else if (s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR) {
		int ws=WordStartPosition(p,true);
		if (chr=='@' || chr=='\\')
			code_helper->AutocompleteDoxygen(this);
		else if (GetCharAt(ws-1)=='\\' || GetCharAt(ws-1)=='@')
			code_helper->AutocompleteDoxygen(this,GetTextRange(ws,p));
		return;
	}
	
	if (chr==':' && p>1 && GetCharAt(p-2)==':') {
		p-=3;
		int s; char c;
		II_BACK(p,II_IS_NOTHING_2(p));
		if ((c=GetCharAt(p))=='>') {
			int lc=1;
			while (p-- && lc) {
				c=GetCharAt(p);
				if (c=='>' && !II_IS_NOTHING_4(p)) lc++;
				else if (c=='<' && !II_IS_NOTHING_4(p)) lc--;
			}
			II_BACK(p,II_IS_NOTHING_2(p));
			if (p<0) p=0;
		}
		wxString key=GetTextRange(WordStartPosition(p,true),WordEndPosition(p,true));
		if (key.Len()!=0) {
			if (!code_helper->AutocompleteScope(this,key,"",false,false))
				ShowBaloon(wxString(LANG(SOURCE_NO_ITEMS_FOR_AUTOCOMPLETION,"No se encontraron elementos para autocompletar el ambito "))<<key);
		} else
			ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
	} else if (chr=='>' && p>1 && GetCharAt(p-2)=='-') {
		p-=2;
		int dims;
		wxString type = FindTypeOf(p-1,dims);
		if (dims==SRC_PARSING_ERROR) {
			ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
		} else if (dims==1) {
			if (!code_helper->AutocompleteScope(this,type,"",true,false))
				ShowBaloon(wxString(LANG(SOURCE_NO_ITEMS_FOR_AUTOCOMPLETION,"No se encontraron elementos para autocompletar el ambito "))<<type);
		} else if (type.Len()!=0 && dims==0) {
			ShowBaloon(LANG(SOURCE_TIP_NO_DEREFERENCE,"Tip: Probablemente no deba desreferenciar este objeto."));
		} else
			ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
	} else if (chr=='.') {
		bool shouldComp=true;
		char c;
		int s,p=GetCurrentPos()-2;
		II_BACK(p,II_IS_NOTHING_4(p));
		if ((c=GetCharAt(p))>='0' && c<='9') {
			while ( (c=GetCharAt(p))>='0' && c<='9' ) {
				p--;
			}
			shouldComp = (c=='_' || ( (c|32)>='a' && (c|32)<='z' ) );
		}
		if (shouldComp) {
			int dims;
			wxString type = FindTypeOf(p,dims);
			if (dims==SRC_PARSING_ERROR) {
				ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
			} else	if (dims==0) {
				if (!code_helper->AutocompleteScope(this,type,"",true,false))
					ShowBaloon(wxString(LANG(SOURCE_NO_ITEMS_FOR_AUTOCOMPLETION,"No se encontraron elementos para autocompletar el ambito "))<<type);
			} else if (type.Len()!=0 && dims>0)
				ShowBaloon(LANG(SOURCE_TIP_DO_DEREFERENCE,"Tip: Probablemente deba desreferenciar este objeto."));
			else
				ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
		} else 
			ShowBaloon(LANG(SOURCE_CANT_AUTOCOMPLETE_HERE,"No se puede autocompletar en este punto"));
	} else {
		int s,e=GetCurrentPos()-1,ctp=0; // el cero en ctp es solo para silenciar un warning
		char c;
		if (chr=='(') {
			ctp=e;
			e--;
			II_BACK(e,II_IS_NOTHING_4(e));
		} else if (chr==',') {
			int p=e;
			c=GetCharAt(--p);
			while (true) {
				II_BACK(p, !II_IS_5(p,';','}','{','(',')') || II_SHOULD_IGNORE(p));
				if (c=='(') {
					e=p-1;
					II_BACK(e,II_IS_NOTHING_4(e));
					if (calltip_brace==p && calltip_mode==MXS_CALLTIP)
						return;
					ctp=p;
					chr='(';
					break;
				} else if (c==')') {
					if ( (p=BraceMatch(p))==wxSTC_INVALID_POSITION )
						break;
					p--;
				} else
					break;
			}
		}
		int p=WordStartPosition(e,true);
		wxString key = GetTextRange(p,e+1);
		if (p && GetCharAt(p-1)=='#' && GetStyleAt(p-1)==wxSTC_C_PREPROCESSOR) {
			code_helper->AutocompletePreprocesorDirective(this,key);
			return;
		}
		if ((e-p+1 && key.Len()>1) || !(key[0]==' ' || key[0]=='\t' || key[0]=='\n' || key[0]=='\r')) {
			int dims;
			p--;
			II_BACK(p,II_IS_NOTHING_4(p));
			if (c=='.') {
				wxString type = FindTypeOf(p-1,dims);
				if (dims==0) {
					if (chr=='('/* && config_source.callTips*/)
						code_helper->ShowFunctionCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				}
			} else if (c=='>' && GetCharAt(p-1)=='-') {
				p--;
				wxString type = FindTypeOf(p-1,dims);
				if (dims==1) {
					if (chr=='('/* && config_source.callTips*/)
						code_helper->ShowFunctionCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				}
			} else if (c==':' && GetCharAt(p-1)==':') {
				p-=2;
				II_BACK(p,II_IS_NOTHING_4(p));
				wxString type = GetTextRange(WordStartPosition(p,true),p+1);
				if (chr=='('/* && config_source.callTips*/)
					code_helper->ShowFunctionCalltip(ctp,this,type,key);
				else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
					code_helper->AutocompleteScope(this,type,key,true,false);
			} else {
				if (chr=='('/* && config_source.callTips*/) {
					if (!code_helper->ShowFunctionCalltip(ctp,this,FindScope(GetCurrentPos()),key,false)) {
						// mostrar calltips para constructores
						p=ctp-1;
						bool f;
						while ( p>0 && ( !II_IS_3(p,'(','{',';') || (f=II_SHOULD_IGNORE(p))) ) {
							if (!f && c==')' && (p=BraceMatch(p))==wxSTC_INVALID_POSITION)
								return;
						p--;
						}
						p++;
						II_FRONT_NC(p,II_IS_NOTHING_4(p) || II_SHOULD_IGNORE(p) || (s==wxSTC_C_WORD));
						int p1=p;
						II_FRONT_NC(p,(c=GetCharAt(p))=='_' || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') );
						wxString key=GetTextRange(p1,p);
						if (!code_helper->ShowConstructorCalltip(ctp,this,key)) {
							// mostrar sobrecarga del operador()
							wxString type=FindTypeOf(key,p1);
							if (type.Len()) {
								code_helper->ShowFunctionCalltip(ctp,this,type,"operator()",true);
							}
						}
					}
				} else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') ) {
					wxString args; int scope_start;
					wxString scope = FindScope(GetCurrentPos(),&args, false,&scope_start);
					code_helper->AutocompleteGeneral(this,scope,key,&args,scope_start);
				}
			}
		} else {
			wxString scope = FindScope(GetCurrentPos());
			if (scope.Len())
				code_helper->AutocompleteScope(this,scope,"",true,true);
		}
	}
	if (calltip_mode==MXS_NULL)	ShowBaloon(LANG(SOURCE_NO_ITEMS_FOR_AUTOCOMPLETION,"No se encontraron opciones para autocompletar"),p);
}

//#endif

DiffInfo *mxSource::MarkDiffs(int from, int to, MXS_MARKER marker, wxString extra) {
	if (mxSTC_MARK_DIFF_NONE==marker) {
		for (int i=from;i<=to;i++) {
			MarkerDelete(i,mxSTC_MARK_DIFF_ADD);	
			MarkerDelete(i,mxSTC_MARK_DIFF_DEL);	
			MarkerDelete(i,mxSTC_MARK_DIFF_CHANGE);	
		}
		while (first_diff_info) delete first_diff_info;
		return nullptr;
	} else {
		int *handles = new int[to-from+1];
		for (int i=from;i<=to;i++)
			handles[i-from]=MarkerAdd(i,marker);
		return new DiffInfo(this,handles,to-from+1,marker,extra);
	}
}

/// @brief looks for the symbol under the cursor in the symbols tress (functions/methods/classes) and goes to its definition (or opens mxGotoFunctionDialog if there are options)
void mxSource::JumpToCurrentSymbolDefinition() {
	int pos = GetCurrentPos();
	int s=WordStartPosition(pos,true);
	int e=WordEndPosition(pos,true);
	wxString key = GetTextRange(s,e);
	if (key.Len()) new mxGotoFunctionDialog(key,main_window,GetFileName(false));
}

void mxSource::OnClick(wxMouseEvent &evt) {
	if (evt.ControlDown()) {
		int p=PositionFromPointClose(evt.GetX(),evt.GetY());
		SetSelectionStart(p); SetSelectionEnd(p);
		JumpToCurrentSymbolDefinition();
//	} else if (evt.AltDown()) {
//		int pos = PositionFromPointClose(evt.GetX(),evt.GetY());
//		int s=WordStartPosition(pos,true);
//		int e=WordEndPosition(pos,true);
//		wxString key = GetTextRange(s,e);
//		if (key.Len()!=0) { // puede ser una directiva de preprocesador
//			if (GetCharAt(s-1)=='#')
//				key = GetTextRange(s-1,e);
//		}
//		if (key.Len()) main_window->ShowQuickHelp(key); 
	} else {
		wxPoint point=evt.GetPosition();
		int ss=GetSelectionStart(), se=GetSelectionEnd(), p=PositionFromPointClose(point.x,point.y);
		if ( p!=wxSTC_INVALID_POSITION && ss!=se && ( (p>=ss && p<se) || (p>=se && p<ss) ) ) {
//			MarkerDelete(current_line,mxSTC_MARK_CURRENT);
			wxTextDataObject my_data(GetSelectedText());
			wxDropSource dragSource(this);
			dragSource.SetData(my_data);
			mxDropTarget::current_drag_source=this;
			mxDropTarget::last_drag_cancel=false;
			wxDragResult result = dragSource.DoDragDrop(wxDrag_AllowMove|wxDrag_DefaultMove);
			if (mxDropTarget::current_drag_source!=nullptr && result==wxDragMove) {
				mxDropTarget::current_drag_source=nullptr;
				SetTargetStart(ss); SetTargetEnd(se); ReplaceTarget("");
			} 
			else if (result==wxDragCancel && ss==GetSelectionStart()) {
				DoDropText(evt.GetX(),evt.GetY(),""); // para evitar que se congele el cursor
				SetSelection(p,p);
//				evt.Skip();
			} else {
				DoDropText(evt.GetX(),evt.GetY(),"");
			}
		} else
			evt.Skip();
	}
}
	
void mxSource::LoadSourceConfig() {
	config_source=config->Source;
	// set spaces and indention
	if (project && project->custom_tabs) {
		config_source.tabWidth=project->custom_tabs;
		config_source.tabUseSpaces=project->tab_use_spaces;
	}
	SetTabWidth (config_source.tabWidth);
	SetUseTabs (!config_source.tabUseSpaces);
	SetIndent(0);
	SetTabIndents (true);
	SetBackSpaceUnIndents (true);
	SetIndent (config_source.indentEnable? config_source.tabWidth: 0);
	SetIndentationGuides(true);
	SetLineNumbers();
	SetMarginWidth (MARGIN_FOLD, config_source.foldEnable? 15: 0);
}

void mxSource::SetLineNumbers() {
	// establecer el margen para los nros de lineas
	wxString ancho("  X");
	int lnct=GetLineCount();
	while (lnct>10) {
		ancho+="X";
		lnct/=10;
	}
	SetMarginWidth (MARGIN_LINENUM, config_source.lineNumber?TextWidth (wxSTC_STYLE_LINENUMBER, ancho):0);
	SetViewWhiteSpace(config_source.whiteSpace?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
	SetViewEOL(config_source.whiteSpace);
}

void mxSource::SetDiffBrother(mxSource *source) {
	diff_brother=source;
}

void mxSource::ApplyDiffChange() {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	UndoActionGuard undo_action(this);
	while (di) {
		for (int i=0;i<di->len;i++) {
			if (MarkerLineFromHandle(di->handles[i])==cl) {
				if (di->marker==mxSTC_MARK_DIFF_ADD) {
					for (int i=0;i<di->len;i++) {
						cl = MarkerLineFromHandle(di->handles[i]);
						MarkerDeleteHandle(di->handles[i]);
						SetSelection(PositionFromLine(cl),PositionFromLine(cl));
						LineDelete();
					}
					if (diff_brother) diff_brother->MarkerDeleteHandle(di->brother->handles[0]);
				} else if (di->marker==mxSTC_MARK_DIFF_DEL) {
					MarkerDeleteHandle(di->handles[0]);
					InsertText(PositionFromLine(cl),di->extra<<"\n");
					if (diff_brother)
						for (int i=0;i<di->brother->len;i++)
							diff_brother->MarkerDeleteHandle(di->brother->handles[i]);
				} else if (di->marker==mxSTC_MARK_DIFF_CHANGE) {
					int ocl=MarkerLineFromHandle(di->handles[0]);
					MarkerDeleteHandle(di->handles[0]);
					for (int i=1;i<di->len;i++) {
						cl = MarkerLineFromHandle(di->handles[i]);
						MarkerDeleteHandle(di->handles[i]);
						SetSelection(PositionFromLine(cl),PositionFromLine(cl));
						LineDelete();
					}
					if (diff_brother)
						for (int i=0;i<di->brother->len;i++)
							diff_brother->MarkerDeleteHandle(di->brother->handles[i]);
					SetTargetStart(PositionFromLine(ocl));
					SetTargetEnd(GetLineEndPosition(ocl));
					ReplaceTarget(di->extra);
				}
				if (diff_brother) {
					delete di->brother;
					diff_brother->Refresh();
				}
				delete di; di=nullptr; break;
				
			}
		}
		if (di) di = di->next;
	}
	Refresh();
}

void mxSource::DiscardDiffChange() {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	UndoActionGuard undo_action(this);
	while (di) {
		for (int i=0;i<di->len;i++) {
			if (MarkerLineFromHandle(di->handles[i])==cl) {
				for (int i=0;i<di->len;i++)
					MarkerDeleteHandle(di->handles[i]);
				if (diff_brother) {
					for (int i=0;i<di->brother->len;i++)
						diff_brother->MarkerDeleteHandle(di->brother->handles[i]);
					diff_brother->Refresh();
				}
				if (diff_brother) {
					delete di->brother;
					diff_brother->Refresh();
				}
				delete di; di=nullptr; break;
			}
		}
		if (di) di = di->next;
	}
	Refresh();
}

void mxSource::GotoDiffChange(bool forward) {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	int lmin=-1,bmin=-1;
	while (di) {
		bool in=false;
		for (int i=0;i<di->len;i++) {
			if (MarkerLineFromHandle(di->handles[i])==cl) {
				in=true; break;
			}
		}
		int lb=MarkerLineFromHandle(di->handles[0]);
		if (!in) {
			if (forward) {
				if (lb>cl&&(lmin==-1||lb<lmin)) {
					if (diff_brother && di->bhandles) bmin=diff_brother->MarkerLineFromHandle(di->bhandles[0]);
					lmin=lb;
				}
			} else {
				if (lb<cl&&(lmin==-1||lb>lmin)) {
					if (diff_brother && di->bhandles) bmin=diff_brother->MarkerLineFromHandle(di->bhandles[0]);
					lmin=lb;
				}
			}
		}
		if (di) di = di->next;
	}
	if (lmin>=0) MarkError(lmin);
	if (bmin>=0) diff_brother->MarkError(bmin);
	else ShowBaloon(LANG(MAINW_DIFF_NO_MORE,"No se encontraron mas cambios"));
}

void mxSource::ShowDiffChange() {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	UndoActionGuard undo_action(this);
	while (di) {
		for (int i=0;i<di->len;i++) {
			if (MarkerLineFromHandle(di->handles[i])==cl) {
				int l = MarkerLineFromHandle(di->handles[0]);
				ShowBaloon(di->extra,PositionFromLine(l));
				di=nullptr; break;
			}
		}
		if (di) di = di->next;
	}
	Refresh();
}

void mxSource::Reload() {
	m_extras->FromSource(this);
	bool old_sin_titulo=sin_titulo;
	LoadFile(GetFullPath());
	sin_titulo=old_sin_titulo; // to keep as untitled when reloading an untitled source (usefull for custom tools such as clang-format)
	m_extras->ToSource(this);
}

void mxSource::AlignComments (int col) {
	UndoActionGuard undo_action(this);
	config_source.alignComments=col;
	int ss = GetSelectionStart();
	int se = GetSelectionEnd();
	if (ss>se) { int aux=ss; ss=se; se=aux; }
	bool sel = se>ss; char c;
	int line_ss=sel?LineFromPosition(ss):0, line_se=sel?LineFromPosition(se):GetLineCount();
	int p3,pl=PositionFromLine(line_ss);
	bool prev=false;
	for (int i=line_ss;i<line_se;i++) {
		int p1=pl, s;
		int p2=PositionFromLine(i+1);
		if (!II_IS_COMMENT(p1) || prev) {
			if (!II_IS_COMMENT(p1)) {
				while (p1<p2 && !II_IS_COMMENT(p1)) p1++;
				p3=p1;
				while (p3<p2 && (II_IS_COMMENT(p3) || II_IS_2(p3,' ','\t')) ) p3++;
			} else {
				while (p1<p2 && II_IS_2(p1,' ','\t')) p1++;
				p3=p2;
			}
			if (p1<p2 && p3==p2) {
				prev=true;
				int cp=GetColumn(p1);
				if (cp<col) {
					InsertText(p1,wxString(wxChar(' '),col-cp));
				} else if (col<cp) {
					SetTargetEnd(p1);
					p1--;
					while (p1>pl && II_IS_NOTHING_2(p1)) p1--;
					SetTargetStart(p1+1);
					cp=GetColumn(p1);
					if (col>=cp)
						ReplaceTarget(wxString(wxChar(' '),col-cp));
					else
						ReplaceTarget(" ");
				} else
					prev=false;
				p2=PositionFromLine(i+1);
			} else {
				prev=false;
				if (p3<p2) { p2=p3;  i--; }
			}
		}
		pl=p2;
	}
}

void mxSource::RemoveComments () {
	UndoActionGuard undo_action(this);
	int ss = GetSelectionStart();
	int se = GetSelectionEnd();
	if (ss>se) { int aux=ss; ss=se; se=aux; }
	bool sel = se>ss;
	int line_ss=sel?LineFromPosition(ss):0, line_se=sel?LineFromPosition(se):GetLineCount();
	int p1,p2;
	for (int i=line_ss;i<line_se;i++) {

		int p3=ss=PositionFromLine(i), s;
		se=PositionFromLine(i+1);
		
		while (p3<se && !II_IS_COMMENT(p3)) p3++;
		if (p3>=se) continue;
		p1=p3;
		while (p3<se && II_IS_COMMENT(p3)) p3++;
		p2=p3;
		if (p1<=GetLineIndentPosition(i) && p2>se-2)  { // si es toda la linea de comentario, borrar entera
			p1=PositionFromLine(i);
			p2=PositionFromLine(i+1);
			line_se--;
		} else if (p2==se) {
			i++; p2--;
		}
		SetTargetEnd(p2);
		SetTargetStart(p1);
		ReplaceTarget("");
		i--;
	}
}

bool mxSource::IsComment(int pos) {
	int s;
	return II_IS_COMMENT(pos);
}


/**
* Lleva el cursor a una posición específica, forzando su visualización
* Reemplaza al GotoPos original porque ese cuando se le da una posición
* que no está al comienzo de la linea hace scroll horizontal para centrarla
* y es basatante incomodo (apareció con wxWidgets-2.8.10?)
**/
void mxSource::GotoPos(int pos) {
	wxStyledTextCtrl::GotoPos(PositionFromLine(LineFromPosition(pos)));
	SetSelectionStart(pos);
	SetSelectionEnd(pos);
}

void mxSource::CheckForExternalModifications() {
	if (!source_filename.FileExists()) {
		SetModify(true);
		return;
	}
	static wxDateTime dt;
	dt = source_filename.GetModificationTime();
	if (dt==source_time) return;
	class SourceModifAction:public mxMainWindow::AfterEventsAction {
	public: 
		SourceModifAction(mxSource *who):AfterEventsAction(who){}
		void Do() override { source->ThereAreExternalModifications(); }
	};
	main_window->CallAfterEvents(new SourceModifAction(this));
//	ThereAreExternalModifications();
}

void mxSource::ThereAreExternalModifications() {
	wxDateTime dt = source_filename.GetModificationTime();
	if (dt==source_time) return; else SetSourceTime(dt);
	if (!source_time_dont_ask) {
		int res=mxMessageDialog(main_window,LANG(SOURCE_EXTERNAL_MODIFICATION_ASK_RELOAD,"El archivo fue modificado por otro programa.\nDesea recargarlo para obtener las modificaciones?"), source_filename.GetFullPath(), mxMD_YES_NO|mxMD_WARNING,"No volver a preguntar",false).ShowModal();
		source_time_reload=(res&mxMD_YES);
		source_time_dont_ask=(res&mxMD_CHECKED);
	}
	if (source_time_reload) {
		Reload();
	} else {
		SetModify(true);
	}
}

bool mxSource::MySaveFile(const wxString &fname) {
	wxFile file(fname, wxFile::write);
	if (!file.IsOpened())
		return false;
	bool success = file.Write(GetText(), *wxConvCurrent);
	file.Flush(); file.Close();
	if (success) SetSavePoint();
	return success;
}
	
void mxSource::OnModifyOnRO (wxStyledTextEvent &event) {
	if (readonly_mode==ROM_DEBUG) {
		ro_quejado=true;
		int ans = mxMessageDialog(main_window,LANG(DEBUG_CANT_EDIT_WHILE_DEBUGGING,""
			"Por defecto, no se puede modificar el fuente mientras se encuentra\n"
			"depurando un programa, ya que de esta forma pierde la relación que\n"
			"existe entre la información que brinda el depurador a partir del\n"
			"archivo binario, y el fuente que está visualizando. Puede configurar\n"
			"este comportamiento en la seccion Depuracion del cuadro de\n"
			"Preferencias (desde menu Archivo)"),
			LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK,
			LANG(DEBUG_ALLOW_EDIT_WHILE_DEBUGGING,"Permitir editar igualemente"),config->Debug.allow_edition).ShowModal();
		if (ans&mxMD_CHECKED) {
			config->Debug.allow_edition=true;
			SetReadOnlyMode(ROM_NONE);
			event.Skip();
		}
	} else {
		event.Skip();
	}
}

bool mxSource::IsEmptyLine(int l, bool ignore_comments, bool ignore_preproc) {
	int p=PositionFromLine(l), pf=(l==GetLineCount()-1)?GetLength():PositionFromLine(l+1);
	while (p<pf) {
		char c; int s;
		if ( ! ( II_IS_4(p,' ','\t','\n','\r') 
			|| (ignore_comments && II_IS_COMMENT(p))
			|| (ignore_preproc && s==wxSTC_C_PREPROCESSOR)
			) ) return false;
		p++;
	}
	return true;
}

void mxSource::OnKeyDown(wxKeyEvent &evt) {
	if (calltip_mode==MXS_AUTOCOMP && evt.GetKeyCode()==WXK_BACK && config_source.autocompFilters) {
		/*evt.Skip();*/
		int cp = GetCurrentPos()-1;
		SetTargetStart(cp); SetTargetEnd(cp+1); ReplaceTarget(""); // manually delete character, event.Skip whould hide autocompletion menu
		if (cp>=autocomp_helper.GetUserPos())
			code_helper->FilterAutocomp(this,GetTextRange(autocomp_helper.GetBasePos(),cp),true);
		else HideCalltip();
		return;
	}
	if (config_source.autocloseStuff && evt.GetKeyCode()==WXK_BACK) {
		int p=GetCurrentPos();
		if (p) {
			char c2=GetCharAt(p);
			char c1=GetCharAt(p-1);
			if (
				(c1=='('&&c2==')')||
				(c1=='['&&c2==']')||
				(c1=='{'&&c2=='}')||
				(c1=='\''&&c2=='\'')||
				(c1=='\"'&&c2=='\"')
			) {
				SetTargetStart(p); SetTargetEnd(p+1); ReplaceTarget("");
			}
		}
	}
	if (evt.GetKeyCode()==WXK_ESCAPE) {
		rect_sel.is_on=false;
		if (calltip_mode!=MXS_NULL) {
			HideCalltip();
		} else {
			wxCommandEvent evt;
			main_window->OnEscapePressed(evt);
		}
	} else if (config_source.autotextEnabled && evt.GetKeyCode()==WXK_TAB && ApplyAutotext()){
		return;
	} else {
		evt.Skip();
		// autocompletion list selected item could have been changed
		if (calltip_mode==MXS_AUTOCOMP && config_source.autocompTips) 
			autocomp_helper.Restart();
	}
}

void mxSource::SplitFrom(mxSource *orig) {
	LoadFile(orig->source_filename);
	treeId = orig->treeId;
	never_parsed=false; sin_titulo=false; 
	if (m_owns_extras) delete m_extras;
	m_extras=orig->m_extras;
	m_owns_extras=false;
	SetDocPointer(orig->GetDocPointer());
	next_source_with_same_file=orig;
	mxSource *iter=orig;
	while (iter->next_source_with_same_file!=orig) 
		iter=iter->next_source_with_same_file;
	iter->next_source_with_same_file=this;
	GotoPos(orig->GetCurrentPos());
}

void mxSource::SetSourceTime(wxDateTime stime) {
	source_time=stime;
	mxSource *iter=next_source_with_same_file;
	while (iter!=this) {
		iter->source_time=stime;
		iter=iter->next_source_with_same_file;
	}
}

wxString mxSource::WhereAmI() {
	int cp=GetCurrentPos(); wxString args;
	wxString res = FindScope(cp,&args,true); res<<args;
#ifdef _ZINJAI_DEBUG
	res<<"\n"<<cp;
#endif
	return res;
}

bool mxSource::ApplyAutotext() {
	return autocoder->Apply(this);
}

void mxSource::SetColours(bool also_style) {
	SetSelBackground(true,ctheme->SELBACKGROUND);
	StyleSetForeground (wxSTC_STYLE_DEFAULT, ctheme->DEFAULT_FORE);
	StyleSetBackground (wxSTC_STYLE_DEFAULT, ctheme->DEFAULT_BACK);
	StyleSetForeground (wxSTC_STYLE_LINENUMBER, ctheme->LINENUMBER_FORE);
	StyleSetBackground (wxSTC_STYLE_LINENUMBER, ctheme->LINENUMBER_BACK);
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, ctheme->INDENTGUIDE);
	SetFoldMarginColour(true,ctheme->FOLD_TRAMA_BACK);
	SetFoldMarginHiColour(true,ctheme->FOLD_TRAMA_FORE);
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, ctheme->INDENTGUIDE);

	CallTipSetBackground(ctheme->CALLTIP_BACK);
	CallTipSetForeground(ctheme->CALLTIP_FORE);
	
	SetCaretForeground (ctheme->CARET);
	MarkerDefine (wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_BOXPLUS, ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_BOXMINUS, ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_VLINE,     ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_CIRCLEPLUS, ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_CIRCLEMINUS, ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNERCURVE,     ctheme->FOLD_BACK,ctheme->FOLD_FORE);
	MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_LCORNERCURVE,     ctheme->FOLD_BACK,ctheme->FOLD_FORE);
//	MarkerDefine(mxSTC_MARK_CURRENT,wxSTC_MARK_BACKGROUND,ctheme->CURRENT_LINE,ctheme->CURRENT_LINE);
//	SetCaretLineVisible(true); SetCaretLineBackground(ctheme->CURRENT_LINE); SetCaretLineBackAlpha(50);
	SetCaretLineVisible(true); SetCaretLineBackground(ctheme->CURRENT_LINE); SetCaretLineBackAlpha(35);
	MarkerDefine(mxSTC_MARK_USER,wxSTC_MARK_BACKGROUND,ctheme->USER_LINE,ctheme->USER_LINE);
	// markers
	
	if (ctheme->inverted) {
		MarkerDefine(mxSTC_MARK_DIFF_ADD,wxSTC_MARK_BACKGROUND,"DARK GREEN","DARK GREEN");
		MarkerDefine(mxSTC_MARK_DIFF_DEL,wxSTC_MARK_ARROW,"WHITE","RED");
		MarkerDefine(mxSTC_MARK_DIFF_CHANGE,wxSTC_MARK_BACKGROUND,"BROWN","BROWN");
	} else {
		MarkerDefine(mxSTC_MARK_DIFF_ADD,wxSTC_MARK_BACKGROUND,"Z DIFF GREEN","Z DIFF GREEN");
		MarkerDefine(mxSTC_MARK_DIFF_DEL,wxSTC_MARK_ARROW,"BLACK","RED");
		MarkerDefine(mxSTC_MARK_DIFF_CHANGE,wxSTC_MARK_BACKGROUND,"Z DIFF YELLOW","Z DIFF YELLOW");
	}
	MarkerDefine(mxSTC_MARK_BAD_BREAKPOINT,wxSTC_MARK_CIRCLE, ctheme->DEFAULT_BACK, "LIGHT GRAY");
	MarkerDefine(mxSTC_MARK_BREAKPOINT,wxSTC_MARK_CIRCLE, ctheme->DEFAULT_BACK, "RED");
	MarkerDefine(mxSTC_MARK_EXECPOINT,wxSTC_MARK_SHORTARROW, ctheme->DEFAULT_FORE, "Z GREEN");
	MarkerDefine(mxSTC_MARK_FUNCCALL,wxSTC_MARK_SHORTARROW, ctheme->DEFAULT_FORE, "YELLOW");
	MarkerDefine(mxSTC_MARK_STOP,wxSTC_MARK_SHORTARROW, ctheme->DEFAULT_FORE, "RED");
	
	if (also_style) SetStyle(config_source.syntaxEnable);
	
}

/**
* @brief Returns the path (dirs, without filename) of the file as debugger knows it (temp_filename if untitled, source_filename else)
*
* @param for_user  indicates that this path is for user, not for some internal ZinjaI operation, so if source is untitled it will use user's home instead of temp dir
**/
wxString mxSource::GetPath(bool for_user) {
	if (sin_titulo) {
		if (for_user)
			return wxFileName::GetHomeDir();
		else
			return temp_filename.GetPath();
	} else {
			return source_filename.GetPath();
	}
}


/**
* @brief Returns the full path of the file as debugger knows it (temp_filename if untitled, source_filename else)
**/
wxString mxSource::GetFullPath() {
	if (sin_titulo) return temp_filename.GetFullPath();
	else return source_filename.GetFullPath();
}

wxString mxSource::GetFileName(bool with_extension) {
	if (with_extension) {
		if (sin_titulo) return temp_filename.GetFullName();
		else return source_filename.GetFullName();
	} else {
		if (sin_titulo) return temp_filename.GetName();
		else return source_filename.GetName();
	}
}

/**
* @brief Save the code in source_filename (or in temp_filename if untitled) and returns its full path
*
* To be called from tools that need to parse it (cppcheck)
**/
wxString mxSource::SaveSourceForSomeTool() {
	if (sin_titulo) SaveTemp(); else SaveSource();
	return GetFullPath();
}

/**
* @brief return config_runnign.compiler_options parsed (variables replaced and subcommands executed, current_toolchain must be setted)
**/
wxString mxSource::GetCompilerOptions(bool parsed) {
	wxString comp_opts = cpp_or_just_c?config_running.cpp_compiler_options:config_running.c_compiler_options;
	if (parsed) {
		wxArrayString args; 
		mxUT::Split(comp_opts,args,false,true);
		for(unsigned int i=0;i<args.GetCount();i++) args[i]=current_toolchain.FixArgument(cpp_or_just_c,args[i]);
		comp_opts=mxUT::UnSplit(args);
		mxUT::ParameterReplace(comp_opts,"${MINGW_DIR}",current_toolchain.mingw_dir);
		comp_opts = mxUT::ExecComas(working_folder.GetFullPath(),comp_opts);
	}
	return comp_opts;
}

void mxSource::SetCompilerOptions(const wxString &comp_opts) {
	if (cpp_or_just_c) 
		config_running.cpp_compiler_options = comp_opts;
	else 
		config_running.c_compiler_options = comp_opts;
}

bool mxSource::IsCppOrJustC() {
	return cpp_or_just_c;
}

wxFileName mxSource::GetBinaryFileName ( ) {
	if (project && !sin_titulo) 
		return DIR_PLUS_FILE(project->GetTempFolder(),source_filename.GetName()+".o");
	else
		return binary_filename;
}

void mxSource::OnPainted (wxStyledTextEvent & event) {
	char c; int p=GetCurrentPos();
	if ((c=GetCharAt(p))=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']') {
		MyBraceHighLight(p,BraceMatch(p));
	} else if ((c=GetCharAt(p-1))=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']') {
		int m=BraceMatch(p-1);
		if (m!=wxSTC_INVALID_POSITION)
			MyBraceHighLight(p-1,m);
		else
			MyBraceHighLight();
	} else
		MyBraceHighLight();
	event.Skip();
	if (main_window->gcov_sidebar) main_window->gcov_sidebar->Refresh(this);
}

/**
* Esta funcion esta para evitar el flickering que produce usar el bracehighlight
* del stc cuando se llama desde el evento de udateui. A cambio, para que igual sea
* instantaneo se llama desde el evento painted, y para evitar que reentre mil veces
* se guardan las ultimas posiciones y no se vuelve a llamar si son las mismas.
* El problema es que está recalculando el BraceMatch en cada paint.
**/
void mxSource::MyBraceHighLight (int b1, int b2) {
	if (b1==brace_1&&b2==brace_2) return;
	brace_1=b1; brace_2=b2;
	if (b2==wxSTC_INVALID_POSITION) BraceBadLight (b1);
	else BraceHighlight (b1,b2);
	Refresh(false);
}

void mxSource::OnHighLightWord (wxCommandEvent & event) {
	int pos=GetCurrentPos();
	int s=WordStartPosition(pos,true);
	int e=WordEndPosition(pos,true);
	wxString key = GetTextRange(s,e);
	SetKeyWords(3,key);
	Colourise(0,GetLength());
}

void mxSource::OnDoubleClick (wxStyledTextEvent & event) {
	event.Skip();
	wxCommandEvent e;
	OnHighLightWord(e);
}

void mxSource::UpdateExtras ( ) {
	m_extras->FromSource(this);
}

void mxSource::SetReadOnlyMode (ReadOnlyModeEnum mode) {
	if (mode>ROM_SPECIALS) {
		if (mode==ROM_ADD_DEBUG) {
			if (readonly_mode==ROM_NONE) readonly_mode=ROM_DEBUG;
			else if (readonly_mode==ROM_PROJECT) readonly_mode=ROM_PROJECT_AND_DEBUG;
		} else if (mode==ROM_DEL_DEBUG) {
			if (readonly_mode==ROM_DEBUG) readonly_mode=ROM_NONE;
			else if (readonly_mode==ROM_PROJECT_AND_DEBUG) readonly_mode=ROM_PROJECT;
		} else if (mode==ROM_ADD_PROJECT) {
			if (readonly_mode==ROM_NONE) readonly_mode=ROM_PROJECT;
			else if (readonly_mode==ROM_DEBUG) readonly_mode=ROM_PROJECT_AND_DEBUG;
		} else if (mode==ROM_DEL_PROJECT) {
			if (readonly_mode==ROM_PROJECT) readonly_mode=ROM_NONE;
			else if (readonly_mode==ROM_PROJECT_AND_DEBUG) readonly_mode=ROM_DEBUG;
		}
	} else {
		readonly_mode=mode;
	}
	SetReadOnly(readonly_mode!=ROM_NONE);
}

void mxSource::OnMacroAction (wxStyledTextEvent & evt) {
	if (main_window->m_macro&&(*main_window->m_macro)[0].msg==1) {
		main_window->m_macro->Add(MacroAction(evt.GetMessage(),evt.GetWParam(),evt.GetLParam()));
	}
}

wxString mxSource::GetCurrentKeyword (int pos) {
	if (pos==-1) pos=GetCurrentPos();
	int s=WordStartPosition(pos,true);
	if (GetCharAt(s-1)=='#') s--;
	int e=WordEndPosition(pos,true);
	return GetTextRange(s,e);
}

int mxSource::GetMarginForThisX (int x) {
	int x0=GetMarginWidth(MARGIN_LINENUM);
	if (x<x0) return MARGIN_LINENUM;
	x0+=GetMarginWidth(MARGIN_BREAKS);
	if (x<x0) return MARGIN_BREAKS;
	x0+=GetMarginWidth(MARGIN_FOLD);
	if (x<x0) return MARGIN_FOLD;
	return MARGIN_NULL;
}

void mxSource::UserReload ( ) {
	if (!sin_titulo && GetModify()) {
		main_window->notebook_sources->SetSelection(main_window->notebook_sources->GetPageIndex(this));
		if (mxMD_NO==mxMessageDialog(this,LANG(MAINW_CHANGES_CONFIRM_RELOAD,"Hay Cambios sin guardar, desea recargar igualmente la version del disco?"),GetFullPath(),mxMD_YES_NO|mxMD_QUESTION).ShowModal()) 
			return;
	}
	SetSourceTime((sin_titulo?temp_filename:source_filename).GetModificationTime());
	Reload();
	parser->ParseFile(GetFullPath());
}

void mxSource::ShowCallTip (int brace_pos, int calltip_pos, const wxString & s) {
	focus_helper.Mask();
	SetCalltipMode(MXS_CALLTIP);
	last_failed_autocompletion.Reset(); 
	if (!calltip) calltip = new mxCalltip(this);
	calltip_brace = brace_pos;
	calltip_line = LineFromPosition(brace_pos);
	calltip->Show(calltip_pos,s);
}

void mxSource::HideCalltip () {
	switch (calltip_mode) { 
		case MXS_NULL: break;
		case MXS_INSPECTION: HideInspection(); break;
		case MXS_CALLTIP: calltip->Hide(); break;
		case MXS_BALOON: wxStyledTextCtrl::CallTipCancel(); break;
		case MXS_AUTOCOMP: wxStyledTextCtrl::AutoCompCancel(); if (calltip) calltip->Hide(); break;
	}
	calltip_mode = MXS_NULL;
}

void mxSource::ShowAutoComp (int p, const wxString & s, bool is_filter) { 
	SetCalltipMode(MXS_AUTOCOMP);
	last_failed_autocompletion.Reset(); 
	focus_helper.Mask();
#ifdef _STC_HAS_ZASKARS_RESHOW
	if (is_filter) wxStyledTextCtrl::AutoCompReShow(p,s);
	else 
#endif
		wxStyledTextCtrl::AutoCompShow(p,s);
	int pbase = GetCurrentPos()-p;
	wxPoint pt1=PointFromPosition(pbase);
	wxPoint pt2=GetScreenPosition();
	if (calltip_mode==MXS_AUTOCOMP && config_source.autocompTips) 
		autocomp_helper.Start(pbase,is_filter?-1:(pbase+p), pt1.x+pt2.x, pt1.y+pt2.y);
}


void mxSource::OnAutocompSelection(wxStyledTextEvent &event) {
	if (calltip) calltip->Hide();
	calltip_mode = MXS_NULL;
}


void mxSource::OnTimer(wxTimerEvent &event) {
	wxTimer *timer = reinterpret_cast<wxTimer*>(event.GetEventObject());
	if (focus_helper.IsThisYourTimer(timer)) { 
		focus_helper.Unmask();
	} else if (autocomp_helper.IsThisYourTimer(timer)) {
		if (!wxStyledTextCtrl::AutoCompActive()) { // si se cancelo el menu (por ejemplo, el usuario siguio escribiendo la palabra y ya no hay conicidencias)
			if (calltip_mode==MXS_AUTOCOMP) calltip_mode=MXS_NULL; // solo si el modo es autocomp, porque puede ya haber lanzado un calltip real
		}
		if (calltip_mode!=MXS_AUTOCOMP) return;
		wxString help_text = autocomp_list.GetHelp(AutoCompGetCurrent());
		if (!help_text.Len()) { if (calltip) calltip->Hide(); return; }
		if (!calltip) calltip = new mxCalltip(this);
		int autocomp_max_len = autocomp_list.GetMaxLen();
		focus_helper.Mask();
		calltip->Show(autocomp_helper.GetX(),autocomp_helper.GetY(),autocomp_max_len,help_text);
	}
}

void mxSource::OnEditMakeLowerCase (wxCommandEvent & event) {
	LowerCase();
}

void mxSource::OnEditMakeUpperCase (wxCommandEvent & event) {
	UpperCase();
}



/// @todo: ver si es mejor sacar todo esto y delegarselo a scintilla
///
/// nota importante: despues de implementar la edicion en multiples lineas
/// para el scintilla de wx2.8, veo que el de 3.0 ya lo tiene resuelto 
/// (ver SetAdditionalSelectionTyping)... cuando logra migrar a wx3 tendre
/// que cambiar esto :(
void mxSource::InitRectEdit (bool keep_rect_select) {
	// get ordered selection limits
	int beg=GetSelectionStart(), end=GetSelectionEnd(); if (beg>end) swap(beg,end);
	// get lines range
	rect_sel.line_from=LineFromPosition(beg);
	rect_sel.line_to=LineFromPosition(end);
	// check for multiline selection
	if (rect_sel.line_from==rect_sel.line_to) return;
	// get offset for selected range inside a line (will use first one as reference)
	rect_sel.offset_beg=beg-PositionFromLine(rect_sel.line_from);
	rect_sel.offset_end = GetLineEndPosition(rect_sel.line_from)-FindColumn(rect_sel.line_from,GetColumn(end));
	// rember original text in order to detect changes later
	rect_sel.ref_str=GetTextRange(beg,GetLineEndPosition(rect_sel.line_from)-rect_sel.offset_end);
	// enable rectangular edition, and remember if selection is kept (in that case, first keystroke will modify all lines before ApplyRectEdit)
	rect_sel.is_on=true;
	if (!keep_rect_select) SetSelection(beg,beg+rect_sel.ref_str.Len());
	rect_sel.was_rect_select = keep_rect_select;
//	main_window->SetStatusText(wxString("*")<<rect_sel.ref_str<<"*   "<<rect_sel.offset_beg<<" - "<<rect_sel.offset_end);
}

void mxSource::ApplyRectEdit ( ) {
	// si cambia de linea, se termina la edicion
	if (GetCurrentLine()!=rect_sel.line_from) { 
		main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
		rect_sel.is_on=false; return;
	}
	// cur = pos actual, lbeg y lend son las pos globales de inicio y fin de la linea editada
	int cur=GetCurrentPos(), lbeg=PositionFromLine(rect_sel.line_from),lend=GetLineEndPosition(rect_sel.line_from); 
	// [pbeg,pend) son las pos actuales donde empieza y termina la zona de edicion
	int pbeg=lbeg+rect_sel.offset_beg, pend=lend-rect_sel.offset_end;
	// si se salio de la zona de edicion en esa linea, termina la edicion
	if (cur<pbeg || cur>pend) { 
		main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
		rect_sel.is_on=false; return;
	}
	// new_str es el nuevo contenido en esa linea, ref_str el viejo, ambos para toda la zona de edicion
	wxString new_str = GetTextRange(pbeg,pend);
	wxString &ref_str = rect_sel.ref_str;
	// lr y ln son los largos de ambos contenidos
	int i=0, lr=ref_str.Len(), ln=new_str.Len(); 
	// acotar la parte modificada, avanzando desde afuera hacia adentro mientras no haya cambio
	while (i<lr && i<ln && ref_str[i]==new_str[i]) { i++; }
	while (lr>i && ln>i && ref_str[lr-1]==new_str[ln-1]) { lr--; ln--; }
	// si no cambio nada, no hacer nada
	if (i==lr && lr==ln) return;
	// cortar las partes que son diferentes de cada cadena de ref
	wxString sfrom = ref_str.Mid(i,lr-i), sto = new_str.Mid(i,ln-i);
	UndoActionGuard undo_action(this);
	// ahora pbeg y pend acotan solo la parte modificada, en terminos de la cadena original
	pbeg += i; /*pend = pbeg+sfrom.Len();*/
	// traducir a columnas (por los tabs y otros caracteres que ocupan mas de un espacio)
	int cbeg = GetColumn(pbeg), cend=cbeg+sfrom.Len();
	// para cada linea de la seleccion...
	for(int line=rect_sel.line_from+1;line<=rect_sel.line_to;line++) { 
		// obtenes posiciones para la linea actual, segun columnas
		int tbeg = FindColumn(line,cbeg), tend = FindColumn(line,cend);
		// si realmente habia un seleccion rectangular, la edicion ya borro lo seleccionado, por eso contraer a 0 el area original
		if (rect_sel.was_rect_select) tend=tbeg;
		// asegurarse de que no se salga de la linea (puede pasar esto?)
		if (tend>GetLineEndPosition(line)) continue;
		// reemplazar desde tbeg a tend, con sto
		SetTargetStart(tbeg);
		SetTargetEnd(tend);
		ReplaceTarget(sto);
	}
	// la selección ya no será rectangular
	rect_sel.was_rect_select=false;
	// guardar la linea modificada como nueva referencia para la próxima edición
	ref_str=new_str;
	
}

void mxSource::OnClickUp(wxMouseEvent & evt) {
	evt.Skip();
	if (evt.AltDown() && SelectionIsRectangle()) InitRectEdit(true);
}

void mxSource::OnEditRectangularEdition (wxCommandEvent & evt) {
	InitRectEdit(false);
	if (rect_sel.is_on) main_window->SetStatusText(LANG(MAINW_PRESS_ESC_TO_FINISH_RECT_EDIT,"Presione ESC o mueva el cursor de texto a otra linea para volver al modo de edición normal."));
}

void mxSource::HideInspection ( ) {
	if (!inspection_baloon) return;
	inspection_baloon->Destroy();
	inspection_baloon = nullptr;
}

void mxSource::ShowInspection (const wxPoint &pos, const wxString &exp, const wxString &val) {
	focus_helper.Mask();
	SetCalltipMode(MXS_INSPECTION);
	wxPoint p2(pos.x-5, pos.y-5); // para que el mouse quede dentro del inspection_baloon
	inspection_baloon = new mxInspectionBaloon(p2,exp,val);
}

void mxSource::OnMouseWheel (wxMouseEvent & event) {
	if (event.ControlDown()) {
		if (event.m_wheelRotation>0) {
			ZoomIn();
		} else {
			ZoomOut();
		}
	} else
		event.Skip();
}
