#include "mxStatusBar.h"

/* El siguiente codigo fue escrito por Anders Sandvig y tomado de 
	http://lists.wxwidgets.org/archive/wx-users/msg55135.html */


// Event table
BEGIN_EVENT_TABLE(mxStatusBar, wxStatusBar)
  EVT_SIZE(mxStatusBar::OnSize)
END_EVENT_TABLE()

	
	
/*
 * Constructor
 */
mxStatusBar::mxStatusBar(wxWindow *parent, wxWindowID id, long style, const wxString &name) : wxStatusBar(parent, id, style, name) {
  progress_bar = new wxGauge(this, -1, 100, wxPoint(0, 0), wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH);
  SetProgress(0);
}


/*
 * Destructor
 */
mxStatusBar::~mxStatusBar() {
  if (progress_bar)
    delete progress_bar;
}


/*
 * Size event handler
 */
void mxStatusBar::OnSize(wxSizeEvent &event) {
  wxRect r;
  GetFieldRect(0, r);
  r.x=(r.width=r.width/2);
  progress_bar->SetSize(r);
}

/*
 * Set progress bar value
 */
void mxStatusBar::SetProgress(int progress) {
	if (progress > 0) {
		progress_bar->Show(true);
		progress_bar->SetValue(progress);
	} else {
		progress_bar->Show(false);
	}
}

