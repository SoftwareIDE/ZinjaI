
#include "mxHelpWindow.h"
#include "ConfigManager.h"
#include <wx/treectrl.h>
#include "mxMessageDialog.h"
#include "mxMainWindow.h"

#define ERROR_PAGE(page) wxString(_T("<I>ERROR</I>: La pagina \""))<<page<<_T("\" no se encuentra. <br><br> La ayuda de <I>ZinjaI</I> aun esta en contruccion.")
#define _index "index"

mxHelpWindow *mxHelpWindow::instance=NULL;

mxHelpWindow::mxHelpWindow(wxString file) : mxGenericHelpWindow(true) { 
	// populate index tree
	wxString index_file=DIR_PLUS_FILE(config->Help.guihelp_dir,_T("index_")+config->Init.language_file);
	if (!wxFileName::FileExists(index_file))
		index_file=DIR_PLUS_FILE(config->Help.guihelp_dir,_T("index_spanish"));
	wxTextFile fil(index_file);
	if (fil.Exists()) {
		fil.Open();
		wxTreeItemId root = tree->AddRoot(_T("Temas de Ayuda"),0);
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
		tree->Expand(root);
		wxTreeItemIdValue cokkie;
		node = tree->GetFirstChild(root,cokkie);
		while (node.IsOk()) {
			tree->Expand(node);
			node = tree->GetNextSibling(node);
		}
	}
	
	if (!file.Len()) file="index"; LoadHelp(file);
}

mxHelpWindow::~mxHelpWindow() { instance=NULL; }

void mxHelpWindow::OnSearch(wxString value) {
	wxArrayString aresults, keywords;
	utils->Split(value.MakeUpper(),keywords,true,false);
	unsigned int kc=keywords.GetCount();
	if (kc==0) {
		mxMessageDialog(this,LANG(HELPW_FORUM_SEARCH_ERROR_EMPTY,"Debe introducir al menos una palabra clave para buscar"),LANG(GENERAL_ERROR,"Error"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	unsigned char *bfound = new unsigned char[keywords.GetCount()];
	html->SetPage(wxString("<HTML><HEAD></HEAD><BODY><I><B>")<<LANG(HELPW_FORUM_SEARCH_SEARCHING,"Buscando...")<<"</B></I></BODY></HTML>");
	HashStringTreeItem::iterator it = items.begin(), ed = items.end();
	wxString result(wxString("<HTML><HEAD></HEAD><BODY><I><B>")<<LANG(HELPW_FORUM_SEARCH_RESULTS,"Resultados:")<<"</B></I><UL>");
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
					aresults.Add(wxString(_T("<!--"))<<tree->GetItemText(it->second)<<_T("--><LI><A href=\"")<<it->first<<_T("\">")<<tree->GetItemText(it->second)<<_T("</A></LI>"));
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
	result<<_T("</UL></BODY></HTML>");
	if (count)		
		html->SetPage(result);
	else
		html->SetPage(wxString("<HTML><HEAD></HEAD><BODY><B>")<<LANG(HELPW_FORUM_SEARCH_NO_RESULTS_FOR,"No se encontraron coincidencias para \"")<<value<<_T("\".</B></BODY></HTML>"));
	delete [] bfound;
}

void mxHelpWindow::ShowIndex() {
	wxArrayTreeItemIds ta;
	if (tree->GetSelections(ta))
		tree->SelectItem(tree->GetSelection(),false);
	LoadHelp(_index);
}

void mxHelpWindow::ShowHelp(wxString page) {
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
	if (file.Len()>5 && file.Mid(0,5)==_T("http:"))
		html->LoadPage(file);
	else
		html->LoadPage(GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,file)));
	HashStringTreeItem::iterator it = items.find(file);
	if (it!=items.end())
		tree->SelectItem(it->second);
	Show();
	html->SetFocus();
	Raise();
}

bool mxHelpWindow::OnLink (wxString href) {
	if (href.StartsWith(_T("foropen:"))) {
		main_window->NewFileFromTemplate(DIR_PLUS_FILE(config->Help.guihelp_dir,href.AfterFirst(':')));
	} else if (href.StartsWith(_T("http://"))) {
		utils->OpenInBrowser(href);
	} else {
		wxString fname = href.BeforeFirst('#');
		if (fname.Len()) {
			HashStringTreeItem::iterator it = items.find(fname);
			if (it!=items.end())
				tree->SelectItem(it->second);
			fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,href));
			if (!fname.Len())
				html->SetPage(ERROR_PAGE(href));
			else {
				html->LoadPage(fname);
			}
		} else
			return false;
	}
	return true;
}

void mxHelpWindow::OnTree (wxTreeItemId item) {
	tree->Expand(item);
	HashStringTreeItem::iterator it = items.begin();
	while (it!=items.end() && it->second != item) ++it;
	if (it==items.end()) return;
	wxString fname=it->first,anchor; if (fname.Contains("#")) { anchor=fname.AfterFirst('#'); fname=fname.BeforeFirst('#'); }
	fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,fname));
	if (fname.Len())
		html->LoadPage(fname+(anchor.Len()?wxString("#")+anchor:""));
	else
		html->SetPage(ERROR_PAGE(it->first));
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
