#ifndef MXLOCALSGRID_H
#define MXLOCALSGRID_H
#include "mxGrid.h"
#include "Inspection.h"
#include "mxInspectionsPanel.h"

enum { LG_COL_NAME, LG_COL_TYPE, LG_COL_VALUE, LG_COLS_COUNT };

class mxLocalsGrid : public mxGrid, public myDIGlobalEventHandler, public mxInspectionsPanelTab {
	wxString command;
public:
	mxLocalsGrid(wxWindow *parent);
	void Update();
	void OnColumnHideOrUnhide (int col, bool visible) /*override*/ { Update(); }
	void OnDebugPausePost() /*override*/ { Update(); }
	void OnInspectionsPanelTabSelected() /*override*/ { Update(); }
	void OnCellPopupMenu(int row, int col);
	
	void OnAddToInspectionsGrid(wxCommandEvent &evt);
	void OnShowInText(wxCommandEvent &evt);
	void OnShowInTable(wxCommandEvent &evt);
	void OnExploreExpression(wxCommandEvent &evt);
	void OnCopyValue(wxCommandEvent &evt);
	void OnCopyType(wxCommandEvent &evt);
	void OnCopyExpression(wxCommandEvent &evt);
	void OnCopyAll(wxCommandEvent &evt);
	
	DECLARE_EVENT_TABLE();
};

#endif

