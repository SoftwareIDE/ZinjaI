#include "mxCompilerArgEnabler.h"
#include "Cpp11.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxCompilerArgEnabler,wxDialog)
	EVT_CHECKBOX(wxID_FIND,mxCompilerArgEnabler::OnArgCheck)
	EVT_BUTTON(wxID_OK,mxCompilerArgEnabler::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxCompilerArgEnabler::OnButtonCancel)
END_EVENT_TABLE()

mxCompilerArgEnabler::mxCompilerArgEnabler(wxWindow *parent, wxString title, 
										   wxString help_text, wxString comp_arg, 
										   wxString link_arg) 
	: mxDialog(parent, title), m_comp_arg(comp_arg), m_link_arg(link_arg)
{
	CreateSizer sizer(this);
	sizer.BeginLabel(help_text).EndLabel();
	sizer.BeginCheck(LANG(COMP_ARG_ADD_ARGS,"Incluir argumentos")).Id(wxID_FIND).EndCheck(m_enable_arg);
	if (project) sizer.BeginCheck(LANG(COMP_ARG_RECOMPILE,"Recompilar todo")).EndCheck(m_recompile);
	else m_recompile = nullptr;
	sizer.BeginBottom().Ok().Cancel().EndBottom(this);
	sizer.SetAndFit();
	
	bool b1, b2;
	if (project) {
		b1 = mxUT::IsArgumentPresent(project->active_configuration->compiling_extra, comp_arg);
		b2 = mxUT::IsArgumentPresent(project->active_configuration->linking_extra, link_arg); 
	} else {
		mxSource *src = main_window->GetCurrentSource();
		wxString src_comp_opts = src->GetCompilerOptions(false);
		b1 = mxUT::IsArgumentPresent(src_comp_opts,comp_arg);
		b2 = mxUT::IsArgumentPresent(src_comp_opts,link_arg);
	}
	m_enable_arg->SetValue( b1 && b2 );
}

void mxCompilerArgEnabler::OnButtonOk (wxCommandEvent & evt) {
	bool arg_enable = m_enable_arg->GetValue();
	wxCommandEvent aux_event;
	if (project) {
		if (!m_comp_arg.IsEmpty())
			mxUT::SetArgument(project->active_configuration->compiling_extra,m_comp_arg,arg_enable);
		if (!m_link_arg.IsEmpty())
			mxUT::SetArgument(project->active_configuration->linking_extra,m_link_arg,arg_enable);
		if (!m_recompile || m_recompile->GetValue()) {
			project->Clean();
			main_window->OnRunCompile(aux_event);
		}
	} else {
		mxSource *src = main_window->GetCurrentSource();
		wxString src_comp_opts = src->GetCompilerOptions(false);
		if (!m_comp_arg.IsEmpty())
			mxUT::SetArgument(src_comp_opts,m_comp_arg,arg_enable);
		if (!m_link_arg.IsEmpty() && m_link_arg!=m_comp_arg)
			mxUT::SetArgument(src_comp_opts,m_link_arg,arg_enable);
		src->SetCompilerOptions(src_comp_opts);
		if (!m_recompile || m_recompile->GetValue()) {
			main_window->OnRunClean(aux_event);
		}
	}
	EndModal(1);
}

void mxCompilerArgEnabler::OnButtonCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxCompilerArgEnabler::OnArgCheck (wxCommandEvent & evt) {
	if (m_recompile) m_recompile->SetValue(true);
}

bool mxCompilerArgEnabler::GetUserSelection ( ) {
	return m_enable_arg->GetValue();
}

