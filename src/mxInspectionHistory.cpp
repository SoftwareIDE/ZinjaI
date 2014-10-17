#include <wx/listbox.h>
#include "mxInspectionHistory.h"
#include "mxMainWindow.h"
#include "mxSizers.h"
#include "ids.h"

BEGIN_EVENT_TABLE(mxInspectionHistory,wxListBox)
	EVT_MENU(mxID_INSPHISTORY_LOG_ALL,mxInspectionHistory::OnLogAll)
	EVT_MENU(mxID_INSPHISTORY_LOG_NONE,mxInspectionHistory::OnLogNone)
	EVT_MENU(mxID_INSPHISTORY_LOG_CHANGE,mxInspectionHistory::OnLogChange)
	EVT_MENU(mxID_INSPHISTORY_CLEAR_LOG,mxInspectionHistory::OnClearLog)
	EVT_MENU(wxID_SELECTALL,mxInspectionHistory::OnSelectAll)
	EVT_MENU(wxID_COPY,mxInspectionHistory::OnCopy)
	EVT_RIGHT_UP(mxInspectionHistory::OnPopupMenu)
END_EVENT_TABLE()
	
mxInspectionHistory::mxInspectionHistory(wxString expression, bool is_frameless) 
	: wxListBox(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL, wxLB_MULTIPLE|wxLB_EXTENDED), lista(this)
//	: wxPanel(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize) 
{
//	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
//	lista = new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL, wxLB_MULTIPLE|wxLB_EXTENDED);
//	lista->Connect(wxID_ANY,wxEVT_RIGHT_UP,wxMouseEventHandler(mxInspectionHistory::OnPopupMenu),NULL,this);
//	sizer->Add(lista,sizers->Exp1);
//	SetSizer(sizer);
	wxSize sz(100,150);
	log_mode = 1;
	(di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,is_frameless)|DIF_AUTO_IMPROVE,this,false))->Init();
	main_window->aui_manager.AddPane(this,wxAuiPaneInfo().Name("inspection_print").Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption(expression).Show().FloatingPosition(wxGetMousePosition()-wxPoint(25,10)).BestSize(sz));
	main_window->aui_manager.Update();	
}

mxInspectionHistory::~mxInspectionHistory() {
	di->Destroy();
}

void mxInspectionHistory::OnDICreated (DebuggerInspection * di) {
	OnNewValue(di->GetValue());
}

void mxInspectionHistory::OnDIError (DebuggerInspection * di) {
	OnNewValue(DebuggerInspection::GetUserStatusText(DIMSG_PENDING));
}

void mxInspectionHistory::OnDIValueChanged (DebuggerInspection * di) {
	OnNewValue(di->GetValue());
}

void mxInspectionHistory::OnDIOutOfScope (DebuggerInspection * di) {
	OnNewValue(DebuggerInspection::GetUserStatusText(DIMSG_OUT_OF_SCOPE));
}

void mxInspectionHistory::OnDIInScope (DebuggerInspection * di) {
	OnNewValue(di->GetValue());
}

void mxInspectionHistory::OnDINewType (DebuggerInspection * di) {
	OnNewValue(di->GetValue());
}

void mxInspectionHistory::OnNewValue (const wxString &new_value, bool force) {
	if (force || (log_mode==1 && new_value!=last_value)) {
		lista->Append(new_value);
		lista->SetFirstItem(lista->GetCount()-1);
	}
	last_value=new_value;
}

void mxInspectionHistory::OnPopupMenu (wxMouseEvent & evt) {
	wxMenu menu("");
	menu.AppendCheckItem(mxID_INSPHISTORY_LOG_NONE,LANG(INSPECTION_HISTORY_LOG_NONE,"No registrar"))->Check(log_mode==0);
	menu.AppendCheckItem(mxID_INSPHISTORY_LOG_CHANGE,LANG(INSPECTION_HISTORY_LOG_CHANGE,"Registrar solo cuando cambia"))->Check(log_mode==1);
	menu.AppendCheckItem(mxID_INSPHISTORY_LOG_ALL,LANG(INSPECTION_HISTORY_LOG_ALL,"Registrar en cada pausa"))->Check(log_mode==2);
	menu.AppendSeparator();
	menu.Append(wxID_SELECTALL,LANG(INSPECTION_HISTORY_SELECT_ALL,"Seleccionar todo"));
	menu.Append(wxID_COPY,LANG(INSPECTION_HISTORY_COPY,"Copiar"));
	menu.Append(mxID_INSPHISTORY_CLEAR_LOG,LANG(INSPECTION_HISTORY_CLEAR_LOG,"Limpiar lista"));
	PopupMenu(&menu);
}

void mxInspectionHistory::OnLogNone (wxCommandEvent & evt) {
	log_mode=0;
}

void mxInspectionHistory::OnLogChange (wxCommandEvent & evt) {
	log_mode=1;
}

void mxInspectionHistory::OnLogAll (wxCommandEvent & evt) {
	log_mode=2;
}

void mxInspectionHistory::OnClearLog (wxCommandEvent & evt) {
	lista->Clear();
}

void mxInspectionHistory::OnCopy (wxCommandEvent & evt) {
	wxString data;
	for(unsigned int i=0;i<lista->GetCount();i++) { 
		if (lista->IsSelected(i)) {
			if (data.Len()) data+="\n";
			data+=lista->GetString(i);
		}
	}
	mxUT::SetClipboardText(data);
}

void mxInspectionHistory::OnSelectAll (wxCommandEvent & evt) {
	for(unsigned int i=0;i<lista->GetCount();i++) { 
		if (lista->IsSelected(i)) {
			lista->Select(i);
		}
	}
}

void mxInspectionHistory::OnDebugPausePost ( ) {
	if (log_mode==2) OnNewValue(last_value,true);
}

