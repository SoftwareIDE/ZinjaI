#ifndef MXMESSAGEDIALOG_H
#define MXMESSAGEDIALOG_H

#include <wx/dialog.h>

class wxCheckBox;

class mxMessageDialog : public wxDialog {
	
public:
	mxMessageDialog(wxWindow *parent, wxString message);
	mxMessageDialog &ButtonsOk() { m_ok=true; m_cancel=m_yes=m_no=false; return *this; }
	mxMessageDialog &ButtonsOkCancel() { m_ok=m_cancel=true; m_yes=m_no=false; return *this; }
	mxMessageDialog &ButtonsYesNo() { m_yes=m_no=true; m_ok=m_cancel=false; return *this; }
	mxMessageDialog &ButtonsYesNoCancel() { m_cancel=m_yes=m_no=true; m_ok=false; return *this; }
	mxMessageDialog &Title(const wxString &title) { m_title=title; return *this; }
	mxMessageDialog &IconError() { m_icon=mxMDError; return *this; }
	mxMessageDialog &IconInfo() { m_icon=mxMDInfo; return *this; }
	mxMessageDialog &IconWarning() { m_icon=mxMDWarning; return *this; }
	mxMessageDialog &IconQuestion() { m_icon=mxMDQuestion; return *this; }
	mxMessageDialog &Check1(const wxString &label, bool def_value) { m_check1_str=label; m_check1_val=def_value; return *this; }
	mxMessageDialog &Check2(const wxString &label, bool def_value) { m_check2_str=label; m_check2_val=def_value; return *this; }
	
	struct mdAns {
		bool ok, yes, no, cancel, closed, check1, check2;
		mdAns() : ok(false), yes(false), no(false), cancel(false), closed(false), check1(false), check2(false) {}
	};
	mdAns m_result;
	mdAns Run();
	
private:
	bool m_ok, m_cancel, m_yes, m_no, m_check1_val, m_check2_val;
	wxString m_check1_str, m_check2_str, m_title, m_message; 
	enum mxMDIcon { mxMDNull, mxMDInfo, mxMDQuestion, mxMDWarning, mxMDError };
	mxMDIcon m_icon;
	wxCheckBox *m_check1_ctrl, *m_check2_ctrl;
	
	void OnCancelButton(wxCommandEvent &event);
	void OnOkButton(wxCommandEvent &event);
	void OnYesButton(wxCommandEvent &event);
	void OnNoButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnCharHook(wxKeyEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

