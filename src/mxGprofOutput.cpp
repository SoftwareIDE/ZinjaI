#include "mxGprofOutput.h"
#include "mxSizers.h"
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "mxMessageDialog.h"
#include "mxPlainResultsTable.h"

static wxString to_str(float f) {
	wxString s;
	s.Printf("%.2f",f);
	return s;
}

class mxPRTGprofTable : public mxPRTData {
	mxGprofOutput *parent;
	GprofData *data;
public:
	mxPRTGprofTable(mxGprofOutput *pparent, GprofData &gprof_data) 
		: parent(pparent), data(&gprof_data) {}
	int GetRowsCount() { return data->table.size(); }
	int GetColumnsCount() { return 7; }
	bool EnableFilter() { return true; }
	bool RowMatches(int row, const wxString &filter) { 
		return !filter.Len() || data->table[row].match(filter.c_str());
	}
	wxString GetColumnName(int colum) {
		switch(colum) {
		case 0: return "% time";
		case 1: return "cum. secs";
		case 2: return "self secs";
		case 3: return "calls";
		case 4: return "self s/call";
		case 5: return "total s/call";
		case 6: return "name";
		default: return "";
		}
	}
	wxString GetData(int row, int column) {
		switch(column) {
		case 0: return to_str(data->table[row].percent_time);
		case 1: return to_str(data->table[row].cumulative_seconds);
		case 2: return to_str(data->table[row].self_seconds);
		case 3: return wxString()<<data->table[row].calls;
		case 4: return to_str(data->table[row].self_s_calls);
		case 5: return to_str(data->table[row].total_s_calls);
		case 6: return data->table[row].name.c_str();
		default: return "";
		}
	}
	bool Sort(int column, bool ascending) { 
		data->Sort(column,ascending); return true;
	}
	void OnDClick(int row, int column) {
		parent->ShowFunction(GetCellValue(row,6));
	}
};


class mxPRTGprofFunction: public mxPRTData {
	mxGprofOutput *parent;
	GprofData *data;
	int i; ///< cual funcion de data.graph mostrar
public:
	mxPRTGprofFunction(mxGprofOutput *pparent, GprofData &gprof_data, int ifunc=0) 
		: parent(pparent), data(&gprof_data), i(ifunc) {}
	virtual int GetRowsCount() {
		return data->graph[i].calls_to.size()+data->graph[i].called_by.size()+2;
	}
	virtual int GetColumnsCount() { return 5; }
	virtual wxString GetColumnName(int column) {
		switch(column) {
		case 0: return "% time";
		case 1: return "self";
		case 2: return "children";
		case 3: return "called";
		case 4: return "name";
		default: return "";
		}
	}
	virtual wxString GetData(int row, int column) {
		int pos_calls_to = 1+data->graph[i].called_by.size();
		if (row==0||row==pos_calls_to) {
			if (column!=4) return "";
			return row==0?"Es llamada por":"Llama a";
		} else {
			bool called_by = row<pos_calls_to;
			row -= 1+(called_by?0:pos_calls_to);
			vector<GprofData::call_item> &v = called_by?data->graph[i].called_by:data->graph[i].calls_to;
			switch(column) {
			case 0: return to_str(v[row].percent_time);
			case 1: return to_str(v[row].self);
			case 2: return to_str(v[row].children);
			case 3: return v[row].called.c_str();
			case 4: return v[row].name.c_str();
			default: return "";
			}
		}
	}
	virtual void OnDClick(int row, int column) {
		parent->ShowFunction(GetCellValue(row,4).BeforeLast('['));
	}
};

BEGIN_EVENT_TABLE(mxGprofOutput,wxDialog)
	EVT_COMBOBOX(wxID_ANY,mxGprofOutput::OnComboChange)
END_EVENT_TABLE()

mxGprofOutput::mxGprofOutput(wxWindow *parent, wxString fname)
	: wxDialog(parent, wxID_ANY, "gprof output", wxDefaultPosition, wxSize(700,400),
               wxALWAYS_SHOW_SB|wxALWAYS_SHOW_SB|wxDEFAULT_FRAME_STYLE|wxSUNKEN_BORDER),
	data(fname.c_str()) 
{
	BoolFlagGuard combo_guard(mask_combo_events);
	
	if (!data.graph.size()) { 
		mxMessageDialog(parent,"No se han podido leer resultados.").Title("gprof").IconError().Run();
		Destroy(); return; 
	}
	wxSizer *main_sizer=new wxBoxSizer(wxVERTICAL);
	
	notebook = new wxNotebook(this,wxID_ANY);
	
	grid_table = new mxPlainResultsTable(notebook,new mxPRTGprofTable(this,data));
	
	wxPanel *graph_panel=new wxPanel(notebook,wxID_ANY);
	grid_graph = new mxPlainResultsTable(graph_panel,new mxPRTGprofFunction(this,data));
	
	wxSizer *graph_sizer=new wxBoxSizer(wxVERTICAL);
	wxArrayString array;
	for(unsigned int i=0;i<data.graph.size();i++) array.Add(data.graph[i].name.c_str());
	combo = new wxComboBox(graph_panel,wxID_ANY,array[0],wxDefaultPosition,wxDefaultSize,array,wxCB_READONLY);
	combo->SetSelection(0);
	graph_sizer->Add(combo,sizers->Exp0);
	graph_sizer->Add(grid_graph,sizers->Exp1);
	graph_panel->SetSizer(graph_sizer);
	
	notebook->AddPage(grid_table,"table");
	notebook->AddPage(graph_panel,"callgraph");
	main_sizer->Add(notebook,sizers->Exp1);
	
	SetSizer(main_sizer);
	Show();
}

void mxGprofOutput::ShowFunction(int i) {
	BoolFlagGuard combo_guard(mask_combo_events);
	combo->SetSelection(i);
	grid_graph->SetData(new mxPRTGprofFunction(this,data,i));
	notebook->SetSelection(1);
}

void mxGprofOutput::OnComboChange (wxCommandEvent & event) {
	event.Skip();
	if (mask_combo_events) return;
	int i=combo->GetSelection();
	if (i<0||i>=int(data.graph.size())) return;
	ShowFunction(i);
}

void mxGprofOutput::ShowFunction (wxString func) {
	if (func.Last()!=' ') func+=" ";
	for(unsigned int i=0;i<combo->GetCount();i++) { 
		if (combo->GetString(i).BeforeLast('[')==func) {
			ShowFunction(i);
			break;
		}
	}
}

