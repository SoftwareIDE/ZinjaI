#include <wx/grid.h>
#include <wx/textdlg.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include "mxInspectionMatrix.h"
#include "DebugManager.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxSizers.h"
#include "mxTextDialog.h"

BEGIN_EVENT_TABLE(mxInspectionMatrix, wxDialog)
	EVT_SIZE(mxInspectionMatrix::OnResize)
	EVT_GRID_COL_SIZE(mxInspectionMatrix::OnColResize)
	EVT_CHECKBOX(wxID_ANY,mxInspectionMatrix::OnAdaptCheck)
//	EVT_GRID_CELL_LEFT_DCLICK(mxInspectionMatrix::OnDoubleClick)
//	EVT_CLOSE(mxInspectionMatrix::OnClose)
//	EVT_KEY_DOWN(mxInspectionMatrix::OnCharHook)
END_EVENT_TABLE()

mxInspectionMatrix::mxInspectionMatrix(int inspection_num) : mxExternInspection(debug->inspections[inspection_num].frame,debug->inspections[inspection_num].expr) {
	
	inspectinfo &ii=debug->inspections[inspection_num];
	
	expression=ii.is_vo?ii.expr:ii.name; is_vo=ii.is_vo;
	
	created=false; is_class=ii.is_class;
	frame=debug->current_frame; is_cstring=false;
	// findo out size
	wxString data = ii.is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(ii.name);
	
	if (data[0]!='{' && data[0]!='\"') {
		// TODO: buscar si hay casteo y ver que pasa ( ")(" )
		wxString sn = mxGetTextFromUser(LANG(IMATRIX_ENTER_NUM,"Ingrese la cantidad de elementos a mostrar"),expression,"10",main_window);
		long n;
		sn.ToLong(&n);
		if (n)
			expression = wxString(_T("(*("))<<expression<<_T("))@")<<n;
		else {
			Destroy();
			return;
		}
		data = ii.is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(ii.name);
	}

	wxArrayString names;
	int l=data.Len();
	int dim1=1,dim2=1;
	const wxChar *cdata=data.c_str();
	const wxChar *inidata=cdata;
	const wxChar *enddata=cdata+data.Len();
	if (data[0]=='\"') {
		is_cstring=true;
		dim2=1; dim1=l-2;
		bool last_zero=true;
		for (int i=1;i<l-1;i++) {
			last_zero = false;
			if (data[i]=='\\') {
				i++;
				if (data[i]>='0' && data[i]<='9') {
					last_zero = (data[i]=='0' && data[i+1]=='0' && data[i+2]=='0');
					i+=2;
					dim1-=3;
				} else {
					dim1-=1;
				}
			}
		}
		if (last_zero) dim1++;
	} else {
		int p1=0;
		int llaves=0;
		bool comillas=false;
		while (cdata<enddata) {
			if (comillas) {
				switch (*cdata) {
					case '\"': comillas=false; break;
					case '\\': cdata++; break;			
				}
			} else {
				if (is_class) {
					switch (*cdata) {
						case '{': llaves++; break;
						case '}': llaves--; break;
						case ',': if (llaves==1) { p1=cdata-inidata; dim1++; } break;
						case '\"': comillas=true; break;
						case '\'': cdata++; if (*cdata=='\\') cdata++;
								   while (*cdata!='\'') cdata++; break;
						case '=': if (llaves==1) { if (data[p1+1]==' ') p1++; names.Add(data.Mid(p1+1,cdata-inidata-p1-1)); } break;
					}
				} else {
					switch (*cdata) {
						case '{': llaves++; break;
						case '\"': comillas=true; break;
						case '\'': cdata++; if (*cdata=='\\') cdata++; 
								   while (*cdata!='\'') cdata++; break;
						case '}': case ',': 
							if (llaves==1) dim1++; 
							else if (dim1==1 && llaves==2 && (*cdata!='}')) dim2++; 
							if (*cdata=='}') llaves--;
							break;
					}
				}
			}
			cdata++;
		}
		if (is_class) dim2=1;
	}
//	
	cols_sizes=new int[cols=dim1];
	grid = new wxGrid(this,wxID_ANY);
	if (!is_class)
		grid->SetDefaultCellAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	grid->CreateGrid(rows=dim1,cols=dim2);
	
	if (is_class) {
		for (int i=0;i<dim1&&i<int(names.GetCount());i++)
			grid->SetRowLabelValue(i,names[i]);
		grid->SetColLabelSize(0);
//		grid->SetRowLabelSize(wxGRID_AUTOSIZE);
		AutoSizeRowLabel();
		grid->SetColSize(1,400-grid->GetRowLabelSize());
		dont_adapt=false;
	} else {
		for (int i=0;i<dim1;i++)
			grid->SetRowLabelValue(i,wxString()<<i);
		if (dim2==1)
			grid->SetColLabelSize(0);
		else 
			for (int i=0;i<dim2;i++)
				grid->SetColLabelValue(i,wxString()<<i);
		AutoSizeRowLabel();
//		grid->SetRowLabelSize(wxGRID_AUTOSIZE);
		if (cols==1) {
			dont_adapt=false;
			grid->SetColSize(1,400-grid->GetRowLabelSize());
		} else {
			int csize=int(float(400-grid->GetRowLabelSize())/(dim2));
			dont_adapt=csize<50;
			if (dont_adapt) csize=50;
			for (int i=0;i<dim2;i++)
				grid->SetColSize(i,csize);
		}
	}
	
	mySizer = new wxBoxSizer(wxVERTICAL);
//	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
//	wxButton *close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.ok,_T("&Cerrar"));
//	SetEscapeId(wxID_CANCEL);
//	buttonSizer->Add(close_button,sizers->BA5_Right);
//	
	mySizer->Add(grid,sizers->Exp1);
	mySizer->Add(adapt=new wxCheckBox(this,wxID_ANY,_T("Ajustar columnas")),sizers->Right);
//	mySizer->Add(buttonSizer,sizers->Exp0);
//	
	SetSize(400,300);
	SetSizer(mySizer);
	if (is_class) Fit();
//	
	created=true;
	wxSizeEvent evt(this->GetSize());
	OnResize(evt);
	
//	grid->SetFocus();
	adapt->SetValue(!dont_adapt);
//	debug->RegisterMatrix(this);
	grid->EnableEditing(false);
	Update(); grid->AutoSize();
	Show();	
}

