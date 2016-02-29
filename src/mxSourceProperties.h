#ifndef MXSOURCEPROPERTIES_H
#define MXSOURCEPROPERTIES_H

#include <wx/timer.h>
#include "mxCommonConfigControls.h"

class mxSource;
class wxTextCtrl;
class wxTimer;

class mxSourceProperties : public mxDialog {
private:
	wxTextCtrl *text_type, *text_deps;
	wxString m_fname;
public:
	mxSourceProperties(wxString path, mxSource *src=nullptr);
	void OnOkButton(wxCommandEvent &event);
	void OnTimer(wxTimerEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

