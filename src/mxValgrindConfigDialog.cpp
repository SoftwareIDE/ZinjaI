#include <wx/arrstr.h>
#include <wx/sizer.h>
#include "mxValgrindConfigDialog.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"

mxValgrindConfigDialog *valgrind_config=NULL;

BEGIN_EVENT_TABLE(mxValgrindConfigDialog,wxDialog)
	EVT_BUTTON(wxID_OK,mxValgrindConfigDialog::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxValgrindConfigDialog::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxValgrindConfigDialog::OnButtonHelp)
	EVT_CLOSE(mxValgrindConfigDialog::OnClose)
END_EVENT_TABLE()
	
mxValgrindConfigDialog::mxValgrindConfigDialog(wxWindow *parent):wxDialog(parent,wxID_ANY,"Valgrind Setup",wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	
	wxArrayString tools;
	tools.Add("memcheck");
	tools.Add("cachegrind");
	tools.Add("callgrind");
	tools.Add("massif");
	tools.Add("helgrind");
	tools.Add("drd");
	
	cmb_tool=mxUT::AddComboBox(sizer,this,"Tool",tools,0);
	suppressions=mxUT::AddTextCtrl(sizer,this,"Suppression files:","");
	additional_args=mxUT::AddTextCtrl(sizer,this,"Additional arguments:","");
	
	wxSizer *bottomSizer=new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	sizer->Add(bottomSizer,sizers->Right);
	
	SetSizerAndFit(sizer);
}

mxValgrindConfigDialog::~mxValgrindConfigDialog() {
	
}

void mxValgrindConfigDialog::OnButtonOk (wxCommandEvent & evt) {
	EndModal(1);
}

void mxValgrindConfigDialog::OnButtonCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxValgrindConfigDialog::OnButtonHelp (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("valgrind.html");
}

void mxValgrindConfigDialog::OnClose (wxCloseEvent & evt) {
	EndModal(0);
}

wxString mxValgrindConfigDialog::GetArgs() {
	int i=cmb_tool->GetSelection();
	wxString args;
	switch (i) {
	case 0: args="--leak-check=full --tool=memcheck"; break;
	case 1: args="--tool=cachegrind"; break;
	case 2: args="--tool=callgrind"; break;
	case 3: args="--tool=massif"; break;
	case 4: args="--tool=helgrind"; break;
	case 5: args="--tool=drd"; break;
	}
	if (suppressions->GetValue().Len()) 
		args<<" "<<mxUT::Split(suppressions->GetValue(),"--suppressions=");
	if (additional_args->GetValue().Len()) 
		args<<" "<<additional_args->GetValue();
	return args;
}
