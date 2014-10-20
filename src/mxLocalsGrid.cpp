#include "mxLocalsGrid.h"
#include "Language.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionPrint.h"
#include "mxInspectionMatrix.h"
#include <algorithm>
#include "mxInspectionHistory.h"

BEGIN_EVENT_TABLE(mxLocalsGrid,wxGrid)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TEXT,mxLocalsGrid::OnShowInText)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TABLE,mxLocalsGrid::OnShowInTable)
	EVT_MENU(mxID_INSPECTION_EXPLORE,mxLocalsGrid::OnExploreExpression)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_HISTORY,mxLocalsGrid::OnShowInHistory)
	EVT_MENU(mxID_INSPECTION_COPY_VALUE,mxLocalsGrid::OnCopyValue)
	EVT_MENU(mxID_INSPECTION_COPY_TYPE,mxLocalsGrid::OnCopyType)
	EVT_MENU(mxID_INSPECTION_COPY_EXPRESSION,mxLocalsGrid::OnCopyExpression)
	EVT_MENU(mxID_INSPECTION_COPY_ALL,mxLocalsGrid::OnCopyAll)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxLocalsGrid::OnAddToInspectionsGrid)
END_EVENT_TABLE()

mxLocalsGrid::mxLocalsGrid(wxWindow *parent):mxGrid(parent,LG_COLS_COUNT) {
	mxGrid::InitColumn(LG_COL_NAME,LANG(LOCALS_NAME,"Nombre"),10);
	mxGrid::InitColumn(LG_COL_TYPE,LANG(LOCALS_TYPE,"Tipo"),20);
	mxGrid::InitColumn(LG_COL_VALUE,LANG(LOCALS_VALUE,"Valor"),30);
	mxGrid::SetColumnVisible(LG_COL_TYPE,false);
	mxGrid::DoCreate();
	mxGrid::SetRowSelectionMode();
	EnableEditing(false);
	EnableDragRowSize(false);
	SetColLabelSize(wxGRID_AUTOSIZE);
	command = "-stack-list-variables";
}

static bool GdbParse_GetNext(const wxString &s, int &i, wxString &key, wxString &val) {
	int l=s.Len();
	while (i<l && (s[i]<='a'||s[i]>='z')) i++;
	while(true) {
		int p0=i; // posicion donde empieza el nombre del "campo"
		while (i<l && s[i]!=']' && s[i]!='}' && s[i]!='=') i++;
		int p1=i; // posicion del igual
		if (++i>=l || s[i]!='\"') { // si no era un campo, termino la entrada
			i=-1; return true;
		}
		while (++i<l && s[i]!='\"') { if (s[i]=='\\') i++; }
		int p2=i; // posicion de la comilla que cierra
		key=s.Mid(p0,p1-p0);
		val=s.Mid(p1+2,p2-p1-2);
		while (++i<l && s[i]!=',' && s[i]!=']' && s[i]!='}'); 
		if (i==l || s[i]==']' || s[i]=='}') 
			return true; 
		else { 
				i++; return false;
			}
	}
}

void mxLocalsGrid::Update () {
	if (!IsCurrentInspectionsTab()) return;
	int n = GetNumberRows();
	if (!debug->IsDebugging() || !debug->IsPaused()) {
		DeleteRows(0,n); n=0; return;
	}
	
	BeginBatch();
	
	wxString s = debug->SendCommand(command," --all-values");
	if (s.StartsWith("^error") && s.Contains("Undefined MI command")) {
		// fallback to deprecated command if gdb is old
		command = "-stack-list-locals";
		s = debug->SendCommand(command," --all-values");
	}
	
	int i = s.Find("name="), c=0;
	wxString name,value, key,val;
	while(true) {
		bool add = GdbParse_GetNext(s,i,key,val);
		if (i==-1) break;
		if (key=="name") name=val;
		else if (key=="value") value=val;
		if (add) {
			if (c==n) { ++n; AppendRows(1); }
			mxGrid::SetCellValue(c,LG_COL_NAME,name);
			mxGrid::SetCellValue(c,LG_COL_VALUE,value);
			c++;
		}
	}
	
	if (mxGrid::IsColumnVisible(LG_COL_TYPE)) {
		wxString s = debug->SendCommand(command," --simple-values");
		int i =  s.Find("name="), c=0;
		while(true) {
			/*bool add =*/ GdbParse_GetNext(s,i,key,val);
			if (c==n||i==-1) break;
			if (key=="type") mxGrid::SetCellValue(c++,LG_COL_TYPE,val);
		}
	}
	
	EndBatch();
	
}


