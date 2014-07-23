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
	EVT_CLOSE(mxBreakOptions::OnClose)
END_EVENT_TABLE()
	
mxBreakOptions::mxBreakOptions(BreakPointInfo *_bpi) : wxDialog(main_window, wxID_ANY, LANG(BREAKOPTS_CAPTION,"Propiedades del Breakpoint"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	bpi=_bpi; bpi->UpdateLineNumber();

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	SetEscapeId(wxID_CANCEL);
	ok_button->SetDefault(); 
	buttonSizer->Add(help_button,sizers->BA5_Exp0);
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(cancel_button,sizers->BA5);
	buttonSizer->Add(ok_button,sizers->BA5);

	mxUT::AddTextCtrl(mySizer,this,LANG(BREAKOPTS_FILE,"Archivo"),bpi->fname)->SetEditable(false);
	mxUT::AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_LINE,"Linea"),wxString()<<bpi->line_number+1)->SetEditable(false);
	break_check = mxUT::AddCheckBox(mySizer,this,LANG(BREAKOPTS_INSERT,"Insertar punto de interrupcion"),true,mxID_BREAK_OPTS_ENABLE);
	enable_check = mxUT::AddCheckBox(mySizer,this,LANG(BREAKOPTS_ENABLE,"Habilitar punto de interrupcion"),bpi->enabled);
//	once_check = mxUT::AddCheckBox(mySizer,this,LANG(BREAKOPTS_ONCE,"Interrumpir solo una vez"),bpi->only_once);
	ignore_text = mxUT::AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_IGNORE_TIMES_PRE,"Ignorar"),bpi->ignore_count,wxString(LANG(BREAKOPTS_IGNORE_TIMES_POST,"veces")));
	if (debug->debugging && !debug->waiting && bpi->IsInGDB()) {
		count_text = mxUT::AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_HIT_TIMES_PRE,"Se ha alcanzado"),debug->GetBreakHitCount(bpi->gdb_id),wxString(LANG(BREAKOPTS_HIT_TIMES_POST,"veces")));
		count_text->SetEditable(false);
	} else {
		count_text = NULL;
	}
	cond_text = mxUT::AddTextCtrl(mySizer,this,LANG(BREAKOPTS_CONDITION,"Condicion"),bpi->cond);
	wxArrayString actions_list;
	actions_list.Add(LANG(BREAKTOPS_ACTIONS_ALWAYS,"Detener siempre"));
	actions_list.Add(LANG(BREAKTOPS_ACTIONS_ONCE,"Detener solo la primera vez"));
	actions_list.Add(LANG(BREAKTOPS_ACTIONS_INSPECTIONS,"Solo actualizar inspecciones (no detener)"));
	action = mxUT::AddComboBox(mySizer,this,LANG(BREAKOPTS_ACTION,"Acción"),actions_list,bpi->action);
	
	if (bpi->gdb_status==BPS_ERROR_SETTING)
		mxUT::AddStaticText(mySizer,this,LANG(BREAKOPTS_ERROR_PLACING_BREAKPOINT,"Error al colocar breakpoint"));
	if (bpi->gdb_status==BPS_ERROR_CONDITION)
			mxUT::AddStaticText(mySizer,this,LANG(BREAKOPTS_INVALID_CONDITION,"La condicion actual no es valida"));
	
	annotation_text = mxUT::AddLongTextCtrl(mySizer,this,LANG(BREAKOPTS_ANNTOATION,"Anotación"),bpi->annotation);
	
	mySizer->Add(buttonSizer,sizers->Exp0);
	
	wxCommandEvent evt; OnBreakpointCheck(evt);
	
	SetSizerAndFit(mySizer);
	
	break_check->SetFocus();
	ShowModal();
}

void mxBreakOptions::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxBreakOptions::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxBreakOptions::OnOkButton(wxCommandEvent &evt) {
	if (break_check->GetValue()) {
		if (debug->debugging) {
			BREAK_POINT_STATUS status=BPS_SETTED;
			long l;	ignore_text->GetValue().ToLong(&l); // ignore count
			if (bpi->ignore_count!=l) {
				bpi->ignore_count=l;
				debug->SetBreakPointOptions(bpi->gdb_id,bpi->ignore_count);
			}
			if (bpi->enabled!=enable_check->GetValue() || bpi->action!=action->GetSelection()) { // enabled and only_once
				bpi->enabled=enable_check->GetValue(); bpi->action=action->GetSelection();
				debug->SetBreakPointEnable(bpi->gdb_id,bpi->enabled,bpi->action==BPA_STOP_ONCE);
			}
			if (!bpi->enabled) status=BPS_USER_DISABLED;
			if (bpi->cond!=cond_text->GetValue() || bpi->gdb_status==BPS_ERROR_CONDITION) { // condition
				bpi->cond=cond_text->GetValue();
				if (!debug->SetBreakPointOptions(bpi->gdb_id,bpi->cond)) status=BPS_ERROR_CONDITION;
			}
			bpi->SetStatus(status,bpi->gdb_id);
		} else {
			long l;	ignore_text->GetValue().ToLong(&l); // ignore count
			bpi->ignore_count=l;
			bpi->enabled=enable_check->GetValue(); 
			bpi->action=action->GetSelection();
			bpi->cond=cond_text->GetValue();
			bpi->SetStatus(BPS_UNKNOWN);
		}
		bpi->annotation=annotation_text->GetValue();
	} else {
		if (debug->debugging)
			debug->DeleteBreakPoint(bpi);
		else delete bpi;
	}
	if (bpi->gdb_status==BPS_ERROR_CONDITION) {
		if (mxMD_NO==mxMessageDialog(this,LANG(BREAKOPTS_ERROR_SETTING_CONDITION,"La condición ingresada no es correcta. ¿Colocar el punto de control incondicionalmente?."),LANG(GENERAL_ERROR,"Advertencia"),mxMD_ERROR|mxMD_YES_NO).ShowModal())
			return;
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
