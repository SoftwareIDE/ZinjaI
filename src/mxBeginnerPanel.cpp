#include <wx/choicebk.h>

#include "mxBeginnerPanel.h"

#include "mxSizers.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "Language.h"

mxBeginnerPanel *beginner_panel=NULL;

enum BEGO_PANELS {
	BEGO_FOR,BEGO_IF,BEGO_WHILE,BEGO_DO,BEGO_SWITCH,
	BEGO_COUNT};

BEGIN_EVENT_TABLE(mxBeginnerPanel, wxPanel)
	EVT_BUTTON(mxID_BEGINNER_FOR,mxBeginnerPanel::OnButtonFor)
	EVT_BUTTON(mxID_BEGINNER_WHILE,mxBeginnerPanel::OnButtonWhile)
	EVT_BUTTON(mxID_BEGINNER_IF,mxBeginnerPanel::OnButtonIf)
	EVT_BUTTON(mxID_BEGINNER_DO,mxBeginnerPanel::OnButtonDo)
	EVT_BUTTON(mxID_BEGINNER_SWITCH,mxBeginnerPanel::OnButtonSwitch)
END_EVENT_TABLE()

mxBeginnerPanel::mxBeginnerPanel(wxWindow *aparent):wxPanel(aparent,wxID_ANY) {
	current_panel=new wxPanel(this,wxID_ANY);
	the_sizer= new wxBoxSizer(wxVERTICAL);
	the_sizer->Add(book = new wxChoicebook(this,wxID_ANY),sizers->Exp0);
	the_sizer->Add(current_panel,sizers->Exp1);
	book->AddPage(CreatePageControls(),LANG(BEGPANEL_CONTROL_STRUCTURES,"Estructuras de Control"));
	show_quickhelp = new wxCheckBox(this,wxID_ANY,LANG(BEGPANEL_SHOW_QUICKHELP,"Mostrar ayuda rapida"));
	the_sizer->Add(show_quickhelp,sizers->Exp0);
	show_quickhelp->SetValue(true);
	SetSizerAndFit(the_sizer);
}

wxPanel *mxBeginnerPanel::CreatePageControls() {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(book, wxID_ANY );
	options = new wxPanel*[BEGO_COUNT];
	sizer->Add(new wxButton(panel,mxID_BEGINNER_IF,_T("if")),sizers->BA5_Exp0);
	sizer->Add(new wxButton(panel,mxID_BEGINNER_SWITCH,_T("switch")),sizers->BA5_Exp0);
	sizer->Add(new wxButton(panel,mxID_BEGINNER_WHILE,_T("while")),sizers->BA5_Exp0);
	sizer->Add(new wxButton(panel,mxID_BEGINNER_DO,_T("do...while")),sizers->BA5_Exp0);
	sizer->Add(new wxButton(panel,mxID_BEGINNER_FOR,_T("for")),sizers->BA5_Exp0);
	sizer->Add(CreateOptionsIf(this),sizers->Exp1);
	sizer->Add(CreateOptionsSwitch(this),sizers->Exp1);
	sizer->Add(CreateOptionsDo(this),sizers->Exp1);
	sizer->Add(CreateOptionsWhile(this),sizers->Exp1);
	sizer->Add(CreateOptionsFor(this),sizers->Exp1);
	
	panel->SetSizerAndFit(sizer);
	return panel;
}

void mxBeginnerPanel::WriteTemplate(const wxString &text) {
	if (main_window->notebook_sources->GetPageCount()==0) return;
	mxSource *src = (mxSource*)main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection());
	if (src->debug_time) return;
	int p1=src->GetSelectionStart();
	int p2=src->GetSelectionEnd();
	if (p2<p1) { int aux=p1; p1=p2; p2=aux; }
	if (p1==p2) {
		p1=src->PositionFromLine(src->LineFromPosition(p1));
		p2=src->GetLineEndPosition(src->LineFromPosition(p2));
	}
	src->SetTargetStart(p1);
	src->SetTargetEnd(p2);
	src->ReplaceTarget(text);
	src->Colourise(p1,p1+text.Len());
	src->SetSelection(p1,p1+text.Len());
	wxCommandEvent evt;
	src->OnIndentSelection(evt);
}

void mxBeginnerPanel::ShowOptions(int n, wxString help) {
	if (options[n]!=current_panel) {
		if (current_panel) current_panel->Hide();
		options[n]->Show();
		the_sizer->Replace(current_panel,options[n]);
		current_panel=options[n];
		Layout();
	}
	LoadQuickHelp(help);
}

void mxBeginnerPanel::LoadQuickHelp(wxString file) {
	file = wxString(_T("beg_"))<<file<<_T("_")<<config->Init.language_file<<_T(".html");
	main_window->quick_help->LoadPage(DIR_PLUS_FILE(config->Help.quickhelp_dir,file));
	main_window->ShowQuickHelpPanel();
}

// ----if------

