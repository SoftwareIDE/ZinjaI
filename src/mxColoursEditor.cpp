#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/colordlg.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/combobox.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/bitmap.h>
#include "mxColoursEditor.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "ids.h"
#include "mxSizers.h"
#include "ConfigManager.h"
#include "mxSource.h"

color_theme *ctheme=NULL;

BEGIN_EVENT_TABLE(mxColoursEditor,wxDialog)
	EVT_BUTTON(wxID_SAVE,mxColoursEditor::OnSave)
	EVT_BUTTON(wxID_OPEN,mxColoursEditor::OnOpen)
	EVT_BUTTON(wxID_OK,mxColoursEditor::OnButtonOk)
	EVT_BUTTON(wxID_FIND,mxColoursEditor::OnButtonApply)
	EVT_BUTTON(wxID_CANCEL,mxColoursEditor::OnButtonCancel)
	EVT_BUTTON(mxID_COLORS_PICKER,mxColoursEditor::OnButtonPicker)
	EVT_CHECKBOX(wxID_ANY,mxColoursEditor::OnCheck)
	EVT_TEXT(wxID_ANY,mxColoursEditor::OnText)
	EVT_CLOSE(mxColoursEditor::OnClose)
	EVT_COMBOBOX(wxID_ANY,mxColoursEditor::OnCombo)
END_EVENT_TABLE()


mxColoursEditor::mxColoursEditor(wxWindow *aparent):wxDialog(main_window,wxID_ANY,LANG(COLORS_CAPTION,"Definir Colores"),wxDefaultPosition,wxSize(700,400),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	old_theme=custom_theme=*ctheme;
	parent=aparent; setting=true;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	wxButton *apply_button = new mxBitmapButton (this, wxID_FIND, bitmaps->buttons.next, LANG(GENERAL_APPLY_BUTTON,"A&plicar"));
	ok_button->SetMinSize(wxSize(ok_button->GetSize().GetWidth()<80?80:ok_button->GetSize().GetWidth(),ok_button->GetSize().GetHeight()));
	apply_button->SetDefault(); 
	

	wxArrayString color_profiles;
	utils->GetFilesFromBothDirs(color_profiles,"colours",true,"<personalizado>");
	bottomSizer->Add(new wxStaticText(this,wxID_ANY,LANG(COLOURS_ESCHEME,"Esquema:")),sizers->BA5_Center);
	combo=new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,color_profiles,wxCB_READONLY);
	bottomSizer->Add(combo,sizers->BA5_Center);
	
	
	int sel=color_profiles.Index(config->Init.colour_theme);
	if (sel==wxNOT_FOUND) sel=color_profiles.GetCount()-1;
	combo->SetSelection(sel);
	
	wxBitmapButton *open_button = new wxBitmapButton(this,wxID_OPEN,wxBitmap(SKIN_FILE(DIR_PLUS_FILE("16","abrir.png")),wxBITMAP_TYPE_PNG));
	wxBitmapButton *save_button=new wxBitmapButton(this,wxID_SAVE,wxBitmap(SKIN_FILE(DIR_PLUS_FILE("16","guardar.png")),wxBITMAP_TYPE_PNG));
	open_button->SetToolTip(LANG(COLOURS_TIP_OPEN,"Cargar un esquema de colores personalizado desde un archivo .zcs"));
	save_button->SetToolTip(LANG(COLOURS_TIP_SAVE,"Guardar un esquema de colores personalizado en un archivo .zcs"));
	bottomSizer->Add(open_button,sizers->BA5_Center);
	bottomSizer->Add(save_button,sizers->BA5_Center);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(apply_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	scroll = new wxScrolledWindow(this,wxID_ANY);
	sizer = new wxFlexGridSizer(5);
	sizer->SetFlexibleDirection(wxBOTH);
	sizer->Add(new wxStaticText(scroll,wxID_ANY,""));
	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_FRONT,"Color de Frente")),sizers->BA5_Center);
	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_BACK,"Color de Fondo")),sizers->BA5_Center);
	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_ITALIC,"Cursiva")),sizers->BA5_Center);
	sizer->Add(new wxStaticText(scroll,wxID_ANY,LANG(COLOURS_BOLD,"Negrita")),sizers->BA5_Center);
	scroll->SetScrollRate(10,10);
	scroll->SetSizer(sizer);
	sizer->AddGrowableCol(0);
	
	LoadList();
	
	wxStaticText *helpt=new wxStaticText(this,wxID_ANY,LANG(COLORS_HELP,""
		"Puede ingresar los colores con formato HTML o utilizando el boton de "
		"los tres punto para inciar un selector grafico. El formato html es \"#ABCDEF\" "
		"donde AB, CD y EF son los valores en notación hexadecimal para R, G y B."));
	mySizer->Add(helpt,sizers->BA5_Exp0);
	
	mySizer->Add(scroll,sizers->BA5_Exp1);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizer(mySizer);
	setting=false;
	Show();
}

