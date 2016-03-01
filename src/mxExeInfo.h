#ifndef MXEXEINFO_H
#define MXEXEINFO_H

#include <wx/filename.h>
#include <wx/timer.h>
#include "mxCommonConfigControls.h"

class mxSource;
class wxNotebook;
class wxPanel;
class wxTextCtrl;
class wxTimer;

class mxExeInfo : public mxDialog {
	enum ei_mode { mxEI_PROJECT, mxEI_SIMPLE, mxEI_SOURCE } m_mode;
private:
	mxSource *m_source;
	wxFileName m_fname;
	wxTextCtrl *text_size, *text_type, *text_time, *ldd_ctrl;
	wxTimer m_wait_for_parser;
	void UpdateTypeAndDeps();
public:
	mxExeInfo(wxWindow *parent, ei_mode mode, wxFileName fname, mxSource *src=nullptr);
	wxPanel *CreateGeneralPanel (wxNotebook *notebook);
	wxPanel *CreateDependPanel (wxNotebook *notebook);
	void OnCloseButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnLocationButton(wxCommandEvent &evt);
	void OnStripButton(wxCommandEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	static void RunForProject(wxWindow *parent);
	static void RunForSimpleProgram(wxWindow *parent, mxSource *source);
	static void RunForSource(wxWindow *parent, mxSource * source);
	static void RunForSource(wxWindow *parent, wxFileName fname);
	DECLARE_EVENT_TABLE();
};

#endif