wxPanel *mxBeginnerPanel::CreateOptionsIf(wxPanel *parent) {
	wxPanel *panel = new wxPanel(parent, wxID_ANY);
	wxStaticBoxSizer *sizer= new wxStaticBoxSizer(wxVERTICAL,panel,LANG(BEGPANEL_OPTIONS_IF,"Opciones: if"));
	
	opt_if_cond = utils->AddTextCtrl(sizer,panel,LANG(BEGPANEL_IF_CONDITION,"Condicion"),_T("cond"));
	opt_if_else = utils->AddCheckBox(sizer,panel,LANG(BEGPANEL_IF_INCLUDE_FALSE,"Incluir acciones por falso"),true);
	
	sizer->Add(new wxButton(panel,mxID_BEGINNER_IF,LANG(BEGPANEL_APPLY,"Aplicar")),sizers->BA5_Exp0);
	
	options[BEGO_IF] = panel;
	panel->SetSizer(sizer);
	panel->Hide();
	return panel;
}

void mxBeginnerPanel::OnButtonIf(wxCommandEvent &evt) {
	ShowOptions(BEGO_IF,_T("if"));
	if (!opt_if_cond->GetValue().Trim().Len())
		opt_if_cond->SetValue(_T("cond"));
	wxString str;
	str<<_T("if (")<<opt_if_cond->GetValue()<<_T(") {\n\n");
	if (opt_if_else->GetValue()) str<<_T("} else {\n\n");
	str<<_T("}");
	WriteTemplate(str);
}

// ----for------

wxPanel *mxBeginnerPanel::CreateOptionsFor(wxPanel *parent) {
	wxPanel *panel = new wxPanel(parent, wxID_ANY);
	wxStaticBoxSizer *sizer= new wxStaticBoxSizer(wxVERTICAL,panel,LANG(BEGPANEL_OPTIONS_FOR,"Opciones: for"));
	
	opt_for_var = utils->AddShortTextCtrl(sizer,panel,LANG(BEGPANEL_FOR_COUNTER,"Contador"),_T("i"));
	opt_for_dec = utils->AddCheckBox(sizer,panel,LANG(BEGPANEL_FOR_DECLARE_COUNTER,"Declarar contador"),true);
	opt_for_from = utils->AddShortTextCtrl(sizer,panel,LANG(BEGPANEL_FOR_FROM,"Desde"),_T("0"));
	opt_for_to = utils->AddShortTextCtrl(sizer,panel,LANG(BEGPANEL_FOR_TO,"Hasta"),_T("n"));
	opt_for_last = utils->AddCheckBox(sizer,panel,LANG(BEGPANEL_FOR_INCLUDE_LAST,"Incluir el ultimo valor"),false);
	opt_for_step = utils->AddShortTextCtrl(sizer,panel,LANG(BEGPANEL_INCREMENT,"Incremento"),_T("1"));
	
	sizer->Add(new wxButton(panel,mxID_BEGINNER_FOR,LANG(BEGPANEL_APPLY,"Aplicar")),sizers->BA5_Exp0);
	
	options[BEGO_FOR] = panel;
	panel->SetSizer(sizer);
	panel->Hide();
	return panel;
}

void mxBeginnerPanel::OnButtonFor(wxCommandEvent &evt) {
	ShowOptions(BEGO_FOR,_T("for"));
	long step, from, to;
	bool bstep = opt_for_step->GetValue().ToLong(&step);
	bool bfrom = opt_for_from->GetValue().ToLong(&from);
	bool bto = opt_for_to->GetValue().ToLong(&to);
	wxString str, menor,mayor;
	if (opt_for_last->GetValue()) {
		menor=_T("<=");
		mayor=_T(">=");
	} else {
		menor=_T("<");
		mayor=_T(">");
	}
	if (!opt_for_var->GetValue().Trim().Len())
		opt_for_var->SetValue(_T("i"));
	if (!opt_for_from->GetValue().Trim().Len())
		opt_for_from->SetValue(_T("0"));
	if (!opt_for_to->GetValue().Trim().Len())
		opt_for_to->SetValue(_T("n"));
	if (!opt_for_step->GetValue().Trim().Len())
		opt_for_step->SetValue(_T("1"));
	str<<_T("for (");
	if (opt_for_dec->GetValue()) str<<_T("int ");
	str<<opt_for_var->GetValue()<<_T("=")<<opt_for_from->GetValue()<<_T(";");
	if (bfrom && bto) {
		if (from>to)
			str<<opt_for_var->GetValue()<<mayor<<opt_for_to->GetValue()<<_T(";");
		else
			str<<opt_for_var->GetValue()<<menor<<opt_for_to->GetValue()<<_T(";");
	} else if (bstep) {
		if (bstep>0)
			str<<opt_for_var->GetValue()<<menor<<opt_for_to->GetValue()<<_T(";");
		else
			str<<opt_for_var->GetValue()<<mayor<<opt_for_to->GetValue()<<_T(";");
	} else
		str<<opt_for_var->GetValue()<<menor<<opt_for_to->GetValue()<<_T(";");
	if (bstep) {
		if (bfrom && bto) {
			if (from>to && step>0) step=-step;
			if (to>from && step<0) step=-step;
		}
		if (step==1)
			str<<opt_for_var->GetValue()<<_T("++");
		else if (step==-1)
			str<<opt_for_var->GetValue()<<_T("--");
		else if (step>0)
			str<<opt_for_var->GetValue()<<_T("+=")<<step;
		else
			str<<opt_for_var->GetValue()<<_T("-=")<<-step;
	} else
		str<<opt_for_var->GetValue()<<_T("+=")<<opt_for_step->GetValue();
	str<<_T(") {\n\n}");
	WriteTemplate(str);
}