void mxColoursEditor::Add(wxString name, wxColour *fore, wxColour *back, bool *italic, bool *bold) {
	
	lvalfor[lcount]=fore;
	lvalbak[lcount]=back;
	lvalita[lcount]=italic;
	lvalbol[lcount]=bold;

	sizer->Add(llabel[lcount] = new mxStaticText(scroll,name),sizers->BA5_Exp0);
	llabel[lcount]->SetToolTip(name);
	
	ltfore[lcount]=ltback[lcount]=NULL;
	
	if (fore) {
		wxBoxSizer *btsizer = new wxBoxSizer(wxHORIZONTAL);
		ltfore[lcount]=new wxTextCtrl(scroll,wxID_ANY,"");
		btsizer->Add(ltfore[lcount],sizers->BA5);
		ltfore[lcount]->SetValue(fore->GetAsString(wxC2S_HTML_SYNTAX));
		lbfore[lcount] = new wxButton(scroll,mxID_COLORS_PICKER,"...",wxDefaultPosition,wxSize(30,10));
		btsizer->Add(lbfore[lcount],sizers->BA5_Exp0);
		sizer->Add(btsizer,sizers->Exp0);
	} else {
		sizer->Add(new wxStaticText(scroll,wxID_ANY,""));
	}
	
	if (back) {
		wxBoxSizer *btsizer2 = new wxBoxSizer(wxHORIZONTAL);
		ltback[lcount]=new wxTextCtrl(scroll,wxID_ANY,"");
		btsizer2->Add(ltback[lcount],sizers->BA5);
		ltback[lcount]->SetValue(back->GetAsString(wxC2S_HTML_SYNTAX));
		lbback[lcount] = new wxButton(scroll,mxID_COLORS_PICKER,"...",wxDefaultPosition,wxSize(30,10));
		btsizer2->Add(lbback[lcount],sizers->BA5_Exp0);
		sizer->Add(btsizer2,sizers->Exp0);
	} else {
		sizer->Add(new wxStaticText(scroll,wxID_ANY,""));
	}
	
	if (bold) {
		lcur[lcount] = new wxCheckBox(scroll,wxID_ANY,"  ");
		lcur[lcount]->SetValue(*italic);
		lbold[lcount] = new wxCheckBox(scroll,wxID_ANY,"  ");
		lbold[lcount]->SetValue(*bold);
		sizer->Add(lcur[lcount],sizers->BA5_Right);
		sizer->Add(lbold[lcount],sizers->BA5_Right);
	} else {
		sizer->Add(new wxStaticText(scroll,wxID_ANY,""));
		sizer->Add(new wxStaticText(scroll,wxID_ANY,""));
	}
	
	llabel[lcount]->SetData(
		fore?fore:&(ctheme->DEFAULT_FORE),
		back?back:&(ctheme->DEFAULT_BACK),
		italic,bold);
	
	lcount++;
}


mxColoursEditor::~mxColoursEditor() {
}

