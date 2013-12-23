#ifndef MX_GOTO_FUNCTION_DIALOG_H
#define MX_GOTO_FUNCTION_DIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include "SingleList.h"

class wxListBox;
class wxTextCtrl;
class wxButton;
class wxCheckBox;
class pd_macro;
class pd_class;
class pd_func;

class mxGotoFunctionDialog: public wxDialog {
private:
	struct gotoff_result {
		int type; /// 1=macro, 2=class, 3=func
		void *ptr;
		gotoff_result():type(0),ptr(NULL) {}
		gotoff_result(pd_macro *p):type(1),ptr(p) {}
		gotoff_result(pd_class *p):type(2),ptr(p) {}
		gotoff_result(pd_func *p):type(3),ptr(p) {}
		pd_macro *get_macro() { return (pd_macro*)ptr; }
		pd_class *get_class() { return (pd_class*)ptr; }
		pd_func *get_func() { return (pd_func*)ptr; }
	};
	SingleList<gotoff_result> m_results;
	/// performs an interface-independent search, put results in m_results 
	int FillResults(wxString key, bool ignore_case, bool strict_compare=false);
	
	wxTextCtrl *text_ctrl;
	wxListBox *list_ctrl;
	wxTimer *timer;
	wxButton *goto_button;
	wxButton *cancel_button;
	wxCheckBox *case_sensitive;
	bool strict_compare;
public:
	mxGotoFunctionDialog(wxString text, wxWindow* parent, bool direct_goto=false);
	~mxGotoFunctionDialog();
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTimerInput(wxTimerEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
	void OnCaseCheck(wxCommandEvent &event);
private:
	DECLARE_EVENT_TABLE();
};

#endif

