#include "Inspection.h"

map<wxString,DebuggerInspection*> DebuggerInspection::vo2di_map;

SingleList<DebuggerInspection::DIPendingAction> DebuggerInspection::pending_actions;