#define MXCAdd(id,text) Add(text,&(ctheme->id##_FORE),&(ctheme->id##_BACK),&(ctheme->id##_ITALIC),&(ctheme->id##_BOLD));
#define MXCAdd0(id,text) Add(text,&(ctheme->id##_FORE),&(ctheme->id##_BACK),NULL,NULL);
#define MXCAdd1(id,text) Add(text,&(ctheme->id),NULL,NULL,NULL);
#define MXCAdd2(id,text) Add(text,NULL,&(ctheme->id),NULL,NULL);

void mxColoursEditor::LoadList ( ) {
	lcount=0;
	MXCAdd(DEFAULT,LANG(COLORS_ID_DEFAULT,"Texto por defecto"));
	MXCAdd(IDENTIFIER,LANG(COLORS_ID_IDENTIFIER,"Identificadores"));
	MXCAdd(GLOBALCLASS,LANG(COLORS_ID_USER_KEYWORD,"Identificador resaltado"));	
	MXCAdd(NUMBER,LANG(COLORS_ID_NUMBER,"Numero"));
	MXCAdd(WORD,LANG(COLORS_ID_WORD,"Palabra clave"));
	MXCAdd(WORD2,LANG(COLORS_ID_WORD2,"Tipo de dato funtamental"));
	MXCAdd(STRING,LANG(COLORS_ID_STRING,"Cadena de caracteres"));
	MXCAdd(STRINGEOL,LANG(COLORS_ID_STRINGEOL,"Cadena sin finalizar"));
	MXCAdd(CHARACTER,LANG(COLORS_ID_CHARACTER,"Caracter"));
	MXCAdd(OPERATOR,LANG(COLORS_ID_OPERATOR,"Operador"));
	MXCAdd(BRACELIGHT,LANG(COLORS_ID_BRACELIGHT,"Llave/parentesis/corchete resaltado"));
	MXCAdd(BRACEBAD,LANG(COLORS_ID_BRACEBAD,"Llave/parentesis/corchete incorrecto"));
	MXCAdd(PREPROCESSOR,LANG(COLORS_ID_PREPROCESSOR,"Directiva de preprocesador"));
	MXCAdd(COMMENT,LANG(COLORS_ID_COMMENT,"Comentario /* */"));
	MXCAdd(COMMENTLINE,LANG(COLORS_ID_COMMENTLINE,"Comentario //"));
	MXCAdd(COMMENTDOC,LANG(COLORS_ID_COMMENTDOC,"Comentario Doxygen /** **/"));
	MXCAdd(COMMENTLINEDOC,LANG(COLORS_ID_COMMENTLINEDOC,"Comentario Doxygen ///"));
	MXCAdd(COMMENTDOCKEYWORD,LANG(COLORS_ID_COMMENTDOCKEYWORD,"Palabra clave Doxygen"));
	MXCAdd(COMMENTDOCKEYWORDERROR,LANG(COLORS_ID_COMMENTDOCKEYWORDERROR,"Palabra clave Doxygen incorrecta"));
	
	MXCAdd0(CALLTIP,LANG(COLORS_ID_CALLTIP,"Ayuda emergente"));
	MXCAdd0(LINENUMBER,LANG(COLORS_ID_LINENUMBER,"Numero de linea"));
	MXCAdd0(FOLD,LANG(COLORS_ID_FOLD,"Indicadores de plegado"));
	MXCAdd0(FOLD_TRAMA,LANG(COLORS_ID_FOLD_TRAMA,"Margen para indicadores de plegado"));
	MXCAdd1(INDENTGUIDE,LANG(COLORS_ID_INDENTGUIDE,"Guia de indentado"));
	
	MXCAdd2(SELBACKGROUND,LANG(COLORS_ID_SELBACKGROUND,"Seleccion"));
	MXCAdd2(CURRENT_LINE,LANG(COLORS_ID_CURRENT_LINE,"Linea actual"));
	MXCAdd2(USER_LINE,LANG(COLORS_ID_USER_LINE,"Linea resaltada"));	
	MXCAdd1(CARET,LANG(COLORS_ID_CARET,"Cursor de texto"));
}

