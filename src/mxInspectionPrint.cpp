#include <wx/grid.h>
#include <wx/textdlg.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include "mxInspectionPrint.h"
#include "DebugManager.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxSizers.h"
#include "mxTextDialog.h"

//BEGIN_EVENT_TABLE(mxInspectionPrint, mxExternInspection)
//END_EVENT_TABLE()

mxInspectionPrint::mxInspectionPrint(wxString frame, wxString expression,bool frameless) : mxExternInspection(frameless?frame:"",expression,!frameless) {
	if ((is_macro=expression.StartsWith(">"))) this->expression=expression.Mid(1);
	wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(text=new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_READONLY|wxTE_MULTILINE),sizers->Exp1);
	SetMinSize(wxSize(100,10));
	SetSizerAndFit(sizer);
	Show();
	Update();
}

void mxInspectionPrint::Update() {
	debug->SetFullOutput(true);
	if (is_macro)
		text->SetValue(debug->GetMacroOutput(expression));
	else
		text->SetValue(debug->InspectExpression(expression,true));
	debug->SetFullOutput(true);
}
