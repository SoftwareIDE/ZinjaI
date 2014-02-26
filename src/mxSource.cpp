#include <wx/clipbrd.h>
#include <wx/encconv.h>
#include <iostream>
#include <wx/file.h>
#include "mxGCovSideBar.h"
using namespace std;
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

#define STYLE_IS_CONSTANT(s) (s==wxSTC_C_STRING || s==wxSTC_C_STRINGEOL || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRING || s==wxSTC_C_REGEX || s==wxSTC_C_NUMBER)
#define STYLE_IS_COMMENT(s) (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)

const wxChar* mxSourceWords1 =
	_T("and asm auto break case catch class const const_cast ")
	_T("continue default delete do dynamic_cast else enum explicit ")
	_T("export extern false for friend if goto inline ")
	_T("mutable namespace new not operator or private protected public ")
	_T("reinterpret_cast return sizeof static_cast ")
	_T("struct switch template this throw true try typedef typeid ")
	_T("typename union using virtual while xor ")
	_T("auto constexp decltype static_assert final override noexcept nullptr"); // c++ 2011
const wxChar* mxSourceWords2 =
	_T("bool char const double float int long mutable register ")
	_T("short signed static unsigned void volatile wchar_t");
const wxChar* mxSourceWords3 =
	_T("a addindex addtogroup anchor arg attention author b brief bug c ")
	_T("class code date def defgroup deprecated dontinclude e em endcode ")
	_T("endhtmlonly endif endlatexonly endlink endverbatim enum example ")
	_T("exception f$ f[ f] file fn hideinitializer htmlinclude ")
	_T("htmlonly if image include ingroup internal invariant interface ")
	_T("latexonly li line link mainpage n name namespace nosubgrouping note ")
	_T("overload p page par param post pre ref relates remarks return ")
	_T("retval sa section see showinitializer since skip skipline struct ")
	_T("subsection test throw todo typedef union until var verbatim ")
	_T("verbinclude version warning weakgroup $ @ \"\" & < > # { }");

BEGIN_EVENT_TABLE (mxSource, wxStyledTextCtrl)
	// edit
	EVT_MENU (wxID_CLEAR, mxSource::OnEditClear)
	EVT_MENU (wxID_CUT, mxSource::OnEditCut)
	EVT_MENU (wxID_COPY, mxSource::OnEditCopy)
	EVT_MENU (wxID_PASTE, mxSource::OnEditPaste)
	EVT_MENU (wxID_SELECTALL, mxSource::OnEditSelectAll)
	EVT_MENU (wxID_REDO, mxSource::OnEditRedo)
	EVT_MENU (wxID_UNDO, mxSource::OnEditUndo)
	EVT_MENU (mxID_EDIT_DUPLICATE_LINES, mxSource::OnEditDuplicateLines)
	EVT_MENU (mxID_EDIT_DELETE_LINES, mxSource::OnEditDeleteLines)
	EVT_MENU (mxID_EDIT_MARK_LINES, mxSource::OnEditMarkLines)
	EVT_MENU (mxID_EDIT_GOTO_MARK, mxSource::OnEditGotoMark)
	EVT_MENU (mxID_EDIT_FORCE_AUTOCOMPLETE, mxSource::OnEditForceAutoComplete)
	EVT_MENU (mxID_EDIT_AUTOCODE_AUTOCOMPLETE, mxSource::OnEditAutoCompleteAutocode)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_UP, mxSource::OnEditToggleLinesUp)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_DOWN, mxSource::OnEditToggleLinesDown)
	EVT_MENU (mxID_EDIT_COMMENT, mxSource::OnComment)
	EVT_MENU (mxID_EDIT_UNCOMMENT, mxSource::OnUncomment)
	EVT_MENU (mxID_EDIT_BRACEMATCH, mxSource::OnBraceMatch)
	EVT_MENU (mxID_EDIT_INDENT, mxSource::OnIndentSelection)
	EVT_MENU (mxID_EDIT_HIGHLIGHT_WORD, mxSource::OnHighLightWord)
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
	EVT_RIGHT_DOWN(mxSource::OnPopupMenu)
	EVT_STC_ROMODIFYATTEMPT (wxID_ANY, mxSource::OnModifyOnRO)
	EVT_STC_DOUBLECLICK (wxID_ANY, mxSource::OnDoubleClick)
	
	EVT_STC_PAINTED(wxID_ANY, mxSource::OnPainted)
	
	EVT_KEY_DOWN(mxSource::OnKeyDown)
	EVT_STC_MACRORECORD(wxID_ANY,mxSource::OnMacroAction)
	
END_EVENT_TABLE()

