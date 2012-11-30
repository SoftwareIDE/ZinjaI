#include "mxTreeCtrl.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxTreeCtrl,wxTreeCtrl)
	EVT_KEY_DOWN(mxTreeCtrl::OnKey)
END_EVENT_TABLE()

mxTreeCtrl::mxTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name):
	wxTreeCtrl(parent, id, pos, size, style, validator, name) {
}

void mxTreeCtrl::OnKey(wxKeyEvent &evt) {
	main_window->OnKeyEvent(this,evt);
}
