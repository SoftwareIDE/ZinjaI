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

mxMessageDialog::mxMessageDialog(wxWindow *parent, wxString message, wxString title, unsigned int style, wxString check, bool bval) : wxDialog(parent,wxID_ANY,title,wxDefaultPosition,wxSize(400,100)) {
	CommonConstructor(parent,message,title,style,check,bval);
}

mxMessageDialog::mxMessageDialog(wxString message, wxString title, unsigned int style, wxString check, bool bval) : wxDialog(main_window,wxID_ANY,title,wxDefaultPosition,wxSize(400,100)) {
	CommonConstructor(main_window,message,title,style,check,bval);
}

void mxMessageDialog::CommonConstructor(wxWindow *parent, wxString message, wxString title, unsigned int style, wxString check, bool bval) {
	
	if (splash) splash->Close();

	buttons=style;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	if (!(style&(mxMD_YES_NO_CANCEL|mxMD_OK))) style|=mxMD_OK;
	
	if (style&mxMD_NO) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_NO,bitmaps->buttons.cancel, LANG(GENERAL_NO_BUTTON," &No "));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetEscapeId(wxID_NO);
		btn->SetFocus();
	}
	
	if (style&mxMD_CANCEL) {
		mxBitmapButton *btn;
		if (style&mxMD_YES || style&mxMD_NO)
			btn = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.stop, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
		else
			btn = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetEscapeId(wxID_CANCEL);
		btn->SetFocus();
	}
	
	if (style&mxMD_YES) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_YES,bitmaps->buttons.ok, LANG(GENERAL_YES_BUTTON," &Si "));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetAffirmativeId(wxID_YES);
		btn->SetFocus();
	}
	
	if (style&mxMD_OK) {
		mxBitmapButton *btn = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
		btn->SetMinSize(wxSize(btn->GetSize().GetWidth()<90?90:btn->GetSize().GetWidth(),btn->GetSize().GetHeight()<30?30:btn->GetSize().GetHeight()));
		bottomSizer->Add( btn , sizers->BA5);
		SetAffirmativeId(wxID_OK);
		btn->SetFocus();
	}
	
	if (style&mxMD_ERROR)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.error) , sizers->BA10_Right);
	if (style&mxMD_WARNING)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.warning), sizers->BA10_Right);
	if (style&mxMD_QUESTION)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.question), sizers->BA10_Right);
	if (style&mxMD_INFO)
		topSizer->Add( new wxStaticBitmap(this,wxID_ANY,*bitmaps->icons.info), sizers->BA10_Right);
		

	if ( check.Len() ) {
		wxBoxSizer *inSizer = new wxBoxSizer(wxVERTICAL);
		inSizer->Add( new wxStaticText(this,wxID_ANY,message), sizers->BA10_Exp1);
		checkbox = new wxCheckBox(this, wxID_ANY, check);
		checkbox->SetValue(bval);
		inSizer->Add(checkbox,sizers->BTR10_Right);
		topSizer->Add(inSizer,sizers->Exp1);
	} else {
		checkbox = NULL;
		topSizer->Add( new wxStaticText(this,wxID_ANY,message), sizers->BA10_Exp1);
	}
	
	if (style&mxMD_CANCEL) 
		SetReturnCode(mxMD_CANCEL);
	else if(style&mxMD_NO)
		SetReturnCode(mxMD_NO);
	else 
		SetReturnCode(mxMD_CANCEL);
	
	mySizer->Add(topSizer,sizers->Exp0);
	
	mySizer->InsertStretchSpacer(1);
	
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	SetFocusFromKbd();
	
}


void mxMessageDialog::OnOkButton(wxCommandEvent &event){
	if (checkbox && checkbox->GetValue())
		EndModal(mxMD_OK|mxMD_CHECKED);
	else
		EndModal(mxMD_OK);
}

void mxMessageDialog::OnCancelButton(wxCommandEvent &event){
	if (checkbox && checkbox->GetValue())
		EndModal(mxMD_CANCEL|mxMD_CHECKED);
	else
		EndModal(mxMD_CANCEL);
}

void mxMessageDialog::OnYesButton(wxCommandEvent &event){
	if (checkbox && checkbox->GetValue())
		EndModal(mxMD_YES|mxMD_CHECKED);
	else
		EndModal(mxMD_YES);
}

void mxMessageDialog::OnNoButton(wxCommandEvent &event){
	if (checkbox && checkbox->GetValue())
		EndModal(mxMD_NO|mxMD_CHECKED);
	else
		EndModal(mxMD_NO);
}

void mxMessageDialog::OnClose(wxCloseEvent &event){
	Destroy();
}

void mxMessageDialog::OnCharHook(wxKeyEvent &event) {
	int c = event.GetKeyCode();
	if ( (c|32)=='a' && (buttons&mxMD_OK) )
		EmulateButtonClickIfPresent(wxID_OK);
	else if ( (c|32)=='s' && (buttons&mxMD_YES) )
		EmulateButtonClickIfPresent(wxID_YES);
	else if ( (c|32)=='c' && (buttons&mxMD_CANCEL) )
		EmulateButtonClickIfPresent(wxID_CANCEL);
	else if ( (c|32)=='n' && (buttons&mxMD_NO) )
		EmulateButtonClickIfPresent(wxID_NO);
	else
		event.Skip();
}
