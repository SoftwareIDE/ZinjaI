#ifndef MXEXTERNINSPECTION_H
#define MXEXTERNINSPECTION_H
#include <wx/dialog.h>

class DebugManager;

class mxExternInspection : public wxDialog {
	friend class DebugManager;
protected:
	bool can_be_outdated;
	wxString expression,frame;
public:
	virtual bool SetOutdated();
	mxExternInspection(wxString frame="", wxString expression="", bool can_be_outdated=true);
	void OnClose(wxCloseEvent &e);
	virtual wxString GetFrame();
	virtual void Update()=0;
	~mxExternInspection();
	DECLARE_EVENT_TABLE();
};

#endif

