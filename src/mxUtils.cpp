#include <wx/checkbox.h>
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#include <wx/combobox.h>
#include <wx/textfile.h>
#include <wx/dir.h>
#include <wx/clipbrd.h>

#include "ids.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxArt.h"
#include "mxSizers.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "Language.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"

#define CTRL_BORDER 5

#define min2(a,b) ((a)<(b)?(a):(b))
#include "execution_workaround.h"
#include "MenusAndToolsConfig.h"

bool zinjai_debug_mode=false;

#ifdef _ZINJAI_DEBUG
wxString debug_string;
#endif 
char path_sep = wxFileName::GetPathSeparator();

static wxStaticText *last_label; ///< guarda la ultima etiqueta que se uso en alguno de los AddAlgo
static wxButton *last_button; ///< guarda el ultimo boton colocado por AddDirCtrl

/** 
* Concatena una ruta y un nombre de archivo. Si este es relativo, agregando la 
* barra si es necesario. Si el "archivo" era una ruta absoluta, la devuelve sin cambios.
* Esta función se creó para encapsular las diferencias entre Windows y GNU/Linux,
* particularmente, el caracter de separación, y la presencia/ausencia de la unidad.
* La macro DIR_PLUS_FILE encapsula esta llamada utilizando el objeto utils.
* @param dir El directorio base
* @param fil El archivo. Puede ser solo un nombre, la parte final de una ruta, o una ruta completa
* @return Un wxString con la ruta resultante completa
**/
wxString mxUT::JoinDirAndFile(wxString dir, wxString fil) {
	if (dir.Len()==0 || (fil.Len()>1 && (fil[0]=='\\' || fil[0]=='/' || fil[1]==':')))
		return fil;
	else if (dir.Last()==path_sep)
		return dir+fil;
	else
		return dir+path_sep+fil;
}

/*wxBoxSizer *mxUT::CreateSizerWithLabelAndText(wxWindow *win, wxWindowID id, wxTextCtrl * &text, wxString label) {
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	text = new wxTextCtrl(win, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizerRow->Add(new wxStaticText(win, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->Add(text, sizers->Exp1);
	return sizerRow;
}*/

void mxUT::strcpy(wxChar *cstr, wxString wstr) {
	unsigned int i;
	for (i=0;i<wstr.Len();i++)
		cstr[i]=wstr[i];
	cstr[i]='\0';
} 

void mxUT::strcpy(wxChar *cstr, wxChar *wstr) {
	unsigned int i=0;
	while (wstr[i]!='\0') {
		cstr[i]=wstr[i];
		i++;
	}
	cstr[i]='\0';
} 

void mxUT::SortArrayString(wxArrayString &array, int inf, int sup) {

	if (inf==-1) { inf=0; sup=array.GetCount()-1; if (sup==-1) return; }
/*	cout<<endl;
	cout<<"      "<<array[sup]<<endl;*/
		
	if(inf==sup) return;
	int oinf=inf,osup=sup--;
	wxString med=array[osup],aux;
	int i,l,jl,ml=med.Len();
	bool bi,bs;
	i=0;
	for (i=0;i<ml;i++)
		med[i]|=32;
	while (true) {
// for (int i=oinf;i<=osup;i++) {
// 	if (i==inf) cout<<"--";
// 	else if (i==sup) cout<<"++";
// 	else cout<<"  ";
// 	cout<<array[i]<<"  ";
// }
// cout<<endl;

		while (true) {
			jl=array[inf].Len(); l=(jl<ml?jl:ml)-2;
			for (i=0;i<l;i++) {
				if ( (array[inf][i]|32)!=med[i] ) {
					bi=( (array[inf][i]|32)<med[i] );
					break;
				}
			}
			if (i==l)
				bi=jl<ml;
			if (bi && inf!=sup)
				inf++;
			else
				break;
		}

		while (true) {
			jl=array[sup].Len(); l=(jl<ml?jl:ml)-2;
			for (i=0;i<l;i++) {
				if ( (array[sup][i]|32)!=med[i] ) {
					bs=( (array[sup][i]|32)>med[i] );
					break;
				}
			}
			if (i==l)
				bs=jl>=ml;
			if (bs && inf!=sup)
				sup--;
			else
				break;
		}

		if (inf<sup) {
			aux=array[inf];
			array[inf]=array[sup];
			array[sup]=aux;
		} else {
			if(!bi) {
				aux=array[inf];
				array[inf]=array[osup];
				array[osup]=aux;
			} else
				inf++;

// if (oi>=0&&os>=0) {
// cout<<oi<<" "<<os<<" "<<med<<endl; 
// for (int i=oi;i<=os;i++)
// 	 cout<<array[i]<<endl;
// cout<<endl;
// }
			if (inf-oinf>1)
				SortArrayString(array,oinf,inf-1);
			if (osup-sup>1)
				SortArrayString(array,sup+1,osup);
			return;
		}
	}
	return;
	
}

wxCheckBox *mxUT::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value, wxWindowID id, bool margin) {
	wxCheckBox *checkbox = new wxCheckBox(panel, id, text+_T("   "));
	if (margin) {
		wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
		sizerRow->AddSpacer(15);
		sizerRow->Add(checkbox, sizers->Exp1);
		sizer->Add(sizerRow, sizers->BLRT5_Exp0);
	} else 
		sizer->Add(checkbox,sizers->BA5_Exp0);
	checkbox->SetValue(value);
	last_button=NULL;
	return checkbox;	
}

wxTextCtrl *mxUT::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, int id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	last_button=NULL;
	return textctrl;
}

wxTextCtrl *mxUT::AddLongTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	sizer->Add(last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp1);
	textctrl->SetValue(value);
	last_button=NULL;
	return textctrl;
}

wxTextCtrl *mxUT::AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	wxButton *button = last_button = new wxButton(panel,id,button_text,wxDefaultPosition,button_text== _T("...")?wxSize(30,10):wxSize(-1,10));
	if (margin) sizerRow->AddSpacer(15);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizerRow->Add(button, sizers->Exp0_Right);
	sizer->Add(last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(sizerRow, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	return textctrl;
}

wxTextCtrl *mxUT::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value, bool margin, int id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(15);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	textctrl->SetValue(wxString::Format(_T("%d"), value));
	last_button=NULL;
	return textctrl;
}

wxStaticText* mxUT::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
	wxStaticText *textctrl = new wxStaticText(panel, wxID_ANY, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->AddStretchSpacer(1);
	sizerRow->Add(textctrl, sizers->Exp0);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	last_button=NULL;
	return textctrl;	
}

wxTextCtrl *mxUT::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	last_button=NULL;
	return textctrl;
}

wxTextCtrl *mxUT::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString foot, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxString()<<n, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+" ", wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, wxString(" ")<<foot, wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	last_button=NULL;
	return textctrl;
}

wxComboBox *mxUT::AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID id, bool margin) {
	wxString sdef=def==-1?"":values[def];
	wxComboBox *combo = new wxComboBox(panel, id, sdef, wxDefaultPosition, wxDefaultSize, values, wxCB_READONLY);
	combo->SetSelection(def);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(combo, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	last_button=NULL;
	return combo;
}

wxStaticText *mxUT::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text) {
	wxStaticText *statictext;
	sizer->Add( statictext = new wxStaticText(panel, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), sizers->BA5_Center);
	return statictext;
}

