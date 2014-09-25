#include "mxInspectionsPanel.h"
#include "mxMainWindow.h"
#include "mxInspectionGrid.h"
#include "mxLocalsGrid.h"

BEGIN_EVENT_TABLE(mxInspectionsPanel,wxNotebook)
	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY,mxInspectionsPanel::OnPageChange)
END_EVENT_TABLE()

mxInspectionsPanel::mxInspectionsPanel():wxNotebook(main_window,wxID_ANY,wxDefaultPosition,wxSize(400,300),wxNB_BOTTOM|wxNB_FLAT) {
	created=false;
	current_tab = 0;
	tabs.Add(Tab(new mxLocalsGrid(this),"Locals"));
	tabs.Add(Tab(new mxInspectionGrid(this),"Table 1"));
	for(int i=0;i<tabs.GetSize();i++)
		AddPage(tabs[i].ctrl,tabs[i].name);
	AddPage(new wxStaticText(this,wxID_ANY,""),"+");
	SelectPage(1);
	created=true;
}


void mxInspectionsPanel::OnPageChange (wxNotebookEvent & event) {
//	event.Skip();
	if (!created) return;
	int p = event.GetSelection();
	if (p==tabs.GetSize()) {
		tabs.Add(Tab(new mxInspectionGrid(this),wxString("Table ")<<p));
		InsertPage(p,tabs[p].ctrl,tabs[p].name);
	}
	SelectPage(p);
}

void mxInspectionsPanel::SelectPage (int p) {
	tabs[current_tab].SetVisible(false,created);
	tabs[p].SetVisible(true,created);
	ChangeSelection(p);
}

mxInspectionsPanel::Tab::Tab(mxInspectionGrid *g, const wxString &_name):type(TYPE_GRID),ctrl(g),name(_name) {}
mxInspectionsPanel::Tab::Tab(mxLocalsGrid *l, const wxString &_name):type(TYPE_LOCAL),ctrl(l),name(_name) {}
void mxInspectionsPanel::Tab::SetVisible(bool value, bool generate_event) {
	mxInspectionsPanelTab *tab = NULL;
	switch(type) {
	case TYPE_GRID: tab = static_cast<mxInspectionsPanelTab*>(static_cast<mxInspectionGrid*>(ctrl)); break;
	case TYPE_LOCAL: tab = static_cast<mxInspectionsPanelTab*>(static_cast<mxLocalsGrid*>(ctrl)); break;
	default: break;
	}
	if (tab) { tab->ipt_is_visible = value; if (value && generate_event) tab->OnInspectionsPanelTabSelected(); }
}
