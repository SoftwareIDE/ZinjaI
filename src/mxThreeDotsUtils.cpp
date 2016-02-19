#include <wx/filedlg.h>
#include "mxThreeDotsUtils.h"
#include <forward_list>
#include "mxUtils.h"
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>


static void stGetPathAndName(wxString &def_path, wxString &def_name, const wxFileName &fname){
	// split path and filename
	def_name = fname.GetName(); def_path = fname.GetPath();
	// if the path does not exist, try going one level up
	while (!def_path.IsEmpty() && !wxFileName::DirExists(def_path))
		def_path = wxFileName(def_path).GetPath();
}

template<typename wxCtrl_t>
static bool stHelperFunction1(wxWindow *parent, wxCtrl_t *text_ctrl, wxString relative_path, wxString message, bool file) {
	// obtain full previos value and selection
	wxString orig_value = text_ctrl->GetValue();
	long pbeg, pend; text_ctrl->GetSelection(&pbeg,&pend);
	if (pend<pbeg) std::swap(pbeg,pend);
	wxString sel_value = orig_value.Mid(pbeg,pend-pbeg);
	// use selection and relative_path to get a valid dir for openning the file picker in
	wxString def_path, def_name, res;
	stGetPathAndName(def_path,def_name,DIR_PLUS_FILE(relative_path,sel_value));
	// show dialog, and after that return focus to the text control
	if (file) {
		wxFileDialog dlg(parent,message,def_path,def_name);
		if (wxID_OK!=dlg.ShowModal()) return false;
		res = dlg.GetPath();
	} else {
		wxDirDialog dlg(parent,message,def_path,0);
		if (wxID_OK!=dlg.ShowModal()) return false;
		res = dlg.GetPath();
	}
	text_ctrl->SetFocus();
	// build the new value, set it to the control, and fix selection
	if (!relative_path.IsEmpty()) res = mxUT::Relativize(res,relative_path);
	wxString new_value = orig_value.Mid(0,pbeg) + res + orig_value.Mid(pend);
	text_ctrl->SetValue(new_value);
	text_ctrl->SetSelection(pbeg,pbeg+res.Len());
	return true;
}

template<typename wxCtrl_t>
static bool stHelperFunction2 (wxWindow *parent, wxCtrl_t *text_ctrl, wxString relative_path, wxString message, bool all, bool file) {
	if (message.IsEmpty()) {
		if (file) {
			if (all) message = LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FILE,"Reemplazar todo por archivo");
			else     message = LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FILE,"Reemplazar seleccion por archivo");
		} else {
			if (all) message = LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FOLDER,"Reemplazar todo por directorio");
			else     message = LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FOLDER,"Reemplazar seleccion por directorio");
		}
	}
	if (all) text_ctrl->SetSelection(0,text_ctrl->GetValue().Len());
	return stHelperFunction1(parent, text_ctrl, relative_path, message, file);
}

bool mxThreeDotsUtils::ReplaceSelectionWithFile (wxWindow * parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, text_ctrl, relative_path, message, false, true);
}

bool mxThreeDotsUtils::ReplaceSelectionWithDirectory (wxWindow * parent, wxTextCtrl * text_ctrl, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, text_ctrl, relative_path, message, false, false);
}

bool mxThreeDotsUtils::ReplaceAllWithFile (wxWindow * parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, text_ctrl, relative_path, message, true, true);
}

bool mxThreeDotsUtils::ReplaceAllWithDirectory (wxWindow * parent, wxTextCtrl *text_ctrl, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, text_ctrl, relative_path, message, true, false);
}

bool mxThreeDotsUtils::ReplaceSelectionWithFile (wxWindow * parent, wxComboBox *combo_box, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, combo_box, relative_path, message, false, true);
}

bool mxThreeDotsUtils::ReplaceSelectionWithDirectory (wxWindow * parent, wxComboBox *combo_box, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, combo_box, relative_path, message, false, false);
}

bool mxThreeDotsUtils::ReplaceAllWithFile (wxWindow * parent, wxComboBox *combo_box, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, combo_box, relative_path, message, true, true);
}

bool mxThreeDotsUtils::ReplaceAllWithDirectory (wxWindow * parent, wxComboBox *combo_box, wxString relative_path, wxString message) {
	return stHelperFunction2(parent, combo_box, relative_path, message, true, false);
}

