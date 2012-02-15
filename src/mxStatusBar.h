#ifndef MXSTATUSBAR_H
#define MXSTATUSBAR_H

#include <wx/wx.h>

/* El siguiente codigo fue escrito por Anders Sandvig y tomado de 
	http://lists.wxwidgets.org/archive/wx-users/msg55135.html */

/**
 * Class representing a status bar with a progress bar
 */
class mxStatusBar : public wxStatusBar {
private:
  wxGauge *progress_bar;

public:
  mxStatusBar(wxWindow *parent, wxWindowID id, long style = wxST_SIZEGRIP, const
    wxString &name = _T("statusBar"));
  ~mxStatusBar();


void OnSize(wxSizeEvent &event);

void SetProgress(int progress);

  DECLARE_EVENT_TABLE();
};

#endif

