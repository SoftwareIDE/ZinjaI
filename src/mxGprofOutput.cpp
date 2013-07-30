#include "mxGprofOutput.h"
#include "mxSizers.h"
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxGprofOutput,wxDialog)
	EVT_GRID_CMD_CELL_LEFT_CLICK(wxID_ANY,mxGprofOutput::OnClickTableCell)
	EVT_GRID_CMD_CELL_LEFT_DCLICK(wxID_ANY,mxGprofOutput::OnDClickTableCell)
	EVT_GRID_CMD_LABEL_LEFT_CLICK(wxID_ANY,mxGprofOutput::OnClickTableLabel)
	EVT_TEXT(wxID_FIND,mxGprofOutput::OnSearchTextChange)
	EVT_COMBOBOX(wxID_ANY,mxGprofOutput::OnComboChange)
END_EVENT_TABLE()

mxGprofOutput::mxGprofOutput(wxWindow *parent, wxString fname):wxDialog(parent, wxID_ANY, "gprof output", wxDefaultPosition, wxSize(700,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER),data(fname.c_str()) {
	if (!data.graph.size()) { mxMessageDialog(parent,"No se han podido leer resultados.","gprof",mxMD_ERROR|mxMD_OK).ShowModal(); Destroy(); return; }
	wxSizer *main_sizer=new wxBoxSizer(wxVERTICAL);
	
	notebook = new wxNotebook(this,wxID_ANY);
	
	wxPanel *table_panel=new wxPanel(notebook,wxID_ANY);
	wxSizer *table_sizer=new wxBoxSizer(wxVERTICAL);
	grid_table=new wxGrid(table_panel,wxID_ANY);
	grid_table->CreateGrid(data.table.size(),7);
	grid_table->SetColLabelValue(0,"% time");
	grid_table->SetColLabelValue(1,"cum. secs");
	grid_table->SetColLabelValue(2,"self secs");
	grid_table->SetColLabelValue(3,"calls");
	grid_table->SetColLabelValue(4,"self s/call");
	grid_table->SetColLabelValue(5,"total s/call");
	grid_table->SetColLabelValue(6,"name");
	grid_table->SetRowLabelSize(0);
	FillTable();
	table_sizer->Add(grid_table,sizers->Exp1);
	grid_table->SetSelectionMode(wxGrid::wxGridSelectRows); // hacer que la seleccion sea por fila, y no por celda
	grid_table->EnableEditing(false);
	
	wxSizer *search_sizer=new wxBoxSizer(wxHORIZONTAL);
	search_sizer->Add(new wxStaticText(table_panel,wxID_ANY,"Filter: "),sizers->Center);
	search_text = new wxTextCtrl(table_panel,wxID_FIND,"");
	search_sizer->Add(search_text,sizers->Exp1);
	
	table_sizer->Add(search_sizer,sizers->Exp0);
	table_panel->SetSizer(table_sizer);
	
	wxPanel *graph_panel=new wxPanel(notebook,wxID_ANY);
	wxSizer *graph_sizer=new wxBoxSizer(wxVERTICAL);
	wxArrayString array;
	for(unsigned int i=0;i<data.graph.size();i++) array.Add(data.graph[i].name.c_str());
	combo = new wxComboBox(graph_panel,wxID_ANY,array[0],wxDefaultPosition,wxDefaultSize,array,wxCB_READONLY);
	combo->SetSelection(0);
	graph_sizer->Add(combo,sizers->Exp0);
	grid_graph=new wxGrid(graph_panel,wxID_ANY);
	grid_graph->CreateGrid(0,5);
	grid_graph->SetColLabelValue(0,"% time");
	grid_graph->SetColLabelValue(1,"self");
	grid_graph->SetColLabelValue(2,"children");
	grid_graph->SetColLabelValue(3,"called");
	grid_graph->SetColLabelValue(4,"name");
	grid_graph->SetRowLabelSize(0);
	grid_graph->EnableEditing(false);
	grid_graph->SetSelectionMode(wxGrid::wxGridSelectRows); // hacer que la seleccion sea por fila, y no por celda
	FillGraph();
	graph_sizer->Add(grid_graph,sizers->Exp1);
	graph_panel->SetSizer(graph_sizer);
	
	notebook->AddPage(table_panel,"table");
	notebook->AddPage(graph_panel,"callgraph");
	main_sizer->Add(notebook,sizers->Exp1);
	
	SetSizer(main_sizer);
	Show();
}

static wxString to_str(float f) {
	wxString s;
	s.Printf("%.2f",f);
	return s;
}

void mxGprofOutput::FillTable (const wxString &pattern) {
	int gn=grid_table->GetNumberRows(), dn=data.table.size();
	if (pattern.Len()) {
		for(int i=dn-1;i>=0;i--)
			if (!data.table[i].match(pattern.c_str())) dn--;
	}
	if (gn<dn) grid_table->AppendRows(dn-gn,false);
	else if (gn>dn) grid_table->DeleteRows(dn,gn-dn,false);
	for(unsigned int i=0,j=0;i<data.table.size();i++) { 
		if (!pattern.Len() || data.table[i].match(pattern.c_str())) {
			grid_table->SetCellValue(j,0,to_str(data.table[i].percent_time));	
			grid_table->SetCellValue(j,1,to_str(data.table[i].cumulative_seconds));	
			grid_table->SetCellValue(j,2,to_str(data.table[i].self_seconds));	
			grid_table->SetCellValue(j,3,wxString()<<data.table[i].calls);	
			grid_table->SetCellValue(j,4,to_str(data.table[i].self_s_calls));	
			grid_table->SetCellValue(j,5,to_str(data.table[i].total_s_calls));	
			grid_table->SetCellValue(j,6,data.table[i].name.c_str());	
			j++;
		}
	}
	grid_table->AutoSizeColumns();
}

void mxGprofOutput::OnClickTableLabel (wxGridEvent & event) {
	if (event.GetEventObject()!=grid_table) return;
	static int sorted=0;
	int new_sorted=event.GetCol()+1;
	if (new_sorted==sorted) {
		data.Sort(event.GetCol(),true);
		sorted=-new_sorted;
	} else {
		data.Sort(event.GetCol(),false);
		sorted=new_sorted;
	}
	FillTable();
}

void mxGprofOutput::FillGraph ( ) {
	int i=combo->GetSelection();
	if (i<0||i>=int(data.graph.size())) return;
	int sum=data.graph[i].calls_to.size()+data.graph[i].called_by.size()+2;
	if (sum>grid_graph->GetNumberRows()) grid_graph->AppendRows(sum-grid_graph->GetNumberRows());
	else if (sum<grid_graph->GetNumberRows()) grid_graph->DeleteRows(0,grid_graph->GetNumberRows()-sum);
	sum=0;
	grid_graph->SetCellValue(sum,0,""); grid_graph->SetCellValue(sum,1,""); grid_graph->SetCellValue(sum,2,"");
	grid_graph->SetCellValue(sum,3,""); grid_graph->SetCellValue(sum,4,"Es llamada por"); sum++;
	for(unsigned int j=0;j<data.graph[i].called_by.size();j++) { 
		grid_graph->SetCellValue(sum,0,to_str(data.graph[i].called_by[j].percent_time));
		grid_graph->SetCellValue(sum,1,to_str(data.graph[i].called_by[j].self));
		grid_graph->SetCellValue(sum,2,to_str(data.graph[i].called_by[j].children));
		grid_graph->SetCellValue(sum,3,data.graph[i].called_by[j].called.c_str());
		grid_graph->SetCellValue(sum,4,data.graph[i].called_by[j].name.c_str());
		sum++;
	}
	grid_graph->SetCellValue(sum,0,""); grid_graph->SetCellValue(sum,1,""); grid_graph->SetCellValue(sum,2,"");
	grid_graph->SetCellValue(sum,3,""); grid_graph->SetCellValue(sum,4,"Llama a"); sum++;
	for(unsigned int j=0;j<data.graph[i].calls_to.size();j++) { 
		grid_graph->SetCellValue(sum,0,to_str(data.graph[i].calls_to[j].percent_time));
		grid_graph->SetCellValue(sum,1,to_str(data.graph[i].calls_to[j].self));
		grid_graph->SetCellValue(sum,2,to_str(data.graph[i].calls_to[j].children));
		grid_graph->SetCellValue(sum,3,data.graph[i].calls_to[j].called.c_str());
		grid_graph->SetCellValue(sum,4,data.graph[i].calls_to[j].name.c_str());
		sum++;
	}	
	grid_graph->AutoSizeColumns();
}

void mxGprofOutput::OnComboChange (wxCommandEvent & event) {
	event.Skip();
	FillGraph();
}

void mxGprofOutput::OnSearchTextChange (wxCommandEvent & event) {
	FillTable(search_text->GetValue());
}

void mxGprofOutput::OnDClickTableCell (wxGridEvent & event) {
	wxString func;
	if (grid_table==((wxGrid*)event.GetEventObject())) {
		func=grid_table->GetCellValue(event.GetRow(),6)+" ";
	} else {
		func=grid_graph->GetCellValue(event.GetRow(),4).BeforeLast('[');
	}
	for(unsigned int i=0;i<combo->GetCount();i++) { 
		if (combo->GetString(i).BeforeLast('[')==func) {
			combo->SetSelection(i);
			FillGraph();
			notebook->SetSelection(1);
			break;
		}
	}
}

void mxGprofOutput::OnClickTableCell (wxGridEvent & event) {
	((wxGrid*)event.GetEventObject())->SelectRow(event.GetRow());
}

