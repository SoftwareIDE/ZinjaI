#include "mxDrawClasses.h"
#include "mxMainWindow.h"
#include "parserData.h"
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxSource.h"
#include "mxSizers.h"
#include "Language.h"
#include "mxOSD.h"
#include <wx/textfile.h>
#include "execution_workaround.h"

BEGIN_EVENT_TABLE(mxDrawClasses, wxDialog)
	EVT_BUTTON(wxID_OK,mxDrawClasses::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxDrawClasses::OnCancelButton)
//	EVT_CHECKBOX(mxID_BREAK_OPTS_ENABLE,mxBreakOptions::OnBreakpointCheck)
	EVT_CLOSE(mxDrawClasses::OnClose)
END_EVENT_TABLE()
	
void mxDrawClasses::OnClose(wxCloseEvent &event) {
	Destroy();
}

	
mxDrawClasses::mxDrawClasses() : wxDialog(main_window, wxID_ANY, LANG(DRAWCLASSES_CAPTION,"Dibujar relaciones entre clases"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	// buscar el item de la lista que se corresponde
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	SetEscapeId(wxID_CANCEL);
	ok_button->SetDefault(); 
	buttonSizer->Add(cancel_button,sizers->BA5_Right);
	buttonSizer->Add(ok_button,sizers->BA5_Right);
	
	wxArrayString array1;
	array1.Add(LANG(DRAWCLASSES_RELATED_NONE,"No incluir"));
	array1.Add(LANG(DRAWCLASSES_RELATED_DIRECT,"Incluir solo las relacionadas directamente"));
	array1.Add(LANG(DRAWCLASSES_RELATED_ALL,"Incluir todas la relacionadas"));
	related_classes = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_RELATED,"Clases externas"),array1,1);

	wxArrayString array2;
	array2.Add(_T("dot"));
	array2.Add(_T("fdp"));
//	array2.Add(_T("neato"));
//	array2.Add(_T("twopi"));
//	array2.Add(_T("circo"));
	wich_command = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_GENERADOR,"Generador"),array2,0);
	
	wxArrayString array3;
	pd_class *it_class = parser->last_class->next;
	while (it_class) {
		array3.Add(it_class->name);
		it_class = it_class->next;
	}
	array3.Sort();
	array3.Add(LANG(DRAWCLASSES_WICH_ALL,"<todas las clases>"));
	wich_class = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_DRAW_FROM,"Dibujar relaciones de:"),array3,array3.GetCount()-1);
	
	wxArrayString array4;
	pd_file *it_file = parser->last_file->next;
	while (it_file) {
		array4.Add(it_file->name);
		it_file = it_file->next;
	}
	array4.Sort();
	array4.Add(LANG(DRAWCLASSES_FILES_ALL,"<todos los archivos>"));
	wich_file = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_FROM_FILE,"Dibujar relaciones de:"),array4,array4.GetCount()-1);
	if (!project && main_window->notebook_sources->GetPageCount()) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		wich_file->SetValue(source->GetFullPath());
	}	

	wxArrayString array5;
	array5.Add(LANG(DRAWCLASSES_WHAT_NAME,"Solo Nombre"));
	array5.Add(LANG(DRAWCLASSES_WHAT_PUBLIC_NAME,"Atributos y Metodos Publicos (solo nombre)"));
	array5.Add(LANG(DRAWCLASSES_WHAT_PUBLIC_FULL,"Atributos y Metodos Publicos (prototipo completo)"));
	array5.Add(LANG(DRAWCLASSES_WHAT_ALL_NAME,"Todos los Atributos y Metodos (solo nombre)"));
	array5.Add(LANG(DRAWCLASSES_WHAT_ALL_FULL,"Todos los Atributos y Metodos (prototipo completo)"));
	what_inside = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_WHAT,"Mostrar en cada clase"),array5,4);
	
	wxArrayString array6;
	array6.Add(LANG(DRAWCLASSES_WHERE_SHOW,"Visualizar"));
	array6.Add(LANG(DRAWCLASSES_WHERE_PNG,"Guardar como imagen png"));
	array6.Add(LANG(DRAWCLASSES_WHERE_JPG,"Guardar como imagen jpg"));
//	array6.Add(LANG(DRAWCLASSES_WHERE_BMP,"Guardar como imagen bmp"));
	array6.Add(LANG(DRAWCLASSES_WHERE_PDF,"Guardar como documento pdf"));
	array6.Add(LANG(DRAWCLASSES_WHERE_PS,"Guardar como documento ps"));
	array6.Add(LANG(DRAWCLASSES_WHERE_SVG,"Guardar como dibujo svg"));
	array6.Add(LANG(DRAWCLASSES_WHERE_DIA,"Guardar como diagrama dia"));
	where_store = mxUT::AddComboBox(mySizer,this,LANG(DRAWCLASSES_WHERE,"Resultado"),array6,0);
	
	mySizer->Add(buttonSizer,sizers->Right);
	SetSizerAndFit(mySizer);
	what_inside->SetFocus();
	ShowModal();
}

