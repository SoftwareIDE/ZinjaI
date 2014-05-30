#ifndef MXTREECTRL_H
#define MXTREECTRL_H
#include <wx/treectrl.h>

class mxTreeCtrl : public wxTreeCtrl {
public:
	mxTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "treeCtrl");
	void OnKey(wxKeyEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

