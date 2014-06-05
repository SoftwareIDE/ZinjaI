#include "mxListSharedWindow.h"
#include "mxSource.h"
#include "ids.h"
#include "ShareManager.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxListSharedWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxListSharedWindow::OnCloseButton)
	EVT_BUTTON(wxID_CANCEL,mxListSharedWindow::OnStopSharingButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxListSharedWindow::OnHelpButton)
	EVT_CLOSE(mxListSharedWindow::OnClose)
END_EVENT_TABLE()

mxListSharedWindow::mxListSharedWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(LISTSHARED_CAPTION,"Lista de Archivos Compartidos"), pos, size, style) {
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer= new wxBoxSizer(wxHORIZONTAL);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, LANG(LISTSHARED_LIST_LABEL,"Lista de fuentes compartidos: "), wxDefaultPosition, wxDefaultSize, 0), sizers->BLRT5);
	
	wxArrayString array;
	if (!share) share = new ShareManager();
	share->GetSharedList(array);
	list = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(150,150),array);
	
	mySizer->Add(list,sizers->BA5_Exp1);
	
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *unshare_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(LISTSHARED_REMOVE,"&Quitar de la lista")); 
	wxButton *close_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(unshare_button,sizers->BA5);
	bottomSizer->Add(close_button,sizers->BA5);
	
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);
	
	ShowModal();
	list->SetFocus();
	
}

void mxListSharedWindow::OnStopSharingButton(wxCommandEvent &event){
	bool any=false;
	for (int i=list->GetCount()-1;i>=0;i--) {
		if (list->IsSelected(i)) {
			share->Delete(list->GetString(i));
			list->Delete(i);
			any=true;
		}
	}
	if (!any) 
		mxMessageDialog(this,LANG(LISTSHARED_SELECT_SOMETHING,"Debe seleccionar uno o más fuentes compartidos de la lista."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
}

void mxListSharedWindow::OnHelpButton(wxCommandEvent &event){
	SHOW_HELP_FROM_MODAL("list_shared.html");
}

void mxListSharedWindow::OnCloseButton(wxCommandEvent &event){
	Close();
}

void mxListSharedWindow::OnClose(wxCloseEvent &event){
	Hide();
}


