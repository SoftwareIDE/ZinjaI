#include "Inspection.h"

DebuggerInspection::vo2di_type DebuggerInspection::vo2di_map;

SingleList<DebuggerInspection::DIPendingAction> DebuggerInspection::pending_actions;

SingleList<DebuggerInspection*> DebuggerInspection::all_inspections;

