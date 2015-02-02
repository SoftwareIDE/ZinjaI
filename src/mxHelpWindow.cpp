#include <wx/textfile.h>
#include "mxHelpWindow.h"
#include "ConfigManager.h"
#include <wx/treectrl.h>
#include "mxMessageDialog.h"
#include "mxMainWindow.h"

#define ERROR_PAGE(page) wxString("<I>ERROR</I>: La pagina \"")<<page<<"\" no se encuentra. <br><br> La ayuda de <I>ZinjaI</I> aun esta en contruccion."
#define _index "index"
#include "ids.h"
#include "mxSizers.h"
#include "mxReferenceWindow.h"

mxHelpWindow *mxHelpWindow::instance=nullptr;

mxHelpWindow::mxHelpWindow(wxString file) : mxGenericHelpWindow(LANG(HELPW_CAPTION,"Ayuda de ZinjaI"),true) { 
	ignore_tree_event=false;
	// populate index tree
	wxString index_file=DIR_PLUS_FILE(config->Help.guihelp_dir,"index_"+config->Init.language_file);
	if (!wxFileName::FileExists(index_file))
		index_file=DIR_PLUS_FILE(config->Help.guihelp_dir,"index_spanish");
	wxTextFile fil(index_file);
	if (fil.Exists()) {
		fil.Open();
		wxTreeItemId root = tree->AddRoot("Temas de Ayuda",0);
		wxTreeItemId node = root;
		unsigned int tabs=0;
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			unsigned int i=0;
			while (i<str.Len() && str[i]=='\t') 
				i++;
			if (i!=0 && str.Len()>i+3) {
				if (i==tabs) {
					node=tree->AppendItem(tree->GetItemParent(node),str.Mid(i+2).AfterFirst(' '),str[i]-'0');
				} else if (i>tabs) {
					node=tree->AppendItem(node,str.Mid(i+2).AfterFirst(' '),str[i]-'0');
				} else {
					for (unsigned int j=0;j<tabs-i;j++)
						node=tree->GetItemParent(node);
					node=tree->AppendItem(tree->GetItemParent(node),str.Mid(i+2).AfterFirst(' '),str[i]-'0');
				}
				items[str.Mid(i+2).BeforeFirst(' ')]=node;
				tabs=i;
			}
		}
		fil.Close();
//		tree->Expand(root);
		wxTreeItemIdValue cokkie;
		node = tree->GetFirstChild(root,cokkie);
		while (node.IsOk()) {
			tree->Expand(node);
			node = tree->GetNextSibling(node);
		}
	}
	
	if (!file.Len()) file="index"; LoadHelp(file);
	
	wxBoxSizer *forum_sizer = new wxBoxSizer(wxHORIZONTAL); wxButton *button;
	forum_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(HELPW_FORUM_TEXT,"¿Lo que buscas no está en la ayuda, está desactualizado, erróneo o incompleto? ")),sizers->Center);
	forum_sizer->Add(button=new wxButton(this,mxID_HELPW_FORUM,LANG(HELPW_FORUM_BUTTON,"accede al Foro..."),wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT),sizers->Center);
	button->Connect(wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(mxHelpWindow::OnForum),nullptr,this);
	general_sizer->Add(forum_sizer,sizers->Right);
	Layout();
}

mxHelpWindow::~mxHelpWindow() { instance=nullptr; }

void mxHelpWindow::OnSearch(wxString value) {
	wxArrayString aresults, keywords;
	mxUT::Split(value.MakeUpper(),keywords,true,false);
	unsigned int kc=keywords.GetCount();
	if (kc==0) {
		mxMessageDialog(this,LANG(HELPW_SEARCH_ERROR_EMPTY,"Debe introducir al menos una palabra clave para buscar"),LANG(GENERAL_ERROR,"Error"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	unsigned char *bfound = new unsigned char[keywords.GetCount()];
	html->SetPage(wxString("<HTML><HEAD></HEAD><BODY><I><B>")<<LANG(HELPW_SEARCH_SEARCHING,"Buscando...")<<"</B></I></BODY></HTML>");
	HashStringTreeItem::iterator it = items.begin(), ed = items.end();
	wxString result(wxString("<HTML><HEAD></HEAD><BODY><I><B>")<<LANG(HELPW_SEARCH_RESULTS,"Resultados:")<<"</B></I><UL>");
	int count=0;
	wxArrayString searched;
	while (it!=ed) {
		wxString fname = it->first;
		if (fname.Find('#')!=wxNOT_FOUND)
			fname = fname.BeforeFirst('#');
		bool already=false;
		fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,fname));
		if (!already && fname.Len()) {
			for (unsigned int i=0;i<searched.GetCount();i++)
				if (searched[i]==fname) {
					already=true;
					break;
				}
			memset(bfound,0,kc);
			wxTextFile fil(fname);
			fil.Open();
			unsigned int fc=0;
			for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
				for (unsigned int ik=0;ik<kc;ik++) {
					if (bfound[ik]==0 && str.MakeUpper().Contains(keywords[ik])) {
						fc++; bfound[ik]=1;
					}
				}
				if (fc==kc) {
					count++;
					aresults.Add(wxString("<!--")<<tree->GetItemText(it->second)<<"--><LI><A href=\""<<it->first<<"\">"<<tree->GetItemText(it->second)<<"</A></LI>");
					break;
				}
			}
			fil.Close();
		}
		searched.Add(fname);
		++it;
	}
	aresults.Sort();
	for (unsigned int i=0;i<aresults.GetCount();i++)
		result<<aresults[i];
	result<<"</UL></BODY></HTML>";
	if (count)		
		html->SetPage(result);
	else
		html->SetPage(wxString("<HTML><HEAD></HEAD><BODY><B>")<<LANG(HELPW_SEARCH_NO_RESULTS_FOR,"No se encontraron coincidencias para \"")<<value<<"\".</B></BODY></HTML>");
	delete [] bfound;
}

