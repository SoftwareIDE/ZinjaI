#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/stattext.h>
#include "mxToolchainOptions.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxSizers.h"
#include "ProjectManager.h"

BEGIN_EVENT_TABLE(mxToolchainOptions,wxDialog)
	EVT_BUTTON(wxID_OK,mxToolchainOptions::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxToolchainOptions::OnCancelButton)
END_EVENT_TABLE()

mxToolchainOptions::mxToolchainOptions(wxWindow *parent, wxString toolchain_fname, project_configuration *conf):wxDialog(parent,wxID_ANY,"Toolchain Options",wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	
	const Toolchain &tc=Toolchain::GetInfo(toolchain_fname);
	
	configuration=conf;
	wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	
	wxSizerFlags s1=sizers->BA5_Exp0; s1.DoubleBorder(wxLEFT);
	wxSizerFlags s2=sizers->BLRB5_Exp0; s2.DoubleBorder(wxLEFT);
	
	int args_count=0;
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) {
		if (tc.arguments[i][0]=="") { texts[i]=nullptr; continue; }
		args_count++;
		wxStaticText *st1=new wxStaticText(this,wxID_ANY,tc.arguments[i][0]); 
		texts[i]=new wxTextCtrl(this,wxID_ANY,configuration->toolchain_arguments[i]);
		wxStaticText *st2=new wxStaticText(this,wxID_ANY,wxString("${DEFAULT}=")<<tc.arguments[i][1]); 
		sizer->Add(st1,sizers->BLRT5_Exp0);
		sizer->Add(texts[i],s1);
		sizer->Add(st2,s2);
	}
	
	if (args_count==0) sizer->Add(new wxStaticText(this,wxID_ANY,"No hay argumentos configurables para este toolchain."),sizers->BA10_Exp0);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
//	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
//	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	sizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(sizer);
}

void mxToolchainOptions::OnOkButton (wxCommandEvent & evnt) {
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) { 
		if (texts[i]) configuration->toolchain_arguments[i]=texts[i]->GetValue();
	}
	EndModal(1);
}

void mxToolchainOptions::OnCancelButton (wxCommandEvent & evnt) {
	EndModal(0);
}