void mxDrawClasses::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxDrawClasses::OnOkButton(wxCommandEvent &evt) {
	// obtener la jerarquia de clases
	bool one_class = wich_class->GetSelection()!=int(wich_class->GetCount())-1;
	wxString the_one; // nombre de la clase o del archivo sobre el cual generar
	if (wich_file->GetSelection()!=int(wich_file->GetCount())-1) the_one = wich_file->GetValue();
	if (wich_class->GetSelection()!=int(wich_class->GetCount())-1) the_one = wich_class->GetValue();
	bool inside = what_inside->GetSelection()>0, only_public = what_inside->GetSelection()<3;
	bool only_name = what_inside->GetSelection()==1||what_inside->GetSelection()==3;
	bool related = related_classes->GetSelection()>0, indirect = related_classes->GetSelection()>1;
	if (!GenerateGraph(the_one,one_class,related,indirect,inside,only_public,only_name)) {
		mxMessageDialog(this,LANG(DRAWCLASSES_NO_COINCIDENCE_FOR_CLASS,"No se encontraron clases para las condiciones seleccionadas."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	// generar grafo con dot/fdp y guardar/mostrar
	wxString gout=DIR_PLUS_FILE(config->temp_dir,"graph.dot"), output; // graph_viz input (gout) y archivo de salida para exportar el resultado (output, vacio para solo visualizar)
	if (where_store->GetSelection()) {
		wxString ext = where_store->GetValue().AfterLast(' ');
		wxFileDialog dlg (this, LANG(DRAW_CLASSES_SAVE,"Guardar grafo"), project?DIR_PLUS_FILE(project->path,project->last_dir):config->Files.last_dir, " ", where_store->GetValue().AfterFirst(' ').AfterFirst(' ')+_T("|*.")+ext, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) return;
		output=dlg.GetPath();
	}
	mxUT::ProcessGraph(gout,wich_command->GetStringSelection()=="fdp","",LANG(DRAW_CLASSES_TITLE,"Jerarquía de Clases"));
}

/**
* Genera el grafo de jerarquia de clases para el fuente file y lo guarda en .zinjai/graph.dot
**/

int mxDrawClasses::GenerateGraph(wxString file, bool one_class, bool related, bool indirect, bool inside, bool only_public, bool only_name) {
	
	wxTextFile fil(DIR_PLUS_FILE(config->temp_dir,_T("graph.dot")));
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	fil.AddLine(_T("digraph g {"));
	fil.AddLine(_T("\trankdir=BT;"));
	
	wxArrayString classes,relationships;
	wxArrayString fathers,sons;
	
	if (file.Len()) {
		if (one_class) {
			classes.Add(file);
			sons.Add(file);
			fathers.Add(file);
		} else {
			pd_file *it_file = parser->last_file->next;
			while (it_file) {
				if (it_file->name==file) {
					pd_ref *class_ref = it_file->first_class->next;
					while (class_ref) {
						classes.Add(PD_UNREF(pd_class,class_ref)->name);
						fathers.Add(PD_UNREF(pd_class,class_ref)->name);
						sons.Add(PD_UNREF(pd_class,class_ref)->name);
						class_ref = class_ref->next;
					}
					break;
				}
				it_file = it_file->next;
			}
		}
		
		if (related) {
			int added=1,from=0;
			while (added) {
				int nf = sons.GetCount();
				added=0;
				for (int i=from;i<nf;i++) {
					pd_inherit *item=parser->first_inherit;
					while (item->next) {
						item=item->next;
						if (item->father==sons[i]) {
							int pos = sons.Index(item->son);
							if (pos==wxNOT_FOUND || pos>=nf) {
								added++;
								if (pos==wxNOT_FOUND && classes.Index(item->son)==wxNOT_FOUND) 
									sons.Add(item->son);
								relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
							} else if (from==0)
								relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
						}
					}
				}
				if (!indirect) break;
				from=nf;
			}
		
			added=1;from=0;
			while (added) {
				int nf = fathers.GetCount();
				added=0;
				for (int i=from;i<nf;i++) {
					pd_inherit *item=parser->first_inherit;
					while (item->next) {
						item=item->next;
						if (item->son==fathers[i]) {
							int pos = fathers.Index(item->father);
							if (pos==wxNOT_FOUND || pos>=nf) {
								added++;
								if (pos==wxNOT_FOUND && classes.Index(item->father)==wxNOT_FOUND) 
									fathers.Add(item->father);
								relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
							} else if (from==0  && classes.Index(item->father)==wxNOT_FOUND)
								relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
						}
					}
				}
				if (!indirect) break;
				from=nf;
			}
			
		}		
		
	} else {
		
		pd_class *cls=parser->last_class->next;
		while (cls) {
			classes.Add(cls->name);
			cls=cls->next;
		}
		
		pd_inherit *item=parser->first_inherit;
		while (item->next) {
			item=item->next;
			if (classes.Index(item->son)!=wxNOT_FOUND) {
				if (classes.Index(item->father)==wxNOT_FOUND && fathers.Index(item->father)==wxNOT_FOUND) {
					if (related) {
						fathers.Add(item->father);
						relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
					}
				} else 
					relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
			} else if (classes.Index(item->father)!=wxNOT_FOUND) {
				if (classes.Index(item->son)==wxNOT_FOUND && fathers.Index(item->son)==wxNOT_FOUND) {
					if (related) {
						fathers.Add(item->son);
						relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
					}
				} else
					relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
			}
		}
		
		if (indirect) {
			int added=1,from=0;
			while (added) {
				int nf = fathers.GetCount();
				added=0;
				for (int i=from;i<nf;i++) {
					pd_inherit *item=parser->first_inherit;
					while (item->next) {
						item=item->next;
						if (item->son==fathers[i]) {
							int pos = fathers.Index(item->father);
							if (pos==wxNOT_FOUND || pos>=nf) {
								added++;
								if (pos==wxNOT_FOUND) 
									fathers.Add(item->father);
								relationships.Add(wxString("\t\"")<<item->son<<_T("\"->\"")<<item->father<<_T("\"[arrowhead=onormal];"));
							}
						}
					}
				}
				from=nf;
			}
		}
	}
	
	for (unsigned int i=file.Len()?classes.GetCount():0;i<sons.GetCount();i++)
		fil.AddLine(wxString("\t\"")<<sons[i]<<_T("\"[shape=box,color=gray];"));
	for (unsigned int i=file.Len()?classes.GetCount():0;i<fathers.GetCount();i++)
		fil.AddLine(wxString("\t\"")<<fathers[i]<<_T("\"[shape=box,color=gray];"));
	for (unsigned int i=0;i<classes.GetCount();i++) {
		if (!inside)
			fil.AddLine(wxString("\t\"")<<classes[i]<<_T("\"[shape=box];"));
		else {
			wxString str("\t"), name=classes[i];
			str<<"\""<<name<<"\"[shape=record,label=\"{"<<classes[i]<<"|";
			pd_class *it_class= parser->last_class->next;
			while (it_class) {
				if (it_class->name==classes[i]) break;
				it_class = it_class->next;
			}
			if (it_class) {
				pd_var *it_var = it_class->first_attrib->next;
				while (it_var) {
					if (!only_public || it_var->properties&PD_CONST_PUBLIC) {
						if (it_var->properties&PD_CONST_PUBLIC) 
							str<<_T("+ ")<<(only_name?it_var->name:it_var->proto)<<_T("\\l");
						else if (it_var->properties&PD_CONST_PROTECTED) 
							str<<_T("# ")<<(only_name?it_var->name:it_var->proto)<<_T("\\l");
						else if (it_var->properties&PD_CONST_PRIVATE) 
							str<<_T("- ")<<(only_name?it_var->name:it_var->proto)<<_T("\\l");
						else
							str<<_T("? ")<<(only_name?it_var->name:it_var->proto)<<_T("\\l");
					}
					it_var = it_var->next;
				}
			}
			str<<_T("|");
			if (it_class) {
				pd_func *it_func = it_class->first_method->next;
				while (it_func) {
					if (!only_public || it_func->properties&PD_CONST_PUBLIC) {
						if (it_func->properties&PD_CONST_PUBLIC)
							str<<_T("+ ")<<(only_name?it_func->name:it_func->proto)<<_T("\\l");
						else if (it_func->properties&PD_CONST_PROTECTED)
							str<<_T("# ")<<(only_name?it_func->name:it_func->proto)<<_T("\\l");
						else if (it_func->properties&PD_CONST_PRIVATE)
							str<<_T("- ")<<(only_name?it_func->name:it_func->proto)<<_T("\\l");
						else
							str<<_T("? ")<<(only_name?it_func->name:it_func->proto)<<_T("\\l");
					}
					it_func = it_func->next;
				}
			}
			str<<_T("}\"];");
			str.Replace(_T("<"),_T("\\<"));
			str.Replace(_T(">"),_T("\\>"));
			fil.AddLine(str);
		}
	}
	for (unsigned int i=0;i<relationships.GetCount();i++)
		fil.AddLine(relationships[i]);
	
	fil.AddLine(_T("}"));
	
	fil.Write();
	fil.Close();
	
	return classes.GetCount()+fathers.GetCount()+sons.GetCount();
}


