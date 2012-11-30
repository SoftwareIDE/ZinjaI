#ifndef MXINSPECTIONPRINT_H
#define MXINSPECTIONPRINT_H
#include "mxExternInspection.h"

class wxTextCtrl;

class mxInspectionPrint : public mxExternInspection {
	wxTextCtrl *text;
	bool is_macro;
public:
	mxInspectionPrint(wxString frame, wxString expression, bool frameless);
	void Update();
//	DECLARE_EVENT_TABLE();
};

#endif

