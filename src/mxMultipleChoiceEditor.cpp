#include <wx/textctrl.h>
#include <wx/choicdlg.h>
#include "mxMultipleChoiceEditor.h"
#include "mxUtils.h"

mxMultipleChoiceEditor *mxMultipleChoiceEditor::last_multiple_choice_editor=NULL;

/**
* @brief Construye y muestra una ventana para seleccionar multiples opciones de una lista de posibles
*
* Recibe una lista de posibles opciones y un cuadro de texto, arma una ventana 
* con una lista de checks para las opciones, marca las que estaban presentes
* en el cuadro de texto, y si el usuario presiona aceptar actualiza el contenido
* del cuadro de texto. La ventana se ejecuta directamente en este constructor.
*
* @param parent         ventana padre, para hacerlo modal
* @param title          titulo de la ventana 
* @param message        texto para mostrar sobre la lista de checks
* @param text           control de texto de donde tomar las opciones seleccionadas y donde guardar el resultado
* @param options_array  lista de opciones posibles
* @param comma_splits   indica si la coma separa opciones en el cuadro de texto, para pasar a mxUtils::Split
**/
mxMultipleChoiceEditor::mxMultipleChoiceEditor(wxWindow *parent, wxString title, wxString message, wxTextCtrl *text, wxArrayString &options_array, bool comma_splits) {
	if (last_multiple_choice_editor) delete last_multiple_choice_editor;
	last_multiple_choice_editor=this;
	
	wxArrayString splitted_array;
	wxArrayInt selection;
	mxUT::Split(text->GetValue(),splitted_array,comma_splits,true);
	options_array.Sort();
	int p=options_array.Index("all");
	if (p!=wxNOT_FOUND) {
		for (unsigned int i=0;i<splitted_array.GetCount();i++)
			selection.Add(p);
	} else {
		for (unsigned int i=0;i<splitted_array.GetCount();i++) {
			p=options_array.Index(splitted_array[i]);
			if (p!=wxNOT_FOUND) selection.Add(p);
		}
	}
	if (wxGetMultipleChoices(selection,message,title,options_array,parent)>=0) {
		wxString res;
		if (selection.GetCount()) {
			if (options_array[selection[0]].Contains(wxChar(' '))) 
				res=wxString("\"")<<options_array[selection[0]]<<"\"";
			else
				res=options_array[selection[0]];
			for (unsigned int i=1;i<selection.GetCount();i++) {
				if (options_array[selection[i]].Contains(wxChar(' '))) 
					res<<" \""<<options_array[selection[i]]<<"\"";
				else
					res<<" "<<options_array[selection[i]];
			}
		}
		text->SetValue(res);
	}
}

mxMultipleChoiceEditor::~mxMultipleChoiceEditor() {
	if (this==last_multiple_choice_editor) last_multiple_choice_editor=NULL;
}