wxMenuItem *mxUT::AddItemToMenu(wxMenu *menu, const void *a_myMenuItem) {
	const MenusAndToolsConfig::myMenuItem *mi = reinterpret_cast<const MenusAndToolsConfig::myMenuItem*>(a_myMenuItem);
	if (!mi) return NULL;
	if (mi->properties&(MenusAndToolsConfig::maCHECKED|MenusAndToolsConfig::maCHECKEABLE))
		return mxUT::AddCheckToMenu(menu,mi->wx_id,mi->label,mi->shortcut,mi->description,mi->properties&MenusAndToolsConfig::maCHECKED);
	else 
		return mxUT::AddItemToMenu(menu,mi->wx_id,mi->label,mi->shortcut,mi->description,mi->icon);
}

wxMenuItem *mxUT::AddItemToMenu(wxMenu *menu, const void *a_myMenuItem, const wxString &caption) {
	const MenusAndToolsConfig::myMenuItem *mi = reinterpret_cast<const MenusAndToolsConfig::myMenuItem*>(a_myMenuItem);
	if (!mi) return NULL;
	if (mi->properties&(MenusAndToolsConfig::maCHECKED|MenusAndToolsConfig::maCHECKEABLE))
		return mxUT::AddCheckToMenu(menu,mi->wx_id,caption,mi->shortcut,mi->description,mi->properties&MenusAndToolsConfig::maCHECKED);
	else 
		return mxUT::AddItemToMenu(menu,mi->wx_id,caption,mi->shortcut,mi->description,mi->icon);
}

wxMenuItem *mxUT::AddItemToMenu(wxMenu *menu, wxWindowID id, const wxString &caption, const wxString &accel, const wxString &help, const wxString &filename, int where) {
	wxMenuItem *item = new wxMenuItem(menu,id,accel.Len()?caption+"\t"+accel:caption,help);
	if (filename.Len()) {
		wxString full_filename=SKIN_FILE_OPT(DIR_PLUS_FILE("16",filename));
		if (wxFileName::FileExists(full_filename)) item->SetBitmap(wxBitmap(full_filename,wxBITMAP_TYPE_PNG));
	}
	if (where==-1) menu->Append (item);
	else menu->Insert(where,item);
	return item;
}


wxMenuItem *mxUT::AddSubMenuToMenu(wxMenu *menu, wxMenu *menu_h, const wxString &caption, const wxString &help, const wxString &filename) {
	wxMenuItem *item = 	menu->AppendSubMenu(menu_h, caption, help);
	if (filename.Len()) {
		wxString full_filename=SKIN_FILE(DIR_PLUS_FILE("16",filename));
		if (wxFileName::FileExists(full_filename)) item->SetBitmap(wxBitmap(full_filename,wxBITMAP_TYPE_PNG));
	}
	return item;
}

wxMenuItem *mxUT::AddCheckToMenu(wxMenu *menu, wxWindowID id, const wxString &caption, const wxString &accel, const wxString &help, bool value) {
	wxMenuItem *item = menu->AppendCheckItem (id, accel.Len()?caption+"\t"+accel:caption,help);
	item->Check(value);
	return item;
}

void mxUT::AddTool(wxToolBar *toolbar, wxWindowID id, const wxString &caption, const wxString &filename, const wxString &status_text, wxItemKind kind) {
	wxString full_filename=mxUT::JoinDirAndFile(config->Files.skin_dir,filename);
	if (wxFileName::FileExists(full_filename)) {
		toolbar->AddTool(id, caption, wxBitmap(full_filename,wxBITMAP_TYPE_PNG),caption, kind);
		toolbar->SetToolLongHelp(id,status_text);
	}
}

wxString mxUT::FindObjectDeps(wxFileName filename, wxString ref_path, wxArrayString &header_dirs) {
	wxArrayString already_processed;
	already_processed.Add(filename.GetFullPath());
	FindIncludes(filename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR),filename.GetFullName(),already_processed,header_dirs);
	wxString deps;
	wxFileName file_name;
	for(unsigned int i=0;i<already_processed.GetCount();i++) { 
		file_name=already_processed[i];
		file_name.MakeRelativeTo(ref_path);
		deps+=file_name.GetFullPath()+" ";
	}
	if (deps.Len()==0) 
		return "";
	else
		return deps.RemoveLast();
}

void mxUT::FindIncludes(wxArrayString &deps, wxFileName filename, wxString ref_path, wxArrayString &header_dirs) {
	deps.Clear();
	wxArrayString already_processed;
	already_processed.Add(filename.GetFullPath()); // FindIncludes usa el primer item para sacar el path?
	FindIncludes(filename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR),filename.GetFullName(),already_processed,header_dirs,false);
	wxFileName file_name;
	for(unsigned int i=1;i<already_processed.GetCount();i++) { // empieza de 1 para saltearse el primer item (el propio archivo)
		file_name=already_processed[i];
		file_name.MakeRelativeTo(ref_path);
		deps.Add(file_name.GetFullPath());
	}
}

bool mxUT::AreIncludesUpdated(wxDateTime bin_date, wxFileName filename) {
	wxArrayString header_dirs;
	return AreIncludesUpdated(bin_date,filename,header_dirs);
}

bool mxUT::AreIncludesUpdated(wxDateTime bin_date, wxFileName filename, wxArrayString &header_dirs) {
	// inicializar la lista
	wxArrayString already_processed;
	wxString my_path=filename.GetPathWithSep();
	already_processed.Add(my_path); // el primer elemento (ficticio) guarda la ruta del archivo original
	already_processed.Add(filename.GetFullPath());
	// averiguar las dependencias
	FindIncludes("",filename.GetFullName(),already_processed,header_dirs);
	// comparar las fechas y liberar memoria
	bool ret=false,future=false;
	wxDateTime now=wxDateTime::Now();
	for(unsigned int i=1;i<already_processed.GetCount();i++) { // saltea el elemento ficticio
		if (!ret) {
			if (project) {
				project_file_item *fi=project->FindFromName(already_processed[i]);
				if (fi && fi->force_recompile) ret=true;
			}
			wxDateTime dt = wxFileName(already_processed[i]).GetModificationTime();
			ret = wxFileName(already_processed[i]).GetModificationTime()>bin_date;
			if ((now-dt).GetSeconds().ToLong()<-3) { // si es del futuro, arreglar y recompilar
				wxFileName(already_processed[i]).Touch();
				future=true;
				if (project)
					project->warnings.Add(LANG1(UTILS_FUTURE_FILE,"El archivo incluido <{1}> tenia fecha de modificacion en el futuro. Se reemplazo por la fecha actual.",already_processed[i]));
				mxSource *src = main_window->IsOpen(already_processed[i]);
				if (src) src->Reload();
			}
		}
	}
	return ret||future;
}

