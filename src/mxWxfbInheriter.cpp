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
	

mxWxfbInheriter::mxWxfbInheriter(wxWindow *parent, bool a_new_class):wxDialog(parent,wxID_ANY,new_class?LANG(WXFB_CAPTION_NEWCLASS,"wxFB - Generar Clase Heredada"):LANG(WXFB_CAPTION_UPDATE,"wxFB - Actualizar Clase Heredada"),wxDefaultPosition,wxDefaultSize) {
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	new_class=a_new_class;
	FillBaseArray();
	
	if (base_array.GetCount()==0) {
		mxMessageDialog(LANG(WXFB_NO_WXFB_CLASSES,"No se encontraron clases generadas por wxFormsBuilder."),LANG(WXFB_GENERATE_INHERITED_CLASS,"Heredar Clase wxFormsBuilder"),mxMD_ERROR|mxMD_OK).ShowModal();
		Close();
		return;
	}
	
	wxBoxSizer *baseSizer = new wxBoxSizer(wxHORIZONTAL);
	base_class = new wxComboBox(this, wxID_ANY,base_array[0], wxDefaultPosition, wxDefaultSize, base_array,wxCB_READONLY);
	baseSizer->Add(new wxStaticText(this, wxID_ANY, new_class?LANG(WXFB_BASECLASS,"Clase base:"):LANG(WXFB_INHERITEDCLASS,"Clase heredada:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
	baseSizer->Add(base_class, sizers->BA5_Exp1);
	mySizer->Add(baseSizer, sizers->BA5_Exp1);

	if (new_class) {
		wxBoxSizer *childSizer = new wxBoxSizer(wxHORIZONTAL);
		child_class = new wxTextCtrl(this, wxID_ANY,_T(""));
		childSizer->Add(new wxStaticText(this, wxID_ANY, LANG(WXFB_INHERITEDCLASS,"Clase heredada:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
		childSizer->Add(child_class, sizers->BA5_Exp1);
		mySizer->Add(childSizer, sizers->BA5_Exp1);
	}
	
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
	SetFocusFromKbd();
	
	Show();
}

mxWxfbInheriter::~mxWxfbInheriter() {
}

void mxWxfbInheriter::FillBaseArray() {
	base_array.Clear();
	for (unsigned int i=0; i<project->wxfbHeaders.GetCount();i++) {
//		parser->ParseFile(project->wxfbHeaders[i]);
//		parser->Parse();
		pd_file *pdf=parser->GetFile(project->wxfbHeaders[i]);
		if (pdf) {
			pd_ref *cls_ref = pdf->first_class;
			ML_ITERATE(cls_ref)
				base_array.Add(PD_UNREF(pd_class,cls_ref)->name);
		}
	}
	
	if (!new_class) {
		wxArrayString fathers(base_array);
		base_array.Clear();
		pd_inherit *item=parser->first_inherit;
		while (item->next) {
			item=item->next;
			if (fathers.Index(item->father)!=wxNOT_FOUND) {
				base_array.Add(item->son);
				father_array.Add(item->father);
			}
		}
	}
	
}

void mxWxfbInheriter::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxWxfbInheriter::OnButtonOk(wxCommandEvent &evt) {
	if (new_class) OkNewClass();
	else OkUpdateClass();
}

void mxWxfbInheriter::OkUpdateClass() {
	Hide();
	wxString cname=base_class->GetValue();
	wxString fname=father_array[base_array.Index(cname)];
	if (!project->WxfbUpdateClass(cname,fname)) {
		Show();
		base_class->SetFocus();
		return;
	}
	Close();
}

void mxWxfbInheriter::OkNewClass() {
	wxString base_name = base_class->GetValue();
	wxString name = utils->LeftTrim(child_class->GetValue());
	if (name==_T("")) {
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
