#ifndef MXBEGINNERPANEL_H
#define MXBEGINNERPANEL_H
#include <wx/panel.h>

class wxChoicebook;
class wxStaticBox;
class wxBoxSizer;
class wxCheckBox;
class wxTextCtrl;

class mxBeginnerPanel:public wxPanel {
private:
	wxChoicebook *book;
	wxPanel **options, *current_panel;
	wxBoxSizer *the_sizer;
	
	wxTextCtrl *opt_if_cond;
	wxCheckBox *opt_if_else;
	wxTextCtrl *opt_for_var;
	wxCheckBox *opt_for_dec;
	wxTextCtrl *opt_for_from;
	wxTextCtrl *opt_for_to;
	wxTextCtrl *opt_for_step;
	wxCheckBox *opt_for_last;
	wxTextCtrl *opt_while_cond;
	wxTextCtrl *opt_do_cond;
	wxTextCtrl *opt_switch_expr;
	wxTextCtrl *opt_switch_cases;
	wxCheckBox *opt_switch_default;
	
	wxCheckBox *show_quickhelp;
	
public:
	mxBeginnerPanel(wxWindow *aparent);
	wxPanel *CreatePageControls();
	void ShowOptions(int n, wxString help);
	void LoadQuickHelp(wxString file);
	wxPanel *CreateOptionsFor(wxPanel *parent);
	void OnButtonFor(wxCommandEvent &evt);
	wxPanel *CreateOptionsDo(wxPanel *parent);
	void OnButtonDo(wxCommandEvent &evt);
	wxPanel *CreateOptionsIf(wxPanel *parent);
	void OnButtonIf(wxCommandEvent &evt);
	wxPanel *CreateOptionsSwitch(wxPanel *parent);
	void OnButtonSwitch(wxCommandEvent &evt);
	wxPanel *CreateOptionsWhile(wxPanel *parent);
	void OnButtonWhile(wxCommandEvent &evt);
	void WriteTemplate(const wxString &text);
	DECLARE_EVENT_TABLE();
};

extern mxBeginnerPanel *g_beginner_panel;

#endif