// ----while------

wxPanel *mxBeginnerPanel::CreateOptionsWhile(wxPanel *parent) {
	wxPanel *panel = new wxPanel(parent, wxID_ANY);
	wxStaticBoxSizer *sizer= new wxStaticBoxSizer(wxVERTICAL,panel,LANG(BEGPANEL_OPTIONS_WHILE,"Opciones: while"));
	
	opt_while_cond = utils->AddTextCtrl(sizer,panel,LANG(BEGPANEL_WHILE_CONDITION,"Condicion"),_T("cond"));
	
	sizer->Add(new wxButton(panel,mxID_BEGINNER_WHILE,LANG(BEGPANEL_APPLY,"Aplicar")),sizers->BA5_Exp0);
	
	options[BEGO_WHILE] = panel;
	panel->SetSizer(sizer);
	panel->Hide();
	return panel;
}

void mxBeginnerPanel::OnButtonWhile(wxCommandEvent &evt) {
	ShowOptions(BEGO_WHILE,_T("while"));
	wxString str;
	if (!opt_while_cond->GetValue().Trim().Len())
		opt_while_cond->SetValue(_T("cond"));
	str<<_T("while (")<<opt_while_cond->GetValue()<<_T(") {\n\n}");
	WriteTemplate(str);
}

// ----do------

wxPanel *mxBeginnerPanel::CreateOptionsDo(wxPanel *parent) {
	wxPanel *panel = new wxPanel(parent, wxID_ANY);
	wxStaticBoxSizer *sizer= new wxStaticBoxSizer(wxVERTICAL,panel,LANG(BEGPANEL_OPTIONES_DO,"Opciones: do"));
	
	opt_do_cond = utils->AddTextCtrl(sizer,panel,LANG(BEGPANEL_DO_CONDITION,"Condicion"),_T("cond"));
	
	sizer->Add(new wxButton(panel,mxID_BEGINNER_DO,LANG(BEGPANEL_APPLY,"Aplicar")),sizers->BA5_Exp0);
	
	options[BEGO_DO] = panel;
	panel->SetSizer(sizer);
	panel->Hide();
	return panel;
}

void mxBeginnerPanel::OnButtonDo(wxCommandEvent &evt) {
	ShowOptions(BEGO_DO,_T("do"));
	wxString str;
	if (!opt_do_cond->GetValue().Trim().Len())
		opt_do_cond->SetValue(_T("cond"));
	str<<_T("do {\n\n} while (")<<opt_do_cond->GetValue()<<_T(");");
	WriteTemplate(str);
}

// ----switch------

wxPanel *mxBeginnerPanel::CreateOptionsSwitch(wxPanel *parent) {
	wxPanel *panel = new wxPanel(parent, wxID_ANY);
	wxStaticBoxSizer *sizer= new wxStaticBoxSizer(wxVERTICAL,panel,LANG(BEGPANEL_OPTIONS_SWITCH,"Opciones: switch"));
	
	opt_switch_expr = utils->AddTextCtrl(sizer,panel,LANG(BEGPANEL_SWITCH_EXPRESSION,"Expresion"),_T("num"));
	opt_switch_cases = utils->AddShortTextCtrl(sizer,panel,LANG(BEGPANEL_SWITCH_NUM_CASES,"Cantidad de casos"),_T("3"));
	opt_switch_default = utils->AddCheckBox(sizer,panel,LANG(BEGPANEL_SWITCH_INCLUDE_DEFAULT,"Incluir default"),true);
	
	sizer->Add(new wxButton(panel,mxID_BEGINNER_SWITCH,LANG(BEGPANEL_APPLY,"Aplicar")),sizers->BA5_Exp0);
	
	options[BEGO_SWITCH] = panel;
	panel->SetSizer(sizer);
	panel->Hide();
	return panel;
}

void mxBeginnerPanel::OnButtonSwitch(wxCommandEvent &evt) {
	ShowOptions(BEGO_SWITCH,_T("switch"));
	wxString str;
	if (!opt_switch_expr->GetValue().Trim().Len())
		opt_switch_expr->SetValue(_T("num"));
	str<<_T("switch (")<<opt_switch_expr->GetValue()<<_T(") {\n");
	long n = 0;
	if (!opt_switch_cases->GetValue().ToLong(&n)) {
		n=3;
		opt_switch_cases->SetValue(_T("3"));
	}
	for (int i=1;i<=n;i++)
		str<<_T("case ")<<i<<_T(":\n\nbreak;\n");
	if (opt_switch_default->GetValue()) str<<_T("default:\n\n");
	str<<_T("};");
	WriteTemplate(str);
}
