#ifndef MXVALGRINDOUPUT_H
#define MXVALGRINDOUPUT_H
#include <wx/treectrl.h>

enum mxVOmode {
	mxVO_NULL,
	mxVO_CPPCHECK,
	mxVO_VALGRIND,
	mxVO_DOXYGEN,
};

class mxValgrindOuput:public wxTreeCtrl {
private:
	wxString filename;
	wxTreeItemId root;
	wxTreeItemId last;
	bool is_last;
	wxTreeItemId sel;
	mxVOmode mode;
public:
	mxValgrindOuput(wxWindow* parent, mxVOmode mode, wxString afilename);
	void SetMode(mxVOmode mode, wxString afilename);
	bool LoadOutput(); /// populate panel's tree from given output file, returns false if there's no output
	bool LoadOutputValgrind();
	bool LoadOutputCppCheck();
	bool LoadOutputDoxygen();
	void OnPopup(wxTreeEvent &evt);
	void OnSelect(wxTreeEvent &evt);
	void OnDelete(wxCommandEvent &evt);
	void OnReload(wxCommandEvent &evt);
	void OnOpen(wxCommandEvent &evt);
	void OnKey(wxKeyEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

