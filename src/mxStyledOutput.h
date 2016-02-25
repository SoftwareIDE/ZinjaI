#ifndef MXSTYLEDOUTPUT_H
#define MXSTYLEDOUTPUT_H
#include <wx/stc/stc.h>

class mxStyledOutput : public wxStyledTextCtrl {
	bool is_read_only;
public:
	const int next_available_marker = 3;
	mxStyledOutput(wxWindow *parent, bool read_only=false, bool wrap_lines=true);
	void AppendText(const wxString &str);
	void AppendLine(const wxString &str);
	void AppendLine(const wxString &str, bool start_collapsible_section);
	void OnMarginClick(wxStyledTextEvent &e);
	void OnMouseWheel (wxMouseEvent & event);
	void Clear();
	virtual bool StartCollapsibleSection(const wxString &str) { return false; }
	virtual void AfterMarginClick() {}
	virtual ~mxStyledOutput() {}
	DECLARE_EVENT_TABLE();
};

#endif

