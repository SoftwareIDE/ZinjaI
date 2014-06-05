#ifndef MXSIGNALSSETTINGS_H
#define MXSIGNALSSETTINGS_H
#include <wx/dialog.h>
#include <vector>
#include "DebugManager.h"
using namespace std;

class wxCheckBox;

class mxSignalsSettings : public wxDialog {
private:
	vector<SignalHandlingInfo> vsig; 
	wxCheckBox **checks_stop;
	wxCheckBox **checks_pass;
public:
	mxSignalsSettings();
	~mxSignalsSettings();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

