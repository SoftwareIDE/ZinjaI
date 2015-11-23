#include "mxBacktraceHistory.h"

// por si mañana quiero que esta clase herede de un panel y contenga la lista
#define lista this 
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxBacktraceHistory,wxListBox)
	EVT_LISTBOX(wxID_ANY, mxBacktraceHistory::OnSelect)
END_EVENT_TABLE()

mxBacktraceHistory::mxBacktraceHistory ( ) 
	: wxListBox(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,nullptr, wxLB_SINGLE)
{
	wxSize sz(100,150);
	logging = true;
	main_window->aui_manager.AddPane(this,wxAuiPaneInfo().Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption("Backtrace History").Show().BestSize(sz));
	main_window->aui_manager.Update();	
	debug->UpdateBacktrace(false,true);
}

void mxBacktraceHistory::OnBacktraceUpdated(bool was_running) {
	if (!was_running || !logging) return;
	data.Add(debug->current_stack);
	lista->Append(debug->GetCurrentLocation());
}

void mxBacktraceHistory::OnPopupMenu (wxMouseEvent & evt) {
	
}

void mxBacktraceHistory::OnClear (wxCommandEvent & evt) {
	data.Clear();
	lista->Clear();
}

void mxBacktraceHistory::OnToggleLogging (wxCommandEvent & evt) {
	logging = !logging;
}

void mxBacktraceHistory::OnSelect (wxCommandEvent & evt) {
	evt.Skip();
	int sel = lista->GetSelection();
	if (sel>=0&&sel<data.GetSize())
		debug->UpdateBacktrace(data[sel],false);
}

mxBacktraceHistory::~mxBacktraceHistory ( ) {
	debug->MarkCurrentPoint();
	debug->UpdateBacktrace(false,false);
}

