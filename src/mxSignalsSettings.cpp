#include "mxSignalsSettings.h"
#include "DebugManager.h"
#include "mxSizers.h"
#include <wx/checkbox.h>
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxHelpWindow.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxSignalsSettings,wxDialog)
	EVT_BUTTON(wxID_OK,mxSignalsSettings::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxSignalsSettings::OnButtonCancel)
	EVT_BUTTON(wxID_HELP,mxSignalsSettings::OnButtonHelp)
END_EVENT_TABLE()

mxSignalsSettings::mxSignalsSettings():wxDialog(main_window,wxID_ANY,"Singnals handling settings",wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	
	checks_pass=checks_stop=nullptr;
	if (!debug->GetSignals(vsig)) return;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	wxScrolledWindow *scroll = new wxScrolledWindow(this,wxID_ANY);
	wxFlexGridSizer *sizer = new wxFlexGridSizer(3);
	sizer->SetFlexibleDirection(wxBOTH);
//	sizer->Add(new wxStaticText(scroll,wxID_ANY,"Señal"));
//	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_FRONT,"Recibir")),sizers->BA5_Center);
//	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_FRONT,"Ejecución")),sizers->BA5_Center);
	scroll->SetScrollRate(10,10);
	scroll->SetSizer(sizer);
//	sizer->AddGrowableCol(0);
	
	checks_stop = new wxCheckBox*[vsig.size()];
	checks_pass = new wxCheckBox*[vsig.size()];
	for(unsigned int i=0;i<vsig.size();i++) { 
		wxStaticText *lab=new wxStaticText(scroll,wxID_ANY,vsig[i].name);
		lab->SetToolTip(vsig[i].name+": "+vsig[i].description);
		sizer->Add(lab,sizers->BA5_Center);
		checks_pass[i] = new wxCheckBox(scroll,wxID_ANY,"recibir");
		checks_pass[i]->SetToolTip(wxString("permitir que el proceso reciba la señal ")<<vsig[i].name);
		checks_pass[i]->SetValue(vsig[i].pass);
		sizer->Add(checks_pass[i],sizers->BA5_Center);
		checks_stop[i] = new wxCheckBox(scroll,wxID_ANY,"pausar");
		checks_stop[i]->SetToolTip(wxString("pausar la ejecución cuando se recibe la señal ")<<vsig[i].name);
		checks_stop[i]->SetValue(vsig[i].stop);
		sizer->Add(checks_stop[i],sizers->BA5_Center);
	}
	mySizer->Add(scroll,sizers->BA5_Exp1);
	
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *help_button = new wxBitmapButton (this, wxID_HELP, *bitmaps->buttons.help); 
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
//	ok_button->SetMinSize(wxSize(ok_button->GetSize().GetWidth()<80?80:ok_button->GetSize().GetWidth(),ok_button->GetSize().GetHeight()));
	ok_button->SetDefault(); 
	bottomSizer->Add(help_button,sizers->BA5);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	
	SetSizer(mySizer);
	ShowModal();
}

mxSignalsSettings::~mxSignalsSettings() {
	delete [] checks_pass;
	delete [] checks_stop;
}

void mxSignalsSettings::OnButtonOk (wxCommandEvent & evt) {
	for(unsigned int i=0;i<vsig.size();i++) { 
		if (checks_pass[i]->GetValue()!=vsig[i].pass || checks_stop[i]->GetValue()!=vsig[i].stop) {
			vsig[i].pass=checks_pass[i]->GetValue();
			vsig[i].stop=checks_stop[i]->GetValue();
			debug->SetSignalHandling(vsig[i],i);
		}
	}
	EndModal(1);
}

void mxSignalsSettings::OnButtonCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxSignalsSettings::OnButtonHelp (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("signals.html",this);	
}

