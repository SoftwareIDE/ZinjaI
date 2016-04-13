#include <wx/sizer.h>
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionExplorerWidget.h"
#include "mxMainWindow.h"
#include "mxSizers.h"

mxInspectionExplorerDialog::mxInspectionExplorerDialog (const wxString & expression, bool frameless) : wxPanel(main_window,wxID_ANY) 
{
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	exp = new mxInspectionExplorerWidget(this,expression,frameless);
	sizer->Add(exp,sizers->Exp1);
	SetSizer(sizer);
	main_window->AttachPane(this,expression,wxDefaultPosition,wxSize(250,200));
}

void mxInspectionExplorerDialog::AddExpression (wxString expression, bool frameless) {
	exp->AddExpression(expression,frameless);
}

