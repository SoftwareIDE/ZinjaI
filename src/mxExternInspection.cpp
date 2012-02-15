#include "mxExternInspection.h"
#include "mxMainWindow.h"
#include "DebugManager.h"

BEGIN_EVENT_TABLE(mxExternInspection,wxDialog)
	EVT_CLOSE(mxExternInspection::OnClose)
END_EVENT_TABLE()

mxExternInspection::mxExternInspection(wxString frame, wxString expression, bool can_be_outdated) : wxDialog(main_window, wxID_ANY, expression, wxDefaultPosition, wxSize(50,10) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	this->frame=frame;
	this->expression=expression;
	this->can_be_outdated=can_be_outdated;
	debug->RegisterExternInspection(this);
}

mxExternInspection::~mxExternInspection() {
	
}

bool mxExternInspection::SetOutdated ( ) {
	return can_be_outdated;
}

void mxExternInspection::OnClose(wxCloseEvent &evt) {
	debug->UnRegisterExternInspection(this);
	Destroy();
}

wxString mxExternInspection::GetFrame ( ) {
	return frame;
}

