#ifndef MXCOMMANDFINDER_H
#define MXCOMMANDFINDER_H
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <vector>
#include <wx/timer.h>

class wxTextCtrl;
class wxListBox;

class mxCommandFinderList:public wxDialog {
	wxListBox *list;
	std::vector<int> ids;
	wxRect pos_and_size;
	wxTimer timer;
public:
	mxCommandFinderList(wxWindow *parent);
	void SetPattern(wxString str, int x, int y);
	void KeyUp();
	void KeyDown();
	void Select();
	void OnTimer(wxTimerEvent &evt);
};

class mxCommandFinderText : public wxTextCtrl {
	mxCommandFinderList list;
public:
	mxCommandFinderText(wxWindow *parent);
	void OnText(wxCommandEvent &evt);
	void OnKey(wxKeyEvent &evt);
	DECLARE_EVENT_TABLE();
};

class mxCommandFinder :public wxDialog {
	mxCommandFinderText *text;
public:
	mxCommandFinder();
};

#endif

