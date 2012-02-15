#ifndef MX_INSPECTION_EXPLORER_H
#define MX_INSPECTION_EXPLORER_H
#include <wx/dialog.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/sashwin.h>
#include "mxExternInspection.h"
class wxSashLayoutWindow;
class wxBoxSizer;

#include "DebugManager.h"

class mxIEItemData : public wxTreeItemData {
public:
	wxString name, expr, type, value, real_expr, frame;
	bool opened, use_data_evaluate, always_evaluate, is_root;
	int what_is, children, age;
	mxIEItemData(wxString aname, wxString areal_expr, wxString aexpr, wxString aframe, wxString atype, int achildren) {
		frame=aframe;
		what_is = DI_NONE;
		real_expr=areal_expr;
		name=aname;
		expr=aexpr;
		type=atype;
		age=0;
		opened = use_data_evaluate = always_evaluate = false;
		children = achildren;
		is_root = false;
	}
	~mxIEItemData() {
		if (is_root)
			debug->DeleteVO(name);
	}
};

class mxInspectionExplorer: mxExternInspection {
private:
	DECLARE_EVENT_TABLE();
	wxBoxSizer *treeSizer;
	wxTreeCtrl *tree;
	wxTextCtrl *value_ctrl;
	wxTextCtrl *type_ctrl;
	wxTextCtrl *expr_ctrl;
	void CreateControls();
	int age;
	wxSashLayoutWindow *index_sash;
public:
	void OnSashDrag (wxSashEvent& event);
	mxInspectionExplorer(wxString frame, wxString expr);
	mxInspectionExplorer(wxString title, wxArrayString &exprs, wxArrayString &frames);
//	void OnClose (wxCloseEvent &evt);
	void OnCloseButton (wxCommandEvent &evt);
	void OnItemExpanding(wxTreeEvent &evt);
	void OnItemClick(wxTreeEvent &evt);
	void Grow();
	void Update();
	~mxInspectionExplorer();
};

#endif

