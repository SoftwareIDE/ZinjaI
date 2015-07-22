#ifndef MXREGISTERSGRID_H
#define MXREGISTERSGRID_H
#include "mxGrid.h"
#include "Inspection.h"
#include "mxInspectionsPanel.h"


enum { RG_COL_NAME, RG_COL_VALUE_1, RG_COL_VALUE_2, RG_COLS_COUNT };

class mxRegistersGrid : public mxGrid, public myDIGlobalEventHandler {
private:
	bool all_registers;
public:
	mxRegistersGrid(wxWindow *parent);
	void Update();
	void OnDebugPausePost() override { Update(); }
	void OnCellPopupMenu(int row, int col) override;
	void ToggleShowAllRegisters(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

