#include "ids.h"
#include "mxUtils.h"

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

#include "mxSource.h"
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxArt.h"
#include <wx/dir.h>
#include "mxSizers.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "Language.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"
using namespace std;

#define CTRL_BORDER 5

#define min2(a,b) ((a)<(b)?(a):(b))

#ifdef DEBUG
wxString debug_string;
#endif 

mxUtils *utils;
char path_sep = wxFileName::GetPathSeparator();

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
wxString mxUtils::JoinDirAndFile(wxString dir, wxString fil) {
	if (dir.Len()==0 || (fil.Len()>1 && (fil[0]=='\\' || fil[0]=='/' || fil[1]==':')))
		return fil;
	else if (dir.Last()==path_sep)
		return dir+fil;
	else
		return dir+path_sep+fil;
}

/*wxBoxSizer *mxUtils::CreateSizerWithLabelAndText(wxWindow *win, wxWindowID id, wxTextCtrl * &text, wxString label) {
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	text = new wxTextCtrl(win, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizerRow->Add(new wxStaticText(win, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->Add(text, sizers->Exp1);
	return sizerRow;
}*/

void mxUtils::strcpy(wxChar *cstr, wxString wstr) {
	unsigned int i;
	for (i=0;i<wstr.Len();i++)
		cstr[i]=wstr[i];
	cstr[i]='\0';
} 

void mxUtils::strcpy(wxChar *cstr, wxChar *wstr) {
	unsigned int i=0;
	while (wstr[i]!='\0') {
		cstr[i]=wstr[i];
		i++;
	}
	cstr[i]='\0';
} 

void mxUtils::SortArrayString(wxArrayString &array, int inf, int sup) {

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

wxCheckBox *mxUtils::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value, wxWindowID id, bool margin) {
	wxCheckBox *checkbox = new wxCheckBox(panel, id, text+_T("   "));
	if (margin) {
		wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
		sizerRow->AddSpacer(15);
		sizerRow->Add(checkbox, sizers->Exp1);
		sizer->Add(sizerRow, sizers->BLRT5_Exp0);
	} else 
		sizer->Add(checkbox,sizers->BA5_Exp0);
	checkbox->SetValue(value);
	return checkbox;	
}

wxTextCtrl *mxUtils::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	return textctrl;
}

wxTextCtrl *mxUtils::AddLongTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	sizer->Add(last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp1);
	textctrl->SetValue(value);
	return textctrl;
}

wxTextCtrl *mxUtils::AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text, bool margin) {
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

wxTextCtrl *mxUtils::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(15);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	textctrl->SetValue(wxString::Format(_T("%d"), value));
	return textctrl;
}

wxStaticText* mxUtils::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
	wxStaticText *textctrl = new wxStaticText(panel, wxID_ANY, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->AddStretchSpacer(1);
	sizerRow->Add(textctrl, sizers->Exp0);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	return textctrl;	
}

wxTextCtrl *mxUtils::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	return textctrl;
}

wxTextCtrl *mxUtils::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString foot, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxString()<<n, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(" "), wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, wxString(_T(" "))<<foot, wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	return textctrl;
}

wxComboBox *mxUtils::AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID id, bool margin) {
	wxString sdef=def==-1?_T(""):values[def];
	wxComboBox *combo = new wxComboBox(panel, id, sdef, wxDefaultPosition, wxDefaultSize, values, wxCB_READONLY);
	combo->SetSelection(def);
	wxBoxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(combo, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	return combo;
}

wxStaticText *mxUtils::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text) {
	wxStaticText *statictext;
	sizer->Add( statictext = new wxStaticText(panel, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), sizers->BA5_Center);
	return statictext;
}

wxMenuItem *mxUtils::AddItemToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString accel, wxString help, wxString filename, int where) {
	if (accel.Len()) caption<<_T("\t")<<accel;
	wxMenuItem *item = new wxMenuItem(menu,id,caption,help);
	filename=SKIN_FILE_OPT(filename);
	if (wxFileName::FileExists(filename))
		item->SetBitmap(wxBitmap(filename,wxBITMAP_TYPE_PNG));
	if (where==-1) menu->Append (item);
	else menu->Insert(where,item);
	return item;
}


