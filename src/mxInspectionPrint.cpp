#include <wx/grid.h>
#include <wx/textdlg.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include "mxInspectionPrint.h"
#include "DebugManager.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxMainWindow.h"
#include <cmath>

mxInspectionPrint::mxInspectionPrint(wxString expression, bool frameless) : wxPanel(main_window,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	value  = new wxTextCtrl(this,wxID_ANY,DebuggerInspection::GetUserStatusText(DIMSG_PENDING),wxDefaultPosition,wxDefaultSize,wxTE_READONLY|wxTE_MULTILINE);
	type = new wxStaticText(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxST_NO_AUTORESIZE);
	sizer->Add(value,sizers->Exp1);
	sizer->Add(type,sizers->Exp0);
	SetSizer(sizer);
	wxSize sz(150,50);
	di = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless),this,true);
	wxString s = di->GetValue();
	if (s.Len()) {
		// calcular tamaño de la ventana para el texto que tiene y con la relacion de aspecto a
		// la idea es, con texto en una line tamaño w*h => defino T=w*h, y quiero w*a=h.. con esas
		// dos ecuaciones despejo w y h (en base a T y a), y despues aplico un "coeficiente de 
		// seguridad" cf (porque el texto hace wrap por palabras, y yo calculo como si fuera por 
		// pixeles), y +h0 para considerar el statictext con el tipo a modo de barra de estado
		wxScreenDC dc;
		dc.SetFont(value->GetFont());
		wxSize aux = dc.GetTextExtent(s);
		double w=aux.GetWidth(), h=aux.GetHeight();
		double a=2, T=w*h, h0=h, cs=1.2;
		if (w>200) {
			h=sqrt(T/a); if (h>400) h=400; w=h*a;
			sz = wxSize(w*cs,h*cs+h0);
		}
	}	
	main_window->aui_manager.AddPane(this,wxAuiPaneInfo().Name("inspection_print").Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption(expression).Show().FloatingPosition(wxGetMousePosition()-wxPoint(25,10)).BestSize(sz));
	main_window->aui_manager.Update();
}

void mxInspectionPrint::OnDICreated (DebuggerInspection * di) {
	type->SetLabel(di->GetValueType());
	value->SetValue(di->GetValue());
}

void mxInspectionPrint::OnDIError (DebuggerInspection * di) {
	type->SetLabel(DebuggerInspection::GetUserStatusText(DIMSG_PENDING));
}

void mxInspectionPrint::OnDIValueChanged (DebuggerInspection * di) {
	value->SetValue(di->GetValue());
}

void mxInspectionPrint::OnDIOutOfScope (DebuggerInspection * di) {
	type->SetLabel(DebuggerInspection::GetUserStatusText(DIMSG_OUT_OF_SCOPE));
}

void mxInspectionPrint::OnDIInScope (DebuggerInspection * di) {
	type->SetLabel(di->GetValueType());
	value->SetValue(di->GetValue());
}

void mxInspectionPrint::OnDINewType (DebuggerInspection * di) {
	type->SetLabel(di->GetValueType());
	value->SetValue(di->GetValue());
}

mxInspectionPrint::~mxInspectionPrint ( ) {
	di->Destroy();
}

