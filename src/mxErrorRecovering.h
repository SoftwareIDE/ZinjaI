#ifndef MXERRORRECOVERING_H
#define MXERRORRECOVERING_H
#include <wx/dialog.h>
#include <wx/arrstr.h>

class wxListBox;
class wxTextCtrl;

class mxErrorRecovering : public wxDialog {
private:
	wxListBox *list;
	wxArrayString files,names;
public:
	mxErrorRecovering();
	~mxErrorRecovering();
	void OnClose(wxCloseEvent &evt);
	void OnAllButton(wxCommandEvent &evt);
	void OnSomeButton(wxCommandEvent &evt);
	void OnNoneButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	static bool RecoverSomething();
	DECLARE_EVENT_TABLE();
};


#endif

