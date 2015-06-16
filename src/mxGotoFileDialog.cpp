#include "mxGotoFileDialog.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxGotoFunctionDialog.h"

mxGotoFileDialog::mxGotoFileDialog(wxString text, wxWindow* parent, int _goto_line) 
	: mxGotoListDialog( parent, LANG(GOTOFILE_CAPTION,"Ir a..."), 
		LANG(GOTOFILE_ENTER_FILENAME,"Ingrese parte del nombre del archivo que desea abrir:"),
		"Buscar clase/metodo/función..." )
{
	SetExtraButtonAccelerator(wxACCEL_CTRL|wxACCEL_SHIFT,'g');
	goto_line=_goto_line;
	case_sensitive->SetValue(false);
	if (text.Len()) SetInputValue(text);
	Show();
}

void mxGotoFileDialog::OnGoto(int pos, wxString key) {
	if (key.Len()) {
		if (project) {
			Close();
			mxSource *src=main_window->OpenFile(DIR_PLUS_FILE(project->GetPath(),key),false);
			if (goto_line!=-1 && src && src!=EXTERNAL_SOURCE) src->MarkError(goto_line); /// for errors with incomplete or relative path from external building tools
		} else {
			for (unsigned int i=0;i<main_window->notebook_sources->GetPageCount();i++) {
				mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (source->sin_titulo) {
					if (source->page_text==key) {
						Close();
						main_window->notebook_sources->SetSelection(i);
						main_window->notebook_sources->GetPage(i)->SetFocus();
						break;
					}
				} else {
					if (source->source_filename.GetFullName()==key) {
						Close();
						main_window->notebook_sources->SetSelection(i);
						main_window->notebook_sources->GetPage(i)->SetFocus();
						break;
					}
				}
			}
		}
	}
}


int mxGotoFileDialog::OnSearch(wxString key, bool case_sensitive) {
	bool ignore_case = case_sensitive;
	if (ignore_case) key.MakeUpper();
	if (project) {
		wxString path = project->GetPath();
		wxArrayString array;
		project->GetFileList(array,FT_NULL,true);
		for (unsigned int i=0; i<array.GetCount();i++) {
			if ((ignore_case?array[i].Upper():array[i]).Find(key)!=wxNOT_FOUND)
				list_ctrl->Append(array[i]);
		}
	} else {
		mxSource *source;
		for (unsigned int i=0; i<main_window->notebook_sources->GetPageCount();i++) {
			source = (mxSource*)(main_window->notebook_sources->GetPage(i));
			wxString fullname = source->sin_titulo?source->page_text:source->source_filename.GetFullPath();
			if ((ignore_case?fullname.Upper():fullname).Find(key)!=wxNOT_FOUND)
					list_ctrl->Append(source->page_text);
		}
	}
	if (list_ctrl->GetCount()) {
		return 0;
	} else {
		list_ctrl->Append(LANG(GOTOFILE_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		return -1;
	}
}

void mxGotoFileDialog::OnExtraButton() {
	Hide();
	new mxGotoFunctionDialog(text_ctrl->GetValue(),main_window);
	Close();
}

