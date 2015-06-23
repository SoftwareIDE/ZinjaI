#include "mxPlainResultsTable.h"
#include <wx/sizer.h>
#include "mxSizers.h"
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include "Cpp11.h"

BEGIN_EVENT_TABLE(mxPlainResultsTable,wxPanel)
	EVT_GRID_CMD_CELL_LEFT_CLICK(wxID_ANY,mxPlainResultsTable::OnClickTableCell)
	EVT_GRID_CMD_CELL_LEFT_DCLICK(wxID_ANY,mxPlainResultsTable::OnDClickTableCell)
	EVT_GRID_CMD_LABEL_LEFT_CLICK(wxID_ANY,mxPlainResultsTable::OnClickTableLabel)
	EVT_TEXT(wxID_FIND,mxPlainResultsTable::OnSearchTextChange)
END_EVENT_TABLE()

mxPlainResultsTable::mxPlainResultsTable (wxWindow * parent, mxPRTData * pdata) 
	: wxPanel(parent,wxID_ANY), data(pdata), search(nullptr), sort_status(0)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	grid = new wxGrid(this,wxID_ANY);
	grid->CreateGrid(data->GetRowsCount(),data->GetColumnsCount());
	for(int i=0;i<data->GetColumnsCount();i++)
		grid->SetColLabelValue(i,data->GetColumnName(i));
	grid->SetRowLabelSize(0);
	sizer->Add(grid,sizers->Exp1);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows); // hacer que la seleccion sea por fila, y no por celda
	grid->EnableEditing(false);
	if (data->EnableFilter()) {
		wxSizer *search_sizer=new wxBoxSizer(wxHORIZONTAL);
		search_sizer->Add(new wxStaticText(this,wxID_ANY,"Filter: "),sizers->Center);
		search = new wxTextCtrl(this,wxID_FIND,"");
		search_sizer->Add(search,sizers->Exp1);
		sizer->Add(search_sizer,sizers->Exp0);
	}
	SetSizer(sizer);
	SetData(pdata);
}

void mxPlainResultsTable::SetData (mxPRTData * pdata) {
	if (data!=pdata) { delete data; sort_status=0; }
	data=pdata; pdata->parent=this;
	UpdateTable();
}

mxPlainResultsTable::~mxPlainResultsTable ( ) {
	delete data;
}

void mxPlainResultsTable::UpdateTable() {
	// count how many rows should we display
	wxString filter = search?search->GetValue():"";
	int row_count = data->GetRowsCount(), col_count = data->GetColumnsCount();
	if (!filter.IsEmpty()) {
		int aux = 0;
		for(int i=0;i<row_count ;i++) { 
			if (data->RowMatches(i,filter))
				aux++;
		}
		row_count=aux;
	}
	// ensure the grid has the appropiate number of rows
	int grid_rows = grid->GetNumberRows();
	if (grid_rows<row_count) grid->AppendRows(row_count-grid_rows,false);
	else if (grid_rows>row_count) grid->DeleteRows(row_count,grid_rows-row_count,false);
	// fill the table
	row_count = data->GetRowsCount();
	for(int i=0,aux=0;i<row_count ;i++) {
		if (data->RowMatches(i,filter)){
			for(int j=0;j<col_count;j++) {
				grid->SetCellValue(aux,j,data->GetData(i,j));
			}
			aux++;
		}
	}
	grid->AutoSizeColumns();
}

void mxPlainResultsTable::OnDClickTableCell (wxGridEvent & evt) {
	data->OnDClick(evt.GetRow(),evt.GetCol());
}

void mxPlainResultsTable::OnClickTableLabel (wxGridEvent & evt) {
	if (sort_status>0) { // previous sort was ascending
		if (evt.GetCol()+1 == sort_status) sort_status = -evt.GetCol();
		else                               sort_status = evt.GetCol()+1;
	} else if (sort_status<0) { // previous sort was descending
		if (-evt.GetCol() == sort_status) sort_status = evt.GetCol()+1;
		else                              sort_status = -evt.GetCol();
	} else { // no previous sort
		sort_status = evt.GetCol()+1;
	}
	if (data->Sort(evt.GetCol(),sort_status>0)) UpdateTable();
}

void mxPlainResultsTable::OnSearchTextChange (wxCommandEvent & event) {
	UpdateTable();
}

void mxPlainResultsTable::OnClickTableCell (wxGridEvent & evt) {
	grid->SelectRow(evt.GetRow());
}

wxString mxPRTData::GetCellValue (int row, int col) {
	return parent->grid->GetCellValue(row,col);
}

