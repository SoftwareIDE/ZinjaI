#ifndef MX_INSPECTION_EXPLORER_H
#define MX_INSPECTION_EXPLORER_H
#include <wx/treectrl.h>
#include <wx/sashwin.h>
#include "mxExternInspection.h"
class wxSashLayoutWindow;
class wxBoxSizer;
class wxTextCtrl;


class mxIEItemData : public wxTreeItemData {
public:
	wxString name, expr, type, value, real_expr, frame;
	bool opened, use_data_evaluate, always_evaluate, is_root;
	int what_is, children, age;
	mxIEItemData(wxString aname, wxString areal_expr, wxString aexpr, wxString aframe, wxString atype, int achildren);
	~mxIEItemData();
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