wxMenuItem *mxUtils::AddSubMenuToMenu(wxMenu *menu, wxMenu *menu_h, wxString caption, wxString help, wxString filename) {
	wxMenuItem *item = 	menu->AppendSubMenu(menu_h, caption, help);
	filename=SKIN_FILE(filename);
	if (wxFileName::FileExists(filename))
		item->SetBitmap(wxBitmap(filename,wxBITMAP_TYPE_PNG));
	return item;
}

wxMenuItem *mxUtils::AddCheckToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString accel, wxString help, bool value) {
	if (accel.Len()) caption<<_T("\t")<<accel;
	wxMenuItem *item = menu->AppendCheckItem (id, caption,help);
	item->Check(value);
	return item;
}

void mxUtils::AddTool(wxToolBar *toolbar, wxWindowID id, wxString caption, wxString filename, wxString status_text, wxItemKind kind) {
	filename=mxUtils::JoinDirAndFile(config->Files.skin_dir,filename);
	if (wxFileName::FileExists(filename)) {
		toolbar->AddTool(id, caption, wxBitmap(filename,wxBITMAP_TYPE_PNG),caption, kind);
		toolbar->SetToolLongHelp(id,status_text);
	}
}

bool mxUtils::ToInt(wxString &value, int &what){
	static long l;
	if (value.ToLong(&l)) {
		what=int(l);
		return true;
	} else return false;
}

bool mxUtils::IsTrue(wxString &value){
	return (value[0]=='1' || value[0]=='V' || value[0]=='v' || value[0]=='s' || value[0]=='S' || value[0]=='T' || value[0]=='t');
}



wxString mxUtils::FindIncludes(wxFileName filename, wxString ref_path, wxArrayString &header_dirs) {
	fi_file_item *prev, *item=new fi_file_item;
	item->next=new fi_file_item(filename.GetFullPath());
	FindIncludes(filename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR),filename.GetFullName(),item,header_dirs);
	wxString deps;
	wxFileName file_name;
	ML_WHILE(item) {
		prev=item;
		ML_NEXT(item);
		delete prev;
		file_name=item->name;
		file_name.MakeRelativeTo(ref_path);
		deps+=file_name.GetFullPath()+_T(" ");
	}
	delete item;
	if (deps.Len()==0) 
		return _T("");
	else
		return deps.RemoveLast();
}

void mxUtils::FindIncludes(wxArrayString &deps, wxFileName filename, wxString ref_path, wxArrayString &header_dirs) {
	deps.Clear();
	fi_file_item *prev, *item=new fi_file_item;
	item->next=new fi_file_item(filename.GetFullPath()); // FindIncludes usa el primer item para sacar le path?
	FindIncludes(filename.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR),filename.GetFullName(),item,header_dirs,false);
	prev=item; ML_NEXT(item); delete prev; // borrar el primer item (el mismisimo archivo)
	wxFileName file_name;
	ML_WHILE(item) {
		prev=item;
		ML_NEXT(item);
		delete prev;
		file_name=item->name;
		file_name.MakeRelativeTo(ref_path);
		deps.Add(file_name.GetFullPath());
	}
	delete item;
}

bool mxUtils::AreIncludesUpdated(wxDateTime bin_date, wxFileName filename) {
	wxArrayString as;
	return AreIncludesUpdated(bin_date,filename,as);
}

bool mxUtils::AreIncludesUpdated(wxDateTime bin_date, wxFileName filename, wxArrayString &header_dirs) {
	
	// inicializar la lista
	fi_file_item *prev, *first=new fi_file_item;
	wxString my_path=filename.GetPathWithSep();
	first->name=my_path; // el primer elemento (ficticio) guarda la ruta del archivo original
	first->next=new fi_file_item(filename.GetFullPath());
	// averiguar las dependencias
	FindIncludes(_T(""),filename.GetFullName(),first,header_dirs);
	// comparar las fechas y liberar memoria
	bool ret=false,future=false;
	wxDateTime now=wxDateTime::Now();
	while (first->next!=NULL) {
		prev=first;
		first=first->next;
		if (!ret) {
			if (project) {
				char h='h';
				file_item *fi=project->FindFromName(first->name,&h);
				if (fi && fi->force_recompile) ret=true;
			}
			wxDateTime dt = wxFileName(first->name).GetModificationTime();
			ret = wxFileName(first->name).GetModificationTime()>bin_date;
			if ((now-dt).GetSeconds().ToLong()<-3) { // si es del futuro, arreglar y recompilar
				wxFileName(first->name).Touch();
				future=true;
				if (project)
					project->warnings.Add(wxString(LANG(UTILS_FUTURE_FILE_PRE,"El archivo incluido "))<<first->name<<LANG(UTILS_FUTURE_SOURCE_POST," tenia fecha de modificacion en el futuro. Se reemplazo por la fecha actual."));
				mxSource *src = main_window->IsOpen(first->name);
				if (src) src->Reload();
			}
		}
		delete prev;
	}
	delete first;
	return ret||future;
}