/** 
* Busca los includes del archivo filename que esta en el directorio path, relativo 
* al directorio original first->name 
**/
void mxUT::FindIncludes(wxString path, wxString filename, wxArrayString &already_processed, wxArrayString &header_dirs, bool recursive) {
	wxTextFile text_file(DIR_PLUS_FILE(DIR_PLUS_FILE(already_processed[0],path),filename));
	wxFileName file_name;
	text_file.Open();
	wxString line, this_one;
	int ini, end;
	// recorrer el fuente para buscar que incluye
	for ( line = text_file.GetFirstLine(); !text_file.Eof(); line = text_file.GetNextLine() ) { 
		// buscar donde comienza cada linea
		int pos=0, len=line.Len();
		while (pos<len && ( line[pos]==' ' || line[pos]=='\t' ))
			pos++;
		// si es una directiva de compilador
		if (line[pos++]=='#') {
			// buscar donde comienza la instruccion
			while (pos<len && ( line[pos]==' ' || line[pos]=='\t' ))
				pos++;
			// ver si es un include
			if (pos+8<len && line[pos++]=='i' && line[pos++]=='n' 
			&& line[pos++]=='c' && line[pos++]=='l' && line[pos++]=='u'
			&& line[pos++]=='d' && line[pos++]=='e') {
				// buscar donde comienza el nombre de archivo
				while (pos<len && ( line[pos]==' ' || line[pos]=='\t' ))
					pos++;
				// marcar el comienzo del nombre de archivo y buscar el final
				ini=++pos; end=0;
				while (pos<len && line[pos]!='>' && line[pos]!='\"' ) {
					pos++;end++;
				}
				this_one=line.Mid(ini,end);
				// si el archivo existe
				if (this_one.Len())
					this_one=GetOnePath(DIR_PLUS_FILE(already_processed[0],path),project?project->path:DIR_PLUS_FILE(already_processed[0],path),this_one,header_dirs);
				if (this_one.Len()) {
					file_name=this_one;
					file_name.Normalize(wxPATH_NORM_DOTS);
					this_one=file_name.GetFullPath();
					if (already_processed.Index(this_one)==wxNOT_FOUND) {
						// agregarlo a la lista de archivos ya considerados
						already_processed.Add(this_one);
						// buscar las dependencias recursivamente
						if (recursive) 
							FindIncludes(file_name.GetPathWithSep(),file_name.GetFullName(),already_processed,header_dirs);
					}
				}
			}
		}
	}
}

wxString mxUT::UnSplit(wxArrayString &array) {
	wxString ret;
	if (array.GetCount()) {
		ret<<array[0];
		for (unsigned int i=1;i<array.GetCount();i++) {
			ret<<' '<<array[i];
		}
	}
	return ret;
}

wxString mxUT::Split(wxString str, wxString pre) {
	/// @todo: reescribir este método reusando el otro Split
	int i=0, l=str.Len();
	wxString ret;
	bool comillas = false;
	while (i<l) {
		while (i<l && (str[i]==' ' || str[i]=='\t' || str[i]==',' || str[i]==';')) {
			i++;
		}
		int s=i;
		while (i<l && (comillas || !(str[i]==' ' || str[i]=='\t' || str[i]==',' || str[i]==';'))) {
			if (str[i]=='\"') 
				comillas=!comillas;
			else if (str[i]=='\\' && i+1<l) i++;
			i++;
		}
		if (s<i)
			ret<<pre<<str.Mid(s,i-s)<<" ";
	}
	return ret;
}

int mxUT::Split(wxString str, wxArrayString &array, bool coma_splits, bool keep_quotes) {
	int c=0, i=0, l=str.Len();
	wxString ret;
	bool comillas = false; char cual_comilla;
	while (i<l) {
		while (i<l && (str[i]==' ' || str[i]=='\t' || (coma_splits && (str[i]==',' || str[i]==';') ) ) ) {
			i++;
		}
		int s=i;
		while (i<l && (comillas || !(str[i]==' ' || str[i]=='\t' || (coma_splits && (str[i]==',' || str[i]==';') ) ) ) ) {
			if (comillas && str[i]==cual_comilla) {
				comillas=false;
				if (!keep_quotes) { l--; str.Remove(i,1); i--; }
			} else if (!comillas && (str[i]=='\"'||str[i]=='\'')) {
				comillas=true; cual_comilla=str[i];
				if (!keep_quotes) { l--; str.Remove(i,1); i--; }
			}
//			else if (str[i]=='\\' && i+1<l) {
//				if (!use_scape_char) i++;
//				else { l--; str.Remove(i,1); }
//			}
			i++;
		}
		if (s<i) {
			array.Add(str.Mid(s,i-s));
			c++;
		}
	}
	return c;
}

int mxUT::Execute(wxString path, wxString command, int sync) {
	while (command.Len() && command.Last()==' ') command.RemoveLast();
	static wxProcess *p=NULL;
	return Execute(path,command,sync,p);
}

int mxUT::Execute(wxString path, wxString command, int sync, wxProcess *&process) {
	while (command.Len() && command.Last()==' ') command.RemoveLast();
	wxSetWorkingDirectory(path);
#ifndef __WIN32__
	// por alguna razon, con wx 2.8 no podemos tener mas de 127 argumentos (ver WXEXECUTE_NARGS en los fuentes de wx 2.8)
	// asi que hacemos un script con el comando y llamamos al script si ese es el caso
	int num_args=0, i=0,l=command.Len();
	while (++i<l) {
		if (command[i]=='\'') { while (++i<l && command[i]!='\''); }
		else if (command[i]=='\"') { while (++i<l && command[i]!='\'') if (command[i]=='\\') i++; }
		else if (command[i]=='\\') i++;
		else if (command[i]==' ') num_args++;
	}
	if (num_args>=127) {
		wxString aux_file(DIR_PLUS_FILE(config->home_dir,"long_command"));
		wxTextFile file(aux_file); 
		file.Create();
		file.AddLine(command);
		file.Write();
		file.Close();
		command=wxString("sh ")+aux_file;
	}
#endif
	int ret = (sync&wxEXEC_SYNC) ? mxExecute(command, sync, process) : wxExecute(command, sync, process);
	wxSetWorkingDirectory(config->zinjai_dir);
	return ret;
}

bool mxUT::XCopy(wxString src,wxString dst, bool ask, bool replace) {
	if (src.Last()=='/' || src.Last()=='\\')
		src.RemoveLast();
	if (dst.Last()=='/' || dst.Last()=='\\')
		dst.RemoveLast();
	// el destino y el origen sean directorios y no archivos
	if (!wxFileName::DirExists(src) || wxFileName::FileExists(dst)) 
		return false;
	// crear el directorio destino si no existe
	wxDir dir(src);
	if (!dir.IsOpened())
		return false;
	if (!wxFileName::DirExists(dst)) {
		wxFileName::Mkdir(dst);
		if (!wxFileName::DirExists(dst))
			return false;
	}
	// copiar todo
	wxString filename;
	wxString spec;
	bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
	while ( cont ) {
		if (!XCopy(JoinDirAndFile(src,filename),JoinDirAndFile(dst,filename),true))
			return false;
		cont = dir.GetNext(&filename);
	}	
	cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
	while ( cont ) {
		wxString dest_file = JoinDirAndFile(dst,filename);
		cont = (!ask&&replace)||!(wxFileName::FileExists(dest_file));
		if (!cont && ask) { // si ya existe preguntar si hay que pisar
			int ret = mxMessageDialog(NULL,LANG1(FILE_EXISTS_ASK_REPLACE,"El archivo \"<{1}>\" ya existe. Desea reemplazarlo?",dest_file),"Aviso",mxMD_YES_NO|mxMD_QUESTION,LANG(GENERAL_DONT_ASK_AGAIN,"No volver a preguntar"),false).ShowModal();
			cont = (ret&mxMD_YES);
			if (ret&mxMD_CHECKED) {
				ask=false;
				replace=cont;
			}
		}  
		if (cont && !wxCopyFile(JoinDirAndFile(src,filename),dest_file))
			return false;
		cont = dir.GetNext(&filename);
	}
	return true;
}

