#include "mxInspectionExplorerWidget.h"
#include <wx/msgdlg.h>
#include <wx/dcscreen.h>
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionPrint.h"
#include "mxInspectionMatrix.h"
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionHistory.h"
#include "mxRealTimeInspectionEditor.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxInspectionExplorerWidget,wxTreeCtrl)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanding)
	EVT_TREE_ITEM_EXPANDED(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanded)
	EVT_TREE_ITEM_COLLAPSED(wxID_ANY,mxInspectionExplorerWidget::OnItemExpanded)
	EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY,mxInspectionExplorerWidget::OnItemTooltip)
	EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,mxInspectionExplorerWidget::OnItemPopup)
	
	
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TEXT,mxInspectionExplorerWidget::OnShowInText)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_RTEDITOR,mxInspectionExplorerWidget::OnShowInRTEditor)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TABLE,mxInspectionExplorerWidget::OnShowInTable)
	EVT_MENU(mxID_INSPECTION_EXPLORE,mxInspectionExplorerWidget::OnExploreExpression)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_HISTORY,mxInspectionExplorerWidget::OnShowInHistory)
	EVT_MENU(mxID_INSPECTION_COPY_VALUE,mxInspectionExplorerWidget::OnCopyValue)
	EVT_MENU(mxID_INSPECTION_COPY_TYPE,mxInspectionExplorerWidget::OnCopyType)
	EVT_MENU(mxID_INSPECTION_COPY_EXPRESSION,mxInspectionExplorerWidget::OnCopyExpression)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxInspectionExplorerWidget::OnAddToInspectionsGrid)
	
	EVT_MENU(mxID_INSPECTION_SET_WATCH_WRITE,mxInspectionExplorerWidget::OnSetWatch)
	EVT_MENU(mxID_INSPECTION_SET_WATCH_READ,mxInspectionExplorerWidget::OnSetWatch)
	EVT_MENU(mxID_INSPECTION_SET_WATCH_BOTH,mxInspectionExplorerWidget::OnSetWatch)
	
END_EVENT_TABLE()

mxInspectionExplorerWidget::mxInspectionExplorerWidget (wxWindow * parent, const wxString &expression, bool frameless, bool expand_roots) 
	: wxTreeCtrl(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_DEFAULT_STYLE|wxTR_HAS_BUTTONS|(expression==""?wxTR_HIDE_ROOT:0)), auto_expand_roots(expand_roots) 
{
	event_listener=nullptr;
	AddRoot(expression); 
	hidden_root = expression=="";
	if (!hidden_root) {
		DebuggerInspection *di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless)|DIF_DONT_USE_HELPER,this,false);
		/*int p =*/ AddItem(nullptr,di,true);
//		wxTreeCtrl::Expand(GetRootItem());
	}
}

int mxInspectionExplorerWidget::AddItem (wxTreeItemId *parent, DebuggerInspection *di, bool is_root) {
	mxIEWAux aux (di, parent?AppendItem(*parent,di->GetShortExpression()):GetRootItem(), is_root);
	int p = inspections.Add(aux);
	if (is_root) aux.di->Init(); else SetItemText(aux.item,aux.MakeItemLabel());
	if (!aux.di->IsSimpleType()) SetItemHasChildren(aux.item,true);
	return p;
}

void mxInspectionExplorerWidget::AddExpression (wxString expression, bool frameless) {
	wxTreeItemId item = GetRootItem(); AddItem(&item,DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless)|DIF_DONT_USE_HELPER,this,false),true);
}

void mxInspectionExplorerWidget::OnDICreated (DebuggerInspection * di) {
	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return;// no deberia pasar
	SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel());
	DeleteChildrenInspections(pos);
	SetItemHasChildren(GetRootItem(),!di->IsSimpleType());
	if (auto_expand_roots && inspections[pos].is_root) Expand(inspections[pos].item);
}

void mxInspectionExplorerWidget::OnDIError (DebuggerInspection * di) {
	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
	SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel(DIMSG_ERROR));
	DeleteChildrenInspections(pos);
}

void mxInspectionExplorerWidget::OnDIOutOfScope (DebuggerInspection * di) {
	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
	SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel(DIMSG_OUT_OF_SCOPE));
	DeleteChildrenInspections(pos);
}

void mxInspectionExplorerWidget::OnDIValueChanged (DebuggerInspection * di) {
	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return;// no deberia pasar
	SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel());
}


void mxInspectionExplorerWidget::OnDIInScope (DebuggerInspection * di) {
	OnDICreated(di);
}

