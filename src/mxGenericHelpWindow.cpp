#include <wx/bmpbuttn.h>
#include <wx/clipbrd.h>
#include <wx/sashwin.h>
#include <wx/laywin.h>
#include <wx/treectrl.h>
#include <wx/html/htmprint.h>
#include <wx/settings.h>
#include "mxGenericHelpWindow.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxArt.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxGenericHelpWindow,wxFrame)
	EVT_CLOSE(mxGenericHelpWindow::OnClose)
	EVT_BUTTON(mxID_HELPW_HIDETREE, mxGenericHelpWindow::OnHideTree)
	EVT_BUTTON(mxID_HELPW_HOME, mxGenericHelpWindow::OnHome)
	EVT_BUTTON(mxID_HELPW_PREV, mxGenericHelpWindow::OnPrev)
	EVT_BUTTON(mxID_HELPW_NEXT, mxGenericHelpWindow::OnNext)
	EVT_BUTTON(mxID_HELPW_COPY, mxGenericHelpWindow::OnCopy)
	EVT_BUTTON(mxID_HELPW_SEARCH, mxGenericHelpWindow::OnSearch)
	EVT_BUTTON(mxID_HELPW_PRINT, mxGenericHelpWindow::OnPrint)
	EVT_SASH_DRAGGED(wxID_ANY, mxGenericHelpWindow::OnSashDrag)
	EVT_TREE_SEL_CHANGED(wxID_ANY, mxGenericHelpWindow::OnTree)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxGenericHelpWindow::OnTree)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxGenericHelpWindow::OnLink)
	EVT_CHAR_HOOK(mxGenericHelpWindow::OnCharHook)
	END_EVENT_TABLE();

mxGenericHelpWindow::mxGenericHelpWindow(bool use_tree):wxFrame (NULL,mxID_HELPW, LANG(HELPW_CAPTION,"Ayuda de ZinjaI"), wxDefaultPosition, wxSize(750,550),wxDEFAULT_FRAME_STYLE) {
	
#ifdef __WIN32__
	SetBackgroundColour(wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ));
#endif
	
	printer=NULL;
	
	wxPanel *panel= new wxPanel(this);
	wxBoxSizer *sizer = general_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer = new wxBoxSizer(wxHORIZONTAL);
		
	if (use_tree) {
		index_sash = new wxSashLayoutWindow(this, wxID_ANY,
			wxDefaultPosition, wxSize(200, 30),
			wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
		index_sash->SetDefaultSize(wxSize(120, 1000));
		index_sash->SetOrientation(wxLAYOUT_VERTICAL);
		index_sash->SetAlignment(wxLAYOUT_LEFT);
		index_sash->SetSashVisible(wxSASH_RIGHT, true);
		tree= new wxTreeCtrl(index_sash, wxID_ANY, wxPoint(0,0), wxSize(10,250),wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT);
		bottomSizer->Add(index_sash,sizers->Exp0);
		wxBitmapButton *button_hide = new wxBitmapButton(panel, mxID_HELPW_HIDETREE, wxBitmap(SKIN_FILE(_T("ayuda_tree.png")),wxBITMAP_TYPE_PNG));
		button_hide->SetToolTip(LANG(HELPW_TOGGLE_TREE,"Mostrar/Ocultar Indice"));
		topSizer->Add(button_hide,sizers->BA2);
	} else tree=NULL;
	
	wxBitmapButton *button_home = new wxBitmapButton(panel, mxID_HELPW_HOME, wxBitmap(SKIN_FILE(_T("ayuda_indice.png")),wxBITMAP_TYPE_PNG));
	button_home->SetToolTip(LANG(HELPW_INDEX,"Ir a la pagina de incio"));
	wxBitmapButton *button_prev = new wxBitmapButton(panel, mxID_HELPW_PREV, wxBitmap(SKIN_FILE(_T("ayuda_anterior.png")),wxBITMAP_TYPE_PNG));
	button_prev->SetToolTip(LANG(HELPW_PREVIOUS,"Ir a la pagina anterior"));
	wxBitmapButton *button_next = new wxBitmapButton(panel, mxID_HELPW_NEXT, wxBitmap(SKIN_FILE(_T("ayuda_siguiente.png")),wxBITMAP_TYPE_PNG));
	button_next->SetToolTip(LANG(HELPW_NEXT,"Ir a la pagina siguiente"));
	wxBitmapButton *button_copy = new wxBitmapButton(panel, mxID_HELPW_COPY, wxBitmap(SKIN_FILE(_T("ayuda_copiar.png")),wxBITMAP_TYPE_PNG));
	button_copy->SetToolTip(LANG(HELPW_COPY,"Copiar seleccion al portapapeles"));
	wxBitmapButton *button_print = new wxBitmapButton(panel, mxID_HELPW_PRINT, wxBitmap(SKIN_FILE(_T("ayuda_imprimir.png")),wxBITMAP_TYPE_PNG));
	button_print->SetToolTip(LANG(HELPW_PRINT,"Imprimir pagina actual"));
	topSizer->Add(button_home,sizers->BA2);
	topSizer->Add(button_prev,sizers->BA2);
	topSizer->Add(button_next,sizers->BA2);
	topSizer->Add(button_copy,sizers->BA2);
	topSizer->Add(button_print,sizers->BA2);
	search_text = new wxTextCtrl(panel,wxID_ANY);
	search_text->SetToolTip(LANG(HELPW_SEARCH_LABEL,"Palabras a buscar"));
	topSizer->Add(search_text,sizers->BA2_Exp1);
	
	wxBitmapButton *search_button = new wxBitmapButton(panel, mxID_HELPW_SEARCH, wxBitmap(SKIN_FILE(_T("ayuda_buscar.png")),wxBITMAP_TYPE_PNG));
	search_button->SetToolTip(LANG(HELPW_FIND,"Buscar..."));
	topSizer->Add(search_button,sizers->BA2);
	panel->SetSizer(topSizer);
	html = new mxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(200,100));
	bottomSizer->Add(html,sizers->Exp1);
	sizer->Add(panel,sizers->Exp0);
	sizer->Add(bottomSizer,sizers->Exp1);
	
	SetSizer(sizer);
	bottomSizer->SetItemMinSize(index_sash,250, 10);
	bottomSizer->Layout();