void mxHelpWindow::ShowIndex() {
	LoadHelp(_index);
}

void mxHelpWindow::ShowHelp(wxString page, wxDialog *from_modal) {
	
	if (from_modal) {
		if (mxMD_OK==mxMessageDialog(from_modal,"Se cerrará este cuadro de diálogo (perdiendo los cambios) para poder acceder a la ventana de ayuda.",LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK_CANCEL|mxMD_WARNING).ShowModal())
			from_modal->EndModal(0);
		else
			return;
	}
	
	if (page=="") page=_index;
	if (instance) {
		if (!instance->IsShown()) instance->Show(); 
		else if (instance->IsIconized()) instance->Maximize(false); 
		else instance->Raise();
		instance->LoadHelp(page); 
	} else {
		instance=new mxHelpWindow(page);
	}
}


void mxHelpWindow::LoadHelp(wxString file) {
	if (file.Len()>5 && file.Mid(0,5)=="http:")
		mxUT::OpenInBrowser(file);
	else FixLoadPage(file);
	SelectTreeItem(file);
	Show();
	html->SetFocus();
	Raise();
}

bool mxHelpWindow::OnLink (wxString href) {
	if (href=="cppreference:") {
		Close(); mxReferenceWindow::ShowPage();
	} else if (href.StartsWith("foropen:")) {
		main_window->NewFileFromTemplate(DIR_PLUS_FILE(config->Help.guihelp_dir,href.AfterFirst(':')));
	} else if (href.StartsWith("http://")) {
		mxUT::OpenInBrowser(href);
	} else {
		wxString fname=(href+"#").BeforeFirst('#');
		if (fname.Len()) {
			SelectTreeItem(fname);
			FixLoadPage(href);
		} else
			return false;
	}
	return true;
}

void mxHelpWindow::OnTree (wxTreeItemId item) {
	if (ignore_tree_event) return;
	tree->Expand(item);
	HashStringTreeItem::iterator it = items.begin();
	while (it!=items.end() && it->second != item) ++it;
	if (it!=items.end()) FixLoadPage(it->first);
}

wxString mxHelpWindow::GetHelpFile(wxString file) {
	wxString post; 
	if (file.Contains("#")) post<<"#"<<file.BeforeFirst('#');
	if (file.EndsWith(".html")) file=file.Mid(0,file.Len()-5);
	if (wxFileName::FileExists(file+"_"+config->Init.language_file+".html"))
		return file+"_"+config->Init.language_file+".html"+post;
	else if (wxFileName::FileExists(file+"_spanish.html"))
		return file+"_spanish.html"+post;
	else if (wxFileName::FileExists(file+".html"))
		return file+".html"+post;
	else
		return "";
}

void mxHelpWindow::OnForum (wxCommandEvent & event) {
	mxUT::OpenInBrowser(LANG(HELPW_ADDRESS,"http://zinjai.sourceforge.net/index.php?page=contacto.php"));
}

void mxHelpWindow::FixLoadPage (const wxString &href) {
	if (href.Contains("#")) {
		wxString fname=href.BeforeFirst('#'), anchor=href.AfterFirst('#');
		fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,fname));
		if (!fname.Len()) html->SetPage(ERROR_PAGE(fname));
		else html->LoadPage(fname+"#"+anchor);
	} else {
		wxString fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,href));
		if (!fname.Len()) html->SetPage(ERROR_PAGE(fname));
		else html->LoadPage(fname);
	}
}

void mxHelpWindow::SelectTreeItem (const wxString &fname) {
	HashStringTreeItem::iterator it = items.find(fname);
	if (it==items.end()) {
		wxArrayTreeItemIds ta;
		if (tree->GetSelections(ta)) 
			tree->SelectItem(tree->GetSelection(),false);
		return;	
	}
	ignore_tree_event=true;
	tree->SelectItem(it->second);
	ignore_tree_event=false;
}


