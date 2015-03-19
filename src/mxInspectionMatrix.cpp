#include <wx/grid.h>
//#include <wx/textdlg.h>
#include <wx/checkbox.h>
//#include <wx/stattext.h>
#include "mxInspectionMatrix.h"
//#include "DebugManager.h"
#include "mxMainWindow.h"
//#include "mxBitmapButton.h"
//#include "Language.h"
#include "mxSizers.h"
#include "mxTextDialog.h"
#include "Inspection.h"
#include <iostream>
#include <wx/settings.h>
#include "gdbParser.h"

BEGIN_EVENT_TABLE(mxInspectionMatrix, wxPanel)
	EVT_SIZE(mxInspectionMatrix::OnResize)
//	EVT_GRID_COL_SIZE(mxInspectionMatrix::OnColResize)
	EVT_CHECKBOX(wxID_ANY,mxInspectionMatrix::OnAdaptCheck)
////	EVT_GRID_CELL_LEFT_DCLICK(mxInspectionMatrix::OnDoubleClick)
////	EVT_CLOSE(mxInspectionMatrix::OnClose)
//	EVT_KEY_DOWN(mxInspectionMatrix::OnChar)
END_EVENT_TABLE()

mxInspectionMatrix::mxInspectionMatrix(const wxString &expression, bool is_frameless) : wxPanel(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	
	created=false; do_adapt=true; cols=rows=0; 
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	grid = new wxGrid(this,wxID_ANY); grid->CreateGrid(0,0);
	grid->EnableEditing(false); grid->EnableDragColSize();
	mySizer->Add(grid,sizers->Exp1);
	mySizer->Add(adapt=new wxCheckBox(this,wxID_ANY,LANG(IMATRIX_AUTOFIT_COLUMNS,"Auto-ajustar columnas")),sizers->Right);
	
	wxSize sz(300,200);
	SetSizer(mySizer);
	
	(di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,is_frameless)|DIF_AUTO_IMPROVE|DIF_FULL_OUTPUT,this,false))->Init();
	// si la expresion correspondía a un puntero, preguntar cuantos elementos mostrar
	if (!di->IsSimpleType() && !di->IsCompound()) {
		wxString sn = mxGetTextFromUser(LANG(IMATRIX_ENTER_NUM,"Ingrese la cantidad de elementos a mostrar"),expression,"10",main_window);
		long n; 
		if (sn.ToLong(&n) && n>0) {
			wxString new_expression = wxString("(")<<expression<<")[0]@"<<n;
			di->Destroy();
			(di = DebuggerInspection::Create(new_expression,FlagIf(DIF_FRAMELESS,is_frameless)|DIF_AUTO_IMPROVE,this,false))->Init();
		}
	}
	
	main_window->aui_manager.AddPane(this,wxAuiPaneInfo().Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption(expression).Show().FloatingPosition(wxGetMousePosition()-wxPoint(25,10)).BestSize(sz));
	main_window->aui_manager.Update();
	
	grid->SetRowLabelSize(wxGRID_AUTOSIZE);
	adapt->SetValue(true);
	CalculateColumnSizes();
	
	created = true;
}

void mxInspectionMatrix::SetMatrixSize(int w, int h) {
	if (w<cols) grid->DeleteCols(w,cols-w);
	else if (w>cols) grid->InsertCols(cols,w-cols);
	if (h<rows) grid->DeleteRows(h,rows-h);
	else if (h>rows) grid->InsertRows(rows,h-rows);
	cols=w; rows=h;
	grid->SetColLabelSize(w>1?wxGRID_AUTOSIZE:0);
}

