#include <wx/sizer.h>
#include "mxInspectionBaloon.h"
#include "mxInspectionExplorerWidget.h"
#include "mxMainWindow.h"
#include "mxSizers.h"
#include <wx/stattext.h>
#include "DebugManager.h"

mxInspectionBaloon::mxInspectionBaloon (const wxPoint &pos, const wxString & expression, const wxString &value) 
	:wxFrame(main_window,wxID_ANY,expression,pos,wxDefaultSize,wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT|wxNO_BORDER) 
{
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	// create the tree for exploring the VO
	exp = new mxInspectionExplorerWidget(this,expression,false,false);
	sizer->Add(exp,sizers->Exp1);
	// create the text for the type
	wxString type = exp->GetRootType(), new_expr;
	text = new wxStaticText(this,wxID_ANY,type);
	sizer->Add(text,sizers->Exp0);
	// try to use automatic inspection improving, and show full value in root node
	if (DebuggerInspection::TryToImproveExpression(type,new_expr,expression))
		exp->SetItemText(exp->GetRootItem(),expression+": "+debug->InspectExpression(new_expr));
	else if (!value.StartsWith("@"))
		exp->SetItemText(exp->GetRootItem(),expression+": "+value);
	// deselect root node, so we don't see it with blue background
	exp->SelectItem(exp->GetRootItem(),false);
	// set size and show
	SetSizer(sizer);
	OnTreeSize();
	exp->SetEventListener(new EventListener(this));
	Show();
}

void mxInspectionBaloon::AddExpression (wxString expression, bool frameless) {
	exp->AddExpression(expression,frameless);
}

void mxInspectionBaloon::OnTreeSize ( ) {
	wxSize sz = exp->GetFullSize();
	sz.IncBy(wxSystemSettings::GetMetric(wxSYS_VSCROLL_X),
#ifdef __WIN32__
		wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y)+
#endif
		text->GetSize().GetHeight()+5);
	SetSize(ClientToWindowSize(sz));
}

void mxInspectionBaloon::OnTreeType (const wxString & type) {
	text->SetLabel(type);
}