mxSource::mxSource (wxWindow *parent, wxString ptext, project_file_item *fitem) : wxStyledTextCtrl (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL) {

	// LC_CTYPE and LANG env vars are altered in the launcher, so this is commented now
	// with this path, text shows ok, but files can be saved with utf8 encoding and then it's shown differently when oppened somewhere else with the same zinjai
//#ifndef __WIN32__
//	// esto evita problemas en los ubuntus en castellano donde al agregar acentos, ñs y esas cosas, se desfaza el cursor, o al borrar se borra mal
//	if (wxLocale::GetSystemEncoding()==wxFONTENCODING_UTF8) 
//		SetCodePage(wxSTC_CP_UTF8);
//#endif
	
	brace_1=-1; brace_2=-1;
	
	next_source_with_same_file=this;
	
	source_time_dont_ask=false;
	diff_brother=NULL;
	first_diff_info=last_diff_info=NULL;
	
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
	
	false_calltip=false;
	
	lexer=wxSTC_LEX_CPP;
	
	config_running=config->Running;
	
	source_filename = wxEmptyString;
	temp_filename = DIR_PLUS_FILE(config->temp_dir,_T("sin_titulo.cpp"));
	binary_filename = DIR_PLUS_FILE(config->temp_dir,_T("sin_titulo"))+_T(BINARY_EXTENSION);
	working_folder = wxFileName::GetHomeDir();

	sin_titulo = true;
	cpp_or_just_c = true;
	never_parsed = true;
	first_view = true;
//	current_line = 0; current_marker = -1;
	
	m_LineNrID = 0;
	m_DividerID = 1;
	m_FoldingID = 2;
	
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
//	SetMarginWidth (m_LineNrID, TextWidth (wxSTC_STYLE_LINENUMBER, _T(" XXX")));

	// set margin as unused
	SetMarginType (m_DividerID, wxSTC_MARGIN_SYMBOL);
	SetMarginWidth (m_DividerID, 16);
	SetMarginSensitive (m_DividerID, true);

	// folding
	SetMarginType (m_FoldingID, wxSTC_MARGIN_SYMBOL);
	SetMarginMask (m_FoldingID, wxSTC_MASK_FOLDERS);
	SetMarginWidth (m_FoldingID, 0);
	SetMarginSensitive (m_FoldingID, false);
	// folding enable
	if (config_source.foldEnable) {
		SetMarginWidth (m_FoldingID, true? 15: 0);
		SetMarginSensitive (m_FoldingID, 1);
		SetProperty (_T("fold"), true? _T("1"): _T("0"));
		SetProperty (_T("fold.comment"),
				true? _T("1"): _T("0"));
		SetProperty (_T("fold.compact"),
				true? _T("1"): _T("0"));
		SetProperty (_T("fold.preprocessor"),
				true? _T("1"): _T("0"));
		SetProperty (_T("fold.html.preprocessor"),
				true? _T("1"): _T("0"));
		SetFoldFlags (wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
				wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
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
	SetMouseDwellTime(1500); // mis tooltips bizarros (con showbaloon = calltip)
	
	if (debug->debugging && !config->Debug.allow_edition) SetReadOnlyMode(ROM_ADD_DEBUG);

	er_register_source(this);
	
}


mxSource::~mxSource () {
	
	bool only_view=next_source_with_same_file==this; // there can be more than one view of the same source
	
	if (diff_brother) diff_brother->SetDiffBrother(NULL); diff_brother=NULL;
	while (first_diff_info) delete first_diff_info;
	
	parser->UnregisterSource(this);
	debug->UnregisterSource(this);
	er_unregister_source(this);
	if (main_window) {
		if (compiler->last_compiled==this) compiler->last_compiled=NULL;
		if (compiler->last_runned==this) compiler->last_runned=NULL;
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
		m_extras->ChangeSource(NULL);
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
		if (min>max) { int aux=min; min=max; max=aux; aux=ss; ss=se; se=aux;}
		if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
		GotoPos(ss);
		BeginUndoAction();
		for (int i=min;i<=max;i++)
			LineDelete();
		if (LineFromPosition(ss)!=min) 
			GotoPos(GetLineEndPosition(min));
		else 
			GotoPos(ss);
		EndUndoAction();
	}
}

void mxSource::OnEditGotoMark (wxCommandEvent &event) {
	int cl=GetCurrentLine(), lc=GetLineCount();
	int i=cl+1;
	while (!(MarkerGet(i)&1<<mxSTC_MARK_USER) && i<lc)
		i++;
	if (i<lc) {
		GotoPos(GetLineIndentPosition(i));
		EnsureVisibleEnforcePolicy(GetCurrentLine());
		return;
	}
	i=0;
	while (!(MarkerGet(i)&1<<mxSTC_MARK_USER) && i<cl)
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
		if (MarkerGet(min)&1<<mxSTC_MARK_USER)
			MarkerDelete(min,mxSTC_MARK_USER);
		else
			MarkerAdd(min,mxSTC_MARK_USER);
	} else {
			if (min>max) { int aux=min; min=max; max=aux; }
			if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
			int mark=true;
			for (int i=min;i<=max;i++)
				if (MarkerGet(i)&1<<mxSTC_MARK_USER) {
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
	BeginUndoAction();
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
	EndUndoAction();
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

void mxSource::OnEditClear (wxCommandEvent &event) {
    if (GetReadOnly()) return;
    Clear ();
}

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
//	ReplaceSelection(_T(""));
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
	if (config_source.indentPaste && config_source.syntaxEnable && wxTheClipboard->Open()) {
		wxTextDataObject data;
		if (!wxTheClipboard->GetData(data)) {
			wxTheClipboard->Close();
			return;
		}
		ignore_char_added=true;
		wxString str = data.GetText();
		BeginUndoAction();
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
		wxTheClipboard->Close();
		EndUndoAction();
		// is that the problem?
		if (CallTipActive())
			CallTipCancel();
		else if (AutoCompActive())
			AutoCompCancel();
		
	} else {
		if (!CanPaste()) return;
		Paste ();
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
	
    if (event.GetMargin() == 2) { // margen del folding
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
			if (!debug->debugging || !debug->waiting) {
				if (!bpi) { // si no habia, lo crea
					bpi=new BreakPointInfo(this,l);
					if (debug->debugging) debug->SetBreakPoint(bpi);
					else bpi->SetStatus(BPS_UNKNOWN);
				}
				new mxBreakOptions(bpi); // muestra el dialogo de opciones del bp
			}
		
		// si hay que sacar el breakpoint
		} else if (bpi) { 
			if (debug->debugging) debug->DeleteBreakPoint(bpi); // debug se encarga de hacer el delete y eso se lleva el marker en el destructor
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
			if (debug->debugging) debug->SetLiveBreakPoint(bpi); // esta llamada cambia el estado del bpi y eso pone la marca en el margen
			else bpi->SetStatus(BPS_SETTED);
		}
		
	}
}

void mxSource::OnEditSelectLine (wxCommandEvent &event) {
	int lineStart = PositionFromLine (GetCurrentLine());
	int lineEnd = PositionFromLine (GetCurrentLine() + 1);
	SetSelection (lineStart, lineEnd);
}

void mxSource::OnEditSelectAll (wxCommandEvent &event) {
	SetSelection (0, GetTextLength ());
}


void mxSource::OnEditToggleLinesUp (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	if (min>0) {
		BeginUndoAction();
		wxString line = GetLine(min-1);
		if (max==GetLineCount()-1)
			AppendText(_T("\n"));
		SetTargetStart(PositionFromLine(max+1));
		SetTargetEnd(PositionFromLine(max+1));
		ReplaceTarget(line);
		SetTargetStart(PositionFromLine(min-1));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(_T(""));
		EndUndoAction();
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
		BeginUndoAction();
		wxString line = GetLine(max+1);
		SetTargetStart(GetLineEndPosition(max));
		SetTargetEnd(GetLineEndPosition(max+1));
		ReplaceTarget(_T(""));
		SetTargetStart(PositionFromLine(min));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(line);
		if (ss==-1) SetSelectionStart(PositionFromLine(min+1));
		if (se==-1) SetSelectionStart(PositionFromLine(min+1));
		EndUndoAction();
		EnsureVisibleEnforcePolicy(max);
	}	
}

void mxSource::OnComment (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min==max && se!=ss) {
		ReplaceSelection(wxString(_T("/*"))<<GetSelectedText()<<_T("*/"));
		SetSelection(ss,se+4);
		return;
	}
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	BeginUndoAction();
	for (int i=min;i<=max;i++) {
		//if (GetLine(i).Left(2)!="//") {
		SetTargetStart(PositionFromLine(i));
		SetTargetEnd(PositionFromLine(i));
		ReplaceTarget(_T("//"));
	}	
	EndUndoAction();
}

void mxSource::OnUncomment (wxCommandEvent &event) {
	int ss = GetSelectionStart();
	int min=LineFromPosition(ss);
	
	if (GetStyleAt(ss)==wxSTC_C_COMMENT && GetLine(min).Left((GetLineIndentPosition(min))-PositionFromLine(min)+2).Right(2)!=_T("//")) {
		BeginUndoAction();
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
			ReplaceTarget(_T(""));
		}
		if (GetCharAt(ss)=='/' && GetCharAt(ss+1)=='*') {
			SetTargetStart(ss);
			SetTargetEnd(ss+2);
			ReplaceTarget(_T(""));
		}
		SetSelection(ss,se-3);
		EndUndoAction();
		return;
	}
	
	int max=LineFromPosition(GetSelectionEnd());
	int aux;
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	BeginUndoAction();
	for (int i=min;i<=max;i++) {
		if (GetLine(i).Left(2)==_T("//")) {
			SetTargetStart(PositionFromLine(i));
			SetTargetEnd(PositionFromLine(i)+2);
			ReplaceTarget(_T(""));
		} else if (GetLine(i).Left((aux=GetLineIndentPosition(i))-PositionFromLine(i)+2).Right(2)==_T("//")) {
			SetTargetStart(aux);
			SetTargetEnd(aux+2);
			ReplaceTarget(_T(""));
		}
	}
	EndUndoAction();
}


bool mxSource::LoadFile (const wxFileName &filename) {
	wxString ext = filename.GetExt().MakeUpper();
	if (ext==_T("C") || ext==_T("CPP") || ext==_T("CXX") || ext==_T("C++") || ext==_T("H") || ext==_T("HPP") || ext==_T("HXX") || ext==_T("H++")) {
		SetStyle(wxSTC_LEX_CPP);
	} else if (ext==_T("HTM") || ext==_T("HTML")) {
		SetStyle(wxSTC_LEX_HTML);
	} else if (ext==_T("SH")) {
		SetStyle(wxSTC_LEX_BASH);
	} else if (ext==_T("XML")) {
		SetStyle(wxSTC_LEX_XML);
	} else if (filename.GetName().MakeUpper()==_T("MAKEFILE")) {
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
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!=_T(""))
		AppendText(_T("\n"));
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
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!=_T(""))
		AppendText(_T("\n"));
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
	if (lexer==wxSTC_LEX_CPP && config_source.avoidNoNewLineWarning && GetLine(GetLineCount()-1)!=_T(""))
		AppendText(_T("\n"));
	if (MySaveFile(filename.GetFullPath())) {
		source_filename = filename;
		working_folder = filename.GetPath();
		cpp_or_just_c = source_filename.GetExt().Lower()!="c";
		if (project)
			binary_filename=source_filename.GetPathWithSep()+source_filename.GetName()+_T(".o");
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
	int cl=GetCurrentLine();
	if (first_view) {
		ScrollToColumn(0);
		first_view=false;
//		current_marker=MarkerAdd(current_line=cl,mxSTC_MARK_CURRENT);
	} else {
		if (false_calltip) {
			CallTipCancel();
			false_calltip=false;
		}
//		if (cl!=current_line) {
//	//		MarkerDelete(current_line,mxSTC_MARK_CURRENT);
//	//		MarkerAdd(current_line=cl,mxSTC_MARK_CURRENT);
//			MarkerDeleteHandle(current_marker);
//			current_marker=MarkerAdd(current_line=cl,mxSTC_MARK_CURRENT);
//		}
	}
	int p=GetCurrentPos();
	if (!config_source.lineNumber)
		main_window->status_bar->SetStatusText(wxString("Lin ")<<cl<<" - Col "<<p-PositionFromLine(cl),1);
//	char c;
//	if ((c=GetCharAt(p))=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']') {
//		int m=BraceMatch(p);
//		if (m!=wxSTC_INVALID_POSITION)
//			BraceHighlight (p,m);
//		else
//			BraceBadLight (p);
//	} else if ((c=GetCharAt(p-1))=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']') {
//		int m=BraceMatch(p-1);
//		if (m!=wxSTC_INVALID_POSITION)
//			BraceHighlight (p-1,m);
//		else
//			BraceHighlight (wxSTC_INVALID_POSITION,wxSTC_INVALID_POSITION);
//	} else
//		BraceHighlight (wxSTC_INVALID_POSITION,wxSTC_INVALID_POSITION);
//	event.Skip();
	
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
}


void mxSource::OnCharAdded (wxStyledTextEvent &event) {
	if (ignore_char_added) return;
	char chr = (char)event.GetKey();
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
			if (p+4==e && GetTextRange(p,e)==_T("else")) {
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
			if ( (p+7==e && GetTextRange(p,e)==_T("public:")) ||
				(p+8==e && GetTextRange(p,e)==_T("private:")) ||
				(p+10==e && GetTextRange(p,e)==_T("protected:")) ||
				(p+8==e && GetTextRange(p,e)==_T("default:")) ||
				(p+6<e && GetTextRange(p,p+5)==_T("case ") && (s=GetStyleAt(e))!=wxSTC_C_CHARACTER && s!=wxSTC_C_STRING && s!=wxSTC_C_STRINGEOL) 
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
							InsertText(PositionFromLine(cl),_T("\n"));
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
					AppendText(_T("\n"));
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
								InsertText(PositionFromLine(cl+1),_T("}\n"));
								SetLineIndentation(cl+1,GetLineIndentation(cl-1));
								// sacar la llave si ya estaba al final de la linea, porque la acabamos de agregar abajo
								int pf = GetLineEndPosition(cl)-1; int opf=pf+1;
								II_BACK(pf,II_SHOULD_IGNORE(pf)||II_IS_2(pf,' ','\t'));
								if (GetCharAt(pf)=='}') {
									pf--;
									II_BACK(pf,II_SHOULD_IGNORE(pf)||II_IS_2(pf,' ','\t'));
									SetTargetStart(pf+1);
									SetTargetEnd(opf);
									ReplaceTarget(_T(""));
								}
							}
							SetLineIndentation(cl,GetLineIndentation(cl-1)+config_source.tabWidth);
							wxStyledTextCtrl::GotoPos(GetLineIndentPosition(cl));
							return;
						}
					}
				} else if (e==l || GetLineIndentation(LineFromPosition(e))<GetLineIndentation(cl-1) || ( c!='}' && GetLineIndentation(LineFromPosition(e))==GetLineIndentation(cl-1)  && GetTextRange(e,e+7)!=_T("public:") && GetTextRange(e,e+8)!=_T("private:") && GetTextRange(e,e+10)!=_T("protected:") && GetTextRange(e,e+5)!=_T("case ") ) ) {
					e=GetLineIndentPosition(cl-1);
					if (GetTextRange(e,e+8)==_T("template") && GetStyleAt(e)==wxSTC_C_WORD) {
						e+=8;
						II_FRONT(e,II_IS_2(e,' ','\t') || II_SHOULD_IGNORE(e));
						if (c=='<' && (p=BraceMatch(e))!=wxSTC_INVALID_POSITION) {
							e=p+1;
							II_FRONT(e,II_IS_2(e,' ','\t') || II_SHOULD_IGNORE(e));
						}
					}
					if ( (GetTextRange(e,e+5)==_T("class") || GetTextRange(e,e+6)==_T("struct")) && GetStyleAt(e)==wxSTC_C_WORD)
						InsertText(PositionFromLine(cl),_T("\n};"));
					else
						InsertText(PositionFromLine(cl),_T("\n}"));
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
					if (e>8 && GetStyleAt(e)==wxSTC_C_WORD && c=='e' && GetTextRange(e-7,e)==_T("templat")) {
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
				if (s==wxSTC_C_WORD && ( (c=='p' && (GetTextRange(l+1,l+6)==_T("ublic") || GetTextRange(l+1,l+7)==_T("rivate") || GetTextRange(l+1,l+9)==_T("rotected")) ) || (c=='c' && GetTextRange(l+1,l+4)==_T("ase")) || (c=='d' && GetTextRange(l+1,l+7)==_T("efault")) ) ) 
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
	if (CallTipActive() && chr==')') {
		int p=GetCurrentPos()-1;
		Colourise(p,p+1);
		if (BraceMatch(p)==calltip_brace)
			CallTipCancel();
	} else if ( ((!AutoCompActive() || chr=='.') && config_source.autoCompletion) || ((chr==',' || chr=='(') && config_source.callTips) ) {
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
					code_helper->AutocompleteScope(this,key,_T(""),false,false);
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
					code_helper->AutocompleteScope(this,type,_T(""),true,false);
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
					code_helper->AutocompleteScope(this,type,_T(""),true,false);
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
						if (calltip_brace==p && CallTipActive())
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
			int p=WordStartPosition(e,true);
			wxString key = GetTextRange(p,e+1);
			if (e-p+1>=config->Help.min_len_for_completion || chr==',' || chr=='(') {
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
							code_helper->ShowCalltip(ctp,this,type,key);
						else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
							code_helper->AutocompleteScope(this,type,key,true,false);
					}
				} else if (c=='>' && GetCharAt(p-1)=='-') {
					p--;
					wxString type = FindTypeOf(p-1,dims);
					if (dims==1) {
						if (chr=='(' && config_source.callTips)
							code_helper->ShowCalltip(ctp,this,type,key);
						else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
							code_helper->AutocompleteScope(this,type,key,true,false);
					}
				} else if (c==':' && GetCharAt(p-1)==':') {
					p-=2;
					II_BACK(p,II_IS_NOTHING_4(p));
					wxString type = GetTextRange(WordStartPosition(p,true),p+1);
					if (chr=='(' && config_source.callTips)
						code_helper->ShowCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				} else {
					if (chr=='(' && config_source.callTips) {
						if (!code_helper->ShowCalltip(ctp,this,FindScope(GetCurrentPos()),key,false)) {
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
							code_helper->ShowConstructorCalltip(ctp,this,GetTextRange(p1,p));
						}
					} else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteGeneral(this,FindScope(GetCurrentPos()),key);
				}
			}
		}
	}
}