void mxInspectionMatrix::UpdateMatrix() {
	grid->BeginBatch();
	
	const wxString &s = di->GetValue();
	
	int w=1, h=0, i=0, i0, pos_st, pos_eq, pos_end;
	
	if (GdbParse_IsList(s,i)) i0=++i;
	else {
		// no parece ser vector ni clase
		SetMatrixSize(1,1);
		grid->SetCellValue(0,0,s);
		grid->SetColLabelValue(0,"");
		grid->SetRowLabelValue(0,"");
		grid->EndBatch();
		return;
	}
	
	while (GdbParse_GetPair(s,i,pos_st,pos_eq,pos_end)) { 
		h++; i++;
		// ver si el elemento es lista para armar la matriz 2D
		int aux_i = pos_eq==-1?pos_st:pos_eq+1, aux_w=1;
		if (GdbParse_IsList(s,aux_i,pos_end)) {
			aux_i++; aux_w=0;
			int aux_st, aux_eq, aux_end;
			while (GdbParse_GetPair(s,aux_i,aux_st,aux_eq,aux_end,pos_end)) { 
				aux_w++; aux_i++;
			}
		}
		// asegurarse que todas las filas tengan igual cantidad de columnas
		if (h==1) w = aux_w;
		else if (aux_w!=w) w = 1;
	}
	SetMatrixSize(w,h);
	
	i=i0; h=0;
	while (GdbParse_GetPair(s,i,pos_st,pos_eq,pos_end)) {
		int aux_i=pos_eq==-1?pos_st:pos_eq+1;
		grid->SetRowLabelValue( h, pos_eq==-1 ? wxString()<<h : s.Mid(pos_st,pos_eq-pos_st) ); 
		if (w!=1 && GdbParse_IsList(s,aux_i,pos_end)) {
			aux_i++;
			int aux_w=0, aux_st, aux_eq, aux_end;
			while (GdbParse_GetPair(s,aux_i,aux_st,aux_eq,aux_end,pos_end)) { 
				if (h==0) grid->SetColLabelValue( aux_w, aux_eq==-1 ? wxString()<<aux_w : s.Mid(aux_st,aux_eq-aux_st) ); 
				grid->SetCellValue( h, aux_w, aux_eq==-1 ? s.Mid(aux_st,aux_end-aux_st) : s.Mid(aux_eq+1,aux_end-aux_eq-1) ); 
				aux_w++; aux_i++;
			}
			if (aux_w>w) w=aux_w;
		} else {
			grid->SetCellValue( h, 0, pos_eq==-1 ? s.Mid(pos_st,pos_end-pos_st) : s.Mid(pos_eq+1,pos_end-pos_eq-1) ); 
		}
		h++; i++;
	}
	
	grid->EndBatch();
}

void mxInspectionMatrix::OnResize(wxSizeEvent &evt) {
	if (created && do_adapt) {
		CalculateColumnSizes();
	}
	evt.Skip();
	Refresh();
}

//void mxInspectionMatrix::OnColResize(wxGridSizeEvent &evt) {
//	int x=grid->GetColSize(evt.GetRowOrCol());
//	grid->BeginBatch();
//	for (int i=0;i<cols;i++)
//		grid->SetColSize(i,x);
//	grid->EndBatch();
//	grid->ForceRefresh();	
////	if (old_size) cols_sizes[evt.GetRowOrCol()]=grid->GetColSize(evt.GetRowOrCol());
//}

////void mxInspectionMatrix::OnClose(wxCloseEvent &evt) {
//////	debug->UnRegisterMatrix(this);
////	Destroy();
//// }

void mxInspectionMatrix::OnAdaptCheck(wxCommandEvent &evt) {
	evt.Skip();
	if ((do_adapt=adapt->GetValue()))
		CalculateColumnSizes();
}

//bool mxInspectionMatrix::SetOutdated() {
//	wxStaticText *st=new wxStaticText(this,wxID_ANY,LANG(IMATRIX_OBSOLETE,"Tabla obsoleta"));
//	st->SetForegroundColour(wxColour("RED"));
//	mySizer->Add(st,sizers->BA5_Center);
//	Layout();
//	return true;
//}

void mxInspectionMatrix::OnDICreated (DebuggerInspection * di) {
	this->di=di; UpdateMatrix();
}

void mxInspectionMatrix::OnDIError (DebuggerInspection * di) {
	this->di=di; UpdateMatrix();
}

void mxInspectionMatrix::OnDIValueChanged (DebuggerInspection * di) {
	UpdateMatrix();
}

void mxInspectionMatrix::OnDIOutOfScope (DebuggerInspection * di) {
	
}

void mxInspectionMatrix::OnDIInScope (DebuggerInspection * di) {
	UpdateMatrix();
}

void mxInspectionMatrix::OnDINewType (DebuggerInspection * di) {
	UpdateMatrix();
}

mxInspectionMatrix::~mxInspectionMatrix ( ) {
	di->Destroy();
}

void mxInspectionMatrix::CalculateColumnSizes () {
	int ancho = this->GetClientSize().GetWidth()-grid->GetRowLabelSize()-wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	int csize=int(float(ancho)/(cols)), minimal_col_width=25;
	if (csize<minimal_col_width) csize=minimal_col_width;
	for (int i=0;i<cols;i++) grid->SetColSize(i,csize);
}

