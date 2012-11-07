#include "Application.h"
#include "VentanaPrincipal.h"
#include <wx/image.h>

IMPLEMENT_APP(mxApplication)

bool mxApplication::OnInit() {
	wxInitAllImageHandlers();
	new VentanaPrincipal(NULL);
	return true;
}

