#include "mxGotoLineWindow.h"
#include "mxBitmapButton.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxGotoLineWindow, wxDialog)

	EVT_BUTTON(wxID_OK,mxGotoLineWindow::OnGotoButton)
	EVT_BUTTON(wxID_CANCEL,mxGotoLineWindow::OnCancelButton)
	EVT_SCROLL(mxGotoLineWindow::OnScroll)
	EVT_CLOSE(mxGotoLineWindow::OnClose)
	EVT_TEXT(wxID_ANY,mxGotoLineWindow::OnTextChange)

END_EVENT_TABLE()

mxGotoLineWindow::mxGotoLineWindow(mxSource *a_source, wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(GOTOLINE_CAPTION,"Ir a linea"), pos, size, style) {
	
	source=a_source;

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	wxButton *goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 

	slider = new wxSlider(this,wxID_ANY,source->GetCurrentLine()+1,1,source->GetLineCount(),wxDefaultPosition,wxSize(100,1));
	text_ctrl = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_RIGHT,wxTextValidator(wxFILTER_NUMERIC));
	topSizer->Add(slider, sizers->BA5_Exp1);
	topSizer->Add(text_ctrl, sizers->BA5_Right);

	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);

	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG2(GOTOLINE_YOURE_HERE,"Estas en la linea <{1}> de <{2}>.",wxString()<<(source->GetCurrentLine()+1),wxString()<<source->GetLineCount())),sizers->BA10_Exp0);
	mySizer->Add(topSizer,sizers->Exp0);
	mySizer->Add(bottomSizer,sizers->Right);
	SetSizerAndFit(mySizer);

	text_ctrl->SetValue(wxString()<<source->GetCurrentLine()+1);
	text_ctrl->SetSelection(-1,-1);
	text_ctrl->SetFocus();
	ShowModal();

}

void mxGotoLineWindow::OnGotoButton(wxCommandEvent &event){
	source->PositionFromLine(slider->GetValue()-1);
	source->MarkError(slider->GetValue()-1);
	Destroy();
}
void mxGotoLineWindow::OnCancelButton(wxCommandEvent &event){
	Destroy();
}

void mxGotoLineWindow::OnClose(wxCloseEvent &event){
	Destroy();
}

void mxGotoLineWindow::OnScroll(wxScrollEvent &event) {
	text_ctrl->SetValue(wxString::Format("%d", slider->GetValue()));
}

void mxGotoLineWindow::OnTextChange(wxCommandEvent &event) {
	long int l; text_ctrl->GetValue().ToLong(&l);
	slider->SetValue(l);
}