void mxColoursEditor::LoadTheme (wxString name) {
	
}

void mxColoursEditor::SaveTheme (wxString name) {
	
}

void mxColoursEditor::OnClose (wxCloseEvent & evt) {
	Destroy();
	if (parent!=main_window) parent->Show();
}

void mxColoursEditor::OnButtonOk (wxCommandEvent & evt) {
	if (combo->GetSelection()==int(combo->GetCount())-1) {
		ctheme->Save(DIR_PLUS_FILE(config->home_dir,"colours.zcs"));
		config->Init.colour_theme="";
	} else {
		config->Init.colour_theme=combo->GetString(combo->GetSelection());
	}
	OnButtonApply(evt); 
	Close();
}

void mxColoursEditor::OnButtonCancel (wxCommandEvent & evt) {
	(*ctheme)=old_theme;
	OnButtonApply(evt);
	Close();
}

void mxColoursEditor::OnButtonPicker (wxCommandEvent & evt) {
	wxObject *b=evt.GetEventObject();
	for (int i=0;i<lcount;i++) {
		if (lbfore[i]==b) {
			wxColour c=wxGetColourFromUser(this,wxColour(ltfore[i]->GetValue()),llabel[i]->GetLabel());
			if (c.IsOk()) ltfore[i]->SetValue(c.GetAsString(wxC2S_HTML_SYNTAX));
		}
		if (lbback[i]==b) {
			wxColour c=wxGetColourFromUser(this,wxColour(ltback[i]->GetValue()),llabel[i]->GetLabel());
			if (c.IsOk()) ltback[i]->SetValue(c.GetAsString(wxC2S_HTML_SYNTAX));
		}
	}
}

void mxColoursEditor::OnButtonApply (wxCommandEvent & evt) {
	for (unsigned int i=0;i<main_window->notebook_sources->GetPageCount();i++)
		((mxSource*)(main_window->notebook_sources->GetPage(i)))->SetColours();
}

color_theme::color_theme (wxString file) {
	SetDefaults();
	if (file.Len()) Load(file);
}

#define ctSet0(id,f,b) id##_FORE=f; id##_BACK=b;
#define ctSet(id,f,b) id##_FORE=f; id##_BACK=b; id##_BOLD=false; id##_ITALIC=false;
#define ctSetI(id,f,b) id##_FORE=f; id##_BACK=b; id##_BOLD=false; id##_ITALIC=true;
#define ctSetB(id,f,b) id##_FORE=f; id##_BACK=b; id##_BOLD=true; id##_ITALIC=false;