void mxInspectionExplorerWidget::OnDINewType (DebuggerInspection * di) {
	int pos = inspections.Find(di); if (pos==inspections.NotFound()) return; // no deberia pasar
	bool has_children = !inspections[pos].di->IsSimpleType();
	if (inspections[pos].is_open) {
		bool was_expanded = IsExpanded(inspections[pos].item); 	
		DeleteChildrenInspections(pos);	
		SetItemHasChildren(inspections[pos].item,has_children);
		if (was_expanded && has_children) Expand(inspections[pos].item);
		else SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel());
	} else {
		SetItemHasChildren(inspections[pos].item,has_children);
		SetItemText(inspections[pos].item,inspections[pos].MakeItemLabel());
	}
}

void mxInspectionExplorerWidget::OnItemExpanding (wxTreeEvent & event) {
	int pos = inspections.Find(event.GetItem()); 
	if (pos==0) SetItemText(inspections[0].item,inspections[0].MakeItemLabel());
	if (pos==inspections.NotFound()) return; // no deberia pasar
	mxIEWAux aux = inspections[pos]; // no usar referencia, porque puede cambiar dentro del AddItem
	if (aux.is_open) return; 
	SingleList<DebuggerInspection*> children;
	if (!aux.di->Break(children,true,false) || !children.GetSize()) return;
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
	for(int i=0;i<inspections.GetSize();i++) inspections[i].di->Destroy();
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

#ifdef __WIN32__
static void auxSetMax(wxSize &size, wxScreenDC &dc, wxTreeCtrl *tree, wxTreeItemId &item, int depth) {
	wxSize aux = dc.GetTextExtent(tree->GetItemText(item));
	int w = aux.GetWidth()+tree->GetIndent()*depth;
	if (w>size.GetWidth()) size.SetWidth(w);
	size.SetHeight(aux.GetHeight()+size.GetHeight()+5);
}

static void auxForAllChildren(wxSize &size, wxScreenDC &dc, wxTreeCtrl *tree, wxTreeItemId &item, int depth) {
	wxTreeItemIdValue cookie;
	wxTreeItemId it = tree->GetFirstChild(item,cookie);
	while (it.IsOk()) {
		/*if (tree->IsVisible(it)) */auxSetMax(size,dc,tree,it,depth);
		if (tree->ItemHasChildren(it)) auxForAllChildren(size,dc,tree,it,depth+1);
		it = tree->GetNextChild(item,cookie);
	}
}

wxSize mxInspectionExplorerWidget::GetFullSize ( ) {
	/// based on code from: http://stackoverflow.com/questions/960489/what-is-the-proper-way-to-compute-the-fully-expanded-width-of-wx-treectrl
	wxSize sz(5,5);
	wxScreenDC dc;
	dc.SetFont(GetFont());
	wxTreeItemId it = GetRootItem();
	if (!hidden_root) auxSetMax(sz,dc,this,it,1);
	auxForAllChildren(sz,dc,this,it,2);
	return sz;
}

#else
wxSize mxInspectionExplorerWidget::GetFullSize ( ) {
	InvalidateBestSize(); return GetBestSize();
}
#endif

void mxInspectionExplorerWidget::OnItemPopup (wxTreeEvent & event) {
	wxMenu menu; 
	
	menu.Append(mxID_INSPECTION_SHOW_IN_TABLE,LANG(INSPECTGRID_POPUP_SHOW_IN_TABLE,"Mostrar en &tabla separada..."));
	menu.Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"Mostrar en &ventana separada..."));
	menu.Append(mxID_INSPECTION_EXPLORE,LANG(INSPECTGRID_POPUP_EXPLORE,"&Explorar datos..."));
	menu.Append(mxID_INSPECTION_SHOW_IN_RTEDITOR,LANG(INSPECTGRID_POPUP_SHOW_IN_RTEDITOR,"Editar durante la ejecución..."));
	menu.Append(mxID_INSPECTION_SHOW_IN_HISTORY,LANG(INSPECTGRID_POPUP_SHOW_IN_HISTORY,"Generar historial de valores..."));
	
	wxMenu *watch = new wxMenu;
	watch->Append(mxID_INSPECTION_SET_WATCH_READ,LANG(WATCHPOINT_READ,"Lectura"));
	watch->Append(mxID_INSPECTION_SET_WATCH_WRITE,LANG(WATCHPOINT_WRITE,"Escritura"));
	watch->Append(mxID_INSPECTION_SET_WATCH_BOTH,LANG(WATCHPOINT_BOTH,"Lectura/Escritura"));
	menu.AppendSubMenu(watch,LANG(INSPECTGRID_POPUP_SET_READ,"Agregar como watchpoint"));
	
	menu.AppendSeparator();
	for(int i=0;i<main_window->inspection_ctrl->GetTabsCount();i++) {
		if (main_window->inspection_ctrl->PageIsInspectionsGrid(i))
			menu.Append(mxID_LAST_ID+i,LANG1(LOCALGRID_POPUP_ADD_TO_INSPECTIONS_GRID,"Agregar como inspección en \"<{1}>\"",main_window->inspection_ctrl->GetPageTitle(i)));
	}
	menu.AppendSeparator();
	
	menu.Append(mxID_INSPECTION_COPY_EXPRESSION,wxString(LANG(INSPECTGRID_POPUP_COPY_EXPRESSION,"Copiar &Expresion")));
	menu.Append(mxID_INSPECTION_COPY_TYPE,LANG(INSPECTGRID_POPUP_COPY_TYPE,"Copiar &Tipo"));
	menu.Append(mxID_INSPECTION_COPY_VALUE,LANG(INSPECTGRID_POPUP_COPY_DATA,"Copiar &Valor"));
	
	PopupMenu(&menu);
}