mxInspectionMatrix::~mxInspectionMatrix() {
}

void mxInspectionMatrix::Update() {
	grid->BeginBatch();
	debug->SetFullOutput(true);
	wxString data = is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(expression);
	debug->SetFullOutput(false);
	int l=data.Len();
	bool comillas=false;
	const wxChar *cdata=data.c_str();
	const wxChar *inidata=cdata;
	const wxChar *enddata=cdata+data.Len();
	
	if (is_cstring) {
		int p=0;
		for (int i=1;i<l-1;i++) {
			if (cdata[i]!='\\') {
				grid->SetCellValue(p,0,wxString()<<cdata[i]);
			} else {
				i++;
				if (cdata[i]>='0' && cdata[i]<='9') {
					grid->SetCellValue(p,0,wxString(_T("\\"))<<cdata[i]<<cdata[i+1]<<cdata[i+2]);
					i+=2;
				} else
					grid->SetCellValue(p,0,wxString(_T("\\"))<<cdata[i]);
			}
			p++;
		}
		if (p==rows-1) {
			grid->SetCellValue(p,0,wxString()<<_T("\\000"));
		}
	} else {
		int dim1=1,dim2=1,p1=0, llaves=0;
		while (cdata<enddata) {
			if (comillas) {
				switch (*cdata) {
				case '\"': comillas=false; break;
				case '\\': cdata++; break;			
				}
			} else {
				if (is_class) {
					switch (*cdata) {
						case '{': llaves++; break;
						case '}': llaves--; if (cdata-inidata+1<l) break; // sigue abajo en ','... ya no, sino se come un lugar en clases con arreglo en el 1er atributo
						case ',': if (llaves<2) { 
							if (rows<dim1) { grid->AppendRows(1,false); rows++; }
							grid->SetCellValue(dim1-1,0,data.Mid(p1+1,cdata-inidata-p1-1)); dim1++; 
							p1=cdata-inidata;} break;
						case '\"': comillas=true; break;
						case '\'': cdata++; if (*cdata=='\\') cdata++;
							       while (*cdata!='\'') cdata++; break;
						case '=': if (llaves==1) {
							if (rows<dim1) { grid->AppendRows(1,false); rows++; }
							grid->SetRowLabelValue(dim1-1,data.Mid(p1+1,cdata-inidata-p1-1));
							p1=cdata-inidata; break;
						}
					}
				} else {
					if ( (*cdata==',' || *cdata=='}') && ( ( cols==1 && llaves==1 ) || ( cols>1 && llaves==2 ) ) ) {
						if (rows<dim1) { grid->AppendRows(1,false); rows++; }
						grid->SetCellValue(dim1-1,dim2-1,data.Mid(p1+1,cdata-inidata-p1-1));
						p1=cdata-inidata;
					}
					switch (*cdata) {
//						case '}': llaves--; break;
						case '{': llaves++; break;
						case '\"': comillas=true; break;
						case '\'': cdata++; if (*cdata=='\\') cdata++; 
								   while (*cdata!='\'') cdata++; break;
						case '}': case ',': 
							if (llaves==1) { dim1++;dim2=1;p1=cdata-inidata; } 
							else if (llaves==2) dim2++; 
							if (*cdata=='}') llaves--; 
							break;
					}
					if ( (cols==1 && llaves==1 && *cdata==',') || (cols>1 && ( llaves==2 && ( *cdata==',' || *cdata=='{' ) ) ) ) 
						p1=cdata-inidata;
				}
			}
			cdata++;
		}
		if (rows>=dim1) {
			grid->DeleteRows(rows-(rows+1-dim1),rows+1-dim1,false);
			rows-=rows+1-dim1;
		}
	}
//	grid->AutoSize();
	grid->EndBatch();
}