void mxSource::SetStyle(int idx, const wxChar *fontName, int fontSize, const wxColour &foreground, const wxColour &background, int fontStyle){
	wxFont font (fontSize, wxMODERN, wxNORMAL, wxNORMAL, false, fontName);
	StyleSetFont (idx, font);
	StyleSetForeground (idx, foreground);
	StyleSetBackground (idx, background);
	StyleSetBold (idx, (fontStyle & mxSOURCE_BOLD) > 0);
	StyleSetItalic (idx, (fontStyle & mxSOURCE_ITALIC) > 0);
	StyleSetUnderline (idx, (fontStyle & mxSOURCE_UNDERL) > 0);
	StyleSetVisible (idx, (fontStyle & mxSOURCE_HIDDEN) == 0);
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
		ReplaceTarget(_T(" "));
		SetTargetStart(p); 
		SetTargetEnd(p+1);
		ReplaceTarget(_T(""));
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
	main_window->SetStatusText(wxString(_T("Indentando...")));
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
	config_source.autoCompletion=false;
	bool old_autoclose=config_source.autocloseStuff;
	config_source.autocloseStuff=false; 
	bool old_autotext=config_source.autotextEnabled;
	config_source.autotextEnabled=false; 
	BeginUndoAction();
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
		} else if (c=='p' && GetTextRange(p,p+8)==_T("private:") && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+8);
			OnCharAdded(evt);
		} else if (c=='p' && GetTextRange(p,p+7)==_T("public:") && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+7);
			OnCharAdded(evt);
		} else if (c=='p' && GetTextRange(p,p+10)==_T("protected:") && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+10);
			OnCharAdded(evt);
		} else if (c=='d' && GetTextRange(p,p+8)==_T("default:") && !II_SHOULD_IGNORE(p)) {
			evt.SetKey(':');
			SetCurrentPos(p+8);
			OnCharAdded(evt);
		} else if (c=='c' && GetTextRange(p,p+5)==_T("case ") && !II_SHOULD_IGNORE(p)) {
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
	EndUndoAction();
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

#define AUXSetStyle(who,name) SetStyle(wxSTC_##who##_##name,_T(""),config->Styles.font_size,ctheme->name##_FORE,ctheme->name##_BACK,(ctheme->name##_BOLD?mxSOURCE_BOLD:0)|(ctheme->name##_ITALIC?mxSOURCE_ITALIC:0)); // default
#define AUXSetStyle3(who,name,real) SetStyle(wxSTC_##who##_##name,_T(""),config->Styles.font_size,ctheme->real##_FORE,ctheme->real##_BACK,(ctheme->real##_BOLD?mxSOURCE_BOLD:0)|(ctheme->real##_ITALIC?mxSOURCE_ITALIC:0)); // default
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
			config_source.callTips=config_source.autoCompletion=config_source.smartIndent=config_source.indentPaste=false;
			SetProperty (_T("fold.html"),_T("0"));
			SetProperty (_T("fold.html.preprocessor"), _T("0"));
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
			config_source.callTips=config_source.autoCompletion=config_source.smartIndent=config_source.indentPaste=false;
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
			config_source.callTips=config_source.autoCompletion=config_source.smartIndent=config_source.indentPaste=false;
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
	BeginUndoAction();

	bool using_namespace_std_present=false;
	bool header_present=false;
	
	int lp = GetCurrentPos() , cl = GetCurrentLine();
	wxString oHeader=header;
#if defined(_WIN32) || defined(__WIN32__)
	header=header.MakeLower();
#endif
	wxString str;
	int s,p,p1,p2,lta=0, flag=true, comment;
	int uncomment_line=0;
	for (int i=0;i<cl;i++) {
		p=GetLineIndentPosition(i);
		int le=GetLineEndPosition(i);
		if (GetTextRange(p,p+8)==_T("#include") || GetTextRange(p,p+10)==_T("//#include") || GetTextRange(p,p+11)==_T("// #include")) {
			if ( ( comment = (GetTextRange(p,p+2)==_T("//")) ) ) {
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
#if defined(_WIN32) || defined(__WIN32__)
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
		} else if (GetTextRange(p,p+5)==_T("using")) {
			if (flag) {
				lta=i;
				flag=false;
			}
			p+=5;
			while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
				p++;
			if (GetTextRange(p,p+9)==_T("namespace")) {
				p+=9;
				while (GetCharAt(p)==' ' || GetCharAt(p)=='\t')
					p++;
				if (GetTextRange(p,p+3)==_T("std") && (GetCharAt(p+3)==';' || GetCharAt(p+3)==' ' || GetCharAt(p+3)=='\t' || GetCharAt(p+3)=='/'))
					using_namespace_std_present=true;
			}
		} else if (!II_SHOULD_IGNORE(p)/* && GetTextRange(p,p+7)!=_T("#define")*/)
			flag=false;
	}
	if (!header_present) {
		if (uncomment_line) {
			p = PositionFromLine(uncomment_line-1);
			SetTargetStart(p);
			while (GetCharAt(p)!='#')
				p++;
			SetTargetEnd(p);
			ReplaceTarget(_T(""));
		} else {
			p = PositionFromLine(lta);
			wxString line = wxString(_T("#include "))+oHeader+_T("\n");
			if (p<=lp)
				lp+=line.Len();
			InsertText(p,line);
			if (config_source.syntaxEnable)
				Colourise(p,p+line.Len());
			GotoPos(lp);
			lta++;
		}
	}
	if (!using_namespace_std_present && header.Find(_T("."))==wxNOT_FOUND && header.Last()!='\"') {
		p = PositionFromLine(lta);
		wxString line = wxString(_T("using namespace std;\n"));
		if (p<=lp)
			lp+=line.Len();
		InsertText(p,line);
		if (config_source.syntaxEnable)
			Colourise(p,p+line.Len());
		GotoPos(lp);
	}
	
	EndUndoAction();
	
	if (!header_present || (!using_namespace_std_present && header.Last()!='\"' && header.Right(3)!=_(".h>"))) {
		int lse = GetEndStyled();
		StartStyling(0,wxSTC_INDICS_MASK);
		SetStyling(GetLength(),0);
		StartStyling(lse,0x1F);
		if (!header_present) {
			if (uncomment_line)
				ShowBaloon(LANG2(SOURCE_UNCOMMENTED_FOR_HEADER,"Descomentada linea <{1}>: \"#include <{2}>\".",wxString()<<uncomment_line,oHeader));
			else
				ShowBaloon(LANG1(SOURCE_ADDED_HEADER,"Cabecera agregadad: <{1}>.",oHeader));
		} else {
			ShowBaloon(LANG(SOURCE_ADDED_USING_NAMESPACE_STD,"Agregado \"using namespace std;\""));
		}
		return true;
	} else {
		ShowBaloon(wxString(LANG(SOURCE_HEADER_WAS_ALREADY,"Sin cambios, ya estaba la cabecera "))<<oHeader);
		return false;
	}
	
}

void mxSource::OnPopupMenu(wxMouseEvent &evt) {
	
	int p1=GetSelectionStart();
	int p2=GetSelectionEnd();
	if (p1==p2) {
		int p = PositionFromPointClose(evt.GetX(),evt.GetY());
		if (p!=wxSTC_INVALID_POSITION)
			GotoPos(p);
	}

	wxMenu menu(_T(""));
	
	menu.Append(wxID_UNDO, wxString(LANG(SOURCE_POPUP_UNDO,"&Deshacer"))<<_T("\tCtrl+Z"));
	menu.Append(wxID_REDO, wxString(LANG(SOURCE_POPUP_REDO,"&Rehacer"))<<_T("\tCtrl+Shift+Z"));
	menu.AppendSeparator();
	menu.Append(wxID_CUT, wxString(LANG(SOURCE_POPUP_CUT,"C&ortar"))<<_T("\tCtrl+X"));
	menu.Append(wxID_COPY, wxString(LANG(SOURCE_POPUP_COPY,"&Copiar"))<<_T("\tCtrl+C"));
	menu.Append(wxID_PASTE, wxString(LANG(SOURCE_POPUP_PASTE,"&Pegar"))<<_T("\tCtrl+V"));
	menu.Append(mxID_EDIT_DUPLICATE_LINES, wxString(LANG(SOURCE_POPUP_DUPLICATE_LINES,"&Duplicar Linea(s)"))<<_T("\tCtrl+L"));
	menu.Append(mxID_EDIT_DELETE_LINES, wxString(LANG(SOURCE_POPUP_DELETE_LINES,"&Eliminar Linea(s)"))<<_T("\tShift+Ctrl+L"));
	menu.AppendSeparator();
	menu.Append(mxID_EDIT_COMMENT, wxString(LANG(SOURCE_POPUP_COMMENT_LINES,"&Comentar"))<<_T("\tCtrl+D"));
	menu.Append(mxID_EDIT_UNCOMMENT, wxString(LANG(SOURCE_POPUP_UNCOMMENT_LINES,"&Descomentar"))<<_T("\tShift+Ctrl+D"));
	menu.Append(mxID_EDIT_INDENT, wxString(LANG(SOURCE_POPUP_INDENTE,"&Indentar Blo&que"))<<_T("\tCtrl+I"));
	menu.Append(mxID_EDIT_BRACEMATCH, wxString(LANG(SOURCE_POPUP_SELECT_BLOCK,"&Seleccionar Bloque"))<<_T("\tCtrl+M"));
	menu.Append(wxID_SELECTALL, wxString(LANG(SOURCE_POPUP_SELECT_ALL,"Seleccionar &Todo"))<<_T("\tCtrl+A"));
	menu.Append(mxID_EDIT_MARK_LINES, wxString(LANG(SOURCE_POPUP_HIGHLIGHT_LINES,"&Resaltar Linea(s)"))<<_T("\tCtrl+B"));
	menu.AppendSeparator();
	

	int pos=GetCurrentPos();
	int s=WordStartPosition(pos,true);
	int e=WordEndPosition(pos,true);
	wxString key = GetTextRange(s,e);
	if (key.Len()!=0) {
		menu.Append(mxID_EDIT_GOTO_FUNCTION, wxString(LANG(SOURCE_POPUP_FIND_SYMBOL,"&Buscar en el Arbol de Simbolos..."))<<_T("\tCtrl+Shift+G"));
		if (GetCharAt(s-1)=='#')
			key = GetTextRange(s-1,e);
		menu.Append(mxID_HELP_CODE, LANG1(SOURCE_POPUP_HELP_ON,"Ayuda sobre \"<{1}>\"...",key)<<_T("\tShift+F1"));
//		if ((s=GetStyleAt(s))!=wxSTC_C_PREPROCESSOR && !STYLE_IS_COMMENT(s) && !STYLE_IS_CONSTANT(s) && s!=wxSTC_C_OPERATOR && s!=wxSTC_C_WORD && s!=wxSTC_C_WORD2) {
		if (lexer==wxSTC_LEX_CPP && GetStyleAt(s)==wxSTC_C_IDENTIFIER) {
			menu.Append(mxID_EDIT_INSERT_HEADER, LANG1(SOURCE_POPUP_INSERT_INCLUDE,"Insertar #incl&ude correspondiente a \"<{1}>\"",key)<<_T("\tCtrl+H"));
			menu.Append(mxID_EDIT_HIGHLIGHT_WORD, wxString(LANG(SOURCE_POPUP_HIGHLIGHT_WORD,"Resaltar identificador \""))<<key<<"\"");
		}
	}
	
	
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
		menu.Append(mxID_FILE_OPEN_SELECTED, LANG1(SOURCE_POPUP_OPEN_SELECTED,"&Abrir \"<{1}>\"",GetTextRange(p1,p2))<<"\tCtrl+Enter");
	
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
	
	wxString ret=_T(""),space=_T("");
	
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
		int dims=WordStartPosition(e,true);
		wxString space = GetTextRange(dims,e+1);
		wxString type = FindTypeOf(space,dims);
		if (dims==1) {
			pos=-1;
			type = code_helper->GetAttribType(type,key,pos);
			key = space;
			return type;
		}
	} else if (c==':' && e!=0 && GetCharAt(e-1)==':') {
		e-=2;
		II_BACK(e,II_IS_NOTHING_4(e));
		int dims=WordStartPosition(e,true);
		wxString type = GetTextRange(dims,e+1);
		if (dims==0) {
			dims=-1;
			type = code_helper->GetAttribType(type,key,dims);
			key = _T("");
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
						while  ( p>0 &&  ( (c>='a' && c<='z') || (c>='0' && c<='9') || (c>='A' && c<='Z') || c=='_' || II_SHOULD_IGNORE(p) ) ) {
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
		
//		p_llave_a = FindText(p_from-1,0,_T("{"));
//		while (p_llave_a!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_a))
//			p_llave_a = FindText(p_llave_c-1,0,_T("}"));
		int p_llave_c = FindText(p_from-1,0,_T("}"));
		while (p_llave_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_c))
			p_llave_c = FindText(p_llave_c-1,0,_T("}"));
		int p_par_c = FindText(p_from-1,0,_T(")"));
		while (p_par_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_par_c))
			p_par_c = FindText(p_par_c-1,0,_T(")"));

		
//		if (notSpace && p_llave_a!=wxSTC_INVALID_POSITION && (p_llave_c==wxSTC_INVALID_POSITION || p_llave_a>p_llave_c) ) { // ver si es una declaracion de clase para encontrar el scope
//			p = p_llave_a-1;
//			II_BACK(p,II_IS_NOTHING_4(p));
//			p_llave_a=p;
//			if ( !II_SHOULD_IGNORE(p) && II_IS_KEYWORD_CHAR(c) ) {
//				p--;
//				II_BACK (p, !II_SHOULD_IGNORE(p) && II_IS_KEYWORD_CHAR(c));
//				bool cont=true;
////				while (cont) {
//					if (c=='s' && p>=5 && GetTextRange(p-5,p)==_T("clas") ) {
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
			} while (GetTextRange(p1,p2)==_T("const") || GetTextRange(p1,p2)==_T("volatile") || GetTextRange(p1,p2)==_T("static") || GetTextRange(p1,p2)==_T("extern"));
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
		while ( !(ret!=_T("unsigned") && ret!=_T("const") && ret!=_T("signed") && ret!=_T("extern") && ret!=_T("volatile") && ret!=_T("long") && ret!=_T("static") && (ret!=_T("public") || GetCharAt(p2)!=':') && (ret!=_T("protected") || GetCharAt(p2)!=':') && (ret!=_T("private") || GetCharAt(p2)!=':') ) ) {
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
		if (key==_T("this")) {
			if ( (ret=space).Len()!=0) 
				dims=1;
		} else {
			wxString ans;
			if ( (space!=_T("") && (ans=code_helper->GetAttribType(space,key,pos))!=_T("")) || (ans=code_helper->GetGlobalType(key,pos))!=_T("") )
				ret=ans;
			else
				pos=SRC_PARSING_ERROR;
		}
	}
	key=space;
	return code_helper->UnMacro(ret,pos);
}

