#include <wx/checkbox.h>
#include <wx/textfile.h>
#include "mxDiffWindow.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxMessageDialog.h"
#include "ids.h"
#include "mxHelpWindow.h"
#include "Language.h"
#include "mxDiffSideBar.h"
#include "Parser.h"
#include "execution_workaround.h"

BEGIN_EVENT_TABLE(mxDiffWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxDiffWindow::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxDiffWindow::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxDiffWindow::OnHelpButton)
	EVT_CLOSE(mxDiffWindow::OnClose)
END_EVENT_TABLE()

mxDiffWindow::mxDiffWindow(mxSource *asource, wxString fname) : wxDialog(main_window, wxID_ANY, LANG(DIFF_CAPTION,"Comparar archivos"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	// buscar el item de la lista que se corresponde
	
	source = asource;
	diff_to_name = fname;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(DIFF_COMPARE,"&Comparar"));
	SetEscapeId(wxID_CANCEL);
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	buttonSizer->Add(help_button,sizers->BA5_Exp0);
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(cancel_button,sizers->BA5);
	buttonSizer->Add(ok_button,sizers->BA5);
	
	if (!source) {
		wxArrayString array;
		for (unsigned int i=0;i<main_window->notebook_sources->GetPageCount();i++)  {
			array.Add(((mxSource*)(main_window->notebook_sources->GetPage(i)))->page_text);
		}
		
		file1 = mxUT::AddComboBox(mySizer,this,LANG(DIFF_SOURCE_A,"Archivo A"),array,array.GetCount()-1);
		file2 = mxUT::AddComboBox(mySizer,this,LANG(DIFF_SOURCE_B,"Archivo B"),array,array.GetCount()-1);
		
		if (main_window->notebook_sources->GetPageCount()) {
			file1->SetSelection(main_window->notebook_sources->GetSelection());
			if (main_window->notebook_sources->GetSelection()==0)
				file2->SetSelection(1);
			else
				file2->SetSelection(0);
		}
	}
	
	mySizer->Add(ignore_spaces = new wxCheckBox(this,wxID_ANY,LANG(DIFF_IGNORE_EXTRA_WHITESPACE,"Ignorar espacios extras")),sizers->BA5_Exp0);
	mySizer->Add(ignore_empty_lines = new wxCheckBox(this,wxID_ANY,LANG(DIFF_IGNORE_EMPTY_LINES,"Ignorar lineas en blanco")),sizers->BA5_Exp0);
	mySizer->Add(ignore_case = new wxCheckBox(this,wxID_ANY,LANG(DIFF_NO_CASE_SENSITIVE,"Ignorar mayusculas y minusculas")),sizers->BA5_Exp0);
	mySizer->Add(show_tools = new wxCheckBox(this,wxID_ANY,LANG(DIFF_SHOW_TOOLBOX,"Mostrar barra de herramientas")),sizers->BA5_Exp0);
	mySizer->Add(show_sidebar = new wxCheckBox(this,wxID_ANY,LANG(DIFF_SHOW_SIDEMAP,"Mostrar mapa lateral")),sizers->BA5_Exp0);
	show_tools->SetValue(true);
	show_sidebar->SetValue(true);
	
	mySizer->Add(buttonSizer,sizers->Exp0);
	SetSizerAndFit(mySizer);
	ok_button->SetFocus();
	Show();
	
}

void mxDiffWindow::OnOkButton(wxCommandEvent &evt) {
	if (source) {
		DiffSourceFile(source,diff_to_name);
	} else {
		mxSource *src1 = (mxSource*)(main_window->notebook_sources->GetPage(file1->GetSelection()));
		mxSource *src2 = (mxSource*)(main_window->notebook_sources->GetPage(file2->GetSelection()));
		DiffTwoSources(src1,src2);
		main_window->notebook_sources->Split(file2->GetSelection(),wxRIGHT);
		main_window->notebook_sources->SetSelection(file1->GetSelection());
		src1->SetDiffBrother(src2);
		src2->SetDiffBrother(src1);
		
	}
	Close();
}

