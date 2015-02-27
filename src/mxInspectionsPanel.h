#ifndef MXINSPECTIONSPANEL_H
#define MXINSPECTIONSPANEL_H
#include <wx/aui/auibook.h>
#include "Cpp11.h"
#include "SingleList.h"

class mxInspectionsPanelTab {
	friend class mxInspectionsPanel;
#ifdef __APPLE__
public: /// there seems to be something wrong with nested friendship in my gcc for mac
#endif
	bool ipt_is_visible;
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
		Tab():type(TYPE_NULL),ctrl(nullptr) {}
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
	void OnRedirectedEditEvent(wxCommandEvent &event);
	void SelectPage(int p);
	void SetFocus();
	
	int GetTabsCount() { return tabs.GetSize(); }
	wxString GetPageTitle(int p);
	bool PageIsInspectionsGrid(int p);
	mxInspectionGrid *GetInspectionGrid(int p);
	mxInspectionGrid *GetCurrentInspectionGrid();
	
	mxInspectionGrid *AddGrid(bool and_select=false);
	mxInspectionGrid *Reset();
	
	
};

#endif

