#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "mxTextDialog.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxSizers.h"

BEGIN_EVENT_TABLE(mxTextDialog, wxDialog)
	
	EVT_BUTTON(wxID_OK,mxTextDialog::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxTextDialog::OnCancelButton)
	EVT_CHAR_HOOK(mxTextDialog::OnCharHook)
	EVT_CLOSE(mxTextDialog::OnClose)
	
END_EVENT_TABLE()
	
mxTextDialog::mxTextDialog(wxWindow *parent, wxString message, wxString title, wxString value) : wxDialog(parent,wxID_ANY,title,wxDefaultPosition,wxDefaultSize) {
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
//	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	mxBitmapButton *btn_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	btn_cancel->SetMinSize(wxSize(btn_cancel->GetSize().GetWidth()<90?90:btn_cancel->GetSize().GetWidth(),btn_cancel->GetSize().GetHeight()<30?30:btn_cancel->GetSize().GetHeight()));
	bottomSizer->Add( btn_cancel , sizers->BA5);
	SetEscapeId(wxID_CANCEL);
	btn_cancel->SetFocus();

	mxBitmapButton *btn_ok = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	btn_ok->SetMinSize(wxSize(btn_ok->GetSize().GetWidth()<90?90:btn_ok->GetSize().GetWidth(),btn_ok->GetSize().GetHeight()<30?30:btn_ok->GetSize().GetHeight()));
	bottomSizer->Add( btn_ok , sizers->BA5);
	SetAffirmativeId(wxID_OK);
	
//	if (style&mxMD_ERROR)
//		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.error) , sizers->BA10_Right);
//	if (style&mxMD_WARNING)
//		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.warning), sizers->BA10_Right);
//	if (style&mxMD_QUESTION)
//		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.question), sizers->BA10_Right);
//	if (style&mxMD_INFO)
//		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.info), sizers->BA10_Right);
	
//	topSizer->Add( new wxStaticText(this,wxID_ANY,message), sizers->BA10_Exp1);
	mySizer->Add( new wxStaticText(this,wxID_ANY,message), sizers->BA10_Exp1);
	m_text=new wxTextCtrl(this,wxID_ANY,value);
	mySizer->Add( m_text, sizers->BA10_Exp1);
	
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	m_text->SetFocus();
	
}


void mxTextDialog::OnOkButton(wxCommandEvent &event){
	m_answer = m_text->GetValue();
	EndModal(wxID_OK);
}

void mxTextDialog::OnCancelButton(wxCommandEvent &event){
	EndModal(wxID_CANCEL);
}

void mxTextDialog::OnClose(wxCloseEvent &event){
	Destroy();
}

void mxTextDialog::OnCharHook(wxKeyEvent &event) {
	if (event.GetKeyCode()==WXK_RETURN) {
		m_answer = m_text->GetValue();
		EndModal(wxID_OK);
	} else
		event.Skip();
}



wxString mxTextDialog::GetAnswer() {
	return m_answer;
}

wxString mxGetTextFromUser(wxString text, wxString title, wxString value, wxWindow *parent) {
	mxTextDialog md(parent,text,title,value);
	if (md.ShowModal()==wxID_OK) return md.GetAnswer(); else return "";
}

bool mxGetTextFromUser(wxString &ans, wxString text, wxString title, wxString value, wxWindow *parent) {
	mxTextDialog md(parent,text,title,value);
	if (md.ShowModal()==wxID_OK) {
		ans=md.GetAnswer();
		return true;
	} else {
		ans="";
		return false;
	}
}