void mxDiffWindow::DiffSourceFile(mxSource *src, wxString fname) {
	src->SaveTemp(DIR_PLUS_FILE(config->temp_dir,"fordiff"));
	wxString command( OSDEP_VAL( DIR_PLUS_FILE(config->zinjai_third_dir,"gnuwin32\\bin\\diff.exe") , "diff" ) );
	command << " ";
	if (ignore_empty_lines->GetValue()) command<<"-B ";
	if (ignore_spaces->GetValue()) command<<"-E -b ";
	if (ignore_case->GetValue()) command<<"-i ";
	command<<DIR_PLUS_FILE(config->temp_dir,"fordiff");
	command<<" "<<fname;
	src->MarkDiffs(0,src->GetLineCount()-1,mxSTC_MARK_DIFF_NONE);
	wxArrayString output;
	mxExecute(command,output,wxEXEC_SYNC);
	wxTextFile text_file(fname);
	text_file.Open();
	if (output.GetCount()) {
		for (unsigned int i=0;i<output.GetCount();i++) {
			int n1=0,n2=0,n3=0,n4=0,p=0;
			while (output[i][p]>='0' && output[i][p]<='9')
				n1 = n1*10 + output[i][p++]-'0' ;
			if (output[i][p]==',') {
				p++;
				while (output[i][p]>='0' && output[i][p]<='9')
					n2 = n2*10 + output[i][p++]-'0' ;
			} else
				n2=n1;
			char c=output[i][p++];
			while (output[i][p]>='0' && output[i][p]<='9')
				n3 = n3*10 + output[i][p++]-'0' ;
			if (output[i][p]==',') {
				p++;
				while (output[i][p]>='0' && output[i][p]<='9')
					n4 = n4*10 + output[i][p++]-'0' ;
			} else
				n4=n3;
			n1--; n2--; n3--; n4--;
			int skip=0, line_count=text_file.GetLineCount();
			if (c=='c') {
				wxString s;
				for (int n=n1;n<=n2;n++) 
					if (n<line_count)
						s<<text_file.GetLine(n)<<"\n";
				s.RemoveLast();
				src->MarkDiffs(n1,n2,mxSTC_MARK_DIFF_CHANGE,s);
				skip= n2-n1+1 + n4-n3+1 + 1;
			} else if (c=='a') {
				wxString s;
				for (int n=n3;n<=n4;n++) 
					if (n<line_count)
						s<<text_file.GetLine(n)<<"\n";
				s.RemoveLast();
				src->MarkDiffs(n1+1,n1+1,mxSTC_MARK_DIFF_DEL,s);
				skip = n4-n3+1;
			} else if (c=='d') {
				src->MarkDiffs(n1,n2,mxSTC_MARK_DIFF_ADD,LANG(DIFF_LINES_NOT_IN_OTHER_FILE,"<<Estas lineas no se encuentran en el otro archivo>>"));
				skip = n2-n1+1;
			}
			i+=skip;
		}
		text_file.Close();
		main_window->ShowDiffSideBar(show_sidebar->GetValue(),show_tools->GetValue());
	} else {
		mxMessageDialog(main_window,LANG(DIFF_FILES_EQUAL,"Los archivos son iguales"),LANG(DIFF_CAPTION,"Comparacion"),mxMD_OK|mxMD_INFO).ShowModal();		
	}
}

