#ifndef MXCOLOURSEDITOR_H
#define MXCOLOURSEDITOR_H
#include <wx/dialog.h>
#include "mxUtils.h"
#include <wx/panel.h>
class wxScrolledWindow;
class wxTextCtrl;
class wxButton;
class wxFlexGridSizer;
class wxCheckBox;
class wxRichTextCtrl;
class wxComboBox;

struct color_theme {
	wxString name;
	
	wxColour DEFAULT_FORE, DEFAULT_BACK; bool DEFAULT_ITALIC, DEFAULT_BOLD;
	wxColour IDENTIFIER_FORE, IDENTIFIER_BACK; bool IDENTIFIER_ITALIC, IDENTIFIER_BOLD;
	wxColour NUMBER_FORE, NUMBER_BACK; bool NUMBER_ITALIC, NUMBER_BOLD;
	wxColour WORD_FORE, WORD_BACK; bool WORD_ITALIC, WORD_BOLD;
	wxColour WORD2_FORE, WORD2_BACK; bool WORD2_ITALIC, WORD2_BOLD;
	wxColour STRING_FORE, STRING_BACK; bool STRING_ITALIC, STRING_BOLD;
	wxColour STRINGEOL_FORE, STRINGEOL_BACK; bool STRINGEOL_ITALIC, STRINGEOL_BOLD;
	wxColour CHARACTER_FORE, CHARACTER_BACK; bool CHARACTER_ITALIC, CHARACTER_BOLD;
	wxColour OPERATOR_FORE, OPERATOR_BACK; bool OPERATOR_ITALIC, OPERATOR_BOLD;
	wxColour BRACELIGHT_FORE, BRACELIGHT_BACK; bool BRACELIGHT_ITALIC, BRACELIGHT_BOLD;
	wxColour BRACEBAD_FORE, BRACEBAD_BACK; bool BRACEBAD_ITALIC, BRACEBAD_BOLD;
	wxColour PREPROCESSOR_FORE, PREPROCESSOR_BACK; bool PREPROCESSOR_ITALIC, PREPROCESSOR_BOLD;
	wxColour COMMENT_FORE, COMMENT_BACK; bool COMMENT_ITALIC, COMMENT_BOLD;
	wxColour COMMENTLINE_FORE, COMMENTLINE_BACK; bool COMMENTLINE_ITALIC, COMMENTLINE_BOLD;
	wxColour COMMENTDOC_FORE, COMMENTDOC_BACK; bool COMMENTDOC_ITALIC, COMMENTDOC_BOLD;
	wxColour COMMENTLINEDOC_FORE, COMMENTLINEDOC_BACK; bool COMMENTLINEDOC_ITALIC, COMMENTLINEDOC_BOLD;
	wxColour COMMENTDOCKEYWORD_FORE, COMMENTDOCKEYWORD_BACK; bool COMMENTDOCKEYWORD_ITALIC, COMMENTDOCKEYWORD_BOLD;
	wxColour COMMENTDOCKEYWORDERROR_FORE, COMMENTDOCKEYWORDERROR_BACK; bool COMMENTDOCKEYWORDERROR_ITALIC, COMMENTDOCKEYWORDERROR_BOLD;
//	wxColour GLOBALCLASS_FORE, GLOBALCLASS_BACK; bool GLOBALCLASS_ITALIC, GLOBALCLASS_BOLD; // no se usan, ver para que pueden servir mas keywords
	wxColour CALLTIP_FORE, CALLTIP_BACK;
	wxColour LINENUMBER_FORE, LINENUMBER_BACK;
	wxColour FOLD_FORE, FOLD_BACK, FOLD_TRAMA_FORE, FOLD_TRAMA_BACK;
	wxColour SELBACKGROUND;
	wxColour CARET;
	wxColour CURRENT_LINE;
	wxColour USER_LINE;
	wxColour INDENTGUIDE;
	
	color_theme(wxString file="");
	void SetDefaults(bool inverted=false);
	bool Save(wxString fname);
	bool  Load(wxString fname);
	static void Init();
};

class mxStaticText : public wxPanel {
	wxString text;
	wxColour *fore,*back;
	bool *italic , *bold;
public:
	mxStaticText (wxWindow *parent, wxString text);
	void SetData(wxColour *fore, wxColour *back, bool *italic, bool *bold);
	void OnPaint(wxPaintEvent &evt);
	DECLARE_EVENT_TABLE();
};

#define MAX_STYLES 50

class mxColoursEditor:public wxDialog {
	wxScrolledWindow *scroll;
	wxString lnames[MAX_STYLES];
	mxStaticText *llabel[MAX_STYLES];
	wxButton *lbfore[MAX_STYLES];
	wxButton *lbback[MAX_STYLES];
	wxTextCtrl *ltfore[MAX_STYLES];
	wxTextCtrl *ltback[MAX_STYLES];
	wxCheckBox *lbold[MAX_STYLES];
	wxCheckBox *lcur[MAX_STYLES];
	wxColour *lvalfor[MAX_STYLES];
	wxColour *lvalbak[MAX_STYLES];
	bool *lvalita[MAX_STYLES];
	bool *lvalbol[MAX_STYLES];
	wxFlexGridSizer *sizer;
	int lcount;
	wxComboBox *combo;
	wxWindow *parent;
	bool setting; ///< enmascara eventos en los text y checks para cuando carga una conf nueva
	color_theme old_theme; ///< tema viejo, por si cancela el dialogo
	color_theme custom_theme; ///< tema personalizado, por si cambia el combo
public:
	mxColoursEditor(wxWindow *parent);
	void LoadList();
	void Add(wxString name, wxColour *fore, wxColour *back, bool *italic, bool *bold);
	void LoadTheme(wxString name);
	void SaveTheme(wxString name);
	void OnClose(wxCloseEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonApply(wxCommandEvent &evt);
	void OnButtonPicker(wxCommandEvent &evt);
	void OnCheck(wxCommandEvent &evt);
	void OnText(wxCommandEvent &evt);
	void OnCombo(wxCommandEvent &evt);
	void OnOpen(wxCommandEvent &evt);
	void OnSave(wxCommandEvent &evt);
	void SetValues();
	~mxColoursEditor();
	DECLARE_EVENT_TABLE();
};

extern color_theme *ctheme;

#endif