void color_theme::SetDefaults (bool inverted) {
	if (inverted) {
		
		ctSet(DEFAULT,"WHITE","BLACK"); // default
		ctSetI(COMMENT,"Z DARK GRAY","BLACK"); // comment
		ctSetI(COMMENTLINE,"Z DARK GRAY","BLACK"); // comment line
		ctSetI(COMMENTDOC,"Z ALMOST BLUE","BLACK"); // comment doc
		ctSetI(COMMENTLINEDOC,"Z ALMOST BLUE","BLACK"); // special comment 
		ctSet(COMMENTDOCKEYWORD,"Z CORNFLOWER BLUE","BLACK"); // doxy keywords
		ctSet(COMMENTDOCKEYWORDERROR,"RED","BLACK"); // keywords errors
		ctSet(NUMBER,"YELLOW GREEN","BLACK"); // number
		ctSetB(WORD,"Z CORNFLOWER BLUE","BLACK"); // keywords
		ctSet(STRING,"Z ALMOST RED","BLACK"); // string
		ctSet(CHARACTER,"MAGENTA","BLACK"); // character
		ctSet(PREPROCESSOR,"Z ALMOST GREEN","BLACK"); // preprocessor
		ctSetB(OPERATOR,"WHITE","BLACK"); // operator 
		ctSet(IDENTIFIER,"WHITE","BLACK"); // identifier 
		ctSet(STRINGEOL,"RED","Z REALLY DARKER GRAY"); // string eol
		ctSet(WORD2,"Z CORNFLOWER BLUE","BLACK"); // extra words
		
		ctSetB(BRACELIGHT,"RED","Z REALLY DARK BLUE"); 
		ctSetB(BRACEBAD,"Z DARK RED","BLACK"); 
		
		ctSet0(CALLTIP,"Z LIGHT YELLOW","Z REALLY DARKER GRAY");
		ctSet0(FOLD,"WHITE","BLACK");
		ctSet0(FOLD_TRAMA,"DARK SLATE GREY","BLACK");
		ctSet0(LINENUMBER,"LIGHT GRAY","BLACK");
		INDENTGUIDE="Z DARK GRAY";
		
		SELBACKGROUND="Z REALLY DARKER GRAY";
		CURRENT_LINE=wxColour(0x80,0x80,0xFF);
		USER_LINE="Z REALLY DARK RED";
		ctSet(GLOBALCLASS,"WHITE","Z DARK GREEN"); // string eol
		CARET="WHITE";
		
	} else {
		
		ctSet(DEFAULT,"BLACK","WHITE"); // default
		ctSetI(COMMENT,"Z DARK GRAY","WHITE"); // comment
		ctSetI(COMMENTLINE,"Z DARK GRAY","WHITE"); // comment line
		ctSetI(COMMENTDOC,"Z DOXY BLUE","WHITE"); // comment doc
		ctSet(NUMBER,"SIENNA","WHITE"); // number
		ctSetB(WORD,"Z DARK BLUE","WHITE"); // keywords
		ctSet(STRING,"RED","WHITE"); // string
		ctSet(CHARACTER,"MAGENTA","WHITE"); // character
		ctSet(PREPROCESSOR,"FOREST GREEN","WHITE"); // preprocessor
		ctSetB(OPERATOR,"BLACK","WHITE"); // operator 
		ctSet(IDENTIFIER,"BLACK","WHITE"); // identifier 
		ctSet(STRINGEOL,"RED","LIGHT GRAY"); // string eol
		ctSetI(COMMENTLINEDOC,"Z DOXY BLUE","WHITE"); // special comment 
		ctSet(WORD2,"Z DARK BLUE","WHITE"); // extra words
		ctSet(COMMENTDOCKEYWORD,"Z CORNFLOWER BLUE","WHITE"); // doxy keywords
		ctSet(COMMENTDOCKEYWORDERROR,"RED","WHITE"); // doxy keywords errors
		
		ctSetB(BRACELIGHT,"RED","Z LIGHT BLUE"); 
		ctSetB(BRACEBAD,"Z DARK RED","WHITE"); 
		
		ctSet0(CALLTIP,"Z DARKER GRAY","Z LIGHT YELLOW");
		ctSet0(FOLD,"BLACK","WHITE");
		ctSet0(FOLD_TRAMA,"LIGHT GRAY","WHITE");
		ctSet0(LINENUMBER,"Z DARK GRAY","WHITE");
		INDENTGUIDE="Z DARK GRAY";
		
		SELBACKGROUND="Z LIGHT GRAY";
		CURRENT_LINE="BLUE";
		USER_LINE="Z LIGHT RED";
		ctSet(GLOBALCLASS,"BLACK","Z LIGHT GREEN"); // identifier 
		CARET="BLACK";
	}
}


