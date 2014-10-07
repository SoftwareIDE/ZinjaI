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
//#include "mxTextDialog.h"
#include "Inspection.h"
//
BEGIN_EVENT_TABLE(mxInspectionMatrix, wxPanel)
	EVT_SIZE(mxInspectionMatrix::OnResize)
	EVT_GRID_COL_SIZE(mxInspectionMatrix::OnColResize)
	EVT_CHECKBOX(wxID_ANY,mxInspectionMatrix::OnAdaptCheck)
////	EVT_GRID_CELL_LEFT_DCLICK(mxInspectionMatrix::OnDoubleClick)
////	EVT_CLOSE(mxInspectionMatrix::OnClose)
//	EVT_KEY_DOWN(mxInspectionMatrix::OnChar)
END_EVENT_TABLE()
//
mxInspectionMatrix::mxInspectionMatrix(const wxString &expression, bool is_frameless) : wxPanel(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	
	created=false; do_adapt=true; cols=rows=0; 
	
//	is_class=ii.is_class;
//	frame=debug->current_frame; is_cstring=false;
//	// findo out size
//	wxString data = ii.is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(ii.name);
//	
//	if (data[0]!='{' && data[0]!='\"') {
//		// TODO: buscar si hay casteo y ver que pasa ( ")(" )
//		wxString sn = mxGetTextFromUser(LANG(IMATRIX_ENTER_NUM,"Ingrese la cantidad de elementos a mostrar"),expression,"10",main_window);
//		long n;
//		sn.ToLong(&n);
//		if (n)
//			expression = wxString(_T("(*("))<<expression<<_T("))@")<<n;
//		else {
//			Destroy();
//			return;
//		}
//		data = ii.is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(ii.name);
//	}
//
//	wxArrayString names;
//	int l=data.Len();
//	int dim1=1,dim2=1;
//	const wxChar *cdata=data.c_str();
//	const wxChar *inidata=cdata;
//	const wxChar *enddata=cdata+data.Len();
//	if (data[0]=='\"') {
//		is_cstring=true;
//		dim2=1; dim1=l-2;
//		bool last_zero=true;
//		for (int i=1;i<l-1;i++) {
//			last_zero = false;
//			if (data[i]=='\\') {
//				i++;
//				if (data[i]>='0' && data[i]<='9') {
//					last_zero = (data[i]=='0' && data[i+1]=='0' && data[i+2]=='0');
//					i+=2;
//					dim1-=3;
//				} else {
//					dim1-=1;
//				}
//			}
//		}
//		if (last_zero) dim1++;
//	} else {
//		int p1=0;
//		int llaves=0;
//		bool comillas=false;
//		while (cdata<enddata) {
//			if (comillas) {
//				switch (*cdata) {
//					case '\"': comillas=false; break;
//					case '\\': cdata++; break;			
//				}
//			} else {
//				if (is_class) {
//					switch (*cdata) {
//						case '{': llaves++; break;
//						case '}': llaves--; break;
//						case ',': if (llaves==1) { p1=cdata-inidata; dim1++; } break;
//						case '\"': comillas=true; break;
//						case '\'': cdata++; if (*cdata=='\\') cdata++;
//								   while (*cdata!='\'') cdata++; break;
//						case '=': if (llaves==1) { if (data[p1+1]==' ') p1++; names.Add(data.Mid(p1+1,cdata-inidata-p1-1)); } break;
//					}
//				} else {
//					switch (*cdata) {
//						case '{': llaves++; break;
//						case '\"': comillas=true; break;
//						case '\'': cdata++; if (*cdata=='\\') cdata++; 
//								   while (*cdata!='\'') cdata++; break;
//						case '}': case ',': 
//							if (llaves==1) dim1++; 
//							else if (dim1==1 && llaves==2 && (*cdata!='}')) dim2++; 
//							if (*cdata=='}') llaves--;
//							break;
//					}
//				}
//			}
//			cdata++;
//		}
//		if (is_class) dim2=1;
//	}
////	
//	cols_sizes=new int[cols=dim1];
	
//	if (!is_class)
//		grid->SetDefaultCellAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
//	grid->CreateGrid(rows=dim1,cols=dim2);
//	
//	if (is_class) {
//		for (int i=0;i<dim1&&i<int(names.GetCount());i++)
//			grid->SetRowLabelValue(i,names[i]);
//		grid->SetColLabelSize(0);
////		grid->SetRowLabelSize(wxGRID_AUTOSIZE);
//		AutoSizeRowLabel();
//		grid->SetColSize(1,400-grid->GetRowLabelSize());
//		dont_adapt=false;
//	} else {
//		for (int i=0;i<dim1;i++)
//			grid->SetRowLabelValue(i,wxString()<<i);
//		if (dim2==1)
//			grid->SetColLabelSize(0);
//		else 
//			for (int i=0;i<dim2;i++)
//				grid->SetColLabelValue(i,wxString()<<i);
//		AutoSizeRowLabel();
////		grid->SetRowLabelSize(wxGRID_AUTOSIZE);
//		if (cols==1) {
//			dont_adapt=false;
//			grid->SetColSize(1,400-grid->GetRowLabelSize());
//		} else {
//			int csize=int(float(400-grid->GetRowLabelSize())/(dim2));
//			dont_adapt=csize<50;
//			if (dont_adapt) csize=50;
//			for (int i=0;i<dim2;i++)
//				grid->SetColSize(i,csize);
//		}
//	}
//	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
////	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
////	wxButton *close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.ok,_T("&Cerrar"));
////	SetEscapeId(wxID_CANCEL);
////	buttonSizer->Add(close_button,sizers->BA5_Right);
////	
	grid = new wxGrid(this,wxID_ANY); grid->CreateGrid(0,0);
	mySizer->Add(grid,sizers->Exp1);
	mySizer->Add(adapt=new wxCheckBox(this,wxID_ANY,LANG(IMATRIX_AUTOFIT_COLUMNS,"Auto-ajustar columnas")),sizers->Right);
////	mySizer->Add(buttonSizer,sizers->Exp0);
////	
	wxSize sz(400,300);
	SetSizer(mySizer);
	
	(di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,is_frameless)|DIF_AUTO_IMPROVE,this,false))->Init();
	
	main_window->aui_manager.AddPane(this,wxAuiPaneInfo().Name("inspection_matrix").Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption(expression).Show().FloatingPosition(wxGetMousePosition()-wxPoint(25,10)).BestSize(sz));
	main_window->aui_manager.Update();
	
	created = true;
	
	wxCommandEvent e; OnAdaptCheck(e); // para que ajuste los tamaños de columnas.... mejorar! 
}

