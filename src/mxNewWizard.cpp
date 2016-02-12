#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/html/htmlwin.h>
#include <wx/textfile.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/listbox.h>
#include <wx/dirdlg.h>
#include <wx/treectrl.h>
#include <wx/dir.h>
#include "mxNewWizard.h"
#include "ids.h"
#include "ConfigManager.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "mxHelpWindow.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "mxBitmapButton.h"
#include "mxArt.h"
#include "mxMessageDialog.h"
#include "CodeHelper.h"
#include "mxSizers.h"
#include "Language.h"
#include "mxMultipleFileChooser.h"
#include "parserData.h"
#include "version.h"
#include <wx/tooltip.h>
#include "Cpp11.h"
#include "SimpleTemplates.h"

mxNewWizard *g_wizard = nullptr;

BEGIN_EVENT_TABLE(mxNewWizard, wxDialog)
	EVT_BUTTON(wxID_OK,mxNewWizard::OnButtonNext)
	EVT_BUTTON(wxID_CANCEL,mxNewWizard::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxNewWizard::OnButtonHelp)
	EVT_BUTTON(mxID_WIZARD_NEW_FILE_PATH,mxNewWizard::OnButtonNewFilePath)
	EVT_BUTTON(mxID_WIZARD_FOLDER,mxNewWizard::OnButtonFolder)
	EVT_RADIOBOX(mxID_WIZARD_START_RADIO,mxNewWizard::OnStartRadio)
	EVT_RADIOBOX(mxID_WIZARD_ONPROJECT_RADIO,mxNewWizard::OnProjectRadio)
	EVT_RADIOBOX(mxID_WIZARD_PROJECT_PATH_RADIO,mxNewWizard::OnProjectPathRadio)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxNewWizard::OnButtonNext)
	EVT_TEXT(mxID_WIZARD_PROJECT_NAME_TEXT,mxNewWizard::OnProjectNameChange)
	EVT_TEXT(mxID_WIZARD_PROJECT_FOLDER_TEXT,mxNewWizard::OnProjectFolderChange)
	EVT_TEXT(mxID_WIZARD_ONPROJECT_NAME,mxNewWizard::OnOnProjectNameChange)
	EVT_CHECKBOX(mxID_WIZARD_PROJECT_FOLDER_CHECK,mxNewWizard::OnProjectFolderCheck)
	EVT_CHECKBOX(mxID_WIZARD_PROJECT_FILES_OPEN_CHECK,mxNewWizard::OnProjectFilesOpenCheck)
	EVT_CHECKBOX(mxID_WIZARD_PROJECT_FILES_DIR_CHECK,mxNewWizard::OnProjectFilesDirCheck)
	EVT_CLOSE(mxNewWizard::OnClose)
END_EVENT_TABLE()

mxNewWizard::mxNewWizard(mxMainWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, LANG(CAPTION_NEW_FILE_WIZARD,"Nuevo Archivo"), pos, size, style) {

	SimpleTemplates::Initialize(); // ensure g_templates!=nullptr
	
	FlagGuard<BoolFlag> fg(mask_folder_change_events);
	project_full_path=nullptr;
	
	wxSize psize,msize(400,300);
	panel = new wxPanel(this,wxID_ANY,wxDefaultPosition,wxDefaultSize);
	
	wxBoxSizer *sizerU = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizerD = new wxBoxSizer(wxHORIZONTAL);
	sizer_for_panel = sizerU;
	
	nextButton = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.next,LANG(NEWWIZARD_NEXT," Siguiente ")); 
	cancelButton = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(NEWWIZARD_CANCEL," Cancelar ")); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	sizerU->Add(new wxStaticBitmap(this,wxID_ANY, bitmaps->GetBitmap("newWizard.png")), sizers->BA10);
	sizerU->Add(panel, sizers->BA10_Exp1);
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	sizerD->Add(help_button,sizers->BA10_Exp0);
	sizerD->AddStretchSpacer();
	sizerD->Add(cancelButton,sizers->BA10);
	sizerD->Add(nextButton,sizers->BA10);

	sizer->Add(sizerU, sizers->Exp1);
	sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL) , sizers->Exp0);
	sizer->Add(sizerD, sizers->Exp0);
	
	SetEscapeId(wxID_CANCEL);
	nextButton->SetDefault();
		
	CreatePanelTemplates();
	CreatePanelStart();
	CreatePanelProject1();
	CreatePanelProject2();
	CreatePanelOnProject();
	
	//	psize = panel_templates->GetSize();
	//	if (psize.GetWidth()>msize.GetWidth()) msize.SetWidth(psize.GetWidth());
	//	if (psize.GetHeight()>msize.GetHeight()) msize.SetHeight(psize.GetHeight());
	//	psize = panel_start->GetSize();
	//	if (psize.GetWidth()>msize.GetWidth()) msize.SetWidth(psize.GetWidth());
	//	if (psize.GetHeight()>msize.GetHeight()) msize.SetHeight(psize.GetHeight());
	//	psize = panel_project->GetSize();
	//	if (psize.GetWidth()>msize.GetWidth()) msize.SetWidth(psize.GetWidth());
	//	if (psize.GetHeight()>msize.GetHeight()) msize.SetHeight(psize.GetHeight());
	//	psize = panel_onproject->GetSize();
	//	if (psize.GetWidth()>msize.GetWidth()) msize.SetWidth(psize.GetWidth());
	//	if (psize.GetHeight()>msize.GetHeight()) msize.SetHeight(psize.GetHeight());
	//	SetSize(msize);
	
	SetSizer(sizer);
	int x=0,y=0;
	ShowPanelTemplates(); Fit();
	if (GetSize().x>x) x = GetSize().x;
	if (GetSize().y>y) y = GetSize().y;
	ShowPanelProject1(); Fit();
	if (GetSize().x>x) x = GetSize().x;
	if (GetSize().y>y) y = GetSize().y;
	ShowPanelProject2(); Fit();
	if (GetSize().x>x) x = GetSize().x;
	if (GetSize().y>y) y = GetSize().y;
	ShowPanelStart(); Fit(); 
	if (GetSize().x>x) x = GetSize().x;
	if (GetSize().y>y) y = GetSize().y;
	SetSize(size_w=x,size_h=y);

	fg.Release();
	UpdateProjectFullPath();
	
}


void mxNewWizard::OnStartRadio(wxCommandEvent &event){
	if (start_radio->GetSelection()==0) 
		nextButton->SetThings(bitmaps->buttons.ok,LANG(NEWWIZARD_CREATE," Crear "));
	else
		nextButton->SetThings(bitmaps->buttons.next,LANG(NEWWIZARD_CONTINUE," Continuar "));
	start_tooltip->SetLabel(start_radio->GetItemToolTip(start_radio->GetSelection())->GetTip());
}