void mxInspectionExplorerWidget::OnAddToInspectionsGrid (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	mxInspectionGrid *grid = main_window->inspection_ctrl->GetInspectionGrid(evt.GetId()-mxID_LAST_ID);
	grid->ModifyExpression(-1,aux.di->GetExpression(),false);
	main_window->inspection_ctrl->SetSelection(evt.GetId()-mxID_LAST_ID);
}

void mxInspectionExplorerWidget::OnShowInText (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	new mxInspectionPrint(aux.di->GetExpression(),false);
}

void mxInspectionExplorerWidget::OnShowInTable (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	new mxInspectionMatrix(aux.di->GetExpression(),false);
}

void mxInspectionExplorerWidget::OnShowInRTEditor (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	new mxRealTimeInspectionEditor(aux.di->GetExpression());
}

void mxInspectionExplorerWidget::OnExploreExpression (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	new mxInspectionExplorerDialog(aux.di->GetExpression(),false);
}

void mxInspectionExplorerWidget::OnShowInHistory (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	new mxInspectionHistory(aux.di->GetExpression(),false);
}

void mxInspectionExplorerWidget::OnCopyValue (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	mxUT::SetClipboardText(aux.di->GetValue());
}

void mxInspectionExplorerWidget::OnCopyType (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	mxUT::SetClipboardText(aux.di->GetValueType());
}

void mxInspectionExplorerWidget::OnCopyExpression (wxCommandEvent & evt) {
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	mxUT::SetClipboardText(aux.di->GetExpression());
}


void mxInspectionExplorerWidget::DeleteChildrenInspections (int pos, bool destroy_root) {
	wxTreeItemIdValue cookie;
	// quitarla del arreglo de inspecciones local
	wxTreeItemId item = inspections[pos].item;
	DebuggerInspection *di = inspections[pos].di;
	bool was_open = inspections[pos].is_open;
	inspections[pos].is_open = false;
	if (destroy_root) inspections.Remove(pos);
	// eliminar los hijos del arbol
	if (was_open) {
		Collapse(item);
		wxTreeItemId it = GetFirstChild(item,cookie);
		while (it.IsOk()) {
			int cpos = inspections.Find(it);
			if (cpos!=inspections.NotFound()) // no deberia dar nunca falso
				DeleteChildrenInspections(cpos,true);
			it = GetNextChild(item,cookie);
		}
		DeleteChildren(item);
	}
	// eliminar la inspeccion de DebuggerInspection
	if (destroy_root) di->Destroy();
}

void mxInspectionExplorerWidget::OnSetWatch (wxCommandEvent & evt) {
	bool read = evt.GetId()!=mxID_INSPECTION_WATCH_WRITE;
	bool write = evt.GetId()!=mxID_INSPECTION_WATCH_READ;
	mxIEWAux aux = inspections[ inspections.Find(GetSelection()) ];
	wxString expr = aux.di->GetExpression();
	wxString num =debug->AddWatchPoint(expr,read,write);
	if (!num.IsEmpty())
		mxMessageDialog(main_window,LANG(INSPECTION_WATCH_ADDED_OK,"Watchpoint insertado correctamente."),num+": "+expr,mxMD_OK|mxMD_INFO).ShowModal();
	else
		mxMessageDialog(main_window,LANG(INSPECTION_WATCH_ADDED_ERROR,"Error al insertar watchpoint."),expr,mxMD_OK|mxMD_ERROR).ShowModal();
}

