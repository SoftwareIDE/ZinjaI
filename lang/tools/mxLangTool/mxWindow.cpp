#include "mxWindow.h"
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/textdlg.h>
#include <iostream>
#include <wx/dir.h>
#include <wx/choicdlg.h>
	
	
using namespace std;

enum {
	mxZERO = wxID_HIGHEST,
	mxNEXT, mxPREV, mxAPPLY, mxUNDO,
	mxREF, mxLANG, mxSAVE, mxQUIT, mxHELP,
	mxDONE, mxPENDING, mxDOUBT, mxALL, mxFIND
};

BEGIN_EVENT_TABLE(mxWindow,wxFrame)
	EVT_LISTBOX(wxID_ANY,mxWindow::OnSelect)
	EVT_BUTTON(mxAPPLY,mxWindow::OnApplyNext)
	EVT_BUTTON(mxUNDO,mxWindow::OnUndo)
	EVT_BUTTON(mxSAVE,mxWindow::OnSave)
	EVT_BUTTON(mxALL,mxWindow::OnShowAll)
	EVT_BUTTON(mxDONE,mxWindow::OnShowDone)
	EVT_BUTTON(mxPENDING,mxWindow::OnShowPending)
	EVT_BUTTON(mxREF,mxWindow::OnChangeRef)
	EVT_BUTTON(mxLANG,mxWindow::OnChangeLang)
	EVT_BUTTON(mxFIND,mxWindow::OnFind)
END_EVENT_TABLE();

mxWindow::mxWindow():wxFrame(NULL,wxID_ANY,"mxLangTool") {
	
	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	
	wxSizer *curref_sizer = new wxBoxSizer(wxHORIZONTAL);
	curref_sizer->Add(new wxStaticText(this,wxID_ANY,"Current Reference Language: "));
	curref_sizer->Add(curref_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(1).Expand());
	curref_text->SetEditable(false);
	curref_sizer->Add(new wxButton(this,mxREF,"..."));
	main_sizer->Add(curref_sizer,wxSizerFlags().Proportion(0).Expand());
	
	wxSizer *curlang_sizer = new wxBoxSizer(wxHORIZONTAL);
	curlang_sizer->Add(new wxStaticText(this,wxID_ANY,"Current Language for Edition: "));
	curlang_sizer->Add(curlang_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(1).Expand());
	curlang_text->SetEditable(false);
	curlang_sizer->Add(new wxButton(this,mxLANG,"..."));
	main_sizer->Add(curlang_sizer,wxSizerFlags().Proportion(0).Expand());
	
	wxSizer *status_sizer = new wxBoxSizer(wxHORIZONTAL);
	status_sizer->Add(new wxStaticText(this,wxID_ANY,"Status: "));
	status_sizer->Add(status_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(1).Expand());
	curlang_text->SetEditable(false);
	status_sizer->Add(new wxButton(this,mxSAVE,"Save"));
	main_sizer->Add(status_sizer,wxSizerFlags().Proportion(0).Expand());

	wxSizer *wich_sizer = new wxBoxSizer(wxHORIZONTAL);
	wich_sizer->Add(new wxButton(this,mxALL,"Show All"),wxSizerFlags().Proportion(0).Expand());
	wich_sizer->Add(new wxButton(this,mxDONE,"Show Translated"),wxSizerFlags().Proportion(0).Expand());
	wich_sizer->Add(new wxButton(this,mxPENDING,"Show Not Translated"),wxSizerFlags().Proportion(0).Expand());
	wich_sizer->Add(new wxButton(this,mxFIND,"Find"),wxSizerFlags().Proportion(0).Expand());
	main_sizer->Add(wich_sizer,wxSizerFlags().Proportion(0).Center());
	
	main_sizer->Add(list = new wxListBox(this,wxID_ANY),wxSizerFlags().Proportion(1).Expand());
	
	main_sizer->Add(new wxStaticText(this,wxID_ANY,"Enum Item"),wxSizerFlags().Proportion(0).Expand());
	main_sizer->Add(item_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(0).Expand());
	item_text->SetEditable(false);
	main_sizer->Add(new wxStaticText(this,wxID_ANY,"Reference Text"),wxSizerFlags().Proportion(0).Expand());
	main_sizer->Add(ref_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(0).Expand());
	ref_text->SetEditable(false);
	main_sizer->Add(new wxStaticText(this,wxID_ANY,"Translation"),wxSizerFlags().Proportion(0).Expand());
	main_sizer->Add(trans_text = new wxTextCtrl(this,wxID_ANY),wxSizerFlags().Proportion(0).Expand());
//	main_sizer->Add(new wxCheckBox(this,wxID_ANY,"Doubtful Translation"),wxSizerFlags().Proportion(0).Expand());
	
	wxSizer *nav_sizer = new wxBoxSizer(wxHORIZONTAL);
	nav_sizer->Add(new wxButton(this,mxHELP,"Help"),wxSizerFlags().Proportion(0).Expand());
	nav_sizer->AddStretchSpacer();
//	nav_sizer->Add(new wxButton(this,mxPREV,"Prev"),wxSizerFlags().Proportion(0).Expand());
	nav_sizer->Add(new wxButton(this,mxUNDO,"Undo"),wxSizerFlags().Proportion(0).Expand());
	wxButton *btan;
	nav_sizer->Add(btan = new wxButton(this,mxAPPLY,"Apply And Next"),wxSizerFlags().Proportion(0).Expand());
//	nav_sizer->Add(new wxButton(this,mxNEXT,"Next"),wxSizerFlags().Proportion(0).Expand());
	main_sizer->Add(nav_sizer,wxSizerFlags().Proportion(0).Expand());
	
	btan->SetDefault();	
	SetSizerAndFit(main_sizer);
	list->Append("loading...");
	Show();
	
	LoadEnum();
	lang_file="english";
	curref_text->SetValue("spanish");
	curlang_text->SetValue(lang_file);
	LoadLang(ref_hash,"spanish");
	LoadLang(lang_hash,lang_file);
	wxCommandEvent evt;
	OnShowAll(evt);
	
}