void mxNewWizard::OnProjectRadio(wxCommandEvent &event){
	if (onproject_radio->GetSelection()!=3) {
		onproject_label->SetLabel(LANG(NEWWIZARD_FILENAME,"Nombre del archivo:"));
		onproject_const_def->Enable(false);
		onproject_const_copy->Enable(false);
		onproject_dest->Enable(false);
		onproject_inherit_include->Enable(false);
		onproject_inherit_label->Enable(false);
		for (int i=0;i<config->Init.inherit_num;i++) {
			onproject_inherit_class[i]->Enable(false);
			onproject_inherit_visibility[i]->Enable(false);
		}
	} else {
		onproject_label->SetLabel(LANG(NEWWIZARD_CLASSNAME,"Nombre de la clase:"));
		onproject_const_def->Enable(true);
		onproject_const_copy->Enable(true);
		onproject_dest->Enable(true);
		onproject_inherit_include->Enable(true);
		onproject_inherit_label->Enable(true);
		for (int i=0;i<config->Init.inherit_num;i++) {
			onproject_inherit_class[i]->Enable(true);
			onproject_inherit_visibility[i]->Enable(true);
		}
	}
}

void mxNewWizard::OnButtonCancel(wxCommandEvent &event){
	if (panel==panel_onproject || panel==panel_start || (panel==panel_project_1 && only_for_project))
		Close();
	else if (panel==panel_project_1)
		ShowPanelStart();
	else if (panel==panel_project_2)
		ShowPanelProject1();
	else if (panel==panel_templates)
		ShowPanelStart();
}

void mxNewWizard::OnClose(wxCloseEvent &event){
	Hide();
}

