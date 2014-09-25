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
};

#endif

