#ifndef MXCOLOURSEDITOR_H
#define MXCOLOURSEDITOR_H
#include <wx/dialog.h>
#include <wx/panel.h>
#include "mxUtils.h"
class wxScrolledWindow;
class wxTextCtrl;
class wxButton;
class wxFlexGridSizer;
class wxCheckBox;
class wxRichTextCtrl;
class wxComboBox;

#define CTDeclare(NAME) wxColour NAME##_FORE, NAME##_BACK; bool NAME##_ITALIC, NAME##_BOLD;
#define CTDeclare0(NAME) wxColour NAME##_FORE, NAME##_BACK;
#define CTDeclare1(NAME) wxColour NAME;

#define CTForAll(CTMacro) \
	CTMacro(DEFAULT); \
	CTMacro(IDENTIFIER); \
	CTMacro(NUMBER); \
	CTMacro(WORD); \
	CTMacro(WORD2); \
	CTMacro(STRING); \
	CTMacro(STRINGEOL); \
	CTMacro(CHARACTER); \
	CTMacro(OPERATOR); \
	CTMacro(BRACELIGHT); \
	CTMacro(BRACEBAD); \
	CTMacro(PREPROCESSOR); \
	CTMacro(COMMENT); \
	CTMacro(COMMENTLINE); \
	CTMacro(COMMENTDOC); \
	CTMacro(COMMENTLINEDOC); \
	CTMacro(COMMENTDOCKEYWORD); \
	CTMacro(COMMENTDOCKEYWORDERROR); \
	CTMacro##0(CALLTIP); \
	CTMacro##0(LINENUMBER); \
	CTMacro##0(FOLD); \
	CTMacro##0(FOLD_TRAMA); \
	CTMacro##1(INDENTGUIDE); \
	CTMacro##1(SELBACKGROUND); \
	CTMacro##1(CURRENT_LINE); \
	CTMacro##1(USER_LINE); \
	CTMacro(GLOBALCLASS); \
	CTMacro##1(CARET);

struct color_theme {
	wxString name;
	bool inverted;
	CTForAll(CTDeclare)
	
	color_theme(bool inverted);
	color_theme(wxString file="");
	void SetDefaults(bool inverted=false);
	bool Save(const wxString &full_path);
	bool Load(const wxString &full_path);
	static void Initialize();
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
	wxString lnames[MAX_STYLES]; ///< items for wich style should be defined
	mxStaticText *llabel[MAX_STYLES]; ///< text sample with item name
	wxColour *lvalfor[MAX_STYLES]; ///< foreground colours
	wxColour *lvalbak[MAX_STYLES]; ///< background colours
	wxTextCtrl *ltfore[MAX_STYLES]; ///< foreground colour text controls (hex colour)
	wxTextCtrl *ltback[MAX_STYLES]; ///< background colour text controls (hex colour)
	wxButton *lbfore[MAX_STYLES]; ///< foreground colour buttons (...)
	wxButton *lbback[MAX_STYLES]; ///< background colour buttons(...)
	wxCheckBox *lbold[MAX_STYLES]; ///< bold text checkboxs
	wxCheckBox *lcur[MAX_STYLES]; ///< italic text checkboxs
	bool *lvalita[MAX_STYLES]; ///< italic values
	bool *lvalbol[MAX_STYLES]; ///< bold values
	wxFlexGridSizer *sizer;
	int lcount;
	wxComboBox *combo;
	wxWindow *parent;
	wxCheckBox *inverted;
	bool setting; ///< enmascara eventos en los text y checks para cuando carga una conf nueva
	color_theme old_theme; ///< tema viejo, por si cancela el dialogo
	color_theme custom_theme; ///< tema personalizado, por si cambia el combo
public:
	mxColoursEditor(wxWindow *parent);
	void LoadList();
	void Add(wxString name, wxColour *fore, wxColour *back, bool *italic, bool *bold);
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
	DECLARE_EVENT_TABLE();
};

extern color_theme *g_ctheme;

#endif

