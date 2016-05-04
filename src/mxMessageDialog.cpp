#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxSplashScreen.h"
#include "Language.h"
#include "mxBitmapButton.h"
#include "mxMainWindow.h"
using namespace std;

BEGIN_EVENT_TABLE(mxMessageDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxMessageDialog::OnOkButton)
	EVT_BUTTON(wxID_YES,mxMessageDialog::OnYesButton)
	EVT_BUTTON(wxID_NO,mxMessageDialog::OnNoButton)
	EVT_BUTTON(wxID_CANCEL,mxMessageDialog::OnCancelButton)
	EVT_CHAR_HOOK(mxMessageDialog::OnCharHook)
	EVT_CLOSE(mxMessageDialog::OnClose)
END_EVENT_TABLE()

mxMessageDialog::mxMessageDialog(wxWindow *parent, wxString message) 
	: wxDialog(parent,wxID_ANY,"ZinjaI",wxDefaultPosition,wxSize(400,100)),
	  m_ok(true), m_cancel(false), m_yes(false), m_no(false), m_message(message), 
	  m_icon(mxMDNull), m_check1_ctrl(nullptr), m_check2_ctrl(nullptr) 
{ 
	
}
	
mxMessageDialog::mdAns mxMessageDialog::Run() {
	
	if (g_splash) g_splash->Close();

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	if (m_no) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_NO,bitmaps->buttons.cancel, LANG(GENERAL_NO_BUTTON," &No "));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetEscapeId(wxID_NO);
		btn->SetFocus();
	}
	
	if (m_cancel) {
		mxBitmapButton *btn;
		if (m_yes || m_no)
			btn = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.stop, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
		else
			btn = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetEscapeId(wxID_CANCEL);
		btn->SetFocus();
	}
	
	if (m_yes) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_YES,bitmaps->buttons.ok, LANG(GENERAL_YES_BUTTON," &Si "));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetAffirmativeId(wxID_YES);
		btn->SetFocus();
	}
	
	if (m_ok) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetAffirmativeId(wxID_OK);
		btn->SetFocus();
	}
	
	if (m_icon==mxMDError)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.error) , sizers->BA10_Right);
	if (m_icon==mxMDWarning)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.warning), sizers->BA10_Right);
	if (m_icon==mxMDQuestion)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.question), sizers->BA10_Right);
	if (m_icon==mxMDInfo)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.info), sizers->BA10_Right);
		

	if ( m_check1_str.Len() || m_check2_str.Len() ) {
		wxBoxSizer *inSizer = new wxBoxSizer(wxVERTICAL);
		inSizer->Add(new wxStaticText(this,wxID_ANY,m_message), sizers->BA10_Exp1);
		if (m_check1_str.Len()) {
			m_check1_ctrl = new wxCheckBox(this, wxID_ANY, m_check1_str);
			m_check1_ctrl->SetValue(m_check1_val);
			inSizer->Add(m_check1_ctrl,sizers->BTR10_Right);
		}
		if (m_check2_str.Len()) {
			m_check2_ctrl = new wxCheckBox(this, wxID_ANY, m_check2_str);
			m_check2_ctrl->SetValue(m_check2_val);
			inSizer->Add(m_check2_ctrl,sizers->BTR10_Right);
		}
		topSizer->Add(inSizer,sizers->Exp1);
	} else {
		topSizer->Add(new wxStaticText(this,wxID_ANY,m_message), sizers->BA10_Exp1);
	}
	
	SetReturnCode(0);
	
	mySizer->Add(topSizer,sizers->Exp0);
	
	mySizer->InsertStretchSpacer(1);
	
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	
	if (m_title.Len()) SetTitle(m_title);
	
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	SetFocusFromKbd();
	ShowModal();
	return m_result;
}


void mxMessageDialog::OnOkButton(wxCommandEvent &event){
	m_result.ok = true;
	m_result.check1 = (m_check1_ctrl && m_check1_ctrl->GetValue());
	m_result.check2 = (m_check2_ctrl && m_check2_ctrl->GetValue());
	EndModal(1);
}

void mxMessageDialog::OnCancelButton(wxCommandEvent &event){
	m_result.cancel = true;
	m_result.check1 = (m_check1_ctrl && m_check1_ctrl->GetValue());
	m_result.check2 = (m_check2_ctrl && m_check2_ctrl->GetValue());
	EndModal(0);
}

void mxMessageDialog::OnYesButton(wxCommandEvent &event){
	m_result.yes = true;
	m_result.check1 = (m_check1_ctrl && m_check1_ctrl->GetValue());
	m_result.check2 = (m_check2_ctrl && m_check2_ctrl->GetValue());
	EndModal(1);
}

void mxMessageDialog::OnNoButton(wxCommandEvent &event){
	m_result.no = true;
	m_result.check1 = (m_check1_ctrl && m_check1_ctrl->GetValue());
	m_result.check2 = (m_check2_ctrl && m_check2_ctrl->GetValue());
	EndModal(0);
}

void mxMessageDialog::OnClose(wxCloseEvent &event){
	m_result.closed = true;
	if (!(m_result.ok||m_result.yes)) m_result.cancel = m_result.no = true;
	Destroy();
}

void mxMessageDialog::OnCharHook(wxKeyEvent &event) {
	int c = event.GetKeyCode();
	if ( (c|32)=='o' && (m_ok) )
		EmulateButtonClickIfPresent(wxID_OK);
	if ( (c|32)=='a' && (m_ok) )
		EmulateButtonClickIfPresent(wxID_OK);
	else if ( (c|32)=='y' && (m_yes) )
		EmulateButtonClickIfPresent(wxID_YES);
	else if ( (c|32)=='s' && (m_yes) )
		EmulateButtonClickIfPresent(wxID_YES);
	else if ( (c|32)=='c' && (m_cancel) )
		EmulateButtonClickIfPresent(wxID_CANCEL);
	else if ( (c|32)=='n' && (m_no) )
		EmulateButtonClickIfPresent(wxID_NO);
	else
		event.Skip();
}

