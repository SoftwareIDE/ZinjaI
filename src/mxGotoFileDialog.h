#ifndef MXGOTOFILEDIALOG_H
#define MXGOTOFILEDIALOG_H
#include "mxGotoListDialog.h"

class mxGotoFileDialog: public mxGotoListDialog {
private:
	int goto_line; ///< line withing the file that should be displayed after openning (it is used when its called for an compiler error from a external build tool/script)
public:
	mxGotoFileDialog(wxString text, wxWindow* parent, int _goto_line=-1);
	void OnExtraButton() override;
	int OnSearch(wxString key, bool case_sensitive) override;
	void OnGoto(int pos, wxString key) override;
};

#endif