void mxInspectionMatrix::SetMatrixSize(int w, int h) {
	if (w<cols) grid->DeleteCols(w,cols-w);
	else if (w>cols) grid->InsertCols(cols,w-cols);
	if (h<rows) grid->DeleteRows(h,rows-h);
	else if (h>rows) grid->InsertRows(rows,h-rows);
	cols=w; rows=h;
	grid->SetColLabelSize(w>1?wxGRID_AUTOSIZE:0);
}


/**
* @brief Saltea una cadena completa
*
* Dada una posición donde comienza una lista (donde hay un '{')
* avanza la posición hasta llegar al final de la misma
**/
void GdbParse_SkipList(const wxString &s, int &i, int l) {
	wxChar c=s[i++];
	while (i<l && s[i]!=c) {
		if (s[i]=='\\') i++;
		i++;
	}
}

/**
* @brief Saltea una lista completa
*
* Dada una posición donde comienza una lista (donde hay un '{')
* avanza la posición hasta llegar al final de la misma.
* Para determinar los limites de la lista (y sublistas dentro
* de esta), los delimitares validos son {..}, [..] y (..), y
* son todos equivalentes.
**/
void GdbParse_SkipString(const wxString &s, int &i, int l) {
	int balance=0;
	do {
		if (s[i]=='\"'||s[i]=='\'') GdbParse_SkipString(s,i,l);
		else if (s[i]=='['||s[i]=='{'||s[i]=='(') balance++;
		else if (s[i]==']'||s[i]=='}'||s[i]==')') balance--;
		i++;
	} while (balance && i<l);
}

/**
* @brief Saltea espacios en blanco
*
* Avanza i mientras sea menor a la longitud de la cadena (l), y 
* los caracteres sean enter, espacio o tab.
**/
static void GdbParse_SkipEmpty(const wxString &s, int &i, int l) {
	while(i<l && (s[i]==' '||s[i]=='\t'||s[i]=='\n')) i++;
}

/**
* @brief Determina si el siguiente elemento es el comienzo de una lista
*
* A partir de la posición i, busca el primer caracter que no nulo (nulo=enter,
* tab,espacio) y returna true si este es una llave que abre una lisat ('{'), o
* false si es otra cosa, o si se llegó al final de la cadena
**/
static bool GdbParse_IsList(const wxString s, int &i, int l=-1) {
	if (l==-1) l=s.Len(); 
	GdbParse_SkipEmpty(s,i,l);
	return i<l && s[i]=='{';
}