void mxSource::ShowBaloon(wxString text, int pos) {
	if (CallTipActive())
		CallTipCancel();
	else if (AutoCompActive())
		AutoCompCancel();
	ShowCallTip(pos==-1?GetCurrentPos():pos,text,false);
DEBUG_INFO("wxYield:in  mxSource::ShowBaloon");
	wxYield();
DEBUG_INFO("wxYield:out mxSource::ShowBaloon");
	false_calltip=true;
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
				return _T("La cantidad de corchetes no concuerda.");
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
			return _T("Error: Se cerraron parentesis demas.");
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
			if (p<p_fin && (c<'0'||c>'9')) { // si puede ser un nombre de funcion
				wxString func_name=GetTextRange(p,p_fin);
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
		if (c=='*')
			dims--;
		else if (c=='&')
			dims++;
		p++;
	}
	if (c=='(') {
		p=BraceMatch(p);
		if (p==wxSTC_INVALID_POSITION) {
			dims=SRC_PARSING_ERROR;
			return _T("");
		}
		wxString ans = FindTypeOf(p,dims,false);
		return ans;
	} else {
		if ( ( (c|32)>='a' && (c|32)<='z' ) || c=='_' ) {
			s=p;
			wxString key=GetTextRange(p,WordEndPosition(p,true));
			wxString ans=FindTypeOf(key,s);
			if (s==SRC_PARSING_ERROR) {
				dims=SRC_PARSING_ERROR;
				return _T("");
			}
			dims+=s;
			return ans;
		}
	}
	dims=SRC_PARSING_ERROR;
	return _T("");
}