wxString mxUT::ExecComas(wxString where, wxString line) {
	wxSetWorkingDirectory(where);
	wxString ret;
	int p=0;
	bool flag=true;
	for (unsigned int i=0;i<line.Len();i++) {
		if (line[i]==96) {
			if (flag) {
				flag=false;
				ret<<line.Mid(p,i-p);
				p=i+1;
			} else {
				ret<<GetOutput(line.Mid(p,i-p));
				// TODO: ver que pasa si la ejecucion sale mal!
				p=i+1;
				flag=true;
			}
		}
	}
	ret<<line.Mid(p);
	wxSetWorkingDirectory(config->zinjai_dir);
	return ret;
}

wxString mxUT::GetOutput(wxString command, bool also_error) {
	wxString ret;
	wxArrayString output,errors;
	if (also_error)
		mxExecute(command, output, errors, wxEXEC_NODISABLE|wxEXEC_SYNC);
	else
		mxExecute(command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
	if (also_error) {
		for (unsigned int i=0;i<errors.GetCount();i++)
			if (ret.Len()==0)
				ret<<errors[i];
			else
				ret<<"\n"<<errors[i];
	}
	for (unsigned int i=0;i<output.GetCount();i++)
		if (ret.Len()==0)
			ret<<output[i];
		else
			ret<<"\n"<<output[i];
	return ret;
}

/** 
* Devuelve una cadena convertida a HTML. Reemplaza los espacios y operadores por
* los codigos de escape adecuados (\&...).
* @param text La cadena a convertir
* @param full Determina si reemplazar tambien espacios y \\r (true) o solo operadores y \\n (false)
* @retval La cadena en formato HTML
**/
wxString mxUT::ToHtml(wxString text, bool full) {
	static wxString tabs;
	static int tabs_n;
	if (tabs_n!=config->Source.tabWidth) {
		tabs="";
		for (int i=0;i<config->Source.tabWidth;i++)
			tabs<<_T("&nbsp;");
		tabs_n=config->Source.tabWidth;
	}
	text.Replace(_T("&"),_T("&amp;"));
	text.Replace("\t",tabs);
	text.Replace(_T("<"),_T("&lt;"));
	text.Replace(_T(">"),_T("&gt;"));
	text.Replace("\n",_T("<BR>"));
	if (full) {
		wxChar doce[]=" ";
		doce[0]=12;
		text.Replace(doce,_T("<BR>"));
		text.Replace(" ",_T("&nbsp;"));
	}
	return text;
}

void mxUT::ParameterReplace(wxString &str, wxString from, wxString to) {
	if (str.Find(' ')==wxNOT_FOUND)
		str.Replace(from,to);
	else {
		wxString final;
		int p=str.Find(from), l=str.Len();
		while (p!=wxNOT_FOUND) {
			int p1=p, p2=p;
			// la clave puede ser parte de un argumento, por eso busca que hay pegado para atras y para adelante
			while (p1>0 && str[p1]!=' ' && str[p1]!='\t')
				p1--;
			if (str[p1]==' ' || str[p1]=='\t')
				p1++;
			while (p2<l-1 && str[p2]!=' ' && str[p2]!='\t')
				p2++;
			if (str[p2]==' ' || str[p2]=='\t')
				p2--;
			final<<str.SubString(0,p1-1);
			final<<mxUT::Quotize(str.SubString(p1,p-1)+to+str.SubString(p+from.Len(),p2));
			str=str.Mid(p2+1);
			p=str.Find(from);
			l=str.Len();
		}
		final<<str;
		str=final;
	}
}

//void mxUT::MakeDesktopIcon (wxString desk_dir) {
//	wxTextFile fil(DIR_PLUS_FILE(desk_dir,_T("ZinjaI.desktop")));
//	if (fil.Exists())
//		fil.Open();
//	else
//		fil.Create();
//	fil.Clear();
//	fil.AddLine(_T("[Desktop Entry]"));
//	fil.AddLine(_T("Comment=IDE for C++ programming"));
//	fil.AddLine(_T("Comment[es]=IDE para programar en C++"));
//	fil.AddLine(_T("Encoding=UTF-8"));
//	fil.AddLine(wxString(_T("Icon="))<<DIR_PLUS_FILE(config->zinjai_dir,_T("imgs/zinjai-64x64.png")));
//	fil.AddLine(_T("Name=ZinjaI"));
//	fil.AddLine(_T("Name[es]=ZinjaI"));
//	fil.AddLine(_T("Type=Link"));
//	fil.AddLine(wxString(_T("URL="))<<DIR_PLUS_FILE(config->zinjai_dir,_T("zinjai")));
//	fil.Write();
//	fil.Close();
// }


wxString mxUT::GetOnePath(wxString orig_path, wxString project_path, wxString fname, wxArrayString &header_dirs) {
	wxString theone(DIR_PLUS_FILE(orig_path,fname));
	if (wxFileName::FileExists(theone))
		return theone;
	if (header_dirs.GetCount()) {
		unsigned int hi=0;
		do {
			theone = DIR_PLUS_FILE(DIR_PLUS_FILE(project_path,header_dirs[hi++]),fname);
			if (wxFileName::FileExists(theone))
				return theone;
		} while (hi<header_dirs.GetCount());
	}
	return "";
}

/**
* Compara el final de la primer cadena con la segunda (completa).
* @param s1 primer cadena
* @param s2 segunda cadena
* @return true si el final de s1 es igual a s2 (case-insensitive)
**/
bool mxUT::EndsWithNC(wxString s1, wxString s2) {
	if (s2.Len()>s1.Len()) 
		return false;
	else return (s1.Mid(s1.Len()-s2.Len()).CmpNoCase(s2)==0);
}

/**
* Quita los espacios y tabs del principio de una cadena. Devuelve la cadena
* resultante sin modificar la original.
* @param str la cadena a procesar
* @return La cadena porcesada
**/
wxString mxUT::LeftTrim(wxString str) {
	int i=0, l=str.Len();
	while (i<l && (str[i]==' ' || str[i]=='\t')) i++;
	return str.Mid(i); 
}

/**
* Distancia de Levenshtein, basado en el articulo de la Wikipedia en Español. Esta
* distancia mida la cantidad de operaciones (insercion, eliminacion o reemplazo)
* necesarias para convertir una cadena en la otra. En este caso, la primer cadena
* se asume en minusculas, la segunda se convierte temporalmente durante la comparacion.
* @param s1 primer cadena (en minusculas)
* @param n1 longitud de la primer cadena
* @param s2 segunda cadena 
* @param n2 longitud de la segunda cadena
**/
int mxUT::Levenshtein(const char *s1, int n1, const char *s2, int n2) {
	int i, j;
	if (n2+1>150) return 150;
	static int t[150]; //N2+1
	for ( i = 0; i <= n2; i++ )
		t[i] = i;
	
	for ( i = 0; i < n1; i++ ) {
		t[0] = i+1;
		int corner = i;
		for ( j = 0; j < n2; j++ ) {
			int upper = t[j+1];
			if ( s1[i] == (s2[j]|32) )
				t[j+1] = corner;
			else
				t[j+1] = min2(t[j], min2(upper, corner)) + 1;
			corner = upper;
		}
	}
	return t[n2];
}

/** 
* Elimina los elementos repetidos del wxArrayString. Para eso necesita que esté ordenado
* @param array el arreglo a purgar
* @param sort falso si el arreglo ya viene ordenado, true si se debe ordenar aqui
**/
void mxUT::Purgue(wxArrayString &array, bool sort) {
	if (sort) array.Sort();
	unsigned int i=1, r=0, d=0, n=array.GetCount();
	while (i<n) {
		if (array[i]==array[r]) {
			d++;
		} else if (d) {
			r=i;
			array[i-d]=array[i];
		}
		i++;
	}
	if (d) array.RemoveAt(n-d,d);
}

/** 
* Compara dos wxArrayString elemento a elemento. Para ser iguales deben tener
* los mismos elementos y en el mismo orden.
* @param array1 el primer arreglo
* @param array2 el segundo arreglo
* @param check_case si la comparacion distingue mayusculas y minusculas
* @retval true si son iguales
* @retval false si son diferentes
**/
bool mxUT::Compare(const wxArrayString &array1, const wxArrayString &array2, bool check_case) {
	unsigned int n1=array1.GetCount(), n2=array2.GetCount();
	if (n1!=n2) return false;
	if (check_case) {
		for (n1=0;n1<n2;n1++)
			if (array1[n1]!=array1[n2]) 
				return false;
	} else {
		for (n1=0;n1<n2;n1++)
			if (array1[n1].CmpNoCase(array1[n2])!=0) 
				return false;
	}
	return true;
}

/** 
* @brief Dado un nombre de archivo, busca el complementario. Es decir, para un .h busca
* (verificando que exista) un .c/.cpp /.cxx/.c++ con el mismo nombre y viseversa.
*
* Busca primero en el mismo directorio que the_one, sino, si hay un proyecto
* abierto, busca en los archivos del proyecto (que podrían estar en otros
* directorios)
*
* @param the_one path completo del archivo
* @param force_ext sirve para forzar el tipo de archivo que recibe: cabecera  o
fuente. El default adivina según la extensión
* @return el nombre del complementario, o una cadena vacia si no lo encuentra
**/
wxString mxUT::GetComplementaryFile(wxFileName the_one, eFileType force_ext) {
	if (force_ext==FT_NULL) force_ext=GetFileType(the_one.GetFullPath());
	const int n=5 ;
	static char h_exts[][n]={"h","hpp","hxx","h++","hh"};
	static char cpp_exts[][n]={"c","cpp","cxx","c++","cc"};
	char (*exts)[n] = force_ext==FT_HEADER ? cpp_exts : h_exts;
	// primero, ver si el otro archivo esta en el mismo dierctorio
	for(int i=0;i<n;i++) { 
		the_one.SetExt(exts[i]);
		if (the_one.FileExists())
			return the_one.GetFullPath();
	}
	// si es proyecto, buscar si esta en otro directorio (usando las clategorias de sus archivos y no la extension)
	if (project) {
		wxString only_name = the_one.GetName();
		LocalListIterator<project_file_item*> it( force_ext==FT_HEADER ? &project->files_sources : &project->files_headers );
		while (it.IsValid()) {
			if (wxFileName(it->name).GetName()==only_name) {
				return DIR_PLUS_FILE(project->path,it->name);
			}
			it.Next();
		}
	}
	return wxString();
}

/**
* Busca los procesos en ejecucion que se lanzaron desde zinjai. Para esto
* busca los procesos hijos de zinjai que no empiecen como config->Files.terminal_command
* (proyectos lanzados sin terminal y runner) y los "bisnietos" (lanzados en un terminal,
* zinjai->consola->runner->bisnieto). En windows no está implementado.
**/
void mxUT::GetRunningChilds(wxArrayString &childs) {
	long pid = wxGetProcessId();
	if (!pid) return;

#if !defined(_WIN32) && !defined(__WIN32__)
	
	struct proc { wxString cmd; long pid,ppid; int sel; };
	wxArrayString output;
	wxString ps_command=_T("ps --sort -pid -e --format '%P %p %a'");
	mxExecute(ps_command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
	unsigned int n=output.GetCount();
	proc *list = new proc[n];
	for (unsigned int i=1;i<n;i++) {
		wxString line=output[i];
		int j=0,l=line.Len();
		while (j<l && (line[j]==' '||line[j]=='\t')) j++;
		int p1a=j;
		while (j<l && !(line[j]==' '||line[j]=='\t')) j++;
		int p1b=j;
		while (j<l && (line[j]==' '||line[j]=='\t')) j++;
		int p2a=j;
		while (j<l && !(line[j]==' '||line[j]=='\t')) j++;
		int p2b=j;
		while (j<l && (line[j]==' '||line[j]=='\t')) j++;
		int p3=j;
		line.Mid(p1a,p1b-p1a).ToLong(&(list[i].ppid));
		line.Mid(p2a,p2b-p2a).ToLong(&(list[i].pid));
		list[i].cmd=line.Mid(p3);
		list[i].sel=(list[i].ppid==pid&&!list[i].cmd.StartsWith(_T("ps --sort")))?1:0;
	}
	
	for (unsigned int i=1;i<n;i++) {
		if (list[i].sel==1) {
			for (unsigned int j=1;j<n;j++) {
				if (list[j].ppid==list[i].pid) {
					list[j].sel=2;
				}
			}
		}
	}
	
	for (unsigned int i=1;i<n;i++) {
		if (list[i].sel==2) {
			for (unsigned int j=1;j<n;j++) {
				if (list[j].ppid==list[i].pid) {
					list[j].sel=3;
				}
			}
		}
	}
	
	wxString tc=config->Files.terminal_command.BeforeFirst(' ');
	for (unsigned int i=1;i<n;i++) {
		if ( (list[i].sel==1 && list[i].cmd.Mid(0,tc.Len())!=tc) || list[i].sel==3)
			childs.Add(wxString()<<list[i].pid<<_T("   ")<<list[i].cmd);
	}
	
	delete [] list;

#endif
	
}

void mxUT::OpenInBrowser(wxString url) {
	if (config->Files.browser_command.Len()) {
		if (config->Files.browser_command=="shellexecute.exe" && url.StartsWith("file://"))
			url=url.Mid(7);
		wxExecute(config->Files.browser_command+" \""+url+"\"");	
	} else
		wxLaunchDefaultBrowser(url);
}

/** 
* Convierte un path absoluto en relativo, siempre y cuando no deba subir más
* de 2 niveles desde el path de referencia
**/
wxString mxUT::Relativize(wxString name, wxString path) {
	if (path.Last()=='\\' || path.Last()=='/') path.RemoveLast();
	bool rr=false;
	if (rr) name.RemoveLast();
	wxFileName fname(name);
	fname.MakeRelativeTo(path);
	wxString rname = fname.GetFullPath();
	if (rname.StartsWith(_T("../../..")) || rname.StartsWith(_T("..\\..\\..")))
		return name;
	else 
		return rname;
}

void mxUT::OpenFolder(wxString path) {
	if (config->Files.explorer_command==_T("<<sin configurar>>")) {
		wxMessageBox(LANG(CONFIG_EXPLORER_PROBLEM,"No se ha definido el explorador de archivos a utilizar\n"
			                                       "Puede configurar el mismo en la pestana \"Rutas 2\" del\n"
												   "cuadro de Preferencias (menu Archivo->Preferencias)."),LANG(CONFIG_EXPLORER,"Explorador de archivos"));
	} else {
		wxString cmd = config->Files.explorer_command;
		cmd<<" \""<<path<<"\"";
		mxUT::Execute(path,cmd,wxEXEC_NOHIDE);
	}
}


wxString mxUT::Quotize(const wxString &what) {
	if (what.Find(' ')!=wxNOT_FOUND)
		return wxString("\"")<<what<<"\"";
	else
		return what;
}

wxString mxUT::QuotizeEx(const wxString &what) {
	if (what.Find(' ')==wxNOT_FOUND&&what.Find(',')==wxNOT_FOUND&&what.Find(';')==wxNOT_FOUND&&what.Find('\'')==wxNOT_FOUND&&what.Find('\"')==wxNOT_FOUND)
		return what;
	else if (what.Find('\"')==wxNOT_FOUND)
		return wxString("\"")<<what<<"\"";
	else if (what.Find('\'')==wxNOT_FOUND)
		return wxString("\'")<<what<<"\'";
	wxString r; int i=0,p=0, l=what.Len();
	while(i<l) {
		if (i!=l && what[i]=='\"') { r<<'\''<<what.Mid(p,i-p+1)<<'\''; p=i+1; }
		if (i!=l && what[i]=='\'') { r<<'\"'<<what.Mid(p,i-p+1)<<'\"'; p=i+1; }
		i++;
	}
	return r;
}

wxString mxUT::SingleQuotes(wxString what) {
	what.Replace("\'","\'\\\'\'",true);
	return wxString("\'")<<what<<"\'";
}

wxString mxUT::EscapeString(wxString str, bool add_comillas) {
	int i=0, l=str.Len();
	if (add_comillas) {
		i=1;
		str.Prepend(wxChar('\"'));
		l++;
	}
	while(i!=l) {
		if (str[i]=='\\' || str[i]=='\"') {
			str=str.Mid(0,i)+wxChar('\\')+str.Mid(i);
			i++; l++;
		}
		i++;
	}
	if (add_comillas)
		str.Append('\"');
	return str;
}

wxString mxUT::UnEscapeString(wxString str) {
	int i=0, l=str.Len();
	while(i<l) {
		if (str[i]=='\\' || str[i]=='\"' || str[i]=='\'') {
			if (str[i]=='\\' && i+1<l && str[i+1]=='t') str[i+1]='\t';
			if (str[i]=='\\' && i+1<l && str[i+1]=='n') str[i+1]='\n';
			else if (str[i]=='\\' && i+1<l && str[i+1]=='r') str[i+1]='\r';
			else if (str[i]=='\\' && i+1<l && str[i+1]=='b') str[i+1]='\b';
			str=str.Mid(0,i)+str.Mid(i+1);
			/*i++;*/ l--; i--;
		}
		i++;
	}
	return str;
}


wxString mxUT::Text2Line(const wxString &text) {
	int l=text.Len(),i,p=0;
	wxString line;
	for (i=0;i<l;i++) {
		if (text[i]=='\\') {
			line<<text.Mid(p,i-p)<<_T("\\\\");
			p=++i;
		} else if (text[i]=='\n') {
			line<<text.Mid(p,i-p)<<_T("\\n");
			p=++i;
		}
	}
	if (i!=p) line<<text.Mid(p,i-p);
	return line;
}

wxString mxUT::Line2Text(const wxString &line) {
	int l=line.Len(),i,p=0;
	wxString text;
	for (i=0;i<l;i++) {
		if (line[i]=='\\') {
			if (line[i+1]=='n') {
				text<<line.Mid(p,i-p)<<"\n";
				p=(++i)+1;
			} else {
				text<<line.Mid(p,i-p);
				p=(++i);
			}
		}
	}
	if (i!=p) text<<line.Mid(p,i-p);
	return text;
}

wxString mxUT::UrlEncode(wxString str) {
	str.Replace("%","%25");
	str.Replace(",","%2C");
	str.Replace(";","%3B");
	str.Replace(":","%3A");
	str.Replace("\'","%27");
	str.Replace("\\","%5C");
	str.Replace("/","%2F");
	str.Replace("&","%26");
	str.Replace("@","%40");
	str.Replace("$","%24");
	str.Replace("#","%23");
	str.Replace("[","%5B");
	str.Replace("]","%5D");
	str.Replace("{","%7B");
	str.Replace("}","%7D");
	str.Replace("(","%28");
	str.Replace(")","%29");
	str.Replace(" ","%20");
	str.Replace("!","%21");
	str.Replace("\"","%22");
	str.Replace("?","%3F");
	str.Replace("¿","%BF");
	str.Replace("+","%2B");
	str.Replace("¡","%A1");
	str.Replace("=","%3D");
	return str;
}


// options = "REPLACE|COMMA|TEXT|LIST|FILE|DIR|OUTPUT|DEPS|TEMP_DIR|MINGW_DIR|BROWSER|PROJECT_PATH|PROJECT_BIN|CURRENT_FILE|CURRENT_DIR"
void mxUT::ShowTextPopUp(wxWindow *parent, wxString title, wxTextCtrl *text, wxString options, wxString path) {
	wxString opt; bool replace=false, comma=false;
	wxMenu menu;
	do {
		opt = options.BeforeFirst('|'); options=options.AfterFirst('|');
		if (!opt.size()) { opt=options; options.Clear(); }
		if (opt=="REPLACE") replace=true;
		else if (opt=="COMMA") comma=true;
		else if (opt=="TEXT") menu.Append(mxID_POPUPS_INSERT_TEXT,LANG(GENERAL_POPUP_INSERT_TEXT,"editar como texto..."));
		else if (opt=="LIST") menu.Append(mxID_POPUPS_INSERT_LIST,LANG(GENERAL_POPUP_INSERT_LIST,"editar como lista..."));
		else if (opt=="FILE") menu.Append(mxID_POPUPS_INSERT_FILE,LANG(GENERAL_POPUP_INSERT_FILE,"insertar archivo..."));
		else if (opt=="DIR") menu.Append(mxID_POPUPS_INSERT_DIR,LANG(GENERAL_POPUP_REPLACE_INSERT_DIR,"insertar directorio..."));
		else if (opt=="OUTPUT") menu.Append(mxID_POPUPS_INSERT_OUTPUT,LANG(GENERAL_POPUP_INSERT_OUTPUT,"insertar archivo de salida"));
		else if (opt=="DEPS") menu.Append(mxID_POPUPS_INSERT_DEPS,LANG(GENERAL_POPUP_INSERT_DEPS,"insertar lista de dependencias"));
		else if (opt=="TEMP_DIR") menu.Append(mxID_POPUPS_INSERT_TEMP_DIR,LANG(GENERAL_POPUP_INSERT_TEMP_DIR,"insertar directorio de temporales"));
#ifdef __WIN32__
		else if (opt=="MINGW_DIR") menu.Append(mxID_POPUPS_INSERT_MINGW_DIR,LANG(GENERAL_POPUP_INSERT_MINGW_DIR,"insertar directorio MinGW"));
#else
		else if (opt=="MINGW_DIR") menu.Append(mxID_POPUPS_INSERT_MINGW_DIR,LANG(GENERAL_POPUP_INSERT_MINGW_DIR,"insertar directorio del compilador"));
#endif
		else if (opt=="PROJECT_PATH") menu.Append(mxID_POPUPS_INSERT_PROJECT_PATH,LANG(GENERAL_POPUP_INSERT_PROJECT_PATH,"insertar directorio del proyecto"));
		else if (opt=="PROJECT_BIN") menu.Append(mxID_POPUPS_INSERT_PROJECT_BIN,LANG(GENERAL_POPUP_INSERT_PROJECT_BIN,"insertar ruta del ejecutable"));
		else if (opt=="WORK_DIR") menu.Append(mxID_POPUPS_INSERT_WORK_DIR,LANG(GENERAL_POPUP_INSERT_WORK_DIR,"insertar directorio de trabajo"));
		else if (opt=="CURRENT_FILE") menu.Append(mxID_POPUPS_INSERT_CURRENT_FILE,LANG(GENERAL_POPUP_INSERT_CURRENT_FILE,"insertar archivo actual"));
		else if (opt=="CURRENT_DIR") menu.Append(mxID_POPUPS_INSERT_CURRENT_DIR,LANG(GENERAL_POPUP_INSERT_CURRENT_DIR,"insertar directorio del archivo actual"));
		else if (opt=="BIN_WORKDIR") menu.Append(mxID_POPUPS_INSERT_WORKDIR,LANG(GENERAL_POPUP_INSERT_WORKDIR,"insertar el directorio de trabajo"));
		else if (opt=="ZINJAI_DIR") menu.Append(mxID_POPUPS_INSERT_ZINJAI_DIR,LANG(GENERAL_POPUP_INSERT_ZINJAI_DIR,"insertar el directorio de instalación de ZinjaI"));
		else if (opt=="BROWSER") menu.Append(mxID_POPUPS_INSERT_BROWSER,LANG(GENERAL_POPUP_INSERT_BROWSER,"insertar comando del navegador"));
		else if (opt=="SHELL_EXECUTE") menu.Append(mxID_POPUPS_INSERT_SHELL_EXECUTE,LANG(GENERAL_POPUP_INSERT_SHELL_EXECUTE,"insertar comando para abrir con el programa asociado"));
	} while (options.Len());
	ProcessTextPopup(0,parent,text,path,title,replace,comma);
	parent->PopupMenu(&menu);
}

void mxUT::ProcessTextPopup(int id, wxWindow *parent, wxTextCtrl *t, wxString path, wxString title, bool replace, bool comma_splits) {
	static wxWindow *win = NULL;
	static wxString caption;
	static wxString dir;
	static bool comma = false;
	static bool repl = false;
	static wxTextCtrl *ctrl = NULL;
	if (id && ctrl) {
		wxString text =	repl?ctrl->GetValue():ctrl->GetStringSelection();
		if (id==mxID_POPUPS_INSERT_TEXT) {
			new mxLongTextEditor(win,caption,ctrl);
		} else if (id==mxID_POPUPS_INSERT_LIST) {
			new mxEnumerationEditor(win,caption,ctrl,comma);
		} else if (id==mxID_POPUPS_INSERT_FILE) {
			wxFileDialog dlg(win,caption,text.Len()?text:(project?project->last_dir:config->Files.last_dir));
			if (wxID_OK!=dlg.ShowModal()) return;
			(project?project->last_dir:config->Files.last_dir) = wxFileName(dlg.GetPath()).GetPath(); 
			if (path.Len()) text = mxUT::Relativize(dlg.GetPath(),path); else text=dlg.GetPath();
			if (text.Contains(' ')) text = wxString("\"")<<text<<"\"";
		} else if (id==mxID_POPUPS_INSERT_DIR) {
			wxDirDialog dlg(win,caption,text.Len()?text:(project?project->last_dir:config->Files.last_dir));
			if (wxID_OK!=dlg.ShowModal()) return;
			(project?project->last_dir:config->Files.last_dir) = dlg.GetPath(); 
			if (path.Len()) text = mxUT::Relativize(dlg.GetPath(),path); else text=dlg.GetPath();
			if (text.Contains(' ')) text = wxString("\"")<<text<<"\"";
		} else if (id==mxID_POPUPS_INSERT_MINGW_DIR) {
			text="${MINGW_DIR}";
		} else if (id==mxID_POPUPS_INSERT_TEMP_DIR) {
			text="${TEMP_DIR}";
		} else if (id==mxID_POPUPS_INSERT_BROWSER) {
			text="${BROWSER}";
		} else if (id==mxID_POPUPS_INSERT_SHELL_EXECUTE) {
			text="${OPEN}";
		} else if (id==mxID_POPUPS_INSERT_PROJECT_PATH) {
			text="${PROJECT_PATH}";
		} else if (id==mxID_POPUPS_INSERT_WORK_DIR) {
			text="${WORK_DIR}";
		} else if (id==mxID_POPUPS_INSERT_PROJECT_BIN) {
			text="${PROJECT_BIN}";
		} else if (id==mxID_POPUPS_INSERT_ZINJAI_DIR) {
			text="${ZINJAI_DIR}";
		} else if (id==mxID_POPUPS_INSERT_CURRENT_FILE) {
			text="${CURRENT_SOURCE}";
		} else if (id==mxID_POPUPS_INSERT_CURRENT_DIR) {
			text="${CURRENT_DIR}";
		} else if (id==mxID_POPUPS_INSERT_OUTPUT) {
			text="${OUTPUT}";
		} else if (id==mxID_POPUPS_INSERT_DEPS) {
			text="${DEPS}";
		}
		if (repl) ctrl->SetValue(text); 
		else {
			long f,t;
			ctrl->GetSelection(&f,&t);
			ctrl->Replace(f,t,text);
		}
	}
	ctrl=t; caption=title; comma=comma_splits; win=parent; dir=path; repl=replace;
}

eFileType mxUT::GetFileType(wxString name, bool recognize_projects) {
	wxString pto="."; if (!name.Contains(pto)) return FT_OTHER;
	name=name.AfterLast('.').Lower();
	if (!name.Len()) return FT_OTHER;
	if (name.Last()=='\"') name.RemoveLast();
	pto[0]=path_sep; if (name.Contains(pto)) return FT_OTHER;
	if (ExtensionIsCpp(name))
		return FT_SOURCE;
	if (ExtensionIsH(name))
		return FT_HEADER;
	if (recognize_projects && name==PROJECT_EXT)
		return FT_PROJECT;
	return FT_OTHER;		
}


/**
* @brief Retreaves all files or folders names from a specified path
*
* @param array files/folders will be appended to this array
* @param path folder path where the method should look for files/folders
* @param files true for retriving files, false for retrieving folders
* @return number of files/folder if path exists, -1 if path does not exists
**/
int mxUT::GetFilesFromDir (wxArrayString & array, wxString path, bool files) {
	if (!wxDirExists(path)) return -1;
	int n=0;
	wxDir dir(path);
	if ( dir.IsOpened() ) {
		wxString filename, spec;
		bool cont = dir.GetFirst(&filename, spec , files?wxDIR_FILES:wxDIR_DIRS);
		while ( cont ) {
			n++; array.Add(filename);
			cont = dir.GetNext(&filename);
		}	
	}
	return n;
}

int mxUT::Unique (wxArrayString &array, bool do_sort) {
	if (do_sort) array.Sort();
	int n=array.GetCount(),d=0,i=1;
	while (i<n) {
		if (array[i]==array[i-1]) d++;
		else if (d) array[i-d]=array[i];
		i++;
	}
	if (d) array.RemoveAt(n-d,d);
	return d;
}

wxString mxUT::WichOne(wxString file, wxString dir_name, bool is_file) {
	return WichOne(file,DIR_PLUS_FILE(config->home_dir,dir_name),DIR_PLUS_FILE(config->zinjai_dir,dir_name),is_file);
}

wxString mxUT::WichOne(wxString file, wxString path1, wxString path2, bool is_file) {
	wxString res=DIR_PLUS_FILE(path1,file);
	if (is_file) { if (wxFileExists(res)) return res; }
	else { if (wxDirExists(res)) return res; }
	res=DIR_PLUS_FILE(path2,file);
	if (is_file) { if (wxFileExists(res)) return res; }
	else { if (wxDirExists(res)) return res; }
	return "";
}

void mxUT::GetFilesFromBothDirs (wxArrayString & array, wxString dir_name, bool is_file, wxString extra_element) {
	GetFilesFromDir(array,DIR_PLUS_FILE(config->zinjai_dir,dir_name),is_file);
	GetFilesFromDir(array,DIR_PLUS_FILE(config->home_dir,dir_name),is_file);
	mxUT::Unique(array,true);
	if (extra_element.Len()) array.Add(extra_element);
}


bool mxUT::IsArgumentPresent (const wxString &full, const wxString &arg) {
	wxArrayString array;
	Split(full,array,false,false);
	return array.Index(arg)!=wxNOT_FOUND;
}

void mxUT::SetArgument (wxString &full, const wxString &arg, bool add) {
	if (add) {
		full=mxUT::Quotize(arg)+" "+full;
	} else {
		wxArrayString array;
		Split(full,array,false,true);
		array.Remove(arg);
		array.Remove(wxString("\"")+arg+"\"");
		full=UnSplit(array);
	}
}

/**
* @param graph_file  file containing unprocessed graph description (input for dot or fpd)
* @param use_fdp     if true then process graph_file with fdp, else process with dot
* @param ouput       if empty, the result will be displayed with xdot or an image viewer, 
*                    if not empty the result will be saved in ouput (a file path) and 
*                    format will be guessed from its extension
* @param title       if the file is displayed in an external viewer and the viewer allows
*                    it, this will be the viewer window's caption
*
* @retval 0 graph was saved/displayed successfully
* @retval 1 output does't have an extension (in order to find out output format)
* @retval 2 graphviz error
* @retval 3 xdot/viewer error
**/
int mxUT::ProcessGraph (wxString graph_file, bool use_fdp, wxString output, wxString title) {
	bool show=!output.Len();
	if (show) {
		if (config->Files.xdot_command.Len())
			output=DIR_PLUS_FILE(config->temp_dir,"temp.xdot");
		else
			output=DIR_PLUS_FILE(config->temp_dir,"temp.png");
	}
	wxString format=output.AfterLast('.').Lower(); if (!format.Len()) return 1;
#if defined(__WIN32__)	
	wxString command(DIR_PLUS_FILE(config->Files.graphviz_dir,"draw.exe"));
#else
	wxString command(DIR_PLUS_FILE(config->Files.graphviz_dir,"draw.bin"));
#endif
	command<<(use_fdp?" fdp ":" dot ");
	command<<Quotize(graph_file)<<" -T"<<format<<" -o "<<Quotize(output);
	if (mxExecute(command,wxEXEC_SYNC)) return 1;
	if (show) {
		wxString command2;
		if (config->Files.xdot_command.Len()) {
			command2<<config->Files.xdot_command<<" -n "<<Quotize(output);
		} else {
			command2<<config->Files.img_browser<<" "<<Quotize(output)<<" \""<<title<<"\"";
		}
		wxExecute(command2);
	}
	return 0;
}

wxString mxUT::ReplaceLangArgs(wxString src, wxString arg1) {
	src.Replace("<{1}>",arg1);
	return src;
	
}

wxString mxUT::ReplaceLangArgs(wxString src, wxString arg1, wxString arg2) {
	src.Replace("<{1}>",arg1);
	src.Replace("<{2}>",arg2);
	return src;
	
}

wxString mxUT::ReplaceLangArgs(wxString src, wxString arg1, wxString arg2, wxString arg3) {
	src.Replace("<{1}>",arg1);
	src.Replace("<{2}>",arg2);
	src.Replace("<{3}>",arg3);
	return src;
	
}

void mxUT::OpenZinjaiSite(wxString page) {
	if (!page.Len()) page="portada.php";
	wxString base("http://zinjai.sourceforge.net");
	if (config->Init.language_file=="spanish") {
		base<<"/index.php?page="<<page;
	} else {
		base<<"/index_en.php?page="<<(page.BeforeFirst('.')+"_en."+page.AfterFirst('.'));
	}
	mxUT::OpenInBrowser(base);
}

wxStaticText * mxUT::GetLastLabel ( ) {
	return last_label;
}

wxButton * mxUT::GetLastButton ( ) {
	return last_button;
}

wxString mxUT::GetClipboardText ( ) {
	wxString text;
	if (wxTheClipboard->Open()) {
		wxTextDataObject clip_data;
		if (wxTheClipboard->GetData(clip_data))
			text = clip_data.GetText();
		wxTheClipboard->Close();
	}
	return text;
}

void mxUT::SetClipboardText (const wxString & text) {
	if (!wxTheClipboard->Open()) return;
	wxTheClipboard->SetData( new wxTextDataObject(text) );
	wxTheClipboard->Close();
}

