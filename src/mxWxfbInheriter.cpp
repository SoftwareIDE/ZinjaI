#include <wx/combobox.h>
#include "mxWxfbInheriter.h"
#include "parserData.h"
#include "mxSizers.h"
#include "mxBitmapButton.h"
#include "ProjectManager.h"
#include "mxMessageDialog.h"
#include "mxSource.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxWxfbInheriter, wxDialog)
	EVT_BUTTON(wxID_OK,mxWxfbInheriter::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxWxfbInheriter::OnButtonCancel)
	EVT_CLOSE(mxWxfbInheriter::OnClose)
END_EVENT_TABLE()

/**
* @param parent  		parent window (main_window)
* @param classname 		if !update, name for a base class to be use for generating a 
*						new class; if update, name for a class to be updated (added 
*						missing virtual methods)
* @param update			create a new inherited class or add new methods to an existing 
*						one, see classname
**/
mxWxfbInheriter::mxWxfbInheriter(wxWindow *parent, wxString classname, bool update):wxDialog(parent,wxID_ANY,mode!=WXFBI_UPDATE_EXISTING_CLASS?LANG(WXFB_CAPTION_NEWCLASS,"wxFB - Generar Clase Heredada"):LANG(WXFB_CAPTION_UPDATE,"wxFB - Actualizar Clase Heredada"),wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	if (update) { mode=WXFBI_UPDATE_EXISTING_CLASS; FillBaseArray(); }
	else if (classname.Len()) { mode=WXFBI_NEW_CLASS_SPECIFIC; wxfb_classes.Add(classname); }
	else { mode=WXFBI_NEW_CLASS_ANY; FillBaseArray(); }
		
	if (mode==WXFBI_NEW_CLASS_ANY && wxfb_classes.GetCount()==0) {
		mxMessageDialog(LANG(WXFB_NO_WXFB_CLASSES,"No se encontraron clases generadas por wxFormsBuilder."),LANG(WXFB_GENERATE_INHERITED_CLASS,"Heredar Clase wxFormsBuilder"),mxMD_ERROR|mxMD_OK).ShowModal();
		Close();
		return;
	}
	
	if (mode==WXFBI_UPDATE_EXISTING_CLASS && user_classes.GetCount()==0) {
		mxMessageDialog(LANG(WXFB_NO_WXFB_INHERITED_CLASSES,"No se encontraron clases heredadas a partir de las generadas por wxFormBuilder."),LANG(WXFB_CAPTION_UPDATE,"wxFB - Actualizar Clase Heredada"),mxMD_ERROR|mxMD_OK).ShowModal();
		Close();
		return;
	}
	
	if (mode==WXFBI_NEW_CLASS_SPECIFIC) {
		mySizer->Add(new wxStaticText(this, wxID_ANY, 
			LANG(WXFB_AUTOINHERIT_MESSAGE,""
			"ZinjaI ha detectado una nueva clase (ventana, diálogo, panel, etc)\n"
			"en un archivo generado por wxFormBuilder. Puede generar ahora una \n"
			"herencia a partir de dicha clase. Introduzca el nombre de la nueva \n"
			"clase heradad para hacerlo, o presione cancelar en caso contrario."
			),wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Exp0);
		wxfb_classes.Clear(); wxfb_classes.Add(classname);
	}
	
	wxBoxSizer *baseSizer = new wxBoxSizer(wxHORIZONTAL);
	base_class = new wxComboBox(this, wxID_ANY,mode==WXFBI_UPDATE_EXISTING_CLASS?user_classes[0]:wxfb_classes[0], wxDefaultPosition, wxDefaultSize, mode==WXFBI_UPDATE_EXISTING_CLASS?user_classes:wxfb_classes, wxCB_READONLY);
	baseSizer->Add(new wxStaticText(this, wxID_ANY, mode==WXFBI_NEW_CLASS_ANY?LANG(WXFB_BASECLASS,"Clase base:"):LANG(WXFB_INHERITEDCLASS,"Clase heredada:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
	baseSizer->Add(base_class, sizers->BA5_Exp1);
	mySizer->Add(baseSizer, sizers->BA5_Exp1);
	
	if (mode!=WXFBI_UPDATE_EXISTING_CLASS) {
		wxBoxSizer *childSizer = new wxBoxSizer(wxHORIZONTAL);
		child_class = new wxTextCtrl(this, wxID_ANY,"");
		childSizer->Add(new wxStaticText(this, wxID_ANY, LANG(WXFB_INHERITEDCLASS,"Clase heredada:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
		childSizer->Add(child_class, sizers->BA5_Exp1);
		mySizer->Add(childSizer, sizers->BA5_Exp1);
	} else child_class=nullptr;
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	mxBitmapButton *button_yes = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	mxBitmapButton *button_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	bottomSizer->Add( button_cancel , sizers->BA5);
	bottomSizer->Add( button_yes , sizers->BA5);
	SetAffirmativeId(wxID_OK);
	button_yes->SetDefault();
	SetEscapeId(wxID_CANCEL);
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	if (mode==WXFBI_NEW_CLASS_ANY) base_class->SetFocus();
	else if (mode!=WXFBI_UPDATE_EXISTING_CLASS) child_class->SetFocus();
	
	ShowModal();
}

void mxWxfbInheriter::FillBaseArray() {
	ProjectManager::WxfbAutoCheckData data;
	wxfb_classes.Clear(); user_classes.Clear(); 
	if (mode==WXFBI_UPDATE_EXISTING_CLASS) {
		for(int i=0;i<data.user_fathers.GetSize();i++) 
			wxfb_classes.Add(data.user_fathers[i]);
		for(int i=0;i<data.user_classes.GetSize();i++) 
			user_classes.Add(data.user_classes[i]);
	} else {
		for(int i=0;i<data.wxfb_classes.GetSize();i++) 
			wxfb_classes.Add(data.wxfb_classes[i]);
	}
}

void mxWxfbInheriter::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxWxfbInheriter::OnButtonOk(wxCommandEvent &evt) {
	if (mode==WXFBI_UPDATE_EXISTING_CLASS) OkUpdateClass();
	else OkNewClass();
}

void mxWxfbInheriter::OkUpdateClass() {
	Hide();
	wxString user_class=base_class->GetValue();
	wxString wxfb_class=wxfb_classes[user_classes.Index(user_class)];
	if (!project->WxfbUpdateClass(wxfb_class,user_class)) {
		Show();
		base_class->SetFocus();
		return;
	}
	Close();
}

void mxWxfbInheriter::OkNewClass() {
	wxString base_name = base_class->GetValue();
	wxString name = mxUT::LeftTrim(child_class->GetValue());
	if (name=="") {
		mxMessageDialog(LANG(WXFB_CLASSNAME_MISSING,"Debe introducir el nombre de la clase"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		return;
	}
	Hide();
	if (!project->WxfbNewClass(base_name,name)) {
		Show();
		child_class->SetFocus();
		return;
	}
	Close();
}

void mxWxfbInheriter::OnButtonCancel(wxCommandEvent &evt) {
	Close();
}