wxString mxSource::FindScope(int pos) {
	int l=pos,s;
	char c;
	while (true) {
		int p_llave_a = FindText(pos,0,_T("{"));
		while (p_llave_a!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_a))
			p_llave_a = FindText(p_llave_a-1,0,_T("{"));
		int p_llave_c = FindText(pos,0,_T("}"));
		while (p_llave_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_c))
			p_llave_c = FindText(p_llave_c-1,0,_T("}"));
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
				p-=5; II_BACK(p,II_IS_NOTHING_4(p));
			}
			if (c==')') { // puede ser funcion
				p=BraceMatch(p);
				if (p!=wxSTC_INVALID_POSITION) {
					p--;
					II_BACK(p,II_IS_NOTHING_4(p));
					p=WordStartPosition(p,true)-1;
					II_BACK(p,II_IS_NOTHING_4(p));
					// el "GetCharAt(p)==','" se agrego el 29/09 para los constructores en constructores
					if (GetCharAt(p)==':' || GetCharAt(p)==',' || (p && GetCharAt(p)=='~' && GetCharAt(p-1)==':')) {
						if (GetCharAt(p)=='~') p--; // agregado para arreglar el scope de un destructor
						if (GetCharAt(p-1)==':') {
							p-=2;
							II_BACK(p,II_IS_NOTHING_4(p));
							return code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1));
						} else { // puede ser constructor
							p = FindText(p,0,_T("::"));
							if (p!=wxSTC_INVALID_POSITION) {
								int e=p+2;
								p--;
								II_BACK(p,II_IS_NOTHING_4(p));
								II_FRONT_NC(e,II_IS_NOTHING_4(e));
								if (GetTextRange(WordStartPosition(p,true),p+1)==GetTextRange(e,WordEndPosition(e,true))) {
									return code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1));
								}
							}
						}
					}
				}
			} else { // puede ser clase o struct
				II_BACK(p,II_IS_NOTHING_4(p) || !II_IS_5(p,'{','}',';',')','('));
				p++;
				II_FRONT(p,II_IS_NOTHING_4(p));
				if (GetStyleAt(p)==wxSTC_C_WORD) {
					if (GetTextRange(p,p+6)==_T("struct")) {
						p+=6;
						II_FRONT(p,II_IS_NOTHING_4(p));
						return code_helper->UnMacro(GetTextRange(p,WordEndPosition(p,true)));
					} else if (GetTextRange(p,p+5)==_T("class")) {
						p+=5;
						II_FRONT(p,II_IS_NOTHING_4(p));
						return code_helper->UnMacro(GetTextRange(p,WordEndPosition(p,true)));
					} else if (GetTextRange(p,p+p)==_T("namespace")) {
						p+=9;
						II_FRONT(p,II_IS_NOTHING_4(p));
						return code_helper->UnMacro(GetTextRange(p,WordEndPosition(p,true)));
					}
				}
			}
		}
	}
	return _T("");
}