/** 
* Busca los includes del archivo filename que esta en el directorio path, relativo 
* al directorio original first->name 
**/
void mxUtils::FindIncludes(wxString path, wxString filename, fi_file_item *first, wxArrayString &header_dirs, bool recursive) {
	wxTextFile text_file(DIR_PLUS_FILE(DIR_PLUS_FILE(first->name,path),filename));
	wxFileName file_name;
	text_file.Open();
	fi_file_item *item;
	wxString line, this_one;
	int pos, len, ini, end;
	// recorrer el fuente para buscar que incluye
	for ( line = text_file.GetFirstLine(); !text_file.Eof(); line = text_file.GetNextLine() ) { 
		// buscar donde comienza cada linea
		pos=0; len=line.Len();
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
				if (this_one!=_T(""))
					this_one=GetOnePath(DIR_PLUS_FILE(first->name,path),project?project->path:DIR_PLUS_FILE(first->name,path),this_one,header_dirs);
				if (this_one.Len()) {
					file_name=this_one;
					file_name.Normalize(wxPATH_NORM_DOTS);
					this_one=file_name.GetFullPath();
					// ver que no esta ya en la lista
					item=first;
					bool no_esta=true;
					while (item->next!=NULL) {
						item=item->next;
						if (item->name==this_one) {
							no_esta=false;
							break;
						}
					}
					if (no_esta) {
						// agregarlo a la lista de archivos ya considerados
						item->next=new fi_file_item(this_one);
						// buscar las dependencias recursivamente
						if (recursive) 
							FindIncludes(file_name.GetPathWithSep(),file_name.GetFullName(),first,header_dirs);
					}
				}
			}
		}
	}
}

wxString mxUtils::UnSplit(wxArrayString &array, wxString sep, bool add_quotes) {
	wxString ret;
	if (array.GetCount()) {
		if (add_quotes && array[0].Contains(_T(" ")))
			ret<<_T("\"")<<array[0]<<_T("\"");
		else
			ret<<array[0];
		for (unsigned int i=1;i<array.GetCount();i++) {
			ret<<sep;
			if (add_quotes && array[0].Contains(_T(" ")))
				ret<<_T("\"")<<array[0]<<_T("\"");
			else
				ret<<array[0];
		}
	}
	return ret;
}

wxString mxUtils::Split(wxString str, wxString pre) {
	int i=0,s, l=str.Len();
	wxString ret;
	bool comillas = false;
	while (i<l) {
		while (i<l && (str[i]==' ' || str[i]=='\t' || str[i]==',' || str[i]==';')) {
			i++;
		}
		s=i;
		while (i<l && (comillas || !(str[i]==' ' || str[i]=='\t' || str[i]==',' || str[i]==';'))) {
			if (str[i]=='\"') 
				comillas=!comillas;
			i++;
		}
		if (s<i)
			ret<<pre<<str.Mid(s,i-s)<<_T(" ");
	}
	return ret;
}

int mxUtils::Split(wxString str, wxArrayString &array, bool coma_splits,bool keep_quotes) {
	int c=0, i=0, s, l=str.Len();
	wxString ret;
	bool comillas = false;
	while (i<l) {
		while (i<l && (str[i]==' ' || str[i]=='\t' || (coma_splits && (str[i]==',' || str[i]==';') ) ) ) {
			i++;
		}
		s=i;
		while (i<l && (comillas || !(str[i]==' ' || str[i]=='\t' || (coma_splits && (str[i]==',' || str[i]==';') ) ) ) ) {
			if (str[i]=='\"') 
				comillas=!comillas;
			i++;
		}
		if (s<i) {
			if (!keep_quotes && str[s]=='\"' && str[i-1]=='\"')
				array.Add(str.Mid(s+1,i-s-2));
			else
				array.Add(str.Mid(s,i-s));
			c++;
		}
	}
	return c;
}

int mxUtils::Execute(wxString path, wxString command, int sync) {
	while (command.Len() && command.Last()==' ') command.RemoveLast();
	wxProcess *p=NULL;
	return Execute(path,command,sync,p);
}