void mxInspectionMatrix::OnResize(wxSizeEvent &evt) {
	if (created && !dont_adapt) {
		if (is_class || cols==1) {
			int ns = evt.GetSize().GetWidth()-grid->GetRowLabelSize()-10;
			grid->SetColSize(0,ns);
		} else {
			int csize=int(float(evt.GetSize().GetWidth()-grid->GetRowLabelSize()-10)/(cols));
			for (int i=0;i<cols;i++) grid->SetColSize(i,csize);
		}
	}
	evt.Skip();
	Refresh();
}

void mxInspectionMatrix::OnColResize(wxGridSizeEvent &evt) {
	int x=grid->GetColSize(evt.GetRowOrCol());
	grid->BeginBatch();
	for (int i=0;i<cols;i++)
		grid->SetColSize(i,x);
	grid->EndBatch();
	grid->ForceRefresh();	
//	if (old_size) cols_sizes[evt.GetRowOrCol()]=grid->GetColSize(evt.GetRowOrCol());
}

//void mxInspectionMatrix::OnClose(wxCloseEvent &evt) {
////	debug->UnRegisterMatrix(this);
//	Destroy();
//}

void mxInspectionMatrix::OnAdaptCheck(wxCommandEvent &evt) {
	if (!(dont_adapt=!adapt->GetValue())) {
		int ancho = this->GetSize().GetWidth();
		if (is_class || cols==1) {
			grid->SetColSize(1,ancho-grid->GetRowLabelSize()-10);
		} else {
			int csize=int(float(ancho-grid->GetRowLabelSize()-10)/(cols));
			for (int i=0;i<cols;i++) grid->SetColSize(i,csize);
		}
	}
}

void mxInspectionMatrix::AutoSizeRowLabel() {
	grid->SetRowLabelSize(wxGRID_AUTOSIZE);	
	grid->SetRowLabelSize(grid->GetRowLabelSize());
}

bool mxInspectionMatrix::SetOutdated() {
	wxStaticText *st=new wxStaticText(this,wxID_ANY,LANG(IMATRIX_OBSOLETE,"Tabla obsoleta"));
	st->SetForegroundColour(wxColour("RED"));
	mySizer->Add(st,sizers->BA5_Center);
	Layout();
	return true;
}