void mxSource::OnToolTipTimeOut (wxStyledTextEvent &event) {
	if (CallTipActive()) CallTipCancel();
}

void mxSource::OnToolTipTime (wxStyledTextEvent &event) {
//	if (!FindFocus() || main_window->notebook_sources->GetSelection()!=main_window->notebook_sources->GetPageIndex(this)) // no mostrar tooltips si no se tiene el foco
//	if (FindFocus()!=this) // no mostrar tooltips si no se tiene el foco
//		return;
	
	// no mostrar tooltips si no es la pestana del notebook seleccionada, o el foco no esta en esta ventana
	if (!main_window->IsActive() || main_window->focus_source!=this) return; 
	
	int p = event.GetPosition();
	if (p==-1) {
		int x=event.GetX(), y=event.GetY();
		int x0=GetMarginWidth(m_LineNrID);
		int x1=x0+GetMarginWidth(m_DividerID);
		if (x>=x0 && x<=x1) {
			p=PositionFromPointClose(x1+GetMarginWidth(m_FoldingID)+10,y);
			int l=LineFromPosition(p);
			BreakPointInfo *bpi=m_extras->FindBreakpointFromLine(this,l);
			if (bpi && bpi->annotation.Len()) CallTipShow(PositionFromLine(l),bpi->annotation);
		}
		return;
	}
	
	if (!debug->debugging) {
		if (!config_source.toolTips)
			return;
		int s;
		if (II_SHOULD_IGNORE(p)) 
			return;
		int e = WordEndPosition(p,true);
		s = WordStartPosition(p,true);
		if (s!=e) {
			wxString key = GetTextRange(s,e);
//			wxString bkey=key, type = FindTypeOf(key,s);
//			if ( s!=SRC_PARSING_ERROR && type.Len() ) {
//				while (s>0) {
//					type<<_("*");
//					s--;
//				}
//				ShowBaloon ( bkey +_T(": ")+type , p );
//			}
			
			// buscar en la funcion/metodo
			wxString bkey=key, type = FindTypeOf(e-1,s);
			if ( s!=SRC_PARSING_ERROR && type.Len() ) {
				while (s>0) {
					type<<_("*");
					s--;
				}
				ShowBaloon ( bkey +_T(": ")+type , p );
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
					ShowBaloon ( bkey +_T(": ")+type , p );
				}
			}
		}
	} else {
		int e = GetSelectionEnd();
		int s = GetSelectionStart();
		if (e==s || p<s || p>e) {
			e = WordEndPosition(p,true);
			s = WordStartPosition(p,true);
		}
		if (s!=e) {
			wxString key = GetTextRange(s,e);
			wxString ans = debug->InspectExpression(key,true);
			if (ans.Len())
				ShowBaloon ( key +_T(": ")+ ans , p );
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
	main_window->notebook_sources->SetPageText(main_window->notebook_sources->GetPageIndex(this),page_text+_T("*"));
}

void mxSource::OnKillFocus(wxFocusEvent &event) {
	if (CallTipActive())
		CallTipCancel();
	else if (AutoCompActive())
		AutoCompCancel();
	event.Skip();
}

void mxSource::OnSetFocus(wxFocusEvent &event) {
	ro_quejado=false;
	if (main_window) main_window->focus_source=this;
	event.Skip();
	if (!sin_titulo) CheckForExternalModifications();
}

void mxSource::OnEditAutoCompleteAutocode(wxCommandEvent &evt) {
	int p=GetCurrentPos();
	if (CallTipActive()) CallTipCancel();
	else if (AutoCompActive()) AutoCompCancel();
	int ws=WordStartPosition(p,true);
	code_helper->AutocompleteAutocode(this,GetTextRange(ws,p));
}


//#ifndef DEBUG
void mxSource::OnEditForceAutoComplete(wxCommandEvent &evt) {
	int p=GetCurrentPos();
	char chr = p>0?GetCharAt(p-1):' ';

	if (CallTipActive()) CallTipCancel();
	else if (AutoCompActive()) AutoCompCancel();
	
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
			if (!code_helper->AutocompleteScope(this,key,_T(""),false,false))
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
			if (!code_helper->AutocompleteScope(this,type,_T(""),true,false))
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
// esto era para tratar de autocompletar funciones, pero hay que replantear para que tome varias llamadas, no la primera (recursivo?)
//		if (GetCharAt(p)==')') { 
//			if ( (p=BraceMatch(p))==wxSTC_INVALID_POSITION )
//				ShowBaloon(LANG(SOURCE_UNDEFINED_SCOPE_AUTOCOMPLETION,"No se pudo determinar el ambito a autocompletar"));
//			else {
//				p--;
//				II_BACK(p,II_IS_NOTHING_4(p));
//				wxString key=GetTextRange(WordStartPosition(p,true),p+1);
//				code_helper->AutoCompleteFunction(FindScope(p),key,_T(""));
//			}
//		}
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
				if (!code_helper->AutocompleteScope(this,type,_T(""),true,false))
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
					if (calltip_brace==p && CallTipActive())
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
					if (chr=='(' && config_source.callTips)
						code_helper->ShowCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				}
			} else if (c=='>' && GetCharAt(p-1)=='-') {
				p--;
				wxString type = FindTypeOf(p-1,dims);
				if (dims==1) {
					if (chr=='(' && config_source.callTips)
						code_helper->ShowCalltip(ctp,this,type,key);
					else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
						code_helper->AutocompleteScope(this,type,key,true,false);
				}
			} else if (c==':' && GetCharAt(p-1)==':') {
				p-=2;
				II_BACK(p,II_IS_NOTHING_4(p));
				wxString type = GetTextRange(WordStartPosition(p,true),p+1);
				if (chr=='(' && config_source.callTips)
					code_helper->ShowCalltip(ctp,this,type,key);
				else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
					code_helper->AutocompleteScope(this,type,key,true,false);
			} else {
				if (chr=='(' && config_source.callTips) {
					if (!code_helper->ShowCalltip(ctp,this,FindScope(GetCurrentPos()),key,false)) {
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
						code_helper->ShowConstructorCalltip(ctp,this,GetTextRange(p1,p));
					}
				} else if ( ( (chr|32)>='a'&&(chr|32)<='z' ) || chr=='_' || (chr>='0'&&chr<='9') )
					code_helper->AutocompleteGeneral(this,FindScope(GetCurrentPos()),key);
			}
		} else {
			wxString scope = FindScope(GetCurrentPos());
			if (scope.Len())
				code_helper->AutocompleteScope(this,scope,_T(""),true,true);
		}
	}
	if (!CallTipActive() && !AutoCompActive())
		ShowBaloon(LANG(SOURCE_NO_ITEMS_FOR_AUTOCOMPLETION,"No se encontraron opciones para autocompletar"),p);
}