#define CTWrite(what)\
	fil.AddLine(wxString(#what"_FORE=")<<what##_FORE.GetAsString(wxC2S_HTML_SYNTAX)); \
	fil.AddLine(wxString(#what"_BACK=")<<what##_BACK.GetAsString(wxC2S_HTML_SYNTAX)); \
	fil.AddLine(wxString(#what"_BOLD=")<<(what##_BOLD?_T("1"):_T("0"))); \
	fil.AddLine(wxString(#what"_ITALIC=")<<(what##_ITALIC?_T("1"):_T("0")));
#define CTWrite0(what)\
	fil.AddLine(wxString(#what"_FORE=")<<what##_FORE.GetAsString(wxC2S_HTML_SYNTAX)); \
	fil.AddLine(wxString(#what"_BACK=")<<what##_BACK.GetAsString(wxC2S_HTML_SYNTAX));
#define CTWrite1(what) fil.AddLine(wxString(#what"=")<<what.GetAsString(wxC2S_HTML_SYNTAX))
#define CTWrite2(what) fil.AddLine(wxString(#what"=")<<what.GetAsString(wxC2S_HTML_SYNTAX))

bool color_theme::Save (wxString fname) {
	
	wxTextFile fil(fname);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	CTWrite(DEFAULT);
	CTWrite(IDENTIFIER);
	CTWrite(NUMBER);
	CTWrite(WORD);
	CTWrite(WORD2);
	CTWrite(STRING);
	CTWrite(STRINGEOL);
	CTWrite(CHARACTER);
	CTWrite(OPERATOR);
	CTWrite(BRACELIGHT);
	CTWrite(BRACEBAD);
	CTWrite(PREPROCESSOR);
	CTWrite(COMMENT);
	CTWrite(COMMENTLINE);
	CTWrite(COMMENTDOC);
	CTWrite(COMMENTLINEDOC);
	CTWrite(COMMENTDOCKEYWORD);
	CTWrite(COMMENTDOCKEYWORDERROR);
	
	CTWrite0(CALLTIP);
	CTWrite0(LINENUMBER);
	CTWrite0(FOLD);
	CTWrite0(FOLD_TRAMA);
	CTWrite1(INDENTGUIDE);
	
	CTWrite2(SELBACKGROUND);
	CTWrite2(CURRENT_LINE);
	CTWrite2(USER_LINE);
	CTWrite(GLOBALCLASS);
	CTWrite1(CARET);
	
	fil.Write();
	fil.Close();
	return true;
}

#define CTLoad(name)\
	else if (key==_T(#name"_FORE")) name##_FORE=wxColour(value);\
	else if (key==_T(#name"_BACK")) name##_BACK=wxColour(value);\
	else if (key==_T(#name"_BOLD")) name##_BOLD=utils->IsTrue(value);\
	else if (key==_T(#name"_ITALIC")) name##_ITALIC=utils->IsTrue(value)
#define CTLoad0(name)\
	else if (key==_T(#name"_FORE")) name##_FORE=wxColour(value);\
	else if (key==_T(#name"_BACK")) name##_BACK=wxColour(value)
#define CTLoad1(name) else if (key==_T(#name)) name=wxColour(value)
#define CTLoad2(name) else if (key==_T(#name)) name=wxColour(value)



bool color_theme::Load (wxString fname) {
	
	wxTextFile fil(fname);
	if (!fil.Exists()) return false;
	fil.Open();
	wxString section, key, value;
	wxArrayString last_files; // para compatibilidad hacia atras, guarda el historial unificado y despues lo divide
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		
		key=str.BeforeFirst('=');
		value=str.AfterFirst('=');
		
		if (str[0]=='#') continue;
		
		CTLoad(DEFAULT);
		CTLoad(IDENTIFIER);
		CTLoad(NUMBER);
		CTLoad(WORD);
		CTLoad(WORD2);
		CTLoad(STRING);
		CTLoad(STRINGEOL);
		CTLoad(CHARACTER);
		CTLoad(OPERATOR);
		CTLoad(BRACELIGHT);
		CTLoad(BRACEBAD);
		CTLoad(PREPROCESSOR);
		CTLoad(COMMENT);
		CTLoad(COMMENTLINE);
		CTLoad(COMMENTDOC);
		CTLoad(COMMENTLINEDOC);
		CTLoad(COMMENTDOCKEYWORD);
		CTLoad(COMMENTDOCKEYWORDERROR);
		
		CTLoad0(CALLTIP);
		CTLoad0(LINENUMBER);
		CTLoad0(FOLD);
		CTLoad0(FOLD_TRAMA);
		
		CTLoad1(INDENTGUIDE);
		
		CTLoad2(SELBACKGROUND);
		CTLoad2(CURRENT_LINE);
		CTLoad2(USER_LINE);
		CTLoad(GLOBALCLASS);
		CTLoad1(CARET);
		
	}
	
	fil.Close();
	return true;
}


BEGIN_EVENT_TABLE(mxStaticText,wxPanel)
	EVT_PAINT(mxStaticText::OnPaint)
END_EVENT_TABLE()

mxStaticText::mxStaticText (wxWindow *parent, wxString text):wxPanel(parent,wxID_ANY) {
	this->text=text; fore=NULL;
}

void mxStaticText::OnPaint(wxPaintEvent &evt) {
	if (!fore) return;
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(*back);
	wxFont f(config->Styles.font_size, wxMODERN, wxNORMAL, wxNORMAL);
	f.SetWeight((bold&&*bold)?wxFONTWEIGHT_BOLD:wxFONTWEIGHT_NORMAL);
	f.SetStyle((italic&&*italic)?wxFONTFLAG_ITALIC:wxFONTFLAG_DEFAULT);
	dc.Clear(); dc.SetFont(f);
//	dc.SetTextBackground(*back);
	dc.SetTextForeground(*fore);
	int w,h;
	GetClientSize(&w,&h);
	dc.DrawLabel(text,wxRect(10,0,w-10,h),wxALIGN_LEFT);
}

void mxStaticText::SetData (wxColour * fore, wxColour * back, bool * italic, bool * bold) {
	this->bold=bold;
	this->italic=italic;
	this->fore=fore;
	this->back=back;
}

void color_theme::Init() {
	
	wxTheColourDatabase->AddColour(_T("Z LIGHT GREEN"),wxColour(120,255,120));
	wxTheColourDatabase->AddColour(_T("Z DARK GREEN"),wxColour(0,80,0));
	wxTheColourDatabase->AddColour(_T("Z LIGHT BLUE"),wxColour(235,235,255));
	wxTheColourDatabase->AddColour(_T("Z DOXY BLUE"),wxColour(80,80,255));
	wxTheColourDatabase->AddColour(_T("Z ALMOST BLUE"),wxColour(100,100,230));
	wxTheColourDatabase->AddColour(_T("Z LIGHT RED"),wxColour(255,220,220));
	wxTheColourDatabase->AddColour(_T("Z LIGHT YELLOW"),wxColour(250,250,215));
	wxTheColourDatabase->AddColour(_T("Z REALLY DARK BLUE"),wxColour(0,0,50));
	wxTheColourDatabase->AddColour(_T("Z REALLY DARK RED"),wxColour(50,0,0));
	wxTheColourDatabase->AddColour(_T("Z ALMOST RED"),wxColour(230,100,100));
	wxTheColourDatabase->AddColour(_T("Z ALMOST GREEN"),wxColour(100,230,100));
	wxTheColourDatabase->AddColour(_T("Z DARK BLUE"),wxColour(0,0,128));
	wxTheColourDatabase->AddColour(_T("Z DARK RED"),wxColour(128,0,0));
	wxTheColourDatabase->AddColour(_T("Z LIGHT GRAY"),wxColour(200,200,200));
	wxTheColourDatabase->AddColour(_T("Z DARK GRAY"),wxColour(150,150,150));
	wxTheColourDatabase->AddColour(_T("Z DARKER GRAY"),wxColour(100,100,100)); // para el texto del tooltip
	wxTheColourDatabase->AddColour(_T("Z REALLY DARKER GRAY"),wxColour(66,66,66)); // para el texto del tooltip
	wxTheColourDatabase->AddColour(_T("Z DIFF GREEN"),wxColour(128,255,128));
	wxTheColourDatabase->AddColour(_T("Z DIFF YELLOW"),wxColour(255,255,128));
	wxTheColourDatabase->AddColour(_T("Z CORNFLOWER BLUE"),wxColour(100,150,240));
	
	ctheme = new color_theme;
#if 0
	// para generar los perfiles de color y que concuerden con los valores por defecto
	ctheme->SetDefaults(true);
	ctheme->Save(DIR_PLUS_FILE(("colours"),"inverted.zcs"));
	ctheme->SetDefaults();
	ctheme->Save(DIR_PLUS_FILE(("colours"),"default.zcs"));
#else
	ctheme->SetDefaults();
#endif
}

void mxColoursEditor::OnCheck (wxCommandEvent & evt) {
	evt.Skip();
	wxObject *w=evt.GetEventObject();
	for (int i=0;i<lcount;i++)
		if (w==lbold[i]) (*lvalbol[i])=lbold[i]->GetValue();
		else if (w==lcur[i]) (*lvalita[i])=lcur[i]->GetValue();
	if (!setting) { 
		combo->SetSelection(combo->GetCount()-1); 
		custom_theme=*ctheme;
	}
	scroll->Refresh();
}

void mxColoursEditor::OnText (wxCommandEvent & evt) {
	if (evt.GetEventObject()==combo) return;
	wxObject *w=evt.GetEventObject();
	for (int i=0;i<lcount;i++)
		if (w==ltfore[i]) (*lvalfor[i])=wxColour(ltfore[i]->GetValue());
		else if (w==ltback[i]) (*lvalbak[i])=wxColour(ltback[i]->GetValue());
	if (!setting) { 
		combo->SetSelection(combo->GetCount()-1); 
		custom_theme=*ctheme;
	}
	scroll->Refresh();
}

void mxColoursEditor::OnCombo (wxCommandEvent & evt) {
	if (combo->GetSelection()==int(combo->GetCount())-1) {
		(*ctheme)=custom_theme;
	} else {
		wxString filename = combo->GetString(combo->GetSelection());
		wxString fullpath = utils->WichOne(filename,"colours",true);
		ctheme->Load(fullpath);
	}
	SetValues();
}

void mxColoursEditor::SetValues() {
	setting=true;
	for (int i=0;i<lcount;i++) {
		if (lvalfor[i]) ltfore[i]->SetValue(lvalfor[i]->GetAsString(wxC2S_HTML_SYNTAX));
		if (lvalbak[i]) ltback[i]->SetValue(lvalbak[i]->GetAsString(wxC2S_HTML_SYNTAX));
		if (lvalbol[i]) lbold[i]->SetValue(*(lvalbol[i]));
		if (lvalita[i]) lcur[i]->SetValue(*(lvalita[i]));
	}
	wxYield();
	setting=false;
}

void mxColoursEditor::OnOpen (wxCommandEvent & evt) {
	wxFileDialog dlg (this, LANG(GENERAL_OPEN,"Abrir"),config->Files.last_dir,"", _T("Zinjai Colour Schemes (*.zcs)|*.zcs;*.ZCS|All files(*)|*"), wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK) {
		config->Files.last_dir=wxFileName(dlg.GetPath()).GetPath();
		ctheme->Load(dlg.GetPath());
		combo->SetSelection(combo->GetCount()-1); 
		custom_theme=*ctheme;
		SetValues();
	}
}

void mxColoursEditor::OnSave (wxCommandEvent & evt) {
	wxString fname=wxGetTextFromUser(LANG(COLOURS_ESCHEME,"Esquema:"),LANG(GENERAL_SAVE,"Guardar"),"custom_color_scheme", this);
	if (fname.Len()) {
		if (!fname.Upper().EndsWith(".ZCS")) fname<<".zcs";
		ctheme->Save(DIR_PLUS_FILE(DIR_PLUS_FILE(config->home_dir,"colours"),fname));
#ifdef __WIN32__
		bool os_case=false;
#else
		bool os_case=true;
#endif
		int idx=combo->FindString(fname,os_case);
		if (idx==wxNOT_FOUND) combo->Insert(fname,idx=0);
		combo->Select(idx);
	}
}

