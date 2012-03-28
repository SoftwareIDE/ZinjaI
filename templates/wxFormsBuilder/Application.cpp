#include "Application.h"
#include "VentanaPrincipal.h"
#include <wx/image.h>

bool mxApplication::OnInit() {
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxXPMHandler);
	wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler(new wxBMPHandler);
	new VentanaPrincipal(NULL);
	return true;
}

IMPLEMENT_APP(mxApplication)

