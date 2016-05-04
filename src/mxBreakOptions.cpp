#include "mxBreakOptions.h"

#include "mxSizers.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "DebugManager.h"
#include "ids.h"
#include "Language.h"
#include "ProjectManager.h"
#include "mxHelpWindow.h"
#include "mxBitmapButton.h"
#include "mxSource.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxBreakOptions, wxDialog)
	EVT_BUTTON(wxID_OK,mxBreakOptions::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxBreakOptions::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxBreakOptions::OnHelpButton)
	EVT_CHECKBOX(mxID_BREAK_OPTS_ENABLE,mxBreakOptions::OnBreakpointCheck)
END_EVENT_TABLE()
	
mxBreakOptions::mxBreakOptions(BreakPointInfo *bpi) 
	: mxDialog(main_window, LANG(BREAKOPTS_CAPTION,"Propiedades del Breakpoint") ),
	  m_bpi(bpi)
{
	m_bpi->UpdateLineNumber();

	CreateSizer sizer(this);
	
	sizer.BeginText( LANG(BREAKOPTS_FILE,"Archivo") ).Value(m_bpi->fname).ReadOnly().EndText();
	sizer.BeginText( LANG(BREAKOPTS_LINE,"Linea") ).Value(m_bpi->line_number+1).ReadOnly().EndText();
	
	sizer.BeginCheck( LANG(BREAKOPTS_INSERT,"Insertar punto de interrupcion") )
		.Id(mxID_BREAK_OPTS_ENABLE).Value(true).EndCheck(break_check);
	sizer.BeginCheck( LANG(BREAKOPTS_ENABLE,"Habilitar punto de interrupcion") )
		.Value(m_bpi->enabled).EndCheck(enable_check);
	sizer.BeginLine()
		.BeginText( LANG(BREAKOPTS_IGNORE_TIMES_PRE,"Ignorar") )
			.Value(m_bpi->ignore_count).EndText(ignore_text)
		.Space(10)
		.BeginLabel( LANG(BREAKOPTS_IGNORE_TIMES_POST," veces") ).EndLabel()
		.EndLine();
	if (debug->CanTalkToGDB() && m_bpi->IsInGDB()) {
		sizer.BeginLine()
			.BeginText( LANG(BREAKOPTS_HIT_TIMES_PRE,"Se ha alcanzado") )
				.Value(debug->GetBreakHitCount(bpi->gdb_id)).ReadOnly().EndText(count_text)
			.Space(10)
			.BeginLabel( LANG(BREAKOPTS_HIT_TIMES_POST,"veces") ).EndLabel()
			.EndLine();
	} else {
		count_text = nullptr;
	}
	sizer.BeginText( LANG(BREAKOPTS_CONDITION,"Condicion") ).Value(m_bpi->cond).EndText(cond_text);
	sizer.BeginCombo( LANG(BREAKOPTS_ACTION,"Acción") )
		.Add(LANG(BREAKTOPS_ACTIONS_ALWAYS,"Detener siempre"))
		.Add(LANG(BREAKTOPS_ACTIONS_ONCE,"Detener solo la primera vez"))
		.Add(LANG(BREAKTOPS_ACTIONS_INSPECTIONS,"Solo actualizar inspecciones (no detener)"))
		.Select(m_bpi->action).EndCombo(action);
	if (m_bpi->gdb_status==BPS_ERROR_SETTING)
		sizer.BeginLabel( LANG(BREAKOPTS_ERROR_PLACING_BREAKPOINT,"Error al colocar breakpoint") ).EndLabel();
	if (m_bpi->gdb_status==BPS_ERROR_CONDITION)
		sizer.BeginLabel( LANG(BREAKOPTS_INVALID_CONDITION,"La condicion actual no es valida") ).EndLabel();
	sizer.BeginText( LANG(BREAKOPTS_ANNTOATION,"Anotación") )
		.MultiLine().Value(m_bpi->annotation).EndText(annotation_text);
	
	sizer.BeginBottom().Help().Ok().Cancel().EndBottom(this);
	sizer.SetAndFit();
	wxCommandEvent evt; OnBreakpointCheck(evt);
	break_check->SetFocus();
	ShowModal();
}

void mxBreakOptions::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxBreakOptions::OnOkButton(wxCommandEvent &evt) {
	if (break_check->GetValue()) {
		if (debug->IsDebugging()) {
			BREAK_POINT_STATUS status=BPS_SETTED;
			long l;	ignore_text->GetValue().ToLong(&l); // ignore count
			if (m_bpi->ignore_count!=l) {
				m_bpi->ignore_count=l;
				debug->SetBreakPointOptions(m_bpi->gdb_id,m_bpi->ignore_count);
			}
			if (m_bpi->enabled!=enable_check->GetValue() || m_bpi->action!=action->GetSelection()) { // enabled and only_once
				m_bpi->enabled=enable_check->GetValue(); m_bpi->action=action->GetSelection();
				debug->SetBreakPointEnable(m_bpi);
			}
			if (!m_bpi->enabled) status=BPS_USER_DISABLED;
			if (m_bpi->cond!=cond_text->GetValue() || m_bpi->gdb_status==BPS_ERROR_CONDITION) { // condition
				m_bpi->cond=cond_text->GetValue();
				if (!debug->SetBreakPointOptions(m_bpi->gdb_id,m_bpi->cond)) status=BPS_ERROR_CONDITION;
			}
			m_bpi->SetStatus(status,m_bpi->gdb_id);
		} else {
			long l;	ignore_text->GetValue().ToLong(&l); // ignore count
			m_bpi->ignore_count=l;
			m_bpi->enabled=enable_check->GetValue(); 
			m_bpi->action=action->GetSelection();
			m_bpi->cond=cond_text->GetValue();
			m_bpi->SetStatus(BPS_UNKNOWN);
		}
		m_bpi->annotation=annotation_text->GetValue();
	} else {
		if (debug->IsDebugging())
			debug->DeleteBreakPoint(m_bpi);
		else delete m_bpi;
	}
	if (m_bpi->gdb_status==BPS_ERROR_CONDITION) {
		if ( mxMessageDialog(this,LANG(BREAKOPTS_ERROR_SETTING_CONDITION,""
									   "La condición ingresada no es correcta.\n"
									   "¿Colocar el punto de control incondicionalmente?."))
				.Title(LANG(GENERAL_ERROR,"Advertencia")).IconError().ButtonsYesNo().Run().no )
		{
			return;
		}
	}
	Close();
}

void mxBreakOptions::OnBreakpointCheck(wxCommandEvent &evt) {
	bool enable = break_check->GetValue();
	enable_check->Enable(enable);
	if (count_text) count_text->Enable(enable);
	ignore_text->Enable(enable);
	cond_text->Enable(enable);
	action->Enable(enable);
}

void mxBreakOptions::OnHelpButton(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("break_options.html",this);
}
