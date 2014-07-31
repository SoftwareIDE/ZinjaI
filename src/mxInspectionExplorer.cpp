#warning recuperar toda esta clase

#include <wx/sizer.h>
#include <wx/laywin.h>
#include <wx/treectrl.h>
#include "mxInspectionExplorer.h"
#include "ids.h"
#include "mxBitmapButton.h"
#include "DebugManager.h"
#include "mxUtils.h"
#include "mxSizers.h"


mxIEItemData::mxIEItemData(wxString aname, wxString areal_expr, wxString aexpr, wxString aframe, wxString atype, int achildren) {
	frame=aframe;
	what_is = DI_NONE;
	real_expr=areal_expr;
	name=aname;
	expr=aexpr;
	type=atype;
	age=0;
	opened = use_data_evaluate = always_evaluate = false;
	children = achildren;
	is_root = false;
}

mxIEItemData::~mxIEItemData() {
	if (is_root)
		debug->DeleteVO(name);
}

//BEGIN_EVENT_TABLE(mxInspectionExplorer, wxDialog)
////	EVT_BUTTON(wxID_OK,mxInspectionExplorer::OnCloseButton)
////	EVT_CLOSE(mxInspectionExplorer::OnClose)
//	EVT_TREE_SEL_CHANGED(mxID_INSPECTION_TREE,mxInspectionExplorer::OnItemClick)
//	EVT_TREE_ITEM_EXPANDING(mxID_INSPECTION_TREE,mxInspectionExplorer::OnItemExpanding)
//	EVT_SASH_DRAGGED(wxID_ANY, mxInspectionExplorer::OnSashDrag)
//END_EVENT_TABLE()
//	
//mxInspectionExplorer::mxInspectionExplorer(wxString frame, wxString expr) : mxExternInspection(frame,expr) {
//	CreateControls();
//	wxTreeItemId root = tree->AddRoot(_T("Expresiones"), 0);
//	if (expr.Len()) {
//		wxString name, type;
//		int children;
//		if (frame!=debug->current_frame)
//			debug->SelectFrameForInspeccion(frame);
//		debug->CreateVO(expr,name,type,children);
//		if (name.Len()){
//			mxIEItemData *item_data = new mxIEItemData(name,expr,expr,frame,type,children);
//			item_data->is_root = true;
//			wxTreeItemId item = tree->AppendItem(root,expr,0,0,item_data);
//			if (children)
//				tree->SetItemHasChildren(item,true);
//			wxTreeEvent evt;
//			evt.SetItem(item);
//			OnItemClick(evt);
//		}
//		if (frame!=debug->current_frame)
//			debug->SelectFrameForInspeccion(debug->current_frame);
//	}
//	wxTreeItemIdValue cookie;
//	tree->SelectItem(tree->GetFirstChild(tree->GetRootItem(),cookie));
////	debug->RegisterExplorer(this);
//	age=1;
//	tree->SetFocus();
//	Fit();
//}
//
//mxInspectionExplorer::mxInspectionExplorer(wxString title, wxArrayString &frames, wxArrayString &exprs) : mxExternInspection ("","") {
//	SetTitle(title);
//	CreateControls();
//	wxTreeItemId root = tree->AddRoot(_T("Expresiones"), 0);
//	wxString my_frame = debug->current_frame;
//	for (unsigned int i=0;i<exprs.GetCount();i++) {
//		wxString expr = exprs[i];
//		if (expr.Len()) {
//			wxString name, type;
//			int children;
//			if (frames[i]!=my_frame)
//				debug->SelectFrameForInspeccion(my_frame=frames[i]);
//			debug->CreateVO(expr,name,type,children);
//			if (name.Len()){
//				mxIEItemData *item_data = new mxIEItemData(name,expr,expr,frames[i], type,children);
//				item_data->is_root = true;
//				wxTreeItemId item = tree->AppendItem(root,expr,0,0,item_data);
//				if (children)
//					tree->SetItemHasChildren(item,true);
//				wxTreeEvent evt;
//				evt.SetItem(item);
//				OnItemClick(evt);
//			}
//		}
//	}
//	wxTreeItemIdValue cookie;
//	tree->SelectItem(tree->GetFirstChild(tree->GetRootItem(),cookie));
//	if (my_frame!=debug->current_frame)
//		debug->SelectFrameForInspeccion(debug->current_frame);
////	debug->RegisterExplorer(this);
//	age=1;
//	Fit();
//}
//
//
//void mxInspectionExplorer::CreateControls() {	
//	index_sash = new wxSashLayoutWindow(this, wxID_ANY,
//		wxDefaultPosition, wxSize(200, 30),
//		wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
//	index_sash->SetDefaultSize(wxSize(120, 1000));
//	index_sash->SetOrientation(wxLAYOUT_VERTICAL);
//	index_sash->SetAlignment(wxLAYOUT_LEFT);
//	index_sash->SetSashVisible(wxSASH_RIGHT, true);
//	
//	treeSizer = new wxBoxSizer(wxHORIZONTAL);
//	wxBoxSizer *bigSizer = new wxBoxSizer(wxVERTICAL);
////	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
//	wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
//	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
//	wxBoxSizer *typeSizer = new wxBoxSizer(wxHORIZONTAL);
//	tree = new wxTreeCtrl(index_sash, mxID_INSPECTION_TREE,wxPoint(0,0),wxSize(10,250),wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT | wxTR_FULL_ROW_HIGHLIGHT);
//	value_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE|wxTE_BESTWRAP|wxTE_READONLY);
//	type_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
//	expr_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
//
//	wxButton *close_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.cancel, _T("&Cerrar")); 
//	SetEscapeId(wxID_OK);
//	
//	treeSizer->Add(index_sash,sizers->Exp0);
////	topSizer->Add(treeSizer,sizers->Exp1);
////	topSizer->Add(rightSizer,sizers->Exp1);
//	treeSizer->Add(rightSizer,sizers->Exp1);
//	rightSizer->Add(new wxStaticText(this,wxID_ANY,_T("Expresion: ")));
//	rightSizer->Add(expr_ctrl,sizers->Exp0);
//	typeSizer->Add(new wxStaticText(this,wxID_ANY,_T("Tipo de dato: ")));
//	typeSizer->Add(type_ctrl,sizers->Exp1);
//	rightSizer->Add(typeSizer,sizers->BT5_Exp0);
//	rightSizer->Add(new wxStaticText(this,wxID_ANY,_T("Valor: ")),sizers->BT5);
//	rightSizer->Add(value_ctrl,sizers->BA5_Exp1);
//	bottomSizer->Add(close_button,sizers->BA5);
//	bigSizer->Add(treeSizer,sizers->Exp1);
//	bigSizer->Add(bottomSizer,sizers->Right);
//	SetSizer(bigSizer);
//	treeSizer->SetItemMinSize(index_sash,200, 10);
//	treeSizer->Layout();
//	Show();
//
//}
//
//mxInspectionExplorer::~mxInspectionExplorer() {
//}
//
//void mxInspectionExplorer::OnCloseButton(wxCommandEvent &evt) {
////	debug->UnRegisterExplorer(this);
//	Close();
//}
//
////void mxInspectionExplorer::OnClose(wxCloseEvent &evt) {
////	Destroy();
//// }
//
//void mxInspectionExplorer::OnItemExpanding(wxTreeEvent &event) {
//	wxTreeItemId item = event.GetItem();
//	mxIEItemData *item_data = (mxIEItemData*)tree->GetItemData(item);
//	if (item_data->opened) return;
//	mxIEItemData **data;
//	data = new mxIEItemData*[item_data->children];
//	int vcc = debug->GetVOChildrenData(data,item_data->name,item_data->real_expr,item_data->frame,item_data->what_is);
//	if (vcc > 0) {
//		for (int i=0;i<item_data->children;i++) {
//			if (data[i]) {
//				if (data[i]->children)
//					tree->SetItemHasChildren(tree->AppendItem(item,data[i]->expr,0,0,data[i]),true);
//				else
//					tree->AppendItem(item,data[i]->expr,0,0,data[i]);
//			}
//		}
//		item_data->opened=true;
//	}
//	delete [] data;
//}
//
//void mxInspectionExplorer::OnItemClick(wxTreeEvent &event) {
//	wxTreeItemId item = event.GetItem();
//	mxIEItemData *item_data = (mxIEItemData*)tree->GetItemData(item);
//	if (item_data->age!=age) {
//		if (item_data->what_is==DI_IN_CLASS || item_data->what_is==DI_IN_CLASS_POINTER || item_data->what_is==DI_IN_FATHER_CLASS || item_data->what_is==DI_IN_FATHER_POINTER) {
//			wxString oName = item_data->real_expr;
//			if (item_data->what_is==DI_IN_CLASS_POINTER)
//				oName = wxString("*")<<debug->RewriteExpressionForBreaking(oName);
//			if (item_data->expr==_T("private"))
//				item_data->value=wxString(_T("< atributos privados del objeto \""))<<oName<<_T("\">");
//			else if (item_data->expr==_T("public"))
//				item_data->value=wxString(_T("< atributos publicos del objeto \""))<<oName<<_T("\">");
//			else if (item_data->expr==_T("protected"))
//				item_data->value=wxString(_T("< atributos protegidos del objeto \""))<<oName<<_T("\">");
//			else
//				item_data->value=wxString(_T("< atributos de \""))<<item_data->real_expr<<_T("\">");
//			item_data->type=_T("---");
//		} else if (item_data->what_is==DI_FATHER_CLASS || item_data->what_is==DI_FATHER_POINTER) {
//			wxString oName = item_data->real_expr;
//			if (item_data->what_is==DI_IN_FATHER_POINTER)
//				oName = wxString("*")<<debug->RewriteExpressionForBreaking(oName);
//			item_data->value=wxString(_T("< atributos heredados desde la clase \""))<<item_data->name.AfterLast('.')<<_T("\" del objeto \"")<<oName<<_T("\">");
//			item_data->type=_T("---");
//		} else {
//			item_data->value = debug->GetVOValue(item_data->name/*,item_data->expr*/);
//			if (item_data->value[0]=='@') {
//				wxString value2=item_data->value.Mid(item_data->value.Find(':')+2);
//				if (value2[0]=='{')
//					item_data->what_is=DI_CLASS;
//				else if (value2[0]=='[')
//					item_data->what_is=DI_ARRAY;
//				item_data->always_evaluate = true;
//			} else {
//				if (item_data->value[0]=='{')
//					item_data->what_is=DI_CLASS;
//				else if (item_data->value[0]=='[')
//					item_data->what_is=DI_ARRAY;
//				else
//					item_data->what_is=DI_CLASS_POINTER;
//				item_data->use_data_evaluate = item_data->what_is==DI_ARRAY||item_data->what_is==DI_CLASS;
//			}
//			if (item_data->use_data_evaluate) {
//				if (item_data->frame!=debug->current_frame)
//					debug->SelectFrameForInspeccion(item_data->frame);
//				item_data->value = debug->InspectExpression(item_data->real_expr);
//				if (item_data->frame!=debug->current_frame)
//					debug->SelectFrameForInspeccion(debug->current_frame);
//			}
//		}
//		item_data->age = age;
//	}
//	value_ctrl->SetValue(item_data->value);
//	type_ctrl->SetValue(item_data->type);
//	expr_ctrl->SetValue(item_data->real_expr);
//}
//
//void mxInspectionExplorer::OnSashDrag(wxSashEvent& event) {
//	//index_sash->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
//	treeSizer->SetItemMinSize(index_sash,event.GetDragRect().width<100?100:event.GetDragRect().width, 10);
//	//GetClientWindow()->Refresh();
//	treeSizer->Layout();
//}
//
//void mxInspectionExplorer::Grow() {
//	age++;
//	wxTreeEvent evt;
//	evt.SetItem(tree->GetSelection());
//	OnItemClick(evt);
//}
//
//void mxInspectionExplorer::Update() {
//	Grow();
//}