//#endif

void mxSource::ShowCallTip(int p, wxString str, bool fix_pos) {
	if (fix_pos) { // evitar que tape el cursor
		int cp = GetCurrentPos();
		int cl = LineFromPosition(cp);
		int l = LineFromPosition(p);
		if (l!=cl) {
			int dp = p-PositionFromLine(l);
			p = PositionFromLine(cl)+dp;
			if (LineFromPosition(p)!=cl) 
				p = GetLineEndPosition(cl);
		}
	}
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
							str = str.SubString(0,j)+_T("\n   ")+str.Mid(j+1);
							l+=4;
						}
					}
				}
			}
		}
	}
	CallTipShow(p,str);
}

DiffInfo *mxSource::MarkDiffs(int from, int to, MXS_MARKER marker, wxString extra) {
	if (mxSTC_MARK_DIFF_NONE==marker) {
		for (int i=from;i<=to;i++) {
			MarkerDelete(i,mxSTC_MARK_DIFF_ADD);	
			MarkerDelete(i,mxSTC_MARK_DIFF_DEL);	
			MarkerDelete(i,mxSTC_MARK_DIFF_CHANGE);	
		}
		while (first_diff_info) delete first_diff_info;
		return NULL;
	} else {
		int *handles = new int[to-from+1];
		for (int i=from;i<=to;i++)
			handles[i-from]=MarkerAdd(i,marker);
		return new DiffInfo(this,handles,to-from+1,marker,extra);
	}
}

void mxSource::OnClick(wxMouseEvent &evt) {
	// ¿por que no anda esto?
//	if (evt.ControlDown() && evt.ShiftDown()) {
//		SetSelectionMode(wxSTC_SEL_RECTANGLE);
//		evt.Skip();
//		return;
//	}
	if (evt.ControlDown()) {
		int pos = PositionFromPointClose(evt.GetX(),evt.GetY());
		int s=WordStartPosition(pos,true);
		int e=WordEndPosition(pos,true);
		wxString key = GetTextRange(s,e);
		if (key.Len()) {
			new mxGotoFunctionDialog(key,main_window,GetFileName(false));
		}
	} else if (evt.AltDown()) {
		int pos = PositionFromPointClose(evt.GetX(),evt.GetY());
		int s=WordStartPosition(pos,true);
		int e=WordEndPosition(pos,true);
		wxString key = GetTextRange(s,e);
		if (key.Len()!=0) { // puede ser una directiva de preprocesador
			if (GetCharAt(s-1)=='#')
				key = GetTextRange(s-1,e);
		}
		if (key.Len()) main_window->ShowQuickHelp(key); 
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
			if (mxDropTarget::current_drag_source!=NULL && result==wxDragMove) {
				mxDropTarget::current_drag_source=NULL;
				SetTargetStart(ss); SetTargetEnd(se); ReplaceTarget(_T(""));
			} 
			else if (result==wxDragCancel && ss==GetSelectionStart()) {
				DoDropText(evt.GetX(),evt.GetY(),_T("")); // para evitar que se congele el cursor
				SetSelection(p,p);
//				evt.Skip();
			} else {
				DoDropText(evt.GetX(),evt.GetY(),_T(""));
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
	SetMarginWidth (m_FoldingID, config_source.foldEnable? 15: 0);
}

void mxSource::SetLineNumbers() {
	// establecer el margen para los nros de lineas
	wxString ancho(_T("  X"));
	int lnct=GetLineCount();
	while (lnct>10) {
		ancho+=_T("X");
		lnct/=10;
	}
	SetMarginWidth (m_LineNrID, config_source.lineNumber?TextWidth (wxSTC_STYLE_LINENUMBER, ancho):0);
	SetViewWhiteSpace(config_source.whiteSpace?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
	SetViewEOL(config_source.whiteSpace);
}

void mxSource::SetDiffBrother(mxSource *source) {
	diff_brother=source;
}

void mxSource::ApplyDiffChange() {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	BeginUndoAction();
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
					InsertText(PositionFromLine(cl),di->extra<<_T("\n"));
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
				delete di; di=NULL; break;
				
			}
		}
		if (di) di = di->next;
	}
	EndUndoAction();
	Refresh();
}

void mxSource::DiscardDiffChange() {
	int cl=GetCurrentLine();
	DiffInfo *di = first_diff_info;
	BeginUndoAction();
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
				delete di; di=NULL; break;
			}
		}
		if (di) di = di->next;
	}
	EndUndoAction();
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
	BeginUndoAction();
	while (di) {
		for (int i=0;i<di->len;i++) {
			if (MarkerLineFromHandle(di->handles[i])==cl) {
				int l = MarkerLineFromHandle(di->handles[0]);
				ShowBaloon(di->extra,PositionFromLine(l));
				di=NULL; break;
			}
		}
		if (di) di = di->next;
	}
	EndUndoAction();
	Refresh();
}

void mxSource::Reload() {
	m_extras->FromSource(this);
	LoadFile(source_filename.GetFullPath());
	m_extras->ToSource(this);
}

void mxSource::AlignComments (int col) {
	BeginUndoAction();
	config_source.alignComments=col;
	int ss = GetSelectionStart();
	int se = GetSelectionEnd();
	if (ss>se) { int aux=ss; ss=se; se=aux; }
	bool sel = se>ss; char c;
	int line_ss=sel?LineFromPosition(ss):0, line_se=sel?LineFromPosition(se):GetLineCount();
	int p3,pl=PositionFromLine(line_ss),s;
	bool prev=false;
	for (int i=line_ss;i<line_se;i++) {
		int p1=pl;
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
						ReplaceTarget(_T(" "));
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
	EndUndoAction();
}

void mxSource::RemoveComments () {
	BeginUndoAction();
	int ss = GetSelectionStart();
	int se = GetSelectionEnd();
	if (ss>se) { int aux=ss; ss=se; se=aux; }
	bool sel = se>ss;
	int line_ss=sel?LineFromPosition(ss):0, line_se=sel?LineFromPosition(se):GetLineCount();
	int p1,p2,s;
	for (int i=line_ss;i<line_se;i++) {

		int p3=ss=PositionFromLine(i);
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
		ReplaceTarget(_T(""));
		i--;
	}
	EndUndoAction();
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
	if (dt!=source_time) {
		class SourceModifAction:public mxMainWindow::AfterEventsAction {
			mxSource *source;
		public: 
			SourceModifAction(mxSource *who):source(who){}
			void Do() { source->ThereAreExternalModifications(); }
		};
		main_window->CallAfterEvents(new SourceModifAction(this));
	}
}

void mxSource::ThereAreExternalModifications() {
	SetSourceTime(source_filename.GetModificationTime());
	if (!source_time_dont_ask) {
		int res=mxMessageDialog(main_window,LANG(SOURCE_EXTERNAL_MODIFICATION_ASK_RELOAD,"El archivo fue modificado por otro programa.\nDesea recargarlo para obtener las modificaciones?"), source_filename.GetFullPath(), mxMD_YES_NO|mxMD_WARNING,_T("No volver a preguntar"),false).ShowModal();
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
		}
	}
}

bool mxSource::IsEmptyLine(int l, bool ignore_comments, bool ignore_preproc) {
	int p=PositionFromLine(l), s, pf=(l==GetLineCount()-1)?GetLength():PositionFromLine(l+1);
	char c;
	while (p<pf) {
		if ( ! ( II_IS_4(p,' ','\t','\n','\r') 
			|| (ignore_comments && II_IS_COMMENT(p))
			|| (ignore_preproc && s==wxSTC_C_PREPROCESSOR)
			) ) return false;
		p++;
	}
	return true;
}

