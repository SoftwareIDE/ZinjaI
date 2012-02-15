#include "VentanaPrincipal.h"

VentanaPrincipal::VentanaPrincipal(wxWindow *parent):Frame0(parent){
	Show();
}

void VentanaPrincipal::OnBotonCerrar( wxCommandEvent& event ) { 
	Close();
}