static bool GdbParse_GetPair(const wxString &s, int &i, int &pos_st, int &pos_eq, int &pos_end, int l=-1) {
	if (l==-1) l=s.Len(); 
	GdbParse_SkipEmpty(s,i,l);
	if (i>=l || s[i]=='}' || s[i]==']' || s[i]==')') return false;
	pos_st=i; pos_eq=-1;
	while(i<l && s[i]!='}' && s[i]!=']' && s[i]!=')' && s[i]!=',') {
		if (s[i]=='=') {
			if (pos_eq==-1) pos_eq=i;
		} else if (s[i]=='{'||s[i]=='('||s[i]=='[') {
			GdbParse_SkipList(s,i,l);
		} else if (s[i]=='\"'||s[i]=='\'') {
			GdbParse_SkipString(s,i,l); 
		}
		i++;
	}
	pos_end=i;
	return true;
}

void mxInspectionMatrix::Update() {
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
		return;
	}
	
	while (GdbParse_GetPair(s,i,pos_st,pos_eq,pos_end)) { 
		h++; i++;
		// ver si el elemento es lista para armar la matriz 2D
		int aux_i = pos_eq==-1?pos_st:pos_eq+1;
		if (GdbParse_IsList(s,aux_i,pos_end)) {
			aux_i++;
			int aux_w=0,  aux_st, aux_eq, aux_end;
			while (GdbParse_GetPair(s,aux_i,aux_st,aux_eq,aux_end,pos_end)) { 
				aux_w++; aux_i++;
			}
			if (aux_w>w) w=aux_w;
		}
	}
	SetMatrixSize(w,h);
	
	i=i0; h=0;
	while (GdbParse_GetPair(s,i,pos_st,pos_eq,pos_end)) {
		int aux_i=pos_eq==-1?pos_st:pos_eq+1;
		if (GdbParse_IsList(s,aux_i,pos_end)) {
			aux_i++;
			int aux_w=0, aux_st, aux_eq, aux_end;
			while (GdbParse_GetPair(s,aux_i,aux_st,aux_eq,aux_end,pos_end)) { 
				grid->SetCellValue( h, aux_w, aux_eq==-1 ? s.Mid(aux_st,aux_end-aux_st) : s.Mid(aux_eq+1,aux_end-aux_eq-1) ); 
				aux_w++; aux_i++;
			}
			if (aux_w>w) w=aux_w;
		} else {
			grid->SetRowLabelValue( h, pos_eq==-1 ? wxString()<<h : s.Mid(pos_st,pos_eq-pos_st) ); 
			grid->SetCellValue( h, 0, pos_eq==-1 ? s.Mid(pos_st,pos_end-pos_st) : s.Mid(pos_eq+1,pos_end-pos_eq-1) ); 
		}
		h++; i++;
	}
	
