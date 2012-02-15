#include "mxToolbarEditor.h"
#include "mxPreferenceWindow.h"
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include <wx/stattext.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include <wx/checkbox.h>
#include "mxArt.h"
#include "mxMainWindow.h"
#include "Language.h"
using namespace std;

BEGIN_EVENT_TABLE(mxToolbarEditor, wxDialog)
	EVT_BUTTON(wxID_OK,mxToolbarEditor::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxToolbarEditor::OnButtonCancel)
END_EVENT_TABLE()

mxToolbarEditor::mxToolbarEditor(wxToolBar *atoolbar, wxString name):wxDialog(preference_window,wxID_ANY,name,wxDefaultPosition,wxSize(350,400),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	
	ipre=DIR_PLUS_FILE(wxString()<<config->Toolbars.icon_size,_T(""));
	
	toolbar = atoolbar;
	
	count = 0;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	wxButton *goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);

	scroll = new wxScrolledWindow(this,wxID_ANY);
	sizer = new wxBoxSizer(wxVERTICAL);
	scroll->SetScrollRate(10,10);
	scroll->SetSizer(sizer);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(TOOLBAREDITOR_SELECT_ICONS,"Seleccione los iconos que desea en la barra:")),sizers->BLRT5_Exp0);
	mySizer->Add(scroll,sizers->BA5_Exp1);
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	SetSizer(mySizer);
	
}

mxToolbarEditor::~mxToolbarEditor() {
	
}

void mxToolbarEditor::Add(wxString name, wxString file, bool &config_entry) {
	if (!wxFileName::FileExists(SKIN_FILE(ipre+file))) return;
	wxBoxSizer *btsizer = new wxBoxSizer(wxHORIZONTAL);
	btsizer->Add(10,10);
	btsizer->Add(new wxBitmapButton(scroll,wxID_ANY,wxBitmap(SKIN_FILE(ipre+file),wxBITMAP_TYPE_PNG)),sizers->BLRT5);
	booleans[count] = &config_entry;
	checkboxs[count] = new wxCheckBox(scroll,wxID_ANY,name);
	btsizer->Add(checkboxs[count],sizers->BA5);
	count++;
	sizer->Add(btsizer);
}

void mxToolbarEditor::OnButtonOk(wxCommandEvent &evt) {
	for (int i=0;i<count;i++)
		(*booleans[i])=checkboxs[i]->GetValue();
	main_window->CreateToolbars(toolbar);
	Hide();
}

void mxToolbarEditor::OnButtonCancel(wxCommandEvent &evt) {
	Hide();
}

void mxToolbarEditor::ShowUp() {
	for (int i=0;i<count;i++)
		checkboxs[i]->SetValue(*booleans[i]);
	scroll->SetFocus();
	ShowModal();
}
