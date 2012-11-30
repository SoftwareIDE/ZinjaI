#include <wx/bmpbuttn.h>
#include <wx/clipbrd.h>
#include <wx/sashwin.h>
#include <wx/laywin.h>
#include <wx/textfile.h>
#include <wx/treectrl.h>
#include <wx/html/htmprint.h>

#include "mxHelpWindow.h"

#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"

#define ERROR_PAGE(page) wxString(_T("<I>ERROR</I>: La pagina \""))<<page<<_T("\" no se encuentra. <br><br> La ayuda de <I>ZinjaI</I> aun esta en contruccion.")
#include "mxArt.h"

mxHelpWindow *helpw;

BEGIN_EVENT_TABLE(mxHelpWindow,wxFrame)
	EVT_CLOSE(mxHelpWindow::OnClose)
	EVT_BUTTON(mxID_HELPW_HIDETREE, mxHelpWindow::OnHideTree)
	EVT_BUTTON(mxID_HELPW_HOME, mxHelpWindow::OnHome)
	EVT_BUTTON(mxID_HELPW_PREV, mxHelpWindow::OnPrev)
	EVT_BUTTON(mxID_HELPW_NEXT, mxHelpWindow::OnNext)
	EVT_BUTTON(mxID_HELPW_COPY, mxHelpWindow::OnCopy)
	EVT_BUTTON(mxID_HELPW_SEARCH, mxHelpWindow::OnSearch)
	EVT_BUTTON(mxID_HELPW_PRINT, mxHelpWindow::OnPrint)
	EVT_SASH_DRAGGED(wxID_ANY, mxHelpWindow::OnSashDrag)
	EVT_TREE_SEL_CHANGED(wxID_ANY, mxHelpWindow::OnTree)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxHelpWindow::OnTree)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxHelpWindow::OnLink)
	EVT_CHAR_HOOK(mxHelpWindow::OnCharHook)
END_EVENT_TABLE();

mxHelpWindow::mxHelpWindow(wxString file):wxFrame (NULL,mxID_HELPW, _T("Ayuda de ZinjaI"), wxDefaultPosition, wxSize(750,550),wxDEFAULT_FRAME_STYLE) {

	printer=NULL;
	
	wxPanel *panel= new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	index_sash = new wxSashLayoutWindow(this, wxID_ANY,
		wxDefaultPosition, wxSize(200, 30),
		wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
	index_sash->SetDefaultSize(wxSize(120, 1000));
	index_sash->SetOrientation(wxLAYOUT_VERTICAL);
	index_sash->SetAlignment(wxLAYOUT_LEFT);
	index_sash->SetSashVisible(wxSASH_RIGHT, true);
	
	// create and fill index tree
	tree = new wxTreeCtrl(index_sash, wxID_ANY, wxPoint(0,0), wxSize(10,250), wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);
	
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
	
	bottomSizer->Add(index_sash,sizers->Exp0);
	
	wxBitmapButton *button_hide = new wxBitmapButton(panel, mxID_HELPW_HIDETREE, wxBitmap(SKIN_FILE(_T("ayuda_tree.png")),wxBITMAP_TYPE_PNG));
	button_hide->SetToolTip(_T("Mostrar/Ocultar Indice"));
	wxBitmapButton *button_home = new wxBitmapButton(panel, mxID_HELPW_HOME, wxBitmap(SKIN_FILE(_T("ayuda_indice.png")),wxBITMAP_TYPE_PNG));
	button_home->SetToolTip(_T("Ir al indice"));
	wxBitmapButton *button_prev = new wxBitmapButton(panel, mxID_HELPW_PREV, wxBitmap(SKIN_FILE(_T("ayuda_anterior.png")),wxBITMAP_TYPE_PNG));
	button_prev->SetToolTip(_T("Ir a la pagina anterior"));
	wxBitmapButton *button_next = new wxBitmapButton(panel, mxID_HELPW_NEXT, wxBitmap(SKIN_FILE(_T("ayuda_siguiente.png")),wxBITMAP_TYPE_PNG));
	button_next->SetToolTip(_T("Ir a la pagina siguiente"));
	wxBitmapButton *button_copy = new wxBitmapButton(panel, mxID_HELPW_COPY, wxBitmap(SKIN_FILE(_T("ayuda_copiar.png")),wxBITMAP_TYPE_PNG));
	button_copy->SetToolTip(_T("Copiar seleccion"));
	wxBitmapButton *button_print = new wxBitmapButton(panel, mxID_HELPW_PRINT, wxBitmap(SKIN_FILE(_T("ayuda_imprimir.png")),wxBITMAP_TYPE_PNG));
	button_print->SetToolTip(_T("Imprimir pagina actual"));
	topSizer->Add(button_hide,sizers->BA2);
	topSizer->Add(button_home,sizers->BA2);
	topSizer->Add(button_prev,sizers->BA2);
	topSizer->Add(button_next,sizers->BA2);
	topSizer->Add(button_copy,sizers->BA2);
	topSizer->Add(button_print,sizers->BA2);
	search_text = new wxTextCtrl(panel,wxID_ANY);
	search_text->SetToolTip(_T("Palabras a buscar"));
	topSizer->Add(search_text,sizers->BA2_Exp1);

	wxBitmapButton *search_button = new wxBitmapButton(panel, mxID_HELPW_SEARCH, wxBitmap(SKIN_FILE(_T("ayuda_buscar.png")),wxBITMAP_TYPE_PNG));
	search_button->SetToolTip(_T("Buscar..."));
	topSizer->Add(search_button,sizers->BA2);
	panel->SetSizer(topSizer);
	html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400,300));
	bottomSizer->Add(html,sizers->Exp1);
	sizer->Add(panel,sizers->Exp0);
	sizer->Add(bottomSizer,sizers->Exp1);
	SetSizer(sizer);
	bottomSizer->SetItemMinSize(index_sash,200, 10);
	bottomSizer->Layout();
