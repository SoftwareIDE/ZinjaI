#include <wx/dcclient.h>
#include <wx/scrolwin.h>
#include "mxFlowWindow.h"
#include "mxFlowCanvas.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxFlowWindow, wxFrame)
	EVT_CLOSE(mxFlowWindow::OnClose)
	EVT_CHAR_HOOK(mxFlowWindow::OnCharHook)
END_EVENT_TABLE()


mxFlowWindow::mxFlowWindow(mxSource *src, wxString title) : wxFrame (nullptr,wxID_ANY,"",wxDefaultPosition,wxSize(500,500)) {
	source = src;
	int s,ps = source->GetSelectionStart()<source->GetSelectionEnd()?source->GetSelectionStart():source->GetSelectionEnd();
	int p=ps;
	if (source->GetCharAt(p)=='}') {
			if ((p=source->BraceMatch(p))==wxSTC_INVALID_POSITION) {
				mxMessageDialog(main_window,"El cursor no se encuentra en una funcion o la sintaxis no es correcta")
					.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
				Destroy();
				return;
			}
	}
	while (p>0 && (!FC_IS('{') || FC_SHOULD_IGNORE) ) {
		if (FC_IS('}')) {
			p=source->BraceMatch(p);
			if (p==wxSTC_INVALID_POSITION || source->GetCharAt(p)!='{') {
				mxMessageDialog(main_window,"El cursor no se encuentra en una funcion o la sintaxis no es correcta")
					.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
				Destroy();
				return;
			}
		}
		p--;
	}
	int pe = source->BraceMatch(ps=p);
	if (pe==wxSTC_INVALID_POSITION || source->GetCharAt(ps)!='{') {
		mxMessageDialog (main_window,"El cursor no se encuentra en una funcion o la sintaxis no es correcta")
			.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		Destroy();
		return;
	}
	source->SetSelection(ps,pe+1);
	SetTitle(title<<" "<<source->LineFromPosition(ps)+1<<_T("-")<<source->LineFromPosition(pe)+1);
	canvas = new mxFlowCanvas (this,source);
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(canvas,sizers->Exp1);
	SetSizer(sizer);
	Show();

}

void mxFlowWindow::OnCharHook(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_NUMPAD_ADD) {
		canvas->ChangeScale(1.25);
	} else if (evt.GetKeyCode()==WXK_NUMPAD_SUBTRACT) {
		canvas->ChangeScale(.75);
	}
}

void mxFlowWindow::OnClose(wxCloseEvent &evt){
	Destroy();
}
