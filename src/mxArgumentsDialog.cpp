#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/menu.h>
#include "mxBitmapButton.h"
#include "ids.h"
#include "mxArgumentsDialog.h"
#include "mxSizers.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"
#include "Language.h"
#include "mxThreeDotsUtils.h"
#include "mxUtils.h"
#include "mxCommonPopup.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"

wxString mxArgumentsDialog::last_arguments;
wxString mxArgumentsDialog::last_workdir;
//wxString mxArgumentsDialog::last_dir;
wxArrayString mxArgumentsDialog::list_for_combo_args;
wxArrayString mxArgumentsDialog::list_for_combo_work;


BEGIN_EVENT_TABLE(mxArgumentsDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxArgumentsDialog::OnYesButton)
	EVT_BUTTON(wxID_NO,mxArgumentsDialog::OnNoButton)
	EVT_BUTTON(wxID_CANCEL,mxArgumentsDialog::OnCancelButton)
	EVT_CHAR_HOOK(mxArgumentsDialog::OnCharHook)
	EVT_BUTTON(mxID_ARGS_BUTTON,mxArgumentsDialog::OnArgsButton)
	EVT_BUTTON(mxID_WORKING_FOLDER,mxArgumentsDialog::OnWorkdirButton)
END_EVENT_TABLE()

	
mxArgumentsDialog::mxArgumentsDialog(wxWindow *parent, const wxString &base_path, const wxString &def_args, const wxString &def_dir) 
	: mxDialog(parent, LANG(ARGUMENTS_CAPTION,"Argumentos para la ejecucion"), mxDialog::OCP_HIDE ), // el false es porque al usarla va a ser siempre con instancias en el stack
	  m_base_path (base_path)
{
	CreateSizer sizer(this);
	SetReturnCode(0);
	sizer.BeginLabel( LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion:") ).EndLabel();
	sizer.BeginLine()
		.Space(15)
		.BeginCombo().Add(list_for_combo_args).Editable().Value(def_args).EndCombo(combo_args)
		.BeginButton("...").Id(mxID_ARGS_BUTTON).EndButton()
		.EndLine();
	
	sizer.BeginLabel( LANG(COMPILECONF_WORKDIR,"Directorio de trabajo:") ).EndLabel();
	sizer.BeginLine()
		.Space(15)
		.BeginCombo().Add(list_for_combo_work).Editable().Value(def_dir).EndCombo(combo_work)
		.BeginButton("...").Id(mxID_WORKING_FOLDER).EndButton()
		.EndLine();
		
	sizer.BeginCheck( LANG(GENERAL_DONT_ASK_AGAIN,"no volver a preguntar") ).Value(false).EndCheck(check);
	
	sizer.BeginBottom().Ok().Cancel()
		.Extra(wxID_NO, LANG(ARGUMENTS_WITHOUT_ARGUMENTS,"&Sin Argumentos"), bitmaps->buttons.ok).EndBottom(this);
	
	sizer.SetAndFit();
	SetFocusFromKbd();
}

void mxArgumentsDialog::OnYesButton(wxCommandEvent &evt) {
	last_workdir=combo_work->GetValue();
	if (list_for_combo_work.GetCount()==0 || list_for_combo_work.Last()!=last_workdir)
		list_for_combo_work.Add(last_workdir);
	if (combo_args->GetValue().Len()) {
		last_arguments=combo_args->GetValue();
		if (list_for_combo_args.GetCount()==0 || list_for_combo_args.Last()!=last_arguments)
			list_for_combo_args.Add(last_arguments);
		EndModal(AD_ARGS|(check->GetValue()?AD_REMEMBER:0));
	} else {
		last_arguments="";
		EndModal(AD_EMPTY|(check->GetValue()?AD_REMEMBER:0));
	}
}
void mxArgumentsDialog::OnNoButton(wxCommandEvent &evt) {
	last_arguments="";
	EndModal(AD_EMPTY|(check->GetValue()?AD_REMEMBER:0));
}
void mxArgumentsDialog::OnCancelButton(wxCommandEvent &evt) {
	EndModal(AD_CANCEL);
}

void mxArgumentsDialog::OnArgsButton(wxCommandEvent &evt) {
	wxString base_path = DIR_PLUS_FILE(project
									   ? project->GetPath()
									   : main_window->GetCurrentSource()->GetPath(true)
									  ,combo_work->GetValue());
	CommonPopup(combo_args).Caption(GetCaption())
		.AddEditAsText().AddEditAsList().AddFilename().AddPath()
		.CommaSplit(false).BasePath(base_path).Run(this);
}

void mxArgumentsDialog::OnCharHook(wxKeyEvent &evt)	{
	if (evt.GetKeyCode()==WXK_RETURN || evt.GetKeyCode()==WXK_NUMPAD_ENTER) {
		wxCommandEvent evt;
		OnYesButton(evt);
	} else evt.Skip();
}

void mxArgumentsDialog::OnWorkdirButton (wxCommandEvent & evt) {
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,combo_work,m_base_path, LANG(COMPILECONF_WORKDIR_DLG,"Directorio de trabajo"));
}