int mxUtils::Execute(wxString path, wxString command, int sync, wxProcess *&process) {
	while (command.Len() && command.Last()==' ') command.RemoveLast();
	wxSetWorkingDirectory(path);
	int ret = wxExecute (command, sync, process);
	wxSetWorkingDirectory(config->zinjai_dir);
	return ret;
}

bool mxUtils::XCopy(wxString src,wxString dst, bool ask, bool replace) {
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
			int ret = mxMessageDialog(NULL,wxString(LANG(FILE_EXISTS_ASK_REPLACE_PRE,"El archivo \""))<<dest_file<<LANG(FILE_EXISTS_ASK_REPLACE_POST,"\" ya existe. Desea reemplazarlo?"),_T("Aviso"),mxMD_YES_NO|mxMD_QUESTION,LANG(GENERAL_DONT_ASK_AGAIN,"No volver a preguntar"),false).ShowModal();
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

wxString mxUtils::ExecComas(wxString where, wxString line) {
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

wxString mxUtils::GetOutput(wxString command, bool also_error) {
	wxString ret;
	wxArrayString output,errors;
	if (also_error)
		wxExecute(command, output, errors, wxEXEC_NODISABLE|wxEXEC_SYNC);
	else
		wxExecute(command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
	if (also_error) {
		for (unsigned int i=0;i<errors.GetCount();i++)
			if (ret.Len()==0)
				ret<<errors[i];
			else
				ret<<_T("\n")<<errors[i];
	}
	for (unsigned int i=0;i<output.GetCount();i++)
		if (ret.Len()==0)
			ret<<output[i];
		else
			ret<<_T("\n")<<output[i];
	return ret;
}

/** 
* Devuelve una cadena convertida a HTML. Reemplaza los espacios y operadores por
* los codigos de escape adecuados (\&...).
* @param text La cadena a convertir
* @param full Determina si reemplazar tambien espacios y \\r (true) o solo operadores y \\n (false)
* @retval La cadena en formato HTML
**/
wxString mxUtils::ToHtml(wxString text, bool full) {
	static wxString tabs;
	static int tabs_n;
	if (tabs_n!=config->Source.tabWidth) {
		tabs=_T("");
		for (int i=0;i<config->Source.tabWidth;i++)
			tabs<<_T("&nbsp;");
		tabs_n=config->Source.tabWidth;
	}
	text.Replace(_T("&"),_T("&amp;"));
	text.Replace(_T("\t"),tabs);
	text.Replace(_T("<"),_T("&lt;"));
	text.Replace(_T(">"),_T("&gt;"));
	text.Replace(_T("\n"),_T("<BR>"));
	if (full) {
		wxChar doce[]=_T(" ");
		doce[0]=12;
		text.Replace(doce,_T("<BR>"));
		text.Replace(_T(" "),_T("&nbsp;"));
	}
	return text;
}

void mxUtils::ParameterReplace(wxString &str, wxString from, wxString to) {
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
			final<<utils->Quotize(str.SubString(p1,p-1)+to+str.SubString(p+from.Len(),p2));
			str=str.Mid(p2+1);
			p=str.Find(from);
			l=str.Len();
		}
		final<<str;
		str=final;
	}
}

//void mxUtils::MakeDesktopIcon (wxString desk_dir) {
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
//}


wxString mxUtils::GetOnePath(wxString orig_path, wxString project_path, wxString fname, wxArrayString &header_dirs) {
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
	return _T("");
}

/**
* Compara el final de la primer cadena con la segunda (completa).
* @param s1 primer cadena
* @param s2 segunda cadena
* @return true si el final de s1 es igual a s2 (case-insensitive)
**/
bool mxUtils::EndsWithNC(wxString s1, wxString s2) {
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
wxString mxUtils::LeftTrim(wxString str) {
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
int mxUtils::Levenshtein(const char *s1, int n1, const char *s2, int n2) {
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
void mxUtils::Purgue(wxArrayString &array, bool sort) {
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
bool mxUtils::Compare(const wxArrayString &array1, const wxArrayString &array2, bool check_case) {
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
* Dado un nombre de archivo, busca el complementario. Es decir, para un .h busca
* (verificando que exista) un .c/.cpp /.cxx/.c++ con el mismo nombre y viseversa.
* @param the_one path completo del archivo
* @param force_ext sirve para forzar el tipo de archivo que recibe: cabecera 'h' 
	               o fuente 'c'. El default '*' adivina según la extensión
* @return el nombre del complementario, o una cadena vacia si no lo encuentra
**/
wxString mxUtils::GetComplementaryFile(wxFileName the_one, char force_ext) {
	char ctype=GetFileType(the_one.GetFullPath());
	if ( force_ext=='c' || ctype=='s' ) {
		the_one.SetExt(_T("h"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("hpp"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("hxx"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("h++"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
	} else {
		the_one.SetExt(_T("cpp"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("cxx"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("c++"));
		if (the_one.FileExists())
			return the_one.GetFullPath();
		the_one.SetExt(_T("c"));
		if (the_one.FileExists())
			return the_one.GetFullPath();	
	}
	return wxString();
}

/**
* Busca los procesos en ejecucion que se lanzaron desde zinjai. Para esto
* busca los procesos hijos de zinjai que no empiecen como config->Files.terminal_command
* (proyectos lanzados sin terminal y runner) y los "bisnietos" (lanzados en un terminal,
* zinjai->consola->runner->bisnieto). En windows no está implementado.
**/
void mxUtils::GetRunningChilds(wxArrayString &childs) {
	long pid = wxGetProcessId();
	if (!pid) return;

#if !defined(_WIN32) && !defined(__WIN32__)
	
	struct proc { wxString cmd; long pid,ppid; int sel; };
	wxArrayString output;
	wxString ps_command=_T("ps --sort -pid -e --format '%P %p %a'");
	wxExecute(ps_command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
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

void mxUtils::OpenInBrowser(wxString url) {
	if (config->Files.browser_command.Len()) {
		if (config->Files.browser_command=="shellexecute.exe" && url.StartsWith("file://"))
			url=url.Mid(7);
		wxExecute(config->Files.browser_command+_T(" \"")+url+_T("\""));	
	} else
		wxLaunchDefaultBrowser(url);
}

/** 
* Convierte un path absoluto en relativo, siempre y cuando no deba subir más
* de 2 niveles desde el path de referencia
**/
wxString mxUtils::Relativize(wxString name, wxString path) {
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

void mxUtils::OpenFolder(wxString path) {
	if (config->Files.explorer_command==_T("<<sin configurar>>")) {
		wxMessageBox(LANG(CONFIG_EXPLORER_PROBLEM,"No se ha definido el explorador de archivos a utilizar\n"
			                                       "Puede configurar el mismo en la pestana \"Rutas 2\" del\n"
												   "cuadro de Preferencias (menu Archivo->Preferencias)."),LANG(CONFIG_EXPLORER,"Explorador de archivos"));
	} else {
		wxString cmd = config->Files.explorer_command;
		cmd<<_T(" \"")<<path<<_T("\"");
		utils->Execute(path,cmd,wxEXEC_NOHIDE);
	}
}


wxString mxUtils::Quotize(const wxString &what) {
	if (what.Find(' ')!=wxNOT_FOUND)
		return wxString(_T("\""))<<what<<_T("\"");
	else
		return what;
}

wxString mxUtils::EscapeString(wxString str, bool add_comillas) {
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


wxString mxUtils::Text2Line(wxString &text) {
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

wxString mxUtils::Line2Text(wxString &line) {
	int l=line.Len(),i,p=0;
	wxString text;
	for (i=0;i<l;i++) {
		if (line[i]=='\\') {
			if (line[i+1]=='n') {
				text<<line.Mid(p,i-p)<<_T("\n");
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

wxString mxUtils::UrlEncode(wxString str) {
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
void mxUtils::ShowTextPopUp(wxWindow *parent, wxString title, wxTextCtrl *text, wxString options, wxString path) {
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
		else if (opt=="MINGW_DIR") menu.Append(mxID_POPUPS_INSERT_MINGW_DIR,LANG(GENERAL_POPUP_INSERT_MINGW_DIR,"insertar directorio del compilador"));
		else if (opt=="PROJECT_PATH") menu.Append(mxID_POPUPS_INSERT_PROJECT_PATH,LANG(GENERAL_POPUP_INSERT_PROJECT_PATH,"insertar directorio del proyecto"));
		else if (opt=="PROJECT_BIN") menu.Append(mxID_POPUPS_INSERT_PROJECT_BIN,LANG(GENERAL_POPUP_INSERT_PROJECT_BIN,"insertar ruta del ejecutable"));
		else if (opt=="CURRENT_FILE") menu.Append(mxID_POPUPS_INSERT_CURRENT_FILE,LANG(GENERAL_POPUP_INSERT_CURRENT_FILE,"insertar archivo actual"));
		else if (opt=="CURRENT_DIR") menu.Append(mxID_POPUPS_INSERT_CURRENT_DIR,LANG(GENERAL_POPUP_INSERT_CURRENT_DIR,"insertar directorio del archivo actual"));
		else if (opt=="BIN_WORKDIR") menu.Append(mxID_POPUPS_INSERT_WORKDIR,LANG(GENERAL_POPUP_INSERT_WORKDIR,"insertar el directorio de trabajo"));
		else if (opt=="ZINJAI_DIR") menu.Append(mxID_POPUPS_INSERT_ZINJAI_DIR,LANG(GENERAL_POPUP_INSERT_ZINJAI_DIR,"insertar el directorio de instalacion de ZinjaI"));
		else if (opt=="BROWSER") menu.Append(mxID_POPUPS_INSERT_BROWSER,LANG(GENERAL_POPUP_INSERT_BROWSER,"insertar comando del navegador"));
	} while (options.Len());
	ProcessTextPopup(0,parent,text,path,title,replace,comma);
	parent->PopupMenu(&menu);
}

void mxUtils::ProcessTextPopup(int id, wxWindow *parent, wxTextCtrl *t, wxString path, wxString title, bool replace, bool comma_splits) {
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
			if (path.Len()) text = utils->Relativize(dlg.GetPath(),path);
			if (text.Contains(' ')) text = wxString(_T("\""))<<text<<_T("\"");
		} else if (id==mxID_POPUPS_INSERT_DIR) {
			wxDirDialog dlg(win,caption,text.Len()?text:(project?project->last_dir:config->Files.last_dir));
			if (wxID_OK!=dlg.ShowModal()) return;
			(project?project->last_dir:config->Files.last_dir) = dlg.GetPath(); 
			if (path.Len()) text = utils->Relativize(dlg.GetPath(),path);
			if (text.Contains(' ')) text = wxString(_T("\""))<<text<<_T("\"");
		} else if (id==mxID_POPUPS_INSERT_MINGW_DIR) {
			text="${MINGW_DIR}";
		} else if (id==mxID_POPUPS_INSERT_TEMP_DIR) {
			text="${TEMP_DIR}";
		} else if (id==mxID_POPUPS_INSERT_BROWSER) {
			text="${BROWSER}";
		} else if (id==mxID_POPUPS_INSERT_PROJECT_PATH) {
			text="${PROJECT_PATH}";
		} else if (id==mxID_POPUPS_INSERT_PROJECT_BIN) {
			text="${PROJECT_BIN}";
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

char mxUtils::GetFileType(wxString name, bool recognize_projects) {
	wxString pto="."; if (!name.Contains(pto)) return 'o';
	name=name.AfterLast('.').Lower();
	if (!name.Len()) return 'o';
	if (name.Last()=='\"') name.RemoveLast();
	pto[0]=path_sep; if (name.Contains(pto)) return 'o';
	if (name=="cpp"||name=="c"||name=="c++"||name=="cxx")
		return 's';
	if (name=="h"||name=="hpp"||name=="hxx")
		return 'h';
	if (recognize_projects && name==PROJECT_EXT)
		return 'z';
	return 'o';		
}

#define _dummy_macro_for_extract_enum_bin \
	LANG(GENERAL_WAIT_PARSER,"Debe esperar a que se terminen de analizar todos los fuentes."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();


/**
* @brief Retreaves all files or folders names from a specified path
*
* @param array files/folders will be appended to this array
* @param path folder path where the method should look for files/folders
* @param files true for retriving files, false for retrieving folders
* @return number of files/folder if path exists, -1 if path does not exists
**/
int mxUtils::GetFilesFromDir (wxArrayString & array, wxString path, bool files) {
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

int mxUtils::Unique (wxArrayString &array, bool do_sort) {
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

wxString mxUtils::WichOne(wxString file, wxString path1, wxString path2, bool is_file) {
	wxString res=DIR_PLUS_FILE(path1,file);
	if (is_file) { if (wxFileExists(res)) return res; }
	else { if (wxDirExists(res)) return res; }
	res=DIR_PLUS_FILE(path2,file);
	if (is_file) { if (wxFileExists(res)) return res; }
	else { if (wxDirExists(res)) return res; }
	return "";
}
