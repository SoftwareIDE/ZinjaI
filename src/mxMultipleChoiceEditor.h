#ifndef MXMULTIPLECHOICEEDITOR_H
#define MXMULTIPLECHOICEEDITOR_H
#include <wx/arrstr.h>

class wxWindow;
class wxTextCtrl;

class mxMultipleChoiceEditor {
private:
	// por si hago dos con new y no hago delete, que el segundo elimine al primero, porque los mxEnumerationEditor y mxLongTextEditor suelo hacerlos con new
	// lo planteo asi por si en algun momento reemplazo la funcion de wx por una ventana propia como hice con los otros, y en estos casos se usa Destroy
	static mxMultipleChoiceEditor *last_multiple_choice_editor; ///< para emular al Destroy, porque esta clase en realidad no hereda de un wxDialog, pero se usa como tal
protected:
public:
	mxMultipleChoiceEditor(wxWindow *parent, wxString title, wxString message, wxTextCtrl *text, wxArrayString &options_array, bool comma_splits=true);
	~mxMultipleChoiceEditor();
};

#endif

