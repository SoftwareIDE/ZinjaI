#include "mxLocalsGrid.h"
#include "Language.h"

mxLocalsGrid::mxLocalsGrid(wxWindow *parent):mxGrid(parent,LG_COLS_COUNT) {
	mxGrid::InitColumn(LG_COL_NAME,LANG(LOCALS_NAME,"Nombre"),10);
	mxGrid::InitColumn(LG_COL_TYPE,LANG(LOCALS_TYPE,"Tipo"),19);
	mxGrid::InitColumn(LG_COL_VALUE,LANG(LOCALS_VALUE,"Valor"),21);
	mxGrid::SetColumnVisible(LG_COL_TYPE,false);
	mxGrid::DoCreate();
	mxGrid::SetRowSelectionMode();
	EnableEditing(false);
	EnableDragRowSize(false);
	SetColLabelSize(wxGRID_AUTOSIZE);
	command = "-stack-list-variables";
}

static int AuxParsingFindFirst(const wxString &s, const wxString &name) {
	for(int i=6,l=s.Len(),ln=name.Len();i<l-ln-1;i++) { // empieza en 6 porque primero dice algo como "^done,...."
		bool found=true;
		for(int j=0;found&&j<ln;j++) { if (s[i+j]!=name[j]) found=false; }
		if (found && s[i+ln]=='=') return i;
	}
	return -1;
}

static bool AuxParsingGetNext(const wxString &s, int &i, wxString &key, wxString &val) {
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

void mxLocalsGrid::Update ( ) {
	if (!IsCurrentInspectionsTab()) return;
	int n = GetNumberRows();
	if (!debug->IsDebugging() || !debug->IsPaused()) {
		DeleteRows(0,n); n=0; return;
	}
	
	BeginBatch();
	
	wxString s = debug->SendCommand(command," --all-values");
	if (s.StartsWith("^error") && s.Contains("Undefined MI command")) {
		// fallback to deprecated command is gdb is old
		command = "-stack-list-locals";
		s = debug->SendCommand(command," --all-values");
	}
	
	int i = /*DebugManager::*/AuxParsingFindFirst(s,"name"), c=0;
	wxString name,value, key,val;
	while(true) {
		bool add = /*DebugManager::*/AuxParsingGetNext(s,i,key,val);
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
		int i = /*DebugManager::*/AuxParsingFindFirst(s,"name"), c=0;
		while(true) {
			/*bool add =*/ /*DebugManager::*/AuxParsingGetNext(s,i,key,val);
			if (c==n||i==-1) break;
			if (key=="type") mxGrid::SetCellValue(c++,LG_COL_TYPE,val);
		}
	}
	
	EndBatch();
	
}


