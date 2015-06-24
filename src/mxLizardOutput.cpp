#include "mxLizardOutput.h"
#include <wx/sizer.h>
#include <wx/notebook.h>
#include "mxSizers.h"

BEGIN_EVENT_TABLE(mxLizardOutput,wxDialog)
END_EVENT_TABLE()

mxLizardOutput::mxLizardOutput (wxWindow * parent, wxArrayString & results) 
	: wxDialog(parent, wxID_ANY, "lizard output", wxDefaultPosition, wxSize(700,400),
		wxALWAYS_SHOW_SB|wxALWAYS_SHOW_SB|wxDEFAULT_FRAME_STYLE|wxSUNKEN_BORDER)
{
	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this,wxID_ANY);
	
	ParseResults(results); // creates functions_table && files_table
	
	notebook->AddPage(functions_table,"functions");
	notebook->AddPage(files_table,"files");
	main_sizer->Add(notebook,sizers->Exp1);
	SetSizer(main_sizer);
	Show();
}

static wxString nextColum(wxString &s) {
	int i=0, l=s.Len();
	while (i<l&&(s[i]==' '||s[i]=='\t')) i++;
	int i0=i;
	while (i<l&&(s[i]!=' '&&s[i]!='\t')) i++;
	wxString val = s.Mid(i0,i-i0);
	s=s.Mid(i);
	return val;
}

static int toInt(const wxString &s) {
	long x=0; 
	if (s.Contains('.')) s.BeforeFirst('.').ToLong(&x); 
	else s.ToLong(&x); 
	return x;
}

typedef mxPRTGenericRow<4> LizardFunctionRow;
typedef mxPRTGenericRow<5> LizardFilesRow;

void mxLizardOutput::ParseResults (wxArrayString & results) {
	
	int i=0;
	while (!results[i].StartsWith("=====")) i++;
	i+=3;
	wxArrayString functions_titles;
	functions_titles.Add("# LOC");
	functions_titles.Add("CCN");
	functions_titles.Add("token #");
	functions_titles.Add("params #");
	functions_titles.Add("location");
	functions_titles.Add("file");
	mxPRTGenericTable<LizardFunctionRow> *functions = new mxPRTGenericTable<LizardFunctionRow>(functions_titles);
	
	while (i<int(results.GetCount()) && !results[i].StartsWith("-----")) {
		wxString &s=results[i];
		LizardFunctionRow r;
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		wxString func = nextColum(s);
		r.Add(func.BeforeFirst('@'));
		r.Add(func.AfterLast('@')+":"+func.AfterFirst('@').BeforeFirst('-'));
		functions->AddRow(r);
		i++;
	}
	
	while (!results[i].StartsWith("=====")) i++;
	i+=3;
	wxArrayString files_titles;
	files_titles.Add("# LOC");
	files_titles.Add("Avg. # LOC");
	files_titles.Add("Avg. CCN");
	files_titles.Add("Avg. token #");
	files_titles.Add("functions count");
	files_titles.Add("file");
	mxPRTGenericTable<LizardFilesRow> *files= new mxPRTGenericTable<LizardFilesRow>(files_titles);
	
	while (i<int(results.GetCount()) && !results[i].StartsWith("=====") && !results[i].IsEmpty()) {
		wxString &s=results[i];
		LizardFilesRow r;
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(toInt(nextColum(s)));
		r.Add(nextColum(s));
		files->AddRow(r);
		i++;
	}
	
	functions_table = new mxPlainResultsTable(notebook,functions);
	files_table = new mxPlainResultsTable(notebook,files);
}

