#ifndef MXINSPECTIONEXPLORERDIALOG_H
#define MXINSPECTIONEXPLORERDIALOG_H
#include <wx/dialog.h>


class mxInspectionExplorerWidget;

class mxInspectionExplorerDialog : public wxDialog {
	mxInspectionExplorerWidget *exp;
public:
	mxInspectionExplorerDialog(const wxString &expression="", bool frameless=false);
	void AddExpression(wxString expression, bool frameless);
};

#endif