//	debug->SetFullOutput(true);
//	wxString data = is_vo?debug->InspectExpression(expression):debug->GetMacroOutput(expression);
//	debug->SetFullOutput(false);
//	int l=data.Len();
//	const wxChar *cdata=data.c_str();
//	const wxChar *inidata=cdata;
//	const wxChar *enddata=cdata+data.Len();
//	
//	if (is_cstring) {
//		int p=0;
//		for (int i=1;i<l-1;i++) {
//			if (cdata[i]!='\\') {
//				grid->SetCellValue(p,0,wxString()<<cdata[i]);
//			} else {
//				i++;
//				if (cdata[i]>='0' && cdata[i]<='9') {
//					grid->SetCellValue(p,0,wxString(_T("\\"))<<cdata[i]<<cdata[i+1]<<cdata[i+2]);
//					i+=2;
//				} else
//					grid->SetCellValue(p,0,wxString(_T("\\"))<<cdata[i]);
//			}
//			p++;
//		}
//		if (p==rows-1) {
//			grid->SetCellValue(p,0,wxString()<<_T("\\000"));
//		}
//	} else {
//		bool comillas=false;
//		int dim1=1,dim2=1,p1=0, llaves=0;
//		while (cdata<enddata) {
//			if (comillas) {
//				switch (*cdata) {
//				case '\"': comillas=false; break;
//				case '\\': cdata++; break;			
//				}
//			} else {
//				if (is_class) {
//					switch (*cdata) {
//						case '{': llaves++; break;
//						case '}': llaves--; if (cdata-inidata+1<l) break; // sigue abajo en ','... ya no, sino se come un lugar en clases con arreglo en el 1er atributo
//						case ',': if (llaves<2) { 
//							if (rows<dim1) { grid->AppendRows(1,false); rows++; }
//							grid->SetCellValue(dim1-1,0,data.Mid(p1+1,cdata-inidata-p1-1)); dim1++; 
//							p1=cdata-inidata;} break;
//						case '\"': comillas=true; break;
//						case '\'': cdata++; if (*cdata=='\\') cdata++;
//							       while (*cdata!='\'') cdata++; break;
//						case '=': if (llaves==1) {
//							if (rows<dim1) { grid->AppendRows(1,false); rows++; }
//							grid->SetRowLabelValue(dim1-1,data.Mid(p1+1,cdata-inidata-p1-1));
//							p1=cdata-inidata; break;
//						}
//					}
//				} else {
//					if ( (*cdata==',' || *cdata=='}') && ( ( cols==1 && llaves==1 ) || ( cols>1 && llaves==2 ) ) ) {
//						if (rows<dim1) { grid->AppendRows(1,false); rows++; }
//						grid->SetCellValue(dim1-1,dim2-1,data.Mid(p1+1,cdata-inidata-p1-1));
//						p1=cdata-inidata;
//					}
//					switch (*cdata) {
////						case '}': llaves--; break;
//						case '{': llaves++; break;
//						case '\"': comillas=true; break;
//						case '\'': cdata++; if (*cdata=='\\') cdata++; 
//								   while (*cdata!='\'') cdata++; break;
//						case '}': case ',': 
//							if (llaves==1) { dim1++;dim2=1;p1=cdata-inidata; } 
//							else if (llaves==2) dim2++; 
//							if (*cdata=='}') llaves--; 
//							break;
//					}
//					if ( (cols==1 && llaves==1 && *cdata==',') || (cols>1 && ( llaves==2 && ( *cdata==',' || *cdata=='{' ) ) ) ) 
//						p1=cdata-inidata;
//				}
//			}
//			cdata++;
//		}
//		if (rows>=dim1) {
//			grid->DeleteRows(rows-(rows+1-dim1),rows+1-dim1,false);
//			rows-=rows+1-dim1;
//		}
//	}
////	grid->AutoSize();
	grid->EndBatch();
}

void mxInspectionMatrix::OnResize(wxSizeEvent &evt) {
	if (created && do_adapt) {
		if (/*is_class || */cols==1) {
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

////void mxInspectionMatrix::OnClose(wxCloseEvent &evt) {
//////	debug->UnRegisterMatrix(this);
////	Destroy();
//// }

void mxInspectionMatrix::OnAdaptCheck(wxCommandEvent &evt) {
	if ((do_adapt==adapt->GetValue())) {
		int ancho = this->GetSize().GetWidth();
		if (/*is_class || */cols==1) {
			grid->SetColSize(1,ancho-grid->GetRowLabelSize()-10);
		} else {
			int csize=int(float(ancho-grid->GetRowLabelSize()-10)/(cols));
			for (int i=0;i<cols;i++) grid->SetColSize(i,csize);
		}
	}
}
//
//void mxInspectionMatrix::AutoSizeRowLabel() {
//	grid->SetRowLabelSize(wxGRID_AUTOSIZE);	
//	grid->SetRowLabelSize(grid->GetRowLabelSize());
//}
//
//bool mxInspectionMatrix::SetOutdated() {
//	wxStaticText *st=new wxStaticText(this,wxID_ANY,LANG(IMATRIX_OBSOLETE,"Tabla obsoleta"));
//	st->SetForegroundColour(wxColour("RED"));
//	mySizer->Add(st,sizers->BA5_Center);
//	Layout();
//	return true;
//}
//
//void mxInspectionMatrix::OnChar (wxKeyEvent & event) {
//	wxMessageBox("lala");
//}

void mxInspectionMatrix::OnDICreated (DebuggerInspection * di) {
	this->di=di; Update();
}

void mxInspectionMatrix::OnDIError (DebuggerInspection * di) {
	this->di=di; Update();
}

void mxInspectionMatrix::OnDIValueChanged (DebuggerInspection * di) {
	Update();
}

void mxInspectionMatrix::OnDIOutOfScope (DebuggerInspection * di) {
	
}

void mxInspectionMatrix::OnDIInScope (DebuggerInspection * di) {
	Update();
}

void mxInspectionMatrix::OnDINewType (DebuggerInspection * di) {
	Update();
}

mxInspectionMatrix::~mxInspectionMatrix ( ) {
	di->Destroy();
}

