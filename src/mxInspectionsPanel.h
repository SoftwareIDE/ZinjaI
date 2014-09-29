#ifndef MXINSPECTIONSPANEL_H
#define MXINSPECTIONSPANEL_H
#include <wx/aui/auibook.h>
#include "SingleList.h"

class mxInspectionsPanelTab {
	bool ipt_is_visible;
	friend class mxInspectionsPanel;
public:
	mxInspectionsPanelTab() : ipt_is_visible(false) {}
	bool IsCurrentInspectionsTab() { return ipt_is_visible; }
	virtual void OnInspectionsPanelTabSelected() {}
};

class mxInspectionGrid;
class mxLocalsGrid;

class mxInspectionsPanel : public wxAuiNotebook {
	struct Tab {
		enum { TYPE_NULL,TYPE_GRID, TYPE_LOCAL } type;
		wxWindow *ctrl;
		wxString name;
		Tab():type(TYPE_NULL),ctrl(NULL) {}
		Tab(mxInspectionGrid *g, const wxString &_name);
		Tab(mxLocalsGrid *l, const wxString &_name);
		void SetVisible(bool value, bool generate_event=true);
	};
	SingleList<Tab> tabs;
	DECLARE_EVENT_TABLE();
	int current_tab;
	int name_aux;
	bool created;
public:
	mxInspectionsPanel();
	void OnPageChanging(wxAuiNotebookEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
	void OnPageClosing(wxAuiNotebookEvent &event);
	void SelectPage(int p);
};

#endif

