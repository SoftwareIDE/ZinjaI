#ifndef MXBACKTRACEHISTORY_H
#define MXBACKTRACEHISTORY_H
#include <wx/listbox.h>
#include "DebugManager.h"
#include "SingleList.h"

class mxBacktraceHistory : public wxListBox, public myBTEventHandler {
	bool logging;
	SingleList<DebugManager::BTInfo> data;
public:
	mxBacktraceHistory();
	~mxBacktraceHistory();
	void OnBacktraceUpdated(bool was_running) override;
	void OnPopupMenu(wxMouseEvent &evt);
	void OnClear(wxCommandEvent &evt);
	void OnToggleLogging(wxCommandEvent &evt);
	void OnSelect(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