void mxWindow::LoadEnum() {
	wxFFileInputStream ff("src/lang_index_macros.h");
	if (!ff.IsOk()) {
		wxMessageBox("Error al cargar src/lang_index_macros.h");
		return;
	}
	enum_array.Clear();
	bool add=false;
	wxTextInputStream is(ff);
	while (true) {
		wxString line = is.ReadLine();
		if (line.Contains(",")) line=line.BeforeFirst(',');
		line.Trim(false).Trim(true);
		if (line=="LANGUAGE_MAX") break;
		else if (line.StartsWith("LANGUAGE_ZERO")) add=true;
		else if (add && line.Len()) enum_array.Add(line.Mid(5));
	}
}

void mxWindow::LoadLang(wxHashStringString &hash, wxString lang) {
	wxFFileInputStream ff((wxString("lang/")+lang+".src").c_str());
	if (!ff.IsOk()) {
		wxMessageBox(wxString("Error al cargar lang/")+lang+".src");
		return;
	}
	hash.clear();
	wxTextInputStream is(ff);
	wxString key,text;
	while (true) {
		wxString key = is.ReadLine();
		key.Trim(false).Trim(true);
		if (key=="END") break;
		text = is.ReadLine();
		if (enum_array.Index(key)!=wxNOT_FOUND) hash[key]=text;
	}
	UpdateStatics();
}

void mxWindow::OnShowAll(wxCommandEvent &evt) {
	list->Clear();
	list->Freeze();
	for (unsigned int i=0;i<enum_array.GetCount();i++)
		list->Append(enum_array[i]);
	list->Thaw();
}

void mxWindow::OnShowDone(wxCommandEvent &evt) {
	list->Clear();
	list->Freeze();
	for (unsigned int i=0;i<enum_array.GetCount();i++) {
		wxHashStringString::iterator it = lang_hash.find(enum_array[i]);
		if (it!=lang_hash.end() && it->second.Len())
			list->Append(enum_array[i]);
	}	
	list->Thaw();
}

void mxWindow::OnShowPending(wxCommandEvent &evt) {	
	list->Clear();
	list->Freeze();
	for (unsigned int i=0;i<enum_array.GetCount();i++) {
		wxHashStringString::iterator it = lang_hash.find(enum_array[i]);
		if (it==lang_hash.end() || it->second.Len()==0)
			list->Append(enum_array[i]);
	}	
	list->Thaw();
}

void mxWindow::OnFind(wxCommandEvent &evt) {	
	wxString key = wxGetTextFromUser("Texto a buscar:");
	if (key.Len()==0) return;
	list->Clear();
	list->Freeze();
	key.MakeUpper();
	for (unsigned int i=0;i<enum_array.GetCount();i++) {
		if (enum_array[i].Upper().Contains(key))
			list->Append(enum_array[i]);
		else {
			wxHashStringString::iterator it = lang_hash.find(enum_array[i]);
			if (it!=lang_hash.end() && it->second.Upper().Contains(key))
				list->Append(enum_array[i]);
		}
	}	
	list->Thaw();
}

