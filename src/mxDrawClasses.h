#ifndef MXDRAWCLASSES_H
#define MXDRAWCLASSES_H
#include <wx/dialog.h>

class wxComboBox;
class mxDrawClasses : public wxDialog {
private:
	wxComboBox *related_classes,*what_inside,*wich_command,*wich_file,*wich_class,*where_store;
public:
	mxDrawClasses();
	~mxDrawClasses();
	void OnClose(wxCloseEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	int GenerateGraph(wxString file, bool one_class, bool related,bool indirect_related, bool inside, bool only_public, bool only_name);
	DECLARE_EVENT_TABLE();
};

#endif