//	search_button->SetDefault();
	if (!file.Len()) file="index";
	ShowHelp(file);
}

void mxHelpWindow::ShowIndex() {
	wxArrayTreeItemIds ta;
	if (tree->GetSelections(ta))
		tree->SelectItem(tree->GetSelection(),false);
	ShowHelp("index");
}

void mxHelpWindow::ShowHelp(wxString file) {
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

void mxHelpWindow::OnClose(wxCloseEvent &evt) {
	Hide();
}

mxHelpWindow::~mxHelpWindow() {
	helpw=NULL;
}

void mxHelpWindow::OnHideTree(wxCommandEvent &event) {
	if (bottomSizer->GetItem(index_sash)->GetMinSize().GetWidth()<10)
		bottomSizer->SetItemMinSize(index_sash,200, 10);	
	else
		bottomSizer->SetItemMinSize(index_sash,0, 10);
	bottomSizer->Layout();
}

void mxHelpWindow::OnHome(wxCommandEvent &event) {
	ShowHelp("index");
}

void mxHelpWindow::OnPrev(wxCommandEvent &event) {
	html->HistoryBack();
}

void mxHelpWindow::OnNext(wxCommandEvent &event) {
	html->HistoryForward();
}

void mxHelpWindow::OnCopy(wxCommandEvent &event) {
	if (html->SelectionToText()==_T("")) 
		return;
	wxTheClipboard->Open();
	wxTheClipboard->SetData(new wxTextDataObject(html->SelectionToText()));
	wxTheClipboard->Close();
}

void mxHelpWindow::OnSearch(wxCommandEvent &event) {
	wxArrayString aresults, keywords;
	utils->Split(search_text->GetValue().MakeUpper(),keywords,true,false);
	unsigned int kc=keywords.GetCount();
	if (kc==0) {
		mxMessageDialog(this,_T("Debe introducir al menos una palabra clave para buscar"),_T("Error"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	unsigned char *bfound = new unsigned char[keywords.GetCount()];
	html->SetPage(_T("<HTML><HEAD></HEAD><BODY><I><B>Buscando...</B></I></BODY></HTML>"));
	HashStringTreeItem::iterator it = items.begin(), ed = items.end();
	wxString result(_T("<HTML><HEAD></HEAD><BODY><I><B>Resultados:</B></I><UL>"));
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
		html->SetPage(wxString(_T("<HTML><HEAD></HEAD><BODY><B>No se encontraron coincidencias para \""))<<search_text->GetValue()<<_T("\".</B></BODY></HTML>"));
	delete [] bfound;
}

void mxHelpWindow::OnSearchAll(wxCommandEvent &event) {
}

void mxHelpWindow::OnSashDrag(wxSashEvent& event) {
    //index_sash->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
	bottomSizer->SetItemMinSize(index_sash,event.GetDragRect().width<150?150:event.GetDragRect().width, 10);
	//GetClientWindow()->Refresh();
	bottomSizer->Layout();
}

void mxHelpWindow::OnTree(wxTreeEvent &event) {
	wxTreeItemId item = event.GetItem();
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
//	evt.Skip();
}

void mxHelpWindow::OnLink (wxHtmlLinkEvent &event) {
	if (event.GetLinkInfo().GetHref().StartsWith(_T("foropen:"))) {
		main_window->NewFileFromTemplate(DIR_PLUS_FILE(config->Help.guihelp_dir,event.GetLinkInfo().GetHref().AfterFirst(':')));
	} else if (event.GetLinkInfo().GetHref().StartsWith(_T("http://"))) {
		utils->OpenInBrowser(event.GetLinkInfo().GetHref());
	} else {
		wxString fname = event.GetLinkInfo().GetHref().BeforeFirst('#');
		if (fname.Len()) {
			HashStringTreeItem::iterator it = items.find(fname);
			if (it!=items.end())
				tree->SelectItem(it->second);
			fname=GetHelpFile(DIR_PLUS_FILE(config->Help.guihelp_dir,event.GetLinkInfo().GetHref()));
			if (!fname.Len())
				html->SetPage(ERROR_PAGE(event.GetLinkInfo().GetHref()));
			else {
				html->LoadPage(fname);
			}
		} else
			event.Skip();
	}
}

void mxHelpWindow::OnCharHook(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_RETURN && search_text==FindFocus()) {
		wxCommandEvent evt;
		OnSearch(evt);
	} else if (evt.GetKeyCode()==WXK_ESCAPE)
		Close();
	else
		evt.Skip();
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

void mxHelpWindow::OnPrint(wxCommandEvent &evt) {
	if (!printer) {
		printer = new wxHtmlEasyPrinting("ZinjaI's help",this);
		int sizes[]={6,8,10,12,14,16,18};
		printer->SetFonts("","",sizes);
	}
	printer->PrintFile(html->GetOpenedPage());
}
