#include <wx/wx.h>
#include "mxOpenSharedWindow.h"
#include "mxSource.h"
#include "ids.h"
#include "mxUtils.h"
#include "ShareManager.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Language.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxOpenSharedWindow, wxDialog)

	EVT_BUTTON(mxID_SHARE_GET_LIST,mxOpenSharedWindow::OnGetListButton)
	EVT_BUTTON(wxID_OK,mxOpenSharedWindow::OnGetSourceButton)
	EVT_BUTTON(wxID_CANCEL,mxOpenSharedWindow::OnCloseButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxOpenSharedWindow::OnHelpButton)
	EVT_CLOSE(mxOpenSharedWindow::OnClose)
	EVT_CHAR_HOOK(mxOpenSharedWindow::OnCharHook)
	EVT_LISTBOX(mxID_SHARE_CLIENTS_LIST,mxOpenSharedWindow::OnClientList)
	EVT_LISTBOX(mxID_SHARE_FILES_LIST,mxOpenSharedWindow::OnGetSourceButton)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxOpenSharedWindow::OnGetSourceButton)

END_EVENT_TABLE()

mxOpenSharedWindow *mxOpenSharedWindow::open_shared = nullptr;
	
mxOpenSharedWindow::mxOpenSharedWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(OPENSHARED_CAPTION,"Abrir Compartido"), pos, size, style) {

	if (!g_share_manager) g_share_manager = new ShareManager();
	
	clients_list = new wxListBox(this,mxID_SHARE_CLIENTS_LIST,wxDefaultPosition, wxSize(200,150));
	wxStaticText *help_text_1 = new wxStaticText(this,wxID_ANY,LANG(OPENSHARED_STEP_1,"1) Dirección de origen:"));
	hostname = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	wxBitmapButton *button_step_1 = new wxBitmapButton (this, mxID_SHARE_GET_LIST, *bitmaps->buttons.next);
	
	wxBoxSizer *left_aux_sizer= new wxBoxSizer(wxHORIZONTAL);
	left_aux_sizer->Add(hostname,sizers->Exp1);
	left_aux_sizer->Add(button_step_1);
	
	wxBoxSizer *left_sizer = new wxBoxSizer(wxVERTICAL);
	left_sizer->Add(help_text_1,sizers->BA5_Exp0);
	left_sizer->Add(clients_list,sizers->BA5_Exp1);
	left_sizer->Add(left_aux_sizer,sizers->BA5_Exp0);
	
	
	
	wxStaticText *help_text_2 = new wxStaticText(this,wxID_ANY,LANG(OPENSHARED_STEP_2,"2) Seleccion un archivo:"));
	files_list = new wxListBox(this,mxID_SHARE_FILES_LIST,wxDefaultPosition, wxSize(200,150));
	
	wxBoxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	right_sizer->Add(help_text_2,sizers->BA5_Exp0);
	right_sizer->Add(files_list,sizers->BA5_Exp1);
	
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	top_sizer->Add(left_sizer,sizers->Exp1);
	top_sizer->Add(right_sizer,sizers->Exp1);
	
	
	wxButton *close_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CLOSE_BUTTON," &Cerrar ")); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *source_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(OPENSHARED_GET_SOURCE_BUTTON,"&Obtener fuente"));

	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	bottom_sizer->Add(help_button,sizers->BA5_Exp0);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(close_button,sizers->BA5);
	bottom_sizer->Add(source_button,sizers->BA5);

	
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(top_sizer,sizers->Exp1);
	main_sizer->Add(bottom_sizer,sizers->BA5_Exp0);
	SetSizerAndFit(main_sizer);

	hostname->SetFocus();

}

void mxOpenSharedWindow::OnGetListButton(wxCommandEvent &event){
	if (!(hostname->GetValue().Len())) {
		mxMessageDialog(this,LANG(OPENSHARED_ENTER_HOSTNAME_OR_IP,"Debe introducir primero el nombre de host o el IP de la pc que esta compartiendo el archivo."))
			.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
	} else {
		if (g_share_manager->GetList(hostname->GetValue(),files_list)) {
			files_list->SetFocus();
		} else {
			mxMessageDialog(this,LANG(OPENSHARED_HOST_NOT_FOUND,"No se encontro el host especificado."))
				.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		}
	}
}

void mxOpenSharedWindow::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("open_shared.html");
}

void mxOpenSharedWindow::OnCloseButton(wxCommandEvent &event){
	Close();
}

void mxOpenSharedWindow::OnGetSourceButton(wxCommandEvent &event){
	if (!files_list->GetStringSelection().Len() ) {
		if (files_list->GetCount()==0) {
			mxMessageDialog(this,LANG(OPENSHARED_GET_SHARED_LIST_FIRST,"Debe obtener la lista de fuentes compartidas primero. Para ello ingrese el nombre de host\no el IP de la pc que esta compartiendo el archivo y seleccione \"Actualizar lista\"."))
				.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
			hostname->SetFocus();
		} else {
			mxMessageDialog(this,LANG(OPENSHARED_SELECT_ONE_FIRST,"Debe seleccionar un fuente de la lista primero."))
				.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
			files_list->SetFocus();
		}
	} else {
		if (g_share_manager->AskFor(files_list->GetStringSelection())) {
			Close();
		} else {
			mxMessageDialog(this,LANG(OPENSHARED_HOST_NOT_FOUND,"No se encontro el host especificado."))
				.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		}
	}
}

void mxOpenSharedWindow::OnClose(wxCloseEvent &event){
	Hide();
}

void mxOpenSharedWindow::OnCharHook (wxKeyEvent &event) {
	if ( event.GetKeyCode() == WXK_ESCAPE ) {
		Close();
	} else if (event.GetKeyCode()==WXK_RETURN || event.GetKeyCode()==WXK_NUMPAD_ENTER) {
		if (FindFocus()==hostname) {
			wxCommandEvent evt;
			OnGetListButton(evt);
		} else if (FindFocus()==files_list) {
			wxCommandEvent evt;
			OnGetSourceButton(evt);
		}
	} else event.Skip();
}

void mxOpenSharedWindow::OnClientList (wxCommandEvent & event) {
	hostname->SetValue(clients_list->GetStringSelection().AfterLast('(').BeforeFirst(')'));
	OnGetListButton(event);
}

void mxOpenSharedWindow::AddClient (const wxString & name, const wxString & ip) {
	if (!open_shared) return;
	wxListBox *clients_list = open_shared->clients_list;
	for(unsigned int i=0;i<clients_list->GetCount();i++) { 
		wxString s = open_shared->clients_list->GetString(i).AfterLast('(').BeforeFirst(')');
		if (s==ip) { open_shared->clients_list->SetString(i,name+" ("+ip+")"); return; }
	}
	clients_list->Append(name+" ("+ip+")");
}

void mxOpenSharedWindow::Show ( ) {
	if (!open_shared) open_shared = new mxOpenSharedWindow(main_window);
	open_shared->DoShow(); 
}

void mxOpenSharedWindow::DoShow ( ) {
	wxDialog::Show();
	g_share_manager->StartBroadcastListener(true);
}

