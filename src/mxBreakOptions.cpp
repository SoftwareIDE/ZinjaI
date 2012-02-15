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

BEGIN_EVENT_TABLE(mxBreakOptions, wxDialog)
	EVT_BUTTON(wxID_OK,mxBreakOptions::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxBreakOptions::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxBreakOptions::OnHelpButton)
	EVT_CHECKBOX(mxID_BREAK_OPTS_ENABLE,mxBreakOptions::OnBreakpointCheck)
	EVT_CLOSE(mxBreakOptions::OnClose)
END_EVENT_TABLE()
	
mxBreakOptions::mxBreakOptions(wxString afilename, int aline, mxSource *asource, break_line_item *aitem) : wxDialog(main_window, wxID_ANY, LANG(BREAKOPTS_CAPTION,"Propiedades del Breakpoint"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	source=asource; line=aline; filename=afilename; file=NULL;
	if ( !(bitem=aitem) ) {
		bitem = source->first_break_item;
		while (bitem && source->MarkerLineFromHandle(bitem->handle)!=line)
			bitem = bitem->next;
	}
	CommonConstructorStuff();
}

mxBreakOptions::mxBreakOptions(wxString afilename, int aline, file_item *afile, break_line_item *aitem) : wxDialog(main_window, wxID_ANY, LANG(BREAKOPTS_CAPTION,"Propiedades del Breakpoint"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	file=afile; line=aline; filename=afilename; source=NULL;
	if ( !(bitem=aitem) ) {
		bitem = file->breakpoints;
		while (bitem && bitem->line!=line)
			bitem = bitem->next;
	}
	CommonConstructorStuff();
}

void mxBreakOptions::CommonConstructorStuff() {
	// buscar el item de la lista que se corresponde

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

	utils->AddTextCtrl(mySizer,this,LANG(BREAKOPTS_FILE,"Archivo"),filename)->SetEditable(false);
	utils->AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_LINE,"Linea"),wxString()<<line)->SetEditable(false);
	break_check = utils->AddCheckBox(mySizer,this,LANG(BREAKOPTS_INSERT,"Insertar punto de interrupcion"),bitem,mxID_BREAK_OPTS_ENABLE);
	enable_check = utils->AddCheckBox(mySizer,this,LANG(BREAKOPTS_ENABLE,"Habilitar punto de interrupcion"),bitem?bitem->enabled:true);
	once_check = utils->AddCheckBox(mySizer,this,LANG(BREAKOPTS_ONCE,"Interrumpir solo una vez"),bitem?bitem->only_once:false);
	ignore_text = utils->AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_IGNORE_TIMES_PRE,"Ignorar"),bitem?bitem->ignore_count:0,wxString(LANG(BREAKOPTS_IGNORE_TIMES_POST,"veces")));
	if (debug->debugging && !debug->waiting && bitem && !bitem->error) {
		count_text = utils->AddShortTextCtrl(mySizer,this,LANG(BREAKOPTS_HIT_TIMES_PRE,"Se ha alcanzado"),debug->GetBreakHitCount(bitem->num),wxString(LANG(BREAKOPTS_HIT_TIMES_POST,"veces")));
		count_text->SetEditable(false);
	} else {
		count_text = NULL;
	}
	cond_text = utils->AddTextCtrl(mySizer,this,LANG(BREAKOPTS_CONDITION,"Condicion"),bitem?bitem->cond:wxString(_T("")));
	
	if (bitem && bitem->error) {
		if (bitem->valid_cond)
			utils->AddStaticText(mySizer,this,LANG(BREAKOPTS_ERROR_PLACING_BREAKPOINT,"Error al colocar breakpoint"));
		else 
			utils->AddStaticText(mySizer,this,LANG(BREAKOPTS_INVALID_CONDITION,"La condicion actual no es valida"));
	}
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
		if (!bitem) {
			if (source) {
				bitem = new break_line_item(line,NULL,source->first_break_item);
				if (source->first_break_item) source->first_break_item->prev=bitem;
				source->first_break_item = bitem;
			} else {
				bitem = new break_line_item(line,NULL,file->breakpoints);
				if (file->breakpoints) file->breakpoints->prev=bitem;
				file->breakpoints = bitem;
			}
			bitem->error=debug->debugging;
		} else if (source) {
			source->MarkerDeleteHandle(bitem->handle);
		}
		bitem->enabled = enable_check->GetValue();
		bitem->only_once = once_check->GetValue();
		if (bitem->cond != cond_text->GetValue()) {
			bitem->valid_cond = true;
			bitem->cond = cond_text->GetValue();
		}
		long l;
		ignore_text->GetValue().ToLong(&l);
		bitem->ignore_count = l;
		if (debug->debugging && bitem->error) { // si hay que setearlo en el depurador
			bitem->num = debug->SetBreakPoint(filename,line);
			bitem->error = bitem->num==-1;
			if (!bitem->error) {
				if (debug->SetBreakPointOptions(bitem->num,bitem->cond)) {
					bitem->valid_cond=true;
					debug->SetBreakPointOptions(bitem->num,bitem->ignore_count);
					debug->SetBreakPointEnable(bitem->num,bitem->enabled,bitem->only_once);
				} else {
					bitem->error=true;
					bitem->valid_cond=false;
				}
			} 
		} else {
			if (debug->debugging)
				bitem->valid_cond=debug->SetBreakPointOptions(bitem->num,bitem->cond);
			bitem->error=!bitem->valid_cond;
		}
		if (source) {
			if (bitem->enabled && !bitem->error)
				bitem->handle = source->MarkerAdd(line, mxSTC_MARK_BREAKPOINT);
			else
				bitem->handle = source->MarkerAdd(line, mxSTC_MARK_BAD_BREAKPOINT);
		}
	} else {
		if (bitem) {
			if (!bitem->error && debug->debugging)
				debug->DeleteBreakPoint(bitem->num);
			if (bitem->next)
				bitem->next->prev=bitem->prev;
			if (bitem->prev)
				bitem->prev->next=bitem->next;
			else if(source)
				source->first_break_item=bitem->next;
			else
				file->breakpoints=bitem->next;
			if (source)
				source->MarkerDeleteHandle(bitem->handle);
			delete bitem;
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
	once_check->Enable(enable);
}

void mxBreakOptions::OnHelpButton(wxCommandEvent &evt) {
	SHOW_HELP(_T("break_options.html"));
}
