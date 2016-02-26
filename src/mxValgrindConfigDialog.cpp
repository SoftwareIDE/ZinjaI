#include <wx/arrstr.h>
#include <wx/sizer.h>
#include "mxValgrindConfigDialog.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "ProjectManager.h"
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxValgrindConfigDialog,wxDialog)
	EVT_BUTTON(wxID_OK,mxValgrindConfigDialog::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxValgrindConfigDialog::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxValgrindConfigDialog::OnButtonHelp)
	EVT_CLOSE(mxValgrindConfigDialog::OnClose)
END_EVENT_TABLE()
	
mxValgrindConfigDialog::mxValgrindConfigDialog(wxWindow *parent)
	: wxDialog(parent,wxID_ANY,"Valgrind Setup",wxDefaultPosition,wxDefaultSize) 
{
	
	mxCCC::MainSizer sizer = mxCCC::CreateMainSizer(this);
	
	sizer.BeginCombo("Tool")
		.Add("memcheck").Add("cachegrind").Add("callgrind")
		.Add("massif").Add("helgrind").Add("drd")
		.Select(0).EndCombo(cmb_tool);
	sizer.BeginText( "Suppression files" ).EndText(suppressions);
	sizer.BeginText( "Additional arguments" ).EndText(additional_args);
	
	if (project) {
		valgrind_configuration *valgrind_config = project->GetValgrindConfiguration();
		additional_args->SetValue(valgrind_config->arguments);
		suppressions->SetValue(valgrind_config->suppressions);
	}
	
	sizer.BeginBottom().Help().Ok().Cancel().EndBottom(this).SetAndFit();
}

void mxValgrindConfigDialog::OnButtonOk (wxCommandEvent & evt) {
	if (project) {
		valgrind_configuration *valgrind_config = project->GetValgrindConfiguration();
		valgrind_config->arguments = additional_args->GetValue();
		valgrind_config->suppressions = suppressions->GetValue();
	}
	EndModal(1);
}

void mxValgrindConfigDialog::OnButtonCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxValgrindConfigDialog::OnButtonHelp (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("valgrind.html",this);
}

void mxValgrindConfigDialog::OnClose (wxCloseEvent & evt) {
	EndModal(0);
}

wxString mxValgrindConfigDialog::GetArgs() {
	wxString tool = cmb_tool->GetValue();
	wxString args = "--tool="; args<<tool;
	if (tool=="memcheck") args<<" --leak-check=full";
	if (suppressions->GetValue().Len()) args<<" "<<mxUT::Split(suppressions->GetValue(),"--suppressions=");
	if (additional_args->GetValue().Len()) args<<" "<<additional_args->GetValue();
	if (project) mxUT::ParameterReplace(args,"${PROJECT_PATH}",project->path);
	return args;
}

void mxValgrindConfigDialog::SetArg (const wxString & arg, bool present) {
	wxString args = additional_args->GetValue();
	if (mxUT::IsArgumentPresent(args,arg)!=present) {
		mxUT::SetArgument(args,arg,present);
		additional_args->SetValue(args);
	}
}