void mxSource::OnKeyDown(wxKeyEvent &evt) {
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
	if (evt.GetKeyCode()==WXK_ESCAPE && !CallTipActive() && !AutoCompActive()){
		wxCommandEvent evt;
		main_window->OnEscapePressed(evt);
	} else if (config_source.autotextEnabled && evt.GetKeyCode()==WXK_TAB && ApplyAutotext()){
		return;
	} else
		evt.Skip();
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
	wxString scope, type;
	int pos=GetCurrentPos();
	int s, l=pos;
	int first_p=-1; // guarda el primer scope, para buscar argumentos si es funcion
	char c;
	while (true) {
		int p_llave_a = FindText(pos,0,_T("{"));
		while (p_llave_a!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_a))
			p_llave_a = FindText(p_llave_a-1,0,_T("{"));
		int p_llave_c = FindText(pos,0,_T("}"));
		while (p_llave_c!=wxSTC_INVALID_POSITION && II_SHOULD_IGNORE(p_llave_c))
			p_llave_c = FindText(p_llave_c-1,0,_T("}"));
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
			if (c==')') { // puede ser funcion
				int op=p;
				p=BraceMatch(p);
				// sigue como antes
				if (p!=wxSTC_INVALID_POSITION) {
					p--;
					II_BACK(p,II_IS_NOTHING_4(p));
					op=p; first_p=p+1;
					p=WordStartPosition(p,true)-1;
					scope=GetTextRange(p+1,op+1);
					II_BACK(p,II_IS_NOTHING_4(p));
					// el "GetCharAt(p)==','" se agrego el 29/09 para los constructores en constructores
					if (GetCharAt(p)==':' || GetCharAt(p)==',' || (p && GetCharAt(p)=='~' && GetCharAt(p-1)==':')) {
						if (GetCharAt(p)=='~') { p--; scope=wxString("~")+scope; } // agregado para arreglar el scope de un destructor
//						cerr<<GetTextRange(p-1,p+10)<<endl;
						if (GetCharAt(p-1)==':') {
							p-=2;
							II_BACK(p,II_IS_NOTHING_4(p));
							scope=code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1))+"::"+scope;
						} else { // puede ser constructor
							p = FindText(p,0,_T("::"));
							if (p!=wxSTC_INVALID_POSITION) {
								int e=p+2;
								p--;
								II_BACK(p,II_IS_NOTHING_4(p));
								II_FRONT_NC(e,II_IS_NOTHING_4(e));
								if (GetTextRange(WordStartPosition(p,true),p+1)==GetTextRange(e,WordEndPosition(e,true))) {
									scope = code_helper->UnMacro(GetTextRange(WordStartPosition(p,true),p+1));
									scope=scope+"::"+scope;
								}
							}
						}
					}
				}
			} else { // puede ser clase o struct
//				cerr<<GetTextRange(p,p+10)<<endl;
				II_BACK(p,II_IS_NOTHING_4(p) || !II_IS_6(p,'{','}',':',';',')','('));
//				cerr<<GetTextRange(p,p+10)<<endl;
				p++;
				II_FRONT(p,II_IS_NOTHING_4(p));
//				cerr<<GetTextRange(p,p+10)<<endl;
				if (GetStyleAt(p)==wxSTC_C_WORD) {
					bool some=false;
					if (GetTextRange(p,p+6)==_T("struct"))
						{ if (!type.Len()) type="struct"; p+=6; some=true; }
					else if (GetTextRange(p,p+5)==_T("class"))
						{ if (!type.Len()) type="class"; p+=5; some=true; }
					else if (GetTextRange(p,p+p)==_T("namespace"))
						{ if (!type.Len()) type="namespace"; p+=9; some=true; }
					if (some) {
						II_FRONT(p,II_IS_NOTHING_4(p));
						scope=code_helper->UnMacro(GetTextRange(p,WordEndPosition(p,true)))+"::"+scope;
					}
				}
			}
		}
	}
	if (scope.EndsWith("::")) { // si no es metodo ni funcion
		scope=scope.Mid(0,scope.Len()-2);
		scope=type+" "+scope;
	} else if (first_p!=-1) { // sino agreagr argumentos
		int p=first_p;
		II_FRONT(p,II_IS_NOTHING_4(p));
		if (GetCharAt(p)=='(') {
			int p2=BraceMatch(p);
			if (p2!=wxSTC_INVALID_POSITION)
				scope=scope+" "+GetTextRange(p,p2+1);
		}
	}
	return scope
#ifdef DEBUG
		<<"\n"<<GetCurrentPos()
#endif
		;
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
	
	bool dark = (ctheme->DEFAULT_BACK.Green()+ctheme->DEFAULT_BACK.Blue()+ctheme->DEFAULT_BACK.Red()<256*3/2);
	if (dark) {
		MarkerDefine(mxSTC_MARK_DIFF_ADD,wxSTC_MARK_BACKGROUND,_T("DARK GREEN"),_T("DARK GREEN"));
		MarkerDefine(mxSTC_MARK_DIFF_DEL,wxSTC_MARK_ARROW,_T("WHITE"),_T("RED"));
		MarkerDefine(mxSTC_MARK_DIFF_CHANGE,wxSTC_MARK_BACKGROUND,_T("BROWN"),_T("BROWN"));
		
		MarkerDefine(mxSTC_MARK_BAD_BREAKPOINT,wxSTC_MARK_CIRCLE, _T("BLACK"), _T("LIGHT GRAY"));
		MarkerDefine(mxSTC_MARK_BREAKPOINT,wxSTC_MARK_CIRCLE, _T("BLACK"), _T("RED"));
		MarkerDefine(mxSTC_MARK_EXECPOINT,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("GREEN"));
		MarkerDefine(mxSTC_MARK_FUNCCALL,wxSTC_MARK_SHORTARROW, _T("WHITE"), _T("YELLOW"));
		MarkerDefine(mxSTC_MARK_STOP,wxSTC_MARK_SHORTARROW, _T("WHITE"), _T("RED"));
	} else {
		MarkerDefine(mxSTC_MARK_DIFF_ADD,wxSTC_MARK_BACKGROUND,_T("Z DIFF GREEN"),_T("Z DIFF GREEN"));
		MarkerDefine(mxSTC_MARK_DIFF_DEL,wxSTC_MARK_ARROW,_T("BLACK"),_T("RED"));
		MarkerDefine(mxSTC_MARK_DIFF_CHANGE,wxSTC_MARK_BACKGROUND,_T("Z DIFF YELLOW"),_T("Z DIFF YELLOW"));
		
		MarkerDefine(mxSTC_MARK_BAD_BREAKPOINT,wxSTC_MARK_CIRCLE, _T("WHITE"), _T("Z DARK GRAY"));
		MarkerDefine(mxSTC_MARK_BREAKPOINT,wxSTC_MARK_CIRCLE, _T("WHITE"), _T("RED"));
		MarkerDefine(mxSTC_MARK_EXECPOINT,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("GREEN"));
		MarkerDefine(mxSTC_MARK_FUNCCALL,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("YELLOW"));
		MarkerDefine(mxSTC_MARK_STOP,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("RED"));
	}
	
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
wxString mxSource::GetParsedCompilerOptions() {
	wxArrayString args; 
	utils->Split(config_running.compiler_options,args,false,true);
	for(unsigned int i=0;i<args.GetCount();i++) args[i]=current_toolchain.FixArgument(cpp_or_just_c,args[i]);
	wxString comp_opts=utils->UnSplit(args);
	utils->ParameterReplace(comp_opts,_T("${MINGW_DIR}"),config->mingw_real_path);
	comp_opts = utils->ExecComas(working_folder.GetFullPath(),comp_opts);
	return comp_opts;
}

bool mxSource::IsCppOrJustC() {
	return cpp_or_just_c;
}

wxFileName mxSource::GetBinaryFileName ( ) {
	if (project && !sin_titulo) 
		return DIR_PLUS_FILE(project->GetTempFolder(),source_filename.GetName()+_T(".o"));
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
//		cerr<<evt.GetMessage()<<" "<<evt.GetWParam()<<" "<<evt.GetLParam()<<endl;
		main_window->m_macro->Add(MacroAction(evt.GetMessage(),evt.GetWParam(),evt.GetLParam()));
	}
}

