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

#warning FALTA TRADUCIR

mxToolchainOptions::mxToolchainOptions(wxWindow *parent, wxString toolchain_fname, project_configuration *conf)
	: mxDialog(parent,"Toolchain Options",false), m_configuration(conf) 
{
	const Toolchain &tc=Toolchain::GetInfo(toolchain_fname);
	
	CreateSizer sizer(this);
	
	int args_count=0;
	wxSizerFlags s1=sizers->BA5_Exp0; s1.DoubleBorder(wxLEFT);
	wxSizerFlags s2=sizers->BLRB5_Exp0; s2.DoubleBorder(wxLEFT);
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) {
		if (tc.arguments[i][0]=="") { texts[i]=nullptr; continue; }
		args_count++;
		wxStaticText *st1=new wxStaticText(this,wxID_ANY,tc.arguments[i][0]); 
		texts[i]=new wxTextCtrl(this,wxID_ANY,m_configuration->toolchain_arguments[i]);
		wxStaticText *st2=new wxStaticText(this,wxID_ANY,wxString("${DEFAULT}=")<<tc.arguments[i][1]); 
		sizer.GetSizer()->Add(st1,sizers->BLRT5_Exp0);
		sizer.GetSizer()->Add(texts[i],s1);
		sizer.GetSizer()->Add(st2,s2);
	}
	
	if (args_count==0) sizer.BeginLabel("No hay argumentos configurables para este toolchain.").EndLabel();
	
	sizer.BeginBottom().Ok().Cancel().EndBottom(this);
	sizer.SetAndFit();
}

void mxToolchainOptions::OnOkButton (wxCommandEvent & evnt) {
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) { 
		if (texts[i]) m_configuration->toolchain_arguments[i]=texts[i]->GetValue();
	}
	EndModal(1);
}

void mxToolchainOptions::OnCancelButton (wxCommandEvent & evnt) {
	EndModal(0);
}

