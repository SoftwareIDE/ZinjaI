#include "mxInspectionsPanel.h"
#include "mxMainWindow.h"
#include "mxInspectionGrid.h"
#include "mxLocalsGrid.h"
#include "ids.h"

BEGIN_EVENT_TABLE(mxInspectionsPanel,wxAuiNotebook)
	EVT_AUINOTEBOOK_PAGE_CLOSE(mxID_NOTEBOOK_INSPECTIONS, mxInspectionsPanel::OnPageClosing)
	EVT_AUINOTEBOOK_PAGE_CHANGING(mxID_NOTEBOOK_INSPECTIONS, mxInspectionsPanel::OnPageChanging)
	EVT_AUINOTEBOOK_PAGE_CHANGED(mxID_NOTEBOOK_INSPECTIONS, mxInspectionsPanel::OnPageChanged)
END_EVENT_TABLE()

mxInspectionsPanel::mxInspectionsPanel():wxAuiNotebook(main_window,mxID_NOTEBOOK_INSPECTIONS,wxDefaultPosition,wxSize(400,300),
	wxAUI_NB_MIDDLE_CLICK_CLOSE|wxAUI_NB_BOTTOM|wxNO_BORDER|wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_CLOSE_ON_ACTIVE_TAB) 
{
	created = false;
	SetTabCtrlHeight(24);
	name_aux = current_tab = 0;
	tabs.Add(Tab(new mxLocalsGrid(this),"Locals"));
	tabs.Add(Tab(new mxInspectionGrid(this),"Table 1"));
	for(int i=0;i<tabs.GetSize();i++)
		AddPage(tabs[i].ctrl,tabs[i].name);
	AddPage(new wxStaticText(this,wxID_ANY,""),"+");
	SetSelection(1); SelectPage(1);
	created=true;
}


void mxInspectionsPanel::OnPageChanging (wxAuiNotebookEvent &event) {
	if (!created) return;
	int p = event.GetSelection();
	if (p==tabs.GetSize()) {
		tabs.Add(Tab(new mxInspectionGrid(this),wxString("Table ")<<(++name_aux)));
		InsertPage(p,tabs[p].ctrl,tabs[p].name);
		SetSelection(p);
		event.Veto();
	} 
}

void mxInspectionsPanel::OnPageChanged (wxAuiNotebookEvent &event) {
	if (!created) return;
	int p = event.GetSelection();
	if (p!=tabs.GetSize()) SelectPage(p);
}

void mxInspectionsPanel::SelectPage (int p) {
	tabs[current_tab].SetVisible(false,created);
	tabs[p].SetVisible(true,created);
	current_tab=p;
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

void mxInspectionsPanel::OnPageClosing (wxAuiNotebookEvent & event) {
	int p = event.GetSelection();
	if (p>=tabs.GetSize() || tabs[p].type!=Tab::TYPE_GRID) { event.Veto(); return; }
	tabs.Remove(p);
	if (p==tabs.GetSize()) SetSelection(p-1);
}