//	search_button->SetDefault();
}

void mxGenericHelpWindow::OnClose(wxCloseEvent &evt) {
	Hide();
}

mxGenericHelpWindow::~mxGenericHelpWindow() {
}

void mxGenericHelpWindow::OnHideTree(wxCommandEvent &event) {
	if (bottomSizer->GetItem(index_sash)->GetMinSize().GetWidth()<10)
		bottomSizer->SetItemMinSize(index_sash,200, 10);	
	else
		bottomSizer->SetItemMinSize(index_sash,0, 10);
	bottomSizer->Layout();
}

void mxGenericHelpWindow::OnHome(wxCommandEvent &event) {
	ShowIndex();
}

void mxGenericHelpWindow::OnPrev(wxCommandEvent &event) {
	OnPrev();
}

void mxGenericHelpWindow::OnNext(wxCommandEvent &event) {
	OnNext();
}

void mxGenericHelpWindow::OnCopy(wxCommandEvent &event) {
	if (html->SelectionToText()==_T("")) 
		return;
	wxTheClipboard->Open();
	wxTheClipboard->SetData(new wxTextDataObject(html->SelectionToText()));
	wxTheClipboard->Close();
}

void mxGenericHelpWindow::OnSashDrag(wxSashEvent& event) {
	//index_sash->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
	bottomSizer->SetItemMinSize(index_sash,event.GetDragRect().width<150?150:event.GetDragRect().width, 10);
	//GetClientWindow()->Refresh();
	bottomSizer->Layout();
}

void mxGenericHelpWindow::OnTree(wxTreeEvent &event) {
	OnTree(event.GetItem());
}

void mxGenericHelpWindow::OnLink (wxHtmlLinkEvent &event) {
	if (!OnLink(event.GetLinkInfo().GetHref())) event.Skip();
}

void mxGenericHelpWindow::OnCharHook(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_RETURN && search_text==FindFocus()) {
		wxCommandEvent evt;
		OnSearch(evt);
	} else if (evt.GetKeyCode()==WXK_ESCAPE)
		Close();
	else
		evt.Skip();
}



void mxGenericHelpWindow::OnPrint(wxCommandEvent &evt) {
	if (!printer) {
		printer = new wxHtmlEasyPrinting("ZinjaI's help",this);
		int sizes[]={6,8,10,12,14,16,18};
		printer->SetFonts("","",sizes);
	}
	printer->PrintFile(html->GetOpenedPage());
}

void mxGenericHelpWindow::OnSearch (wxCommandEvent & event) {
	OnSearch(search_text->GetValue());
}

