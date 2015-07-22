#include <wx/menu.h>
#include "mxRegistersGrid.h"
#include "gdbParser.h"
#include "ids.h"

BEGIN_EVENT_TABLE(mxRegistersGrid,wxGrid)
	EVT_MENU(mxID_LAST_ID,mxRegistersGrid::ToggleShowAllRegisters)
END_EVENT_TABLE()

mxRegistersGrid::mxRegistersGrid(wxWindow *parent):mxGrid(parent,RG_COLS_COUNT),all_registers(false) {
	mxGrid::InitColumn(RG_COL_NAME,LANG(LOCALS_NAME,"Nombre"),10);
	mxGrid::InitColumn(RG_COL_VALUE_1,LANG(LOCALS_VALUE,"Valor"),20);
	mxGrid::InitColumn(RG_COL_VALUE_2,LANG(LOCALS_VALUE,"Valor"),30);
	mxGrid::DoCreate();
	EnableEditing(false);
	EnableDragRowSize(false);
	SetColLabelSize(wxGRID_AUTOSIZE);
}

struct BatchGuard {
	wxGrid *the_grid;
	BatchGuard(wxGrid *grid):the_grid(grid) { the_grid->BeginBatch(); }
	~BatchGuard() { the_grid->EndBatch(); }
	
};

wxString GetRegularGdbOutput(wxString s) {
	int l=s.Len(), i0=0, i1=0;
	wxString ret;
	do {
		while(i1<l && s[i1]!='\n') i1++;
		if (s[i0++]=='~') ret+=mxUT::UnEscapeString(s.Mid(i0,i1-i0));
		i0=++i1;
	} while (i1<l);
	return ret;
}

void mxRegistersGrid::Update ( ) {
	BatchGuard batch_guard(this);
	wxString s = debug->SendCommand(all_registers?"info all-registers":"info registers") + "\n";
	int num_rows = GetNumberRows();
	s = GetRegularGdbOutput(s);
	int p0=0, pi=0, l=s.Len(), n=0;
	while (pi!=wxNOT_FOUND) {
		while (pi<l && s[pi]!='\n') pi++;
		if (pi==l) break;
		wxString line = s.Mid(p0,pi-p0); p0=++pi;
		int i=0,ll=line.Len(); 
		if (!ll) continue;
		if (n == num_rows) { AppendRows(1); ++num_rows; }
		while (i<ll && (line[i]!=' '&&line[i]!='\t')) i++;
		wxString name = line.Mid(0,i); 
		mxGrid::SetCellValue(n,RG_COL_NAME,name);
		while (i<ll && (line[i]==' '||line[i]=='\t')) i++;
		if (line[i]=='{') { // algunos son vectores y se muestran en varios formatos como "{int={bla,bla},float={bla,bla}}"
			do {
				++i; while(i<ll && line[i]==' ') i++;
				int i_nombre = i;
				while(i<ll && (line[i]!=' '&&line[i]!='=')) i++;
				int i_fin_nombre = i;
				while(i<ll && (line[i]==' '||line[i]=='=')) i++;
				int i_value = i;
				while(i<ll && (line[i]!=','&&line[i]!='}')) {
					if (line[i]=='{') GdbParse_SkipList(line,i,ll); 
					++i;
				}
				if (n == num_rows) { AppendRows(1); ++num_rows; }
				mxGrid::SetCellValue(n,RG_COL_NAME,name);
				mxGrid::SetCellValue(n,RG_COL_VALUE_1,line.Mid(i_nombre,i_fin_nombre-i_nombre));
				mxGrid::SetCellValue(n,RG_COL_VALUE_2,line.Mid(i_value,i-i_value));
				n++;
			} while (i+1<ll && line[i]!='}');
		} else {
			int i0=i; while (i<ll && (line[i]!='\t')) i++;
			mxGrid::SetCellValue(n,RG_COL_VALUE_1,line.Mid(i0,i-i0));
			if (i0!=ll) mxGrid::SetCellValue(n,RG_COL_VALUE_2,line.Mid(i+1));
			n++;
		}
	}
	if (num_rows!=n) DeleteRows(n,num_rows-n);
}

void mxRegistersGrid::OnCellPopupMenu (int row, int col) {
	wxMenu menu; 
	menu.AppendCheckItem(mxID_LAST_ID,"Mostrar todos los registros")->Check(all_registers);
	PopupMenu(&menu);
}

void mxRegistersGrid::ToggleShowAllRegisters (wxCommandEvent & evt) {
	all_registers=!all_registers;
	Update();
}

