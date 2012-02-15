#ifndef VentanaPrincipal_H
#define VentanaPrincipal_H
#include "Ventanas.h"

class VentanaPrincipal:public Frame0 {
public:
	VentanaPrincipal(wxWindow *parent=NULL);
	void OnBotonCerrar(wxCommandEvent &evt);
};

#endif