void mxNewWizard::OnProjectCreate() {
	int sel=onproject_radio->GetSelection();	
	wxString name = mxUT::LeftTrim(onproject_name->GetValue());
	if (sel==3) { // clase
		wxString folder;
		int pos1=name.Find('\\',true);
		int pos2=name.Find('/',true);
		if (pos1!=wxNOT_FOUND && pos2!=wxNOT_FOUND) {
			int pos = pos1>pos2?pos1:pos2;
			folder=name.Mid(0,pos);
			name=name.Mid(pos+1);
		} else if (pos1!=wxNOT_FOUND) {
			folder=name.Mid(0,pos1);
			name=name.Mid(pos1+1);
		} else if (pos2!=wxNOT_FOUND) {
			folder=name.Mid(0,pos2);
			name=name.Mid(pos2+1);
		}
		if (folder.Len()) {
			folder = DIR_PLUS_FILE(project->path,folder);
			if (!wxFileName::DirExists(folder)) {
				int ans = mxMessageDialog(this,LANG1(NEWWIZARD_DIRECTORY_NOT_FOUND,"El directorio \"<{1}>\" no existe. Desea crearlo?",folder),LANG(GENERIC_ERROR,"Error"),mxMD_YES|mxMD_NO|mxMD_QUESTION).ShowModal();
				if (ans==mxMD_YES) {
					wxFileName::Mkdir(folder,0777,wxPATH_MKDIR_FULL);
					if (!wxFileName::DirExists(folder)) {
						mxMessageDialog(this,LANG(NEWWIZARD_ERROR_MKDIR,"No se pudo crear el directorio."),LANG(GENERIC_ERROR,"Error"),mxMD_YES|mxMD_NO|mxMD_ERROR).ShowModal();
						onproject_name->SetFocus();
						return;
					}
				} else {
					onproject_name->SetFocus();
					return;
				}
			}
		} else
			folder=project->path;
		if (name=="") {
			mxMessageDialog(this,LANG(NEWWIZARD_CLASSNAME_MISSNG,"Debe introducir el nombre de la clase"),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			onproject_name->SetFocus();
			return;
		} else if (wxNOT_FOUND!=name.Find(' ') || wxNOT_FOUND!=name.Find('-') 
					|| wxNOT_FOUND!=name.Find('<') || wxNOT_FOUND!=name.Find('?') 
					|| wxNOT_FOUND!=name.Find('>') || wxNOT_FOUND!=name.Find('*') 
					|| wxNOT_FOUND!=name.Find('.') || wxNOT_FOUND!=name.Find(':') ) {
			mxMessageDialog(this,LANG(NEWWIZARD_INVALID_CLASSNAME,"El nombre de la clase no puede incluir ni espacios ni operadores"),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			onproject_name->SetFocus();
			return;
		}
		// controlar que no exista
		wxString cpp_name = DIR_PLUS_FILE(project->path,(folder.Len()?DIR_PLUS_FILE(folder,name):name)+"."+project->default_fext_source);
		wxString h_name = DIR_PLUS_FILE(project->path,(folder.Len()?DIR_PLUS_FILE(folder,name):name)+"."+project->default_fext_header);
		if (wxFileName::FileExists(cpp_name)) {
			mxMessageDialog(this,LANG(NEWWIZARD_FILE_EXISTS,"Ya existe un archivo con ese nombre"),cpp_name,mxMD_OK|mxMD_ERROR).ShowModal();
			return;
		} else if (wxFileName::FileExists(h_name)) {
			mxMessageDialog(this,LANG(NEWWIZARD_FILE_EXISTS,"Ya existe un archivo con ese nombre"),h_name,mxMD_OK|mxMD_ERROR).ShowModal();				
			return;
		} else {
			
			// armar una arreglo con las clases bases y sus ansestros para buscar metodos virtuales
			wxArrayString base_classes;
			for (int i=0;i<config->Init.inherit_num;i++)
				base_classes.Add(onproject_inherit_class[i]->GetValue());
			for(unsigned int i=0;i<base_classes.GetCount();i++) {
				pd_inherit *father=nullptr;
				while (parser->GetFather(base_classes[i],father)) {
					if (base_classes.Index(father->father)==wxNOT_FOUND)
						base_classes.Add(father->father);
				}
			}
			// buscar todos los métodos virtuales de todas esas clases
			wxArrayString virtual_methods;
			for(unsigned int i=0;i<base_classes.GetCount();i++) {
				pd_class *cls=parser->GetClass(base_classes[i]);
				if (!cls) continue;
				pd_func *func=cls->first_method;
				while (func->next) { func=func->next; } // para atras, para que queden ordenadas como en el h
				while (func->prev) {
					wxString visib="private ";
					if (func->properties&PD_CONST_PROTECTED) visib="protected ";
					else if (func->properties&PD_CONST_PUBLIC) visib="public ";
					if (func->properties&PD_CONST_VIRTUAL_PURE) {
						virtual_methods.Add(visib+func->full_proto+((func->properties&PD_CONST_CONST)?" const =0":" =0")); // const no deberia estar en el full proto?
					} else if (func->properties&PD_CONST_VIRTUAL) {
						virtual_methods.Add(visib+func->full_proto+((func->properties&PD_CONST_CONST)?" const":"")); // const no deberia estar en el full proto?
					}
					func=func->prev;
				}
			}
			if (virtual_methods.GetCount()) {
				// eliminar repetidos
				for(unsigned int i=0;i<virtual_methods.GetCount();i++) {  
					for(unsigned int j=i+1;j<virtual_methods.GetCount();j++) {  
						if (virtual_methods[i].AfterFirst(' ')==virtual_methods[j].AfterFirst(' ')) 
							if (virtual_methods[i].EndsWith(" =0") && !virtual_methods[j].EndsWith(" =0")) {
								virtual_methods[i]=virtual_methods[j]; // si hay una version virtual pura y otra no pura, quedarse con la que no
							}
							virtual_methods.RemoveAt(j--);
					}
				}
				// preguntar cuales va a querer implementar
				wxArrayInt sels; 
				for (unsigned int i=0;i<virtual_methods.GetCount();i++) sels.Add(i);
				/*int nsels=*/wxGetMultipleChoices(sels,"Seleccione los métodos virtuales de las clases ansestras a implementar","Nueva clase",virtual_methods,this);
				// eliminar los no seleccionados
				for(int i=virtual_methods.GetCount()-1;i>=0;i--) { 
					if (sels.Index(i)==wxNOT_FOUND) 
						virtual_methods.RemoveAt(i);
				}
				// quitar el "=0" del final de los prototipos
				for(unsigned int i=0;i<virtual_methods.GetCount();i++) { 
					if (virtual_methods[i].EndsWith(" =0")) {
						for(int j=0;j<3;j++) virtual_methods[i].RemoveLast();
					}
				}
			}
			
			// crear el cpp
			wxTextFile cpp_file(cpp_name);
			cpp_file.Create();
			cpp_file.AddLine(wxString("#include \"")+name+"."+project->default_fext_header+"\"");
			cpp_file.AddLine("");
			if (onproject_const_def->GetValue()) { // constructor por defecto
				cpp_file.AddLine(name+"::"+name+"() {");
				cpp_file.AddLine("\t");
				cpp_file.AddLine("}");
				cpp_file.AddLine("");
			}
			if (onproject_const_copy->GetValue()) { // constructor de copia
				cpp_file.AddLine(name+"::"+name+"(const "<<name<<" &arg) {");
				cpp_file.AddLine("\t");
				cpp_file.AddLine("}");
				cpp_file.AddLine("");
			}
			if (onproject_dest->GetValue()) { // destructor
				cpp_file.AddLine(name+"::~"+name+"() {");
				cpp_file.AddLine("\t");
				cpp_file.AddLine("}");
				cpp_file.AddLine("");
			}
			for(unsigned int i=0;i<virtual_methods.GetCount();i++) { // métodos virtuales heredados
				// los metodos virtuales aparecen en el arreglo, con el scope de la clase original, no la nueva
				virtual_methods[i]=
					virtual_methods[i].BeforeFirst('(').BeforeLast(':').BeforeLast(' ') 
					+" "+name+"::"+
					virtual_methods[i].BeforeFirst('(').AfterLast(':')
					+"("+
					virtual_methods[i].AfterFirst('(');
				cpp_file.AddLine(virtual_methods[i].AfterFirst(' ')+" {");
				cpp_file.AddLine("\t");
				cpp_file.AddLine("}");
				cpp_file.AddLine("");
				// sacar el nombre de la clase del prototipo para que no ponerlo despues en el .h
				virtual_methods[i]=
					virtual_methods[i].BeforeFirst('(').BeforeLast(':').BeforeLast(' ') 
					+" "+
					virtual_methods[i].BeforeFirst('(').AfterLast(':')
					+"("+
					virtual_methods[i].AfterFirst('(');
			}
			cpp_file.Write();
			cpp_file.Close();
			
			// crear el h
			wxString def=name;
			def.MakeUpper();
			wxTextFile h_file(h_name);
			h_file.Create();
			h_file.AddLine(wxString("#ifndef ")+def+"_H");
			h_file.AddLine(wxString("#define ")+def+"_H");
			
			for (int i=0;i<config->Init.inherit_num;i++) { // #includes para las clases bases
				wxString aux = onproject_inherit_class[i]->GetValue();
				if (aux.Len()) {
					aux = g_code_helper->GetIncludeForClass(folder,aux);
					if (aux.Len())
						h_file.AddLine(aux);
				}
			}
			h_file.AddLine("");
			wxString inherits; // texto de la declaración de herencia
			for (int i=0;i<config->Init.inherit_num;i++)
				if (onproject_inherit_class[i]->GetValue().Len()) {
					if (inherits.Len())
						inherits<<", "<<onproject_inherit_visibility[i]->GetValue()<<" "<<onproject_inherit_class[i]->GetValue();
					else
						inherits<<" : "<<onproject_inherit_visibility[i]->GetValue()<<" "<<onproject_inherit_class[i]->GetValue();
				}
			// cuerpo de la clase
			h_file.AddLine(wxString("class ")+name+inherits+" {");
			h_file.AddLine("private:");
			for(unsigned int i=0;i<virtual_methods.GetCount();i++) 
				if (virtual_methods[i].StartsWith("private "))
					h_file.AddLine(wxString("\t")+virtual_methods[i].AfterFirst(' ')+";");
			h_file.AddLine("protected:");
			for(unsigned int i=0;i<virtual_methods.GetCount();i++) 
				if (virtual_methods[i].StartsWith("protected "))
					h_file.AddLine(wxString("\t")+virtual_methods[i].AfterFirst(' ')+";");
			h_file.AddLine("public:");
			if (onproject_const_def->GetValue())
				h_file.AddLine(wxString("\t")+name+"();");
			if (onproject_const_copy->GetValue())
				h_file.AddLine(wxString("\t")+name+"(const "<<name<<" &arg);");
			if (onproject_dest->GetValue())
				h_file.AddLine(wxString("\t~")+name+"();");
			for(unsigned int i=0;i<virtual_methods.GetCount();i++) 
				if (virtual_methods[i].StartsWith("public "))
					h_file.AddLine(wxString("\t")+virtual_methods[i].AfterFirst(' ')+";");
			h_file.AddLine("};");
			
			h_file.AddLine("");
			h_file.AddLine("#endif");
			h_file.AddLine("");
			h_file.Write();
			h_file.Close();
			
			// abrir
			main_window->OpenFile(cpp_name,true);
			main_window->OpenFile(h_name,true);
			Close();
			return;
		}
	} else {
		if (name=="") {
			mxMessageDialog(this,LANG(NEWWIZARD_FILENAME_MISSING,"Debe introducir el nombre del archivo"),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			onproject_name->SetFocus();
			return;
		} else if (wxNOT_FOUND!=name.Find('<') || wxNOT_FOUND!=name.Find('?') 
					|| wxNOT_FOUND!=name.Find('>') || wxNOT_FOUND!=name.Find('*') ) {
			mxMessageDialog(this,LANG(NEWWIZARD_INVALID_CLASSNAME,"El nombre de la clase no puede incluir caracteres especiales (?,*,<,>)"),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			onproject_name->SetFocus();
			return;
		}
		wxFileName filename(DIR_PLUS_FILE(project->path,name));
		if (filename.GetExt()=="") {
			if (sel==0)
				filename.SetExt(project->default_fext_source);
			else if (sel==1)
				filename.SetExt(project->default_fext_header);
		}
		if (filename.FileExists()) {
			mxMessageDialog(this,LANG(NEWWIZARD_FILE_EXISTS,"Ya existe un archivo con ese nombre"),filename.GetFullPath(),mxMD_OK|mxMD_ERROR).ShowModal();				
			return;
		} else {
			wxTextFile cpp_file(filename.GetFullPath());
			cpp_file.Create();
			if (sel==1) {
				wxString cte = filename.GetName()+"_H";
				cte.MakeUpper();
				cpp_file.AddLine(wxString("#ifndef ")+cte);
				cpp_file.AddLine(wxString("#define ")+cte);
				cpp_file.AddLine("");
				cpp_file.AddLine("#endif");
			}
			cpp_file.Write();
			cpp_file.Close();
			if (sel==0)
				project->AddFile(FT_SOURCE,filename);
			else if (sel==1)
				project->AddFile(FT_HEADER,filename);
			else
				project->AddFile(FT_OTHER,filename);
			main_window->OpenFile(filename.GetFullPath(),false);
			Close();
			return;
		}
	}
}

void mxNewWizard::ProjectCreate() {
	
	// si quiere usar los archivos abiertos en lugar de los de la plantilla, guardarlos antes de que se cierren
	wxArrayString filesToAdd;
	if (project_current_files->GetValue()) {
		wxAuiNotebook *nb=main_window->notebook_sources;
		for (unsigned int i=0;i<nb->GetPageCount();i++) {
			mxSource *src=(mxSource*)(nb->GetPage(i));
			if (!src->sin_titulo) {
				filesToAdd.Add(src->source_filename.GetFullPath());
			}
		}
	}
	
	wxString filename = project_name->GetValue();
	wxString folder = project_folder_path->GetValue();
	int cual = project_list->GetSelection();
	if (filename=="") {
		mxMessageDialog(this,LANG(NEWWIZARD_PROJECT_NAME_MISSING,"Debe establecer un nombre de proyecto."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		project_name->SetFocus();
		return;
	}
	if (filename=="") {
		mxMessageDialog(this,LANG(NEWWIZARD_FILENAME_MISSING,"Debe establecer un nombre de archivo."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		project_name->SetFocus();
		return;
	}
	if (folder=="") {
		mxMessageDialog(this,LANG(NEWWIZARD_PROJECT_LOCATION_MISSING,"Debe establecer un ubicacion para el proyecto."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		project_folder_path->SetFocus();
		return;
	}
	if (!wxFileName::DirExists(folder)) {
		wxFileName::Mkdir(folder,0777,wxPATH_MKDIR_FULL);
	}
	if (!wxFileName::DirExists(folder)) {
		mxMessageDialog(this,LANG(NEWWIZARD_INVALID_LOCATION,"La ubicacion seleccionada no es valida."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		project_folder_path->SetFocus();
		return;
	}
	wxString full=project_folder_create->GetValue()?DIR_PLUS_FILE(folder,filename):folder;
	if (wxFileName::FileExists(full)) {
		mxMessageDialog(this,LANG(NEWWIZARD_PROJECT_DIR_IS_FILE,"No se puede crear el proyecto porque ya existe un archivo con ese nombre en la ubicacion seleccionada. Elimine el archivo o modifique el nombre."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		project_name->SetFocus();
		return;
	}
	if (wxFileName::DirExists(full)) {
		if (wxFileName::FileExists(DIR_PLUS_FILE(full,filename+"."+_T(PROJECT_EXT)))) {
			if (mxMD_NO==mxMessageDialog(this,LANG(NEWWIZARD_CONFIRM_REPLACE,"Ya existe un proyecto en esa ubicacion y con ese nombre. Desea reemplazarlo?"),LANG(GENERIC_WARNING,"Aviso"),mxMD_YES_NO|mxMD_WARNING).ShowModal())
				return;
			;
		} else if (project_folder_create->GetValue()) {
			if (mxMD_NO==mxMessageDialog(this,LANG(NEWWIZARD_CONFIRM_OVERWRITE_DIR,"Ya existe un directorio en esa ubicacion y con ese nombre. Desea utilizarlo de todos modos para el proyecto?"),LANG(GENERIC_WARNING,"Aviso"),mxMD_YES_NO|mxMD_WARNING).ShowModal())
				return;
		}
	} else if (project_folder_create->GetValue()) // crear el directorio del proyecto
		wxFileName::Mkdir(full);
	
	config->Files.last_project_dir=full;
	
	bool custom_files = project_current_files->GetValue()||project_dir_files->GetValue();
	if (cual<2) {
		// crear el archivo de proyecto
		wxTextFile project_file(DIR_PLUS_FILE(full,filename+"."+_T(PROJECT_EXT)));
		if (project_file.Exists())
			project_file.Open();
		else
			project_file.Create();
		project_file.Clear();
		
		project_file.AddLine("[general]");
		project_file.AddLine(wxString("version_saved=")<<VERSION);
		
		if (cual==1 && !custom_files) { // si quiere un main
			// registrarlo en el proyecto
			project_file.AddLine("[source]");
			project_file.AddLine("path=main.cpp");
			project_file.AddLine("cursor=78");
			project_file.AddLine("open=true");
			// crearlo en disco
			wxString main_file=DIR_PLUS_FILE(full,"main.cpp");
			wxTextFile fil(main_file);
			if (fil.Exists())
				fil.Open();
			else
				fil.Create();
			fil.Clear();
			fil.AddLine("#include<iostream>");
			fil.AddLine("using namespace std;");
			fil.AddLine("");
			fil.AddLine("int main (int argc, char *argv[]) {");
			fil.AddLine("\t");
			fil.AddLine("\treturn 0;");
			fil.AddLine("}");
			fil.AddLine("");
			fil.Write();
			fil.Close();
		}
		
		// guardar y abrir el proyecto
		project_file.AddLine("[end]");
		project_file.Write();
		project_file.Close();
	} else {
		wxString ofull=mxUT::WichOne(project_templates[cual],"templates",false);
		if (custom_files) { // si usa los archivos actuales, copiar solo el archivo de proyecto
			wxString str;
			wxTextFile fin(DIR_PLUS_FILE(ofull,project_templates[cual]+"."+_T(PROJECT_EXT))); fin.Open();
			wxTextFile fout(DIR_PLUS_FILE(full,filename+"."+_T(PROJECT_EXT)));
			if (fout.Exists()) fout.Open(); else fout.Create();
			fout.Clear();
			bool add=true;
			for ( wxString str = fin.GetFirstLine(); !fin.Eof(); str = fin.GetNextLine() ) {
				if (str[0]=='[') {
					add = ( !str.StartsWith("[source]") && !str.StartsWith("[header]") && !str.StartsWith("[other]") );
				}
				if (add) {
					if (!str.StartsWith("current_source="))
						fout.AddLine(str);
				}
			}
			fout.Write();
			fout.Close();
			fin.Close();
		} else {
			if (!mxUT::XCopy(ofull,full,true)) {
				mxMessageDialog(this,LANG(NEWWIZARD_ERROR_ON_FILE_COPY,"Error al copiar los archivos del proyecto. Compruebe que la ubicacion sea correcta."),LANG(GENERIC_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
				return;
			}
			wxRenameFile(DIR_PLUS_FILE(full,project_templates[cual]+"."+_T(PROJECT_EXT)),DIR_PLUS_FILE(full,filename+"."+_T(PROJECT_EXT)),true);
		}
	}
	
	//cerrar el asistente y abrir el nuevo proyecto
	if (project_check->GetValue()) {
		project_default=cual;
		config->Files.default_project=project_templates[cual];
	}
	Close();
	int multiple=project_current_files->GetValue()?1024:0; // 1024 es valor especial para que no cierre los fuentes abiertos, 0 no significa nada en especial
	main_window->OpenFileFromGui(DIR_PLUS_FILE(full,filename+"."+_T(PROJECT_EXT)),&multiple);
	if (!project) return; // si por alguna razon no se abre el proyecto (puede el usuario cancelarlo, por alguna pregunta?)
	project->FixTemplateData(filename);
	// agregar los archivos abiertos si van
	if (project_current_files->GetValue()) {
//		project->DeleteFile(project->first_header->item); // para que estaba esta linea????
		for (unsigned int i=0;i<filesToAdd.GetCount();i++)
			main_window->OpenFile(filesToAdd[i],true);
	}
	if (project_dir_files->GetValue())
		new mxMultipleFileChooser(project->path,true);
	project->Save();
}

void mxNewWizard::OnButtonNext(wxCommandEvent &event){
	if (panel==panel_onproject) {
		OnProjectCreate();
	} else if (panel==panel_start) {
		current_wizard_mode = (WizardModes)start_radio->GetSelection();
		switch (current_wizard_mode) {
			case WM_Empty:
				main_window->NewFileFromText("",0);
				Close();
				break;
			case WM_Simple:
				ShowPanelTemplates();
				break;
			case WM_Project:
				ShowPanelProject1();
				break;
			case WM_Import:
				ShowPanelProject1();
				break;
		default:
			; // should not happend
		}
	} else if (panel==panel_project_1) {
		ShowPanelProject2();
	} else if (panel==panel_project_2) {
		ProjectCreate();
	} else if (panel==panel_templates) {
		int i = templates_list->GetSelection();
		if (i==wxNOT_FOUND) return;
		wxArrayString templates_files; 
		g_templates->GetFilesList(templates_files,true,true);
		mxSource *source = main_window->NewFileFromTemplate(templates_files[i]);
		if (templates_check->GetValue()) {
			config->Files.default_template = templates_files[i];
			if (g_templates->IsCpp(config->Files.default_template))
				config->Files.cpp_template = config->Files.default_template;
			else
				config->Files.c_template = config->Files.default_template;
		}
		source->SetFocus();
		Close();
	}
}

void mxNewWizard::ShowPanelStart(bool show) {
	SetCurrentPanel(panel_start);
	nextButton->SetThings(bitmaps->buttons.next,LANG(NEWWIZARD_CONTINUE," Continuar "));
	SetSize(size_w,size_h); if (show) Show();
	wxCommandEvent evt; OnStartRadio(evt);
	GetSizer()->Layout();
	start_radio->SetFocus();
}


void mxNewWizard::CreatePanelStart() {
	panel_start = new wxPanel(this,wxID_ANY);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxString choices[4];
	choices[0] = LANG(NEWWIZARD_EMPTY_FILE,"Archivo en Blanco");
	choices[1] = LANG(NEWWIZARD_USE_TEMPLATE,"Utilizar Plantilla");
	choices[2] = LANG(NEWWIZARD_PROYECT,"Proyecto Nuevo");
	choices[3] = LANG(NEWWIZARD_PROYECT_FROM_FILES,"Proyecto Existente");
	
	start_radio = new wxRadioBox(panel_start, mxID_WIZARD_START_RADIO, LANG(NEWWIZARD_FILETYPE,"Tipo de Archivo:"),
					wxDefaultPosition, wxDefaultSize,
					WXSIZEOF(choices), choices,
					1, wxRA_SPECIFY_COLS);
	
	start_radio->SetItemToolTip(0,LANG(NEWWIZARD_TIP_EMPTY,"Crea un nuevo archivo completamente en blanco."));
	start_radio->SetItemToolTip(1,LANG(NEWWIZARD_TIP_TEMPLATE,"Crea un nuevo archivo con un esqueleto basico de programa (crea la funcion main e incluye cabeceras acordes al tipo de programa seleccionado)."));
//	start_radio->SetItemToolTip(2,LANG(NEWWIZARD_TIP_WIZARD,"Permite seleccionar que tipo de funciones y contenedores quiere utilizar ."));
	start_radio->SetItemToolTip(2,LANG(NEWWIZARD_TIP_PROJECT,"Crea un nuevo proyecto. A diferencia de un programa simple, el proyecto puede constar de mas de un archivo fuente y guarda tambien otras configuraciones como bibliotecas utilizadas, parametros para el compilador, etc."));
	start_radio->SetItemToolTip(3,LANG(NEWWIZARD_TIP_IMPORT,"Crea un archivo de proyecto para ZinjaI a partir de fuentes ya existentes. Utilice esta opción para migrar a ZinjaI proyectos creados con otros IDEs o basados en Makefiles o herramientas similares."));

	start_radio->SetSelection(1);
	sizer->Add(start_radio,sizers->Exp0);
	
	sizer->Add(start_tooltip = new wxStaticText(panel_start,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxST_NO_AUTORESIZE),sizers->BA5_Exp1);
	
	if (config->Help.show_extra_panels) {
		start_tooltip->Hide();
		wxHtmlWindow *html = new wxHtmlWindow(panel_start,wxID_ANY);
		sizer->Add(html,sizers->Exp1);
		html->LoadFile(DIR_PLUS_FILE(config->Help.guihelp_dir,wxString("new_help_")<<config->Init.language_file<<(".html")));
	}
	
	
	panel_start->SetSizerAndFit(sizer);
	panel_start->Hide();
}

void mxNewWizard::CreatePanelOnProject() {
	panel_onproject = new wxPanel(this,wxID_ANY);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxString choices[4];
	choices[0] = LANG(NEWWIZARD_SOURCE,"Archivo Fuente ");
	choices[1] = LANG(NEWWIZARD_HEADER,"Archivo Cabecera ");
	choices[2] = LANG(NEWWIZARD_OTHER,"Otro Archivo ");
	choices[3] = LANG(NEWWIZARD_CLASs,"Clase ");
	
	onproject_radio= new wxRadioBox(panel_onproject, mxID_WIZARD_ONPROJECT_RADIO, LANG(NEWWIZARD_FILETYPE,"Tipo de Archivo:"),
					wxDefaultPosition, wxDefaultSize,
					WXSIZEOF(choices), choices,
					1, wxRA_SPECIFY_COLS);
	onproject_radio->SetSelection(3);
	onproject_radio->SetItemToolTip(0,LANG(NEWWIZARD_TIP_SOURCE,"Crea un nuevo archivo fuente en blanco."));
	onproject_radio->SetItemToolTip(1,LANG(NEWWIZARD_TIP_HEADER,"Crea un nuevo archivo de cabecera en blanco."));
	onproject_radio->SetItemToolTip(2,LANG(NEWWIZARD_TIP_OTHER,"Crea un nuevo archivo en blanco."));
	onproject_radio->SetItemToolTip(3,LANG(NEWWIZARD_TIP_CLASS,"Crea dos archivos correspondientes a una clase\n(un archivo fuente y un archivo de cabecera)\ny completa una plantilla de clase."));
	
	sizer->Add(onproject_radio);
	
	onproject_label = new wxStaticText(panel_onproject,wxID_ANY,LANG(NEWWIZARD_FILENAME,"Nombre del archivo:"));
	onproject_name = new wxTextCtrl(panel_onproject,mxID_WIZARD_ONPROJECT_NAME,"");

	sizer->Add(onproject_label,sizers->BT10);
	wxBoxSizer *sizer_name = new wxBoxSizer(wxHORIZONTAL);
	sizer_name->Add(onproject_name,sizers->Exp1);
	wxButton *button_name = new wxButton(panel_onproject,mxID_WIZARD_NEW_FILE_PATH,"...",wxDefaultPosition,wxSize(30,10));	
	sizer_name->Add(button_name,sizers->Exp0);
	sizer->Add(sizer_name,sizers->BL10_Exp0);
	
	wxBoxSizer *cdSizer = new wxBoxSizer(wxHORIZONTAL);
	onproject_inherit_include = new wxStaticText(panel_onproject,wxID_ANY,LANG(NEWWIZARD_INCLUDE,"Incluir"));
	onproject_const_def = new wxCheckBox(panel_onproject,wxID_ANY,LANG(NEWWIZARD_DEFAULT_CONSTRUCTOR,"Ctor. por Defecto"));
	onproject_const_def->SetToolTip(LANG(NEWWIZARD_TIP_DEFAULT_COSNTRUCTOR,"Incluye un constructor que no recibe ningun parametro."));
	onproject_const_def->SetValue(true);
	onproject_const_copy = new wxCheckBox(panel_onproject,wxID_ANY,LANG(NEWWIZARD_COPY_CONSTRUCTOR,"Ctor. de Copia"));
	onproject_const_copy->SetToolTip(LANG(NEWWIZARD_TIP_COPY_CONSTRUCTOR,"Incluye un destructor de copia (recibe un objeto de su misma clase)."));
	onproject_dest = new wxCheckBox(panel_onproject,wxID_ANY,LANG(NEWWIZARD_DESTRUCTOR,"Destructor"));
	onproject_dest->SetToolTip(LANG(NEWWIZARD_TIP_DESTRUCTOR,"Incluye un destructor para la clase."));
	onproject_dest->SetValue(true);
	cdSizer->Add(onproject_inherit_include,sizers->BR10);
	cdSizer->Add(onproject_const_def,sizers->BR10);
	cdSizer->Add(onproject_const_copy,sizers->BR10);
	cdSizer->Add(onproject_dest);
	sizer->Add(cdSizer,sizers->BT10_Exp0);

	onproject_inherit_label = new wxStaticText(panel_onproject,wxID_ANY,LANG(NEWWIZARD_INHERIT_FROM,"Heredar de"));
	sizer->Add(onproject_inherit_label,sizers->BT5);
	wxArrayString inherit_choices;
	inherit_choices.Add("public");
	inherit_choices.Add("protected");
	inherit_choices.Add("private");
	for (int i=0;i<config->Init.inherit_num;i++) {
		wxBoxSizer *inhSizer = new wxBoxSizer(wxHORIZONTAL);
		onproject_inherit_visibility[i] = new wxComboBox(panel_onproject,wxID_ANY,"public",wxDefaultPosition,wxDefaultSize,inherit_choices,wxCB_READONLY);
		onproject_inherit_class[i] = new wxTextCtrl(panel_onproject,wxID_ANY,"");
		inhSizer->Add(onproject_inherit_visibility[i]);
		inhSizer->Add(onproject_inherit_class[i],sizers->BL5_Exp1);
		sizer->Add(inhSizer,sizers->BL10_Exp0);
	}

	panel_onproject->SetSizerAndFit(sizer);
	panel_onproject->Hide();
}

void mxNewWizard::ShowPanelTemplates() {
	SetCurrentPanel(panel_templates);
	// select default template
	wxArrayString templates_files; 
	g_templates->GetFilesList(templates_files,true,true);
	int idx = templates_files.Index(config->Files.default_template);
	if (idx!=wxNOT_FOUND) templates_list->SetSelection(idx);
	// restor buttons and checkbox
	templates_check->SetValue(false);
	nextButton->SetThings(bitmaps->buttons.ok,LANG(NEWWIZARD_CREATE," Crear "));
	cancelButton->SetThings(bitmaps->buttons.prev,LANG(NEWWIZARD_BACK," Volver "));
	// fix sizers and set the focus to the list
	GetSizer()->Layout();
	templates_list->SetFocus();
}

void mxNewWizard::CreatePanelTemplates() {
	
	panel_templates = new wxPanel(this,wxID_ANY);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxArrayString templates_names, templates_files;
	g_templates->GetNamesList(templates_names,true,true);
	g_templates->GetFilesList(templates_files,true,true);
	int default_idx = templates_files.Index(config->Files.default_template);
	templates_list = new wxListBox(panel_templates,wxID_ANY,wxDefaultPosition, wxDefaultSize, templates_names);
	if (default_idx!=wxNOT_FOUND) templates_list->SetSelection(default_idx);
	
	templates_check = new wxCheckBox(panel_templates, wxID_ANY, LANG(NEWWIZARD_SET_DEFAULT,"Guardar como predeterminada"),wxDefaultPosition,wxSize(250,50));
	templates_check->SetValue(false);
	
	wxStaticText *text = new wxStaticText(panel_templates,wxID_ANY,LANG(NEWWIZARD_CHOOSE_TEMPLATE,"Seleccione una plantilla:"));
	
	sizer->Add(text,sizers->Exp0);
	sizer->Add(templates_list,sizers->BL10_Exp1);
	sizer->Add(templates_check,sizers->Exp0);
	
	panel_templates->SetSizerAndFit(sizer);
	panel_templates->Hide();
	
}

void mxNewWizard::ShowPanelProject1() {
	SetCurrentPanel(panel_project_1);
	if (current_wizard_mode==WM_Import) {
		project_folder_radio->SetSelection(3);
		project_folder_create->SetValue(false);
		project_folder_create->Hide();
		project_folder_radio->Hide();
	} else {
		project_folder_create->Show();
		project_folder_radio->Show();
	}
	project_check->SetValue(false);
	project_list->SetSelection(project_default);
	nextButton->SetThings(bitmaps->buttons.next,LANG(NEWWIZARD_NEXT," Siguiente "));
	if (only_for_project)
		cancelButton->SetThings(bitmaps->buttons.cancel,LANG(NEWWIZARD_CANCEL," Cancelar "));
	else
		cancelButton->SetThings(bitmaps->buttons.prev,LANG(NEWWIZARD_BACK," Volver "));
	if (project_folder_radio->GetSelection()==1)
		project_folder_path->SetValue(config->Files.last_dir);
	UpdateProjectFullPath();
	GetSizer()->Layout();
	project_name->SetFocus();
	if (current_wizard_mode==WM_Import) { wxCommandEvent event; OnButtonFolder(event); }
}

void mxNewWizard::ShowPanelProject2() {
	SetCurrentPanel(panel_project_2);
	project_check->SetValue(false);
	project_list->SetSelection(project_default);
	nextButton->SetThings(bitmaps->buttons.ok,LANG(NEWWIZARD_CREATE," Crear "));
	cancelButton->SetThings(bitmaps->buttons.prev,LANG(NEWWIZARD_BACK," Volver "));
	if (!main_window->notebook_sources->GetPageCount()) {
		project_current_files->Hide();
		project_current_files->SetValue(false);
	} else
		project_current_files->Show();
	if (project_folder_create->GetValue()) {
		project_dir_files->Hide();
		project_dir_files->SetValue(false);
	} else if (current_wizard_mode==WM_Import) { // create zpr for a existing project
		project_current_files->SetValue(false);
		project_current_files->Hide();
		project_dir_files->Hide();
		project_dir_files->SetValue(true);
	} else {
		project_dir_files->Show();
	}
	GetSizer()->Layout();
	project_list->SetFocus();
}

void mxNewWizard::ShowPanelOnProject() {
	SetCurrentPanel(panel_onproject);
	nextButton->SetThings(bitmaps->buttons.ok,LANG(NEWWIZARD_CREATE," Crear "));
	cancelButton->SetThings(bitmaps->buttons.cancel,LANG(NEWWIZARD_CANCEL," Cancelar "));
	GetSizer()->Layout();
	onproject_name->SetFocus();
	Fit(); CenterOnParent();
}

void mxNewWizard::CreatePanelProject1() {
	panel_project_1 = new wxPanel(this,wxID_ANY);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxStaticText(panel_project_1,wxID_ANY,LANG(NEWWIZARD_PROJECT_FILENAME,"Nombre del Archivo de Proyecto:"),wxDefaultPosition,wxDefaultSize));
	project_name = new wxTextCtrl(panel_project_1, mxID_WIZARD_PROJECT_NAME_TEXT,LANG(NEWWIZARD_MYPROJECT,"MiProyecto"),wxDefaultPosition,wxDefaultSize);
	sizer->Add(project_name,sizers->Exp0);
	
	sizer->Add(new wxStaticText(panel_project_1,wxID_ANY,LANG(NEWWIZARD_PROJECT_PATH,"Ubicacion del Proyecto:"),wxDefaultPosition,wxDefaultSize),sizers->BT10_Exp0);
	project_folder_path = new wxTextCtrl(panel_project_1,mxID_WIZARD_PROJECT_FOLDER_TEXT,DIR_PLUS_FILE(config->zinjai_dir,config->Files.project_folder),wxDefaultPosition,wxDefaultSize);
	wxButton *folders = new wxButton(panel_project_1,mxID_WIZARD_FOLDER,"...",wxDefaultPosition,wxSize(30,10));	
	
	wxString choices[4];
	choices[0] = LANG(NEWWIZARD_PROJECTS_DIR,"Directorio de Proyectos");
	choices[1] = LANG(NEWWIZARD_CURRENT_DIR,"Directorio Actual");
	choices[2] = LANG(NEWWIZARD_LAST_DIR,"Ultimo Directorio Utilizado");
	choices[3] = LANG(NEWWIZARD_OTHER_PATH,"Otro");
	
	project_folder_radio = new wxRadioBox(panel_project_1, mxID_WIZARD_PROJECT_PATH_RADIO, LANG(NEWWIZARD_PATH,"Ubicacion:"),
		wxDefaultPosition, wxDefaultSize,
		WXSIZEOF(choices), choices,
		1, wxRA_SPECIFY_COLS);
	
	project_folder_radio->SetSelection(0);
	sizer->Add(project_folder_radio,sizers->Exp0);
	
	wxBoxSizer *fsizer = new wxBoxSizer(wxHORIZONTAL);
	fsizer->Add(project_folder_path,sizers->Exp1);
	fsizer->Add(folders,sizers->Exp0);
	sizer->Add(fsizer,sizers->Exp0);

	project_folder_create = new wxCheckBox(panel_project_1, mxID_WIZARD_PROJECT_FOLDER_CHECK, LANG(NEWWIZARD_NEW_FOLDER,"Crear un nuevo directorio"));
	project_folder_create->SetValue(true);
	sizer->Add(project_folder_create);
	
	project_full_path = new wxTextCtrl(panel_project_1,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY|wxBORDER_NONE);
	project_full_path->SetBackgroundColour(panel_project_1->GetBackgroundColour());
//	project_full_path->Enable(false);
	sizer->Add(project_full_path,sizers->BT10_Exp0);

	panel_project_1->SetSizerAndFit(sizer);
	panel_project_1->Hide();
	
}

void mxNewWizard::CreatePanelProject2() {
	panel_project_2 = new wxPanel(this,wxID_ANY);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	wxArrayString templates;
	project_default=1;
	project_templates.Add("<null>");
	project_templates.Add("<main>");
	templates.Add(LANG(NEWWIZARD_TEMPLATE_EMPTY,"<proyecto en blanco>"));
	templates.Add(LANG(NEWWIZARD_TEMPLATE_MAIN,"<incluir archivo y funcion main>"));
	if (project_templates[0]==config->Files.default_project)
		project_default=0;
	
	wxArrayString templates_array;
	mxUT::GetFilesFromBothDirs(templates_array,"templates",false);
	
	for(unsigned int i=0;i<templates_array.GetCount();i++) {
		wxString name = templates_array[i];
		wxString full = mxUT::WichOne(name,"templates",false);
		if (wxFileName::FileExists(DIR_PLUS_FILE(full,name+DOT_PROJECT_EXT))) {
			if (name==config->Files.default_project) project_default=i;
			wxTextFile file(DIR_PLUS_FILE(full,name+"."+_T(PROJECT_EXT)));
			file.Open();
			for (wxString line=file.GetFirstLine();!file.Eof();line=file.GetNextLine()) {
				if (line.Left(13)=="project_name=") {
					name = line.Mid(13).Trim(true).Trim(false);
					break;
				}
			}
			file.Close();
			project_templates.Add(templates_array[i]);
			templates.Add(name);
		}
	}	

	sizer->Add(new wxStaticText(panel_project_2,wxID_ANY,LANG(NEWWIZARD_USE_TEMPLATE,"Utilizar plantilla"),wxDefaultPosition,wxDefaultSize),sizers->BT10);
	project_list = new wxListBox(panel_project_2,wxID_ANY,wxDefaultPosition, wxDefaultSize, templates);
	sizer->Add(project_list,sizers->BL10_Exp1);

	project_check = new wxCheckBox(panel_project_2, wxID_ANY,LANG(NEWWIZARD_SET_DEFAULT,"Guardar como predeterminada"));
	sizer->Add(project_check);

	project_current_files= new wxCheckBox(panel_project_2, mxID_WIZARD_PROJECT_FILES_OPEN_CHECK, LANG(NEWWIZARD_USE_OPEN_FILES,"Utilizar los archivos abiertos"));
	project_current_files->SetValue(false);
	sizer->Add(project_current_files,sizers->BT10);
	project_dir_files= new wxCheckBox(panel_project_2, mxID_WIZARD_PROJECT_FILES_DIR_CHECK, LANG(NEWWIZARD_USE_DIR_FILES,"Utilizar los archivos del directorio de proyecto"));
	project_dir_files->SetValue(false);
	sizer->Add(project_dir_files,sizers->BT10);
	
	panel_project_2->SetSizerAndFit(sizer);
	panel_project_2->Hide();
	
}

void mxNewWizard::RunWizard(wxString how) {
	if (how=="new_project") {
		only_for_project=true;
		ShowPanelProject1();
	} else if (how=="on_project") {
		ShowPanelOnProject();
	} else if (how=="templates") {
		ShowPanelTemplates();
	} else {
		only_for_project=false;
		ShowPanelStart(true);
	}
	CenterOnParent(); Show();
}

void mxNewWizard::OnButtonFolder(wxCommandEvent &event){
	wxDirDialog dlg(this,LANG(NEWWIZARD_CHOOSE_PATH,"Seleccione la ubicacion:"),DIR_PLUS_FILE(config->zinjai_dir,project_folder_path->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		project_folder_path->SetValue(dlg.GetPath());
		project_folder_radio->SetSelection(3);
	}
}

void mxNewWizard::OnButtonHelp(wxCommandEvent &event){
	wxString file;
	if (panel==panel_onproject)
		file = "new_wizard.html#on_project";
	else if (panel==panel_project_1)
		file = "new_wizard.html#project";
	else if (panel==panel_project_2)
		file = "new_wizard.html#project";
	else if (panel==panel_start)
		file = "new_wizard.html#start";
	else if (panel==panel_templates)
		file = "new_wizard.html#templates";
//	else if (panel==panel_wizard_1 || panel==panel_wizard_2)
//		file = "new_wizard.html#wizard";
	mxHelpWindow::ShowHelp(file);
}


void mxNewWizard::OnProjectPathRadio(wxCommandEvent &event){
	BoolFlagGuard fg(mask_folder_change_events);
	switch (project_folder_radio->GetSelection()) {
	case 0:
		project_folder_create->SetValue(true);
		project_folder_path->SetValue(DIR_PLUS_FILE(config->zinjai_dir,config->Files.project_folder));
		break;
	case 1:
		project_folder_create->SetValue(false);
		project_folder_path->SetValue(config->Files.last_dir);
		break;
	case 2:
		project_folder_create->SetValue(true);
		project_folder_path->SetValue(config->Files.last_project_dir);
		break;
	case 3:
		project_folder_create->SetValue(true);
		OnButtonFolder(event);
		break;
	}
}

void mxNewWizard::OnOnProjectNameChange(wxCommandEvent &evt) {
	wxString name=onproject_name->GetValue();
	if (name.Contains("/")) name=name.AfterLast('/');
	if (name.Contains("\\")) name=name.AfterLast('\\');
	if (name.Contains(".")) {
		eFileType t = mxUT::GetFileType(name,false);
		if (t==FT_SOURCE) onproject_radio->SetSelection(0);
		else if (t==FT_HEADER) onproject_radio->SetSelection(1);
		else onproject_radio->SetSelection(2);
	}
}

void mxNewWizard::OnProjectFolderChange(wxCommandEvent &evt) {
	if (!mask_folder_change_events) project_folder_radio->SetSelection(3);
	UpdateProjectFullPath();
}

void mxNewWizard::OnProjectFolderCheck(wxCommandEvent &evt) {
	UpdateProjectFullPath();
}

void mxNewWizard::OnProjectNameChange(wxCommandEvent &evt) {
	UpdateProjectFullPath();
}

void mxNewWizard::UpdateProjectFullPath() {
	if (!project_full_path) return;
	if (project_folder_create->GetValue())
		project_full_path->SetValue(wxString(LANG(NEWWIZARD_FINAL_PATH,"Destino final: "))<<DIR_PLUS_FILE_2(project_folder_path->GetValue(),project_name->GetValue(),project_name->GetValue())+"."+_T(PROJECT_EXT));
	else
		project_full_path->SetValue(wxString(LANG(NEWWIZARD_FINAL_PATH,"Destino final: "))<<DIR_PLUS_FILE(project_folder_path->GetValue(),project_name->GetValue())+"."+_T(PROJECT_EXT));	
}

void mxNewWizard::OnButtonNewFilePath(wxCommandEvent &evt) {
	wxFileName fname(DIR_PLUS_FILE(project->path,onproject_name->GetValue()));
	wxFileDialog dlg(this, LANG(NEWWIZARD_CREATE_FILE,"Crear Archivo"), fname.GetPath(), fname.GetFullName(), wxString(LANG(WILDCARD_ANY,"Todos los archivos"))<<"|*", wxFD_SAVE);
	switch (onproject_radio->GetSelection()) {
	case 0:
		dlg.SetWildcard(wxString(LANG(WILDCARD_SOURCES,"Fuentes"))<<"|"WILDCARD_SOURCE"|"<<LANG(WILDCARD_ANY,"Todos los archivos")<<"|*");
		break;
	case 1:
		dlg.SetWildcard(wxString(LANG(WILDCARD_HEADERS,"Cabeceras"))<<"|"WILDCARD_HEADER"|"<<LANG(WILDCARD_ANY,"Todos los archivos")<<"|*");
		break;
	}
	if (dlg.ShowModal() == wxID_OK) {
		fname=dlg.GetPath();
		fname.MakeRelativeTo(project->path);
		if (onproject_radio->GetSelection()==3) 
			fname.SetExt("");
		onproject_name->SetValue(fname.GetFullPath());
	}
}

void mxNewWizard::OnProjectFilesOpenCheck(wxCommandEvent &evt) {
	project_dir_files->SetValue(false);
}

void mxNewWizard::OnProjectFilesDirCheck(wxCommandEvent &evt) {
	project_current_files->SetValue(false);
}

void mxNewWizard::SetCurrentPanel(wxPanel *new_panel) {
	panel->Hide();
	new_panel->Show();
	sizer_for_panel->Replace(panel,new_panel,false);
	panel=new_panel;
}

