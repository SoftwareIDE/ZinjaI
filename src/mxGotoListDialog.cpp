#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "Language.h"
#include "mxSizers.h"
#include "mxBitmapButton.h"
#include "mxGotoListDialog.h"
#include "ids.h"
#include "mxBitmapButton.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxGotoListDialog, wxDialog)
	EVT_BUTTON(mxID_EDIT_GOTO_FILE,mxGotoListDialog::OnGotoFileButton)
	EVT_BUTTON(wxID_OK,mxGotoListDialog::OnGotoButton)
	EVT_BUTTON(wxID_CANCEL,mxGotoListDialog::OnCancelButton)
	EVT_CLOSE(mxGotoListDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxGotoListDialog::OnTextChange)
	EVT_TIMER(mxID_TIMER_INPUT, mxGotoListDialog::OnTimerInput)
	EVT_CHAR_HOOK(mxGotoListDialog::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxGotoListDialog::OnGotoButton)
	EVT_CHECKBOX(wxID_ANY,mxGotoListDialog::OnCaseCheck)
END_EVENT_TABLE()

mxGotoListDialog::mxGotoListDialog(wxWindow* parent, wxString window_title,
	                               wxString input_label, wxString extra_button_label) 
	: wxDialog(parent, wxID_ANY, window_title, wxDefaultPosition, wxDefaultSize,
	           wxALWAYS_SHOW_SB|wxALWAYS_SHOW_SB|wxDEFAULT_FRAME_STYLE|wxSUNKEN_BORDER)
{
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->Enable(false);
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,"");
	list_ctrl = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(450,300),0,nullptr,wxLB_SINGLE|wxLB_SORT);
	case_sensitive = new wxCheckBox(this,wxID_ANY,LANG(GOTOFUNCTION_CASE_SENSITIVE,"&Distinguir mayusculas y minusculas"));
	
	bottomSizer->Add(new wxButton(this,mxID_EDIT_GOTO_FILE,extra_button_label,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT),sizers->Center);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	
	mySizer->Add(label_ctrl = new wxStaticText(this,wxID_ANY,input_label),sizers->BLRT5_Exp0);
	mySizer->Add(text_ctrl,sizers->BA5_Exp0);
	mySizer->Add(list_ctrl,sizers->BA5_Exp1);
	mySizer->Add(case_sensitive,sizers->BA5_Exp0);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);
	
	text_ctrl->SetFocus();
	
	timer = new wxTimer(GetEventHandler(),mxID_TIMER_INPUT);
	
	goto_button->Enable(false);
	
}




void mxGotoListDialog::OnGotoButton(wxCommandEvent &event) {
	GotoNow();
}





void mxGotoListDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxGotoListDialog::OnClose(wxCloseEvent &event) {
	if (timer && timer->IsRunning()) timer->Stop();
	Destroy();
}

void mxGotoListDialog::OnTextChange(wxCommandEvent &event) {
	if (timer) timer->Start(500,true);
}

void mxGotoListDialog::OnTimerInput(wxTimerEvent &event) {
	SearchNow();
}

void mxGotoListDialog::SearchNow() {
	list_ctrl->Freeze(); list_ctrl->Clear();
	int n = OnSearch(text_ctrl->GetValue(),case_sensitive->IsChecked());
	list_ctrl->Thaw();
	if (n!=-1) {
		list_ctrl->SetSelection(n);
		goto_button->Enable(true);
	} else {
		goto_button->Enable(false);
	}
}

void mxGotoListDialog::OnCharHook (wxKeyEvent &event) {
	if (!list_ctrl->GetCount())  {
		event.Skip();
	} else if ( event.GetKeyCode() == WXK_UP ) {
		int sel = list_ctrl->GetSelection();
		sel--;
		if (sel==-1)
			sel=list_ctrl->GetCount()-1;
		list_ctrl->SetSelection(sel);
		list_ctrl->SetFirstItem(sel);
	} else if (event.GetKeyCode()==WXK_DOWN) {
		int sel = list_ctrl->GetSelection();
		sel++;
		if (sel==int(list_ctrl->GetCount()))
			sel=0;
		list_ctrl->SetSelection(sel);
		list_ctrl->SetFirstItem(sel);
	} else event.Skip();
}

void mxGotoListDialog::OnCaseCheck(wxCommandEvent &event) {
	if (timer) timer->Stop();
	wxTimerEvent evt;
	OnTimerInput(evt);
	text_ctrl->SetFocus();
}

void mxGotoListDialog::OnGotoFileButton (wxCommandEvent & event) {
	OnExtraButton();
}

void mxGotoListDialog::SetInputValue (wxString key) {
	text_ctrl->SetValue(key);
	text_ctrl->SetSelection(0,-1);
	SearchNow();
}

void mxGotoListDialog::HideInput() {
	if (timer) timer->Stop();
	case_sensitive->Hide(); 
	label_ctrl->Hide();
	text_ctrl->Hide();
	GetSizer()->Layout(); SetMinSize(wxDefaultSize); Fit();
	list_ctrl->SetFocus();
}

void mxGotoListDialog::GotoNow() {
	// update results if needed
	if (timer && timer->IsRunning()) {
		timer->Stop();
		wxTimerEvent evt;
		OnTimerInput(evt);
	}
	if (list_ctrl->IsEmpty()) return;
	OnGoto(list_ctrl->GetSelection(), list_ctrl->GetString(list_ctrl->GetSelection()));
}

void mxGotoListDialog::SetExtraButtonAccelerator (int flags, char key) {
	wxAcceleratorEntry accel(flags,key,mxID_EDIT_GOTO_FILE);
	SetAcceleratorTable(wxAcceleratorTable(1,&accel));
}