void mxLocalsGrid::OnCellPopupMenu (int row, int col) {
	
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	// ensure clicked row is selected... if it is, use current selection, if not make it current selection
	if (find(sel.begin(),sel.end(),row)==sel.end()) { sel.clear(); mxGrid::Select(row); sel.push_back(row); }
	
	wxMenu menu; 
	
	menu.Append(mxID_INSPECTION_SHOW_IN_TABLE,LANG(INSPECTGRID_POPUP_SHOW_IN_TABLE,"Mostrar en &tabla separada..."));
	menu.Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"Mostrar en &ventana separada..."));
	menu.Append(mxID_INSPECTION_EXPLORE,LANG(INSPECTGRID_POPUP_EXPLORE,"&Explorar datos..."));
	menu.Append(mxID_INSPECTION_SHOW_IN_HISTORY,LANG(INSPECTGRID_POPUP_SHOW_IN_HISTORY,"Generar historial de valores..."));
	
	menu.AppendSeparator();
	for(int i=0;i<main_window->inspection_ctrl->GetTabsCount();i++) {
		if (main_window->inspection_ctrl->PageIsInspectionsGrid(i))
			menu.Append(mxID_LAST_ID+i,LANG1(LOCALGRID_POPUP_ADD_TO_INSPECTIONS_GRID,"Agregar como inspección en \"<{1}>\"",main_window->inspection_ctrl->GetPageTitle(i)));
	}
	menu.AppendSeparator();
	
	menu.Append(mxID_INSPECTION_COPY_EXPRESSION,wxString(LANG(INSPECTGRID_POPUP_COPY_EXPRESSION,"Copiar &Expresion")));
	if (mxGrid::IsColumnVisible(LG_COL_TYPE)) menu.Append(mxID_INSPECTION_COPY_TYPE,LANG(INSPECTGRID_POPUP_COPY_TYPE,"Copiar &Tipo"));
	menu.Append(mxID_INSPECTION_COPY_VALUE,LANG(INSPECTGRID_POPUP_COPY_DATA,"Copiar &Valor"));
	menu.Append(mxID_INSPECTION_COPY_ALL,LANG(INSPECTGRID_POPUP_COPY_ALL,"&Copiar Toda la Tabla"));
		
	PopupMenu(&menu);
}


void mxLocalsGrid::OnAddToInspectionsGrid (wxCommandEvent & evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	mxInspectionGrid *grid = main_window->inspection_ctrl->GetInspectionGrid(evt.GetId()-mxID_LAST_ID);
	for(unsigned int i=0;i<sel.size();i++) {
		grid->ModifyExpression(-1,mxGrid::GetCellValue(sel[i],LG_COL_NAME),false);
	}
	main_window->inspection_ctrl->SetSelection(evt.GetId()-mxID_LAST_ID);
}

void mxLocalsGrid::OnShowInTable(wxCommandEvent &evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++)
		new mxInspectionMatrix(mxGrid::GetCellValue(sel[i],LG_COL_NAME),false);
}

void mxLocalsGrid::OnShowInText(wxCommandEvent &evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++)
		new mxInspectionPrint(mxGrid::GetCellValue(sel[i],LG_COL_NAME),false);
}

void mxLocalsGrid::OnExploreExpression (wxCommandEvent & evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	if (sel.size()==1) {
		new mxInspectionExplorerDialog(mxGrid::GetCellValue(sel[0],LG_COL_NAME),false);
	} else {
		mxInspectionExplorerDialog *dialog = new mxInspectionExplorerDialog();
		for(unsigned int i=0;i<sel.size();i++)
			dialog->AddExpression(mxGrid::GetCellValue(sel[i],LG_COL_NAME),false);
	}
}

void mxLocalsGrid::OnCopyValue (wxCommandEvent & evt) {
	CopyToClipboard(true,LG_COL_VALUE);
}

void mxLocalsGrid::OnCopyType (wxCommandEvent & evt) {
	CopyToClipboard(true,LG_COL_TYPE);
}

void mxLocalsGrid::OnCopyExpression (wxCommandEvent & evt) {
	CopyToClipboard(true,LG_COL_NAME);
}

void mxLocalsGrid::OnCopyAll (wxCommandEvent & evt) {
	CopyToClipboard(false);
}

void mxLocalsGrid::OnShowInHistory (wxCommandEvent & evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++)
		new mxInspectionHistory(mxGrid::GetCellValue(sel[i],LG_COL_NAME),false);
}

