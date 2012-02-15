#ifndef MXPROJECTSTATISTICS_H
#define MXPROJECTSTATISTICS_H
#include <wx/dialog.h>
#include <wx/timer.h>

class wxStaticText;

class mxProjectStatistics : public wxDialog {
private:
	wxStaticText *cant_sources;
	wxStaticText *cant_headers;
	wxStaticText *cant_others;
	wxStaticText *cant_total;
	wxStaticText *total_size;
	wxStaticText *cant_class;
	wxStaticText *cant_globs;
	wxStaticText *cant_funcs;
	wxStaticText *cant_defs;
	wxStaticText *cant_lines;
	wxTimer *wait_for_parser;
public:
	mxProjectStatistics();
	void OnClose(wxCloseEvent &event);
	void OnHelpButton(wxCommandEvent &evt);
	void OnCloseButton(wxCommandEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	void SetValues(bool all=true);
	DECLARE_EVENT_TABLE();
};

#endif