void mxWindow::OnApplyNext(wxCommandEvent &evt) {
	if (sel_item)
		lang_hash[sel_item]=trans_text->GetValue();
	int is = list->GetSelection()+1;
	if (is!=int(list->GetCount()))
		list->Select(is);
	UpdateStatics();
	OnSelect(evt);
}

void mxWindow::OnUndo(wxCommandEvent &evt) {
	wxHashStringString::iterator it_lang = lang_hash.find(sel_item);
	if (it_lang!=lang_hash.end())
		trans_text->SetValue(it_lang->second);
	else
		trans_text->SetValue("");	
}

void mxWindow::OnSave(wxCommandEvent &evt) {
	wxFFileOutputStream ffs((wxString("lang/")+lang_file+".src").c_str());
	wxFFileOutputStream ffp((wxString("lang/")+lang_file+".pre").c_str());
	wxFFileOutputStream ffg((wxString("lang/")+lang_file+".sgn").c_str());
	wxTextOutputStream top(ffp);
	wxTextOutputStream tos(ffs);
	wxTextOutputStream tog(ffg);
	top<<"ZINJAI_LANGUAGE_FILE\n";
	top<<lang_file<<"\n";
	for (unsigned int i=0;i<enum_array.GetCount();i++) {
		wxHashStringString::iterator it_lang = lang_hash.find(enum_array[i]);
		top<<enum_array[i]<<"\n";
		if (it_lang!=lang_hash.end()) {
			tos<<enum_array[i]<<"\n";
			tos<<it_lang->second<<"\n";
			top<<it_lang->second<<"\n";
		} else {
			wxHashStringString::iterator it_ref = ref_hash.find(enum_array[i]);
			if (it_ref!=ref_hash.end())
				top<<it_ref->second<<"\n";
			else
				top<<enum_array[i]<<"\n";
		}
	}
	tos<<"END"<<"\n";
	top<<"END"<<"\n";
	tog<<lang_file<<"-"<<wxDateTime::Now().Format("%y%m%d%H%M%S")<<"\n";
	ffs.Close();
	ffp.Close();
	ffg.Close();
}

void mxWindow::OnChangeLang(wxCommandEvent &evt) {
	wxDir dir("lang");
	wxString spec="*.src", filename;
	bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
	wxArrayString langs;
	while ( cont ) {
		langs.Add(filename.BeforeLast('.'));
		cont = dir.GetNext(&filename);
	}
	wxString newlang = wxGetSingleChoice("Select a Language:","mxLangTool",langs);
	if (newlang.Len()) {
		LoadLang(lang_hash,newlang);
		curlang_text->SetValue(newlang);
		OnSelect(evt);
	}
}

void mxWindow::OnChangeRef(wxCommandEvent &evt) {
	wxDir dir("lang");
	wxString spec="*.src", filename;
	bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
	wxArrayString langs;
	while ( cont ) {
		langs.Add(filename.BeforeLast('.'));
		cont = dir.GetNext(&filename);
	}
	wxString newlang = wxGetSingleChoice("Select a Language:","mxLangTool",langs);
	if (newlang.Len()) {
		LoadLang(ref_hash,newlang);
		curref_text->SetValue(newlang);
		OnSelect(evt);
	}
}

void mxWindow::OnSelect(wxCommandEvent &evt) {
	int is = list->GetSelection();
	if (is==-1) return;
	sel_item = list->GetString(is);
	item_text->SetValue(sel_item);
	wxHashStringString::iterator it_ref = ref_hash.find(sel_item);
	wxHashStringString::iterator it_lang = lang_hash.find(sel_item);
	if (it_ref!=ref_hash.end())
		ref_text->SetValue(it_ref->second);
	else
		ref_text->SetValue("");
	if (it_lang!=lang_hash.end())
		trans_text->SetValue(it_lang->second);
	else
		trans_text->SetValue("");
}

void mxWindow::UpdateStatics() {
	status_text->SetValue(wxString()<<lang_hash.size()<<" of "<<enum_array.GetCount()<<" completed ("<<int(lang_hash.size()*100/enum_array.GetCount())<<"%)");
}