void mxDiffWindow::DiffTwoSources(mxSource *src1, mxSource *src2) {
	src1->SaveTemp(DIR_PLUS_FILE(config->temp_dir,"fordiff1"));
	src2->SaveTemp(DIR_PLUS_FILE(config->temp_dir,"fordiff2"));
	wxString command("diff --strip-trailing-cr ");
	if (ignore_empty_lines->GetValue()) command<<"-B ";
	if (ignore_spaces->GetValue()) command<<"-E -b ";
	if (ignore_case->GetValue()) command<<"-i ";
	command<<"\""<<DIR_PLUS_FILE(config->temp_dir,"fordiff1\"");
	command<<" \""<<DIR_PLUS_FILE(config->temp_dir,"fordiff2\"");
	src1->MarkDiffs(0,src1->GetLineCount()-1,mxSTC_MARK_DIFF_NONE);
	src2->MarkDiffs(0,src2->GetLineCount()-1,mxSTC_MARK_DIFF_NONE);
	wxArrayString output;
	mxExecute(command,output,wxEXEC_SYNC);
	if (output.GetCount()) {
		for (unsigned int i=0;i<output.GetCount();i++) {
			int n1=0,n2=0,n3=0,n4=0,p=0;
			while (output[i][p]>='0' && output[i][p]<='9')
				n1 = n1*10 + output[i][p++]-'0' ;
			if (output[i][p]==',') {
				p++;
				while (output[i][p]>='0' && output[i][p]<='9')
					n2 = n2*10 + output[i][p++]-'0' ;
			} else
				n2=n1;
			char c=output[i][p++];
			while (output[i][p]>='0' && output[i][p]<='9')
				n3 = n3*10 + output[i][p++]-'0' ;
			if (output[i][p]==',') {
				p++;
				while (output[i][p]>='0' && output[i][p]<='9')
					n4 = n4*10 + output[i][p++]-'0' ;
			} else
				n4=n3;
			n1--; n2--; n3--; n4--;
			int skip=0;
			if (c=='c') {
				DiffInfo *di1=src1->MarkDiffs(n1,n2,mxSTC_MARK_DIFF_CHANGE,src2->GetTextRange(src2->PositionFromLine(n3),src2->GetLineEndPosition(n4)));
				DiffInfo *di2=src2->MarkDiffs(n3,n4,mxSTC_MARK_DIFF_CHANGE,src1->GetTextRange(src1->PositionFromLine(n1),src1->GetLineEndPosition(n2)));
				di1->brother=di2; di2->brother=di1;
				skip= n2-n1+1 + n4-n3+1 + 1;
			} else if (c=='a') {
				DiffInfo *di1=src1->MarkDiffs(n1+1,n1+1,mxSTC_MARK_DIFF_DEL,src2->GetTextRange(src2->PositionFromLine(n3),src2->GetLineEndPosition(n4)));
				DiffInfo *di2=src2->MarkDiffs(n3,n4,mxSTC_MARK_DIFF_ADD,LANG(DIFF_LINES_NOT_IN_OTHER_FILE,"<<Estas lineas no se encuentran en el otro archivo>>"));
				di1->brother=di2; di2->brother=di1;
				skip = n4-n3+1;
			} else if (c=='d') {
				DiffInfo *di1=src2->MarkDiffs(n3+1,n3+1,mxSTC_MARK_DIFF_DEL,src1->GetTextRange(src1->PositionFromLine(n1),src1->GetLineEndPosition(n2)));
				DiffInfo *di2=src1->MarkDiffs(n1,n2,mxSTC_MARK_DIFF_ADD,LANG(DIFF_LINES_NOT_IN_OTHER_FILE,"<<Estas lineas no se encuentran en el otro archivo>>"));
				di1->brother=di2; di2->brother=di1;
				skip = n2-n1+1;
			}
			i+=skip;
		}
		DiffInfo *d1 = src1->first_diff_info, *d2 = src2->first_diff_info;
		while (d1&&d2) {
			int l1=d1->len, l2=d2->len;
			d1->bhandles=new int[l1];
			d2->bhandles=new int[l2];
			for (int i=0;i<l1||i<l2;i++) {
				if (i>=l1)
					d2->bhandles[i]=d1->handles[l1-1];
				else if (i>=l2) 
					d1->bhandles[i]=d2->handles[l2-1];
				else {
					d1->bhandles[i]=d2->handles[i];
					d2->bhandles[i]=d1->handles[i];
				}
			}
			d1=d1->next; d2=d2->next;
		}
		main_window->ShowDiffSideBar(show_sidebar->GetValue(),show_tools->GetValue());
	} else {
		mxMessageDialog(this,LANG(DIFF_FILES_EQUAL,"Los archivos son iguales"),LANG(DIFF_CAPTION,"Comparacion"),mxMD_OK|mxMD_INFO).ShowModal();		
	}
}

void mxDiffWindow::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxDiffWindow::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxDiffWindow::OnHelpButton(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("diff.html");
}
