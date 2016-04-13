#ifndef MXINSPECTIONEXPLORERDIALOG_H
#define MXINSPECTIONEXPLORERDIALOG_H
#include <wx/panel.h>

class mxInspectionExplorerWidget;

class mxInspectionExplorerDialog : public wxPanel {
	mxInspectionExplorerWidget *exp;
public:
	mxInspectionExplorerDialog(const wxString &expression="", bool frameless=false);
	void AddExpression(wxString expression, bool frameless);
};

#endif

