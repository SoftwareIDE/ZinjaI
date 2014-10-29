#ifndef MXEXTERNCOMPILEROUTPUT_H
#define MXEXTERNCOMPILEROUTPUT_H
#include <wx/listbox.h>

class mxExternCompilerOutput : public wxListBox {
	wxArrayString lines;
	bool showing_only_errors;
	void PopulateArray();
	bool IsErrorLine(int i) { return GetString(i).StartsWith("!! "); }
public:
	mxExternCompilerOutput(wxWindow *compiler_panel);
	void OnDClick(wxCommandEvent &evt);
	void OnPopup(wxMouseEvent &evt);
	void OnOnlyErrors(wxCommandEvent &evt);
	void OnViewFullOutput(wxCommandEvent &evt);
	void AddLine(const wxString &pre, const wxString &message);
	void Clear();
	void OnErrorNext();
	void OnErrorPrev();
	
	DECLARE_EVENT_TABLE();
};

#endif

