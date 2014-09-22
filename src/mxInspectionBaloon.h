#ifndef MXINSPECTIONBALOON_H
#define MXINSPECTIONBALOON_H
#include <wx/frame.h>
#include "mxInspectionExplorerWidget.h"


class mxInspectionExplorerWidget;
class wxStaticText;

class mxInspectionBaloon : public wxFrame {
	mxInspectionExplorerWidget *exp;
	wxStaticText *text;
	class EventListener : public mxInspectionExplorerWidget::EventListener {
		mxInspectionBaloon *parent;
	public:
		EventListener(mxInspectionBaloon *p) : parent(p) {}
		void OnSize() { parent->OnTreeSize(); }
		void OnType(const wxString &type) { parent->OnTreeType(type); }
	};
public:
	mxInspectionBaloon(const wxPoint &pos, const wxString &expression, const wxString &value);
	void AddExpression(wxString expression, bool frameless);
	void OnTreeSize();
	void OnTreeType(const wxString &type);
};

#endif

