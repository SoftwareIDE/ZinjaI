#include <wx/sizer.h>
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionExplorerWidget.h"
#include "mxMainWindow.h"
#include "mxSizers.h"

mxInspectionExplorerDialog::mxInspectionExplorerDialog (const wxString & expression, bool frameless) 
	:wxDialog(main_window,wxID_ANY,expression,wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	exp = new mxInspectionExplorerWidget(this,expression,frameless);
	sizer->Add(exp,sizers->Exp1);
	SetSizer(sizer);
	Show();
}

void mxInspectionExplorerDialog::AddExpression (wxString expression, bool frameless) {
	exp->AddExpression(expression,frameless);
}

