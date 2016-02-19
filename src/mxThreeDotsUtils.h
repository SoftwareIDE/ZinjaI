#ifndef MXTHREEDOTSUTILS_H
#define MXTHREEDOTSUTILS_H
#include <wx/string.h>
class wxWindow;
class wxTextCtrl;
class wxComboBox;

class mxThreeDotsUtils {
public:
	static bool ReplaceAllWithFile(wxWindow *parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message="");
	static bool ReplaceAllWithFile(wxWindow *parent, wxComboBox *combo_box, wxString relative_path, wxString message="");
	
	static bool ReplaceAllWithDirectory(wxWindow *parent, wxComboBox *combo_box, wxString relative_path, wxString message="");
	static bool ReplaceAllWithDirectory(wxWindow *parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message="");
	
	static bool ReplaceSelectionWithFile(wxWindow *parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message="");
	static bool ReplaceSelectionWithFile(wxWindow *parent, wxComboBox *combo_box, wxString relative_path, wxString message="");
	
	static bool ReplaceSelectionWithDirectory(wxWindow *parent, wxComboBox *combo_box, wxString relative_path, wxString message="");
	static bool ReplaceSelectionWithDirectory(wxWindow *parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message="");
};

#endif

