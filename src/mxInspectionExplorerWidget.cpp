#include "mxInspectionExplorerWidget.h"
#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE(mxInspectionExplorerWidget,wxTreeCtrl)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanding)
	EVT_TREE_ITEM_EXPANDED(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanded)
	EVT_TREE_ITEM_COLLAPSED(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanded)
	EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY,mxInspectionExplorerWidget::OnItemTooltip)
END_EVENT_TABLE()

mxInspectionExplorerWidget::mxInspectionExplorerWidget (wxWindow * parent, const wxString &expression, bool frameless) 
	: wxTreeCtrl(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_DEFAULT_STYLE|wxTR_HAS_BUTTONS|(expression==""?wxTR_HIDE_ROOT:0)) 
{
	event_listener=NULL;
	AddRoot(expression); 
	if (expression!="") {
		DebuggerInspection *di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless)|DIF_DONT_USE_HELPER,this,false);
		/*int p =*/ AddItem(NULL,di,true);
		wxTreeCtrl::SetItemHasChildren(GetRootItem());
		wxTreeCtrl::Expand(GetRootItem());
	}
}

int mxInspectionExplorerWidget::AddItem (wxTreeItemId *parent, DebuggerInspection *di, bool is_root) {
	mxIEWAux aux (di, parent?AppendItem(*parent,di->GetExpression()):GetRootItem(), is_root);
	int p = inspections.Add(aux);
	if (is_root) aux.di->Init(); else SetItemText(aux.item,aux.MakeItemLabel());
	if (aux.di->IsCompound()) SetItemHasChildren(aux.item,true);
	return p;
}

void mxInspectionExplorerWidget::AddExpression (wxString expression, bool frameless) {
	wxTreeItemId item = GetRootItem(); AddItem(&item,DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless)|DIF_DONT_USE_HELPER,this,false),true);
}

//void mxInspectionExplorerWidget::Expand (int pos) {
	
//	wxTreeCtrl::Expand(aux.item);
//}


void mxInspectionExplorerWidget::OnDICreated (DebuggerInspection * di) {
	int pos = inspections.Find(di); 
	if (pos==inspections.NotFound()) return; // no deberia pasar
	SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel());
}

void mxInspectionExplorerWidget::OnDIError (DebuggerInspection * di) {
//	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
}

void mxInspectionExplorerWidget::OnDIValueChanged (DebuggerInspection * di) {
//	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
//	SetItemText(inspections[pos].item,inspections[pos].di->GetExpression()+": "<<inspections[pos].di->GetValue());
}

void mxInspectionExplorerWidget::OnDIOutOfScope (DebuggerInspection * di) {
//	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
//	SetItemText(inspections[pos].item,inspections[pos].di->GetExpression()+": "<<LANG(INSPECTGRID_OUT_OF_SCOPE,"<<< fuera de ámbito >>>"));
}

void mxInspectionExplorerWidget::OnDIInScope (DebuggerInspection * di) {
	
}

void mxInspectionExplorerWidget::OnDINewType (DebuggerInspection * di) {
	
}

void mxInspectionExplorerWidget::OnItemExpanding (wxTreeEvent & event) {
	int pos = inspections.Find(event.GetItem()); 
	if (pos==inspections.NotFound()) return; // no deberia pasar
	mxIEWAux aux = inspections[pos];
	if (aux.is_open) return; 
	SingleList<DebuggerInspection*> children;
	if (!aux.di->Break(children,true,false,false) || !children.GetSize()) return;
	inspections[pos].is_open=true; // don't try to generate same
	for(int i=0;i<children.GetSize();i++) 
		AddItem(&aux.item,children[i],false);
	SetItemText(aux.item,aux.MakeItemLabel());
}

void mxInspectionExplorerWidget::OnItemExpanded (wxTreeEvent & event) {
	event.Skip();
	if (event_listener) event_listener->OnSize();
}

void mxInspectionExplorerWidget::SetEventListener (EventListener * s) {
	SetQuickBestSize(false);
	event_listener = s;
}

mxInspectionExplorerWidget::~mxInspectionExplorerWidget ( ) {
	delete event_listener;
}

void mxInspectionExplorerWidget::OnItemTooltip (wxTreeEvent & event) {
	int pos = inspections.Find(event.GetItem()); 
	if (pos==inspections.NotFound()) return; // no deberia pasar
	mxIEWAux aux = inspections[pos];
	if (event_listener) event_listener->OnType(aux.di->GetValueType());
}

wxString mxInspectionExplorerWidget::GetRootType ( ) {
	return inspections[0].di->GetValueType();
}

