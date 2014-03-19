#include <wx/textfile.h>
#include "Autocoder.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "ProjectManager.h"

Autocoder *autocoder=NULL;

Autocoder::Autocoder() {
	if (!LoadFromFile(config->Files.autocodes_file)) {
		SetDefaults(); 
		SaveToFile();
	}
}

void Autocoder::Reset(wxString pfile) {
	Clear();
	if (config->Files.autocodes_file.Len()) 
		LoadFromFile(config->Files.autocodes_file);
	if (pfile.Len()) 
		LoadFromFile(pfile);
}

void Autocoder::SetDefaults() {
	Clear();
	if (config->Init.language_file=="spanish") {
		description=
			"// Este archivo contiene las definiciones de las plantillas de autocódigo. Estas\n"
			"// plantillas se invocan en zinjai typeando su nombre y sus argumentos de la misma\n"
			"// manera en la que se llama a una macro de preprocesador o a una función, y\n"
			"// presinoando la tecla TAB inmediatamente despues. ZinjaI reemplazará el código\n"
			"// de la llamada por el código/texto completo definido para el mismo.\n"
			"// Para definir plantillas de autocódigo se utiliza la palabra clave #autocode\n"
			"// seguida de su nombre. Opcionalmente se pueden colocar argumentos entre\n"
			"// paréntesis y separados por comas. Luego se coloca el código que reemplazará a la\n"
			"// llamada, de igual forma que para una macro. Para crear plantillas de más de una\n"
			"// linea no hace falta agregar \\ al final de cada una. Una definición de autocódigo\n"
			"// termina donde empieza la siguiente, pero las lineas en blanco entre dos plantillas\n"
			"// son ignoradas. Se puede usar la palabra clave #here# para indicar donde colocar el\n"
			"// cursor de texto luego del reemplazo. Se debe notar que las lineas con comentarios\n"
			"// se consideran parte de la plantilla y por lo tanto se incluyen al realizar el\n"
			"// reemplazo, con la única excepción de los comentarios colocados en la primer línea\n"
			"// (#autocode...) y que comienzan con //. En este caso, se considera a ese comentario\n"
			"// como una descripción corta del autocódigo.\n"
			"// Si edita este archivo de definiciones con ZinjaI, al guardarlo los cambios se\n"
			"// reacargan automáticamente.";
	} else {
		description=
			"// This file contains autocodes definitions. Autocodes are some kind of code templates\n"
			"// that you can invoke while coding in ZinjaI, calling them in exactly the same way you\n"
			"// call a macro and pressing TAB key just after it. ZinjaI will inmediatly replace\n"
			"// that call with its full code.\n"
			"// To define an autocode template, use keywords #autocode followed by the name you want\n"
			"// to use to invoke it, as with macros. Optionally you can define arguments in the same\n"
			"// way you do for macros. To create a multiline autocode you dont need to add \\ at the\n"
			"// end of each line. An autocode definition extends until next #autocode tag, but empty\n"
			"// lines at the end are stripped. You can use char #here# to say where the text cursor\n"
			"// should be placed after replacing the autocode. Note that comment lines will be\n"
			"// part of the autocode template and so won't be ignored when replacing the call,\n"
			"// with the exception of any comment in the first #autocode line. In that case, that\n"
			"// comment will be interpreted as that particular autocode's brief descripcion (only\n"
			"// comments starting with // will be recognized when parsing this file).\n"
			"// If you edit autocode definitions file with ZinjaI, changes will apply automatically\n"
			"// when you save the file.";
	}
	
	{ auto_code a; a.code="for(int i=0;i<N;i++) { #here# }"; a.args.Add("i"); a.args.Add("N"); list["for2"]=a; }
	{ auto_code a; a.code="for(int i=0;i<N;i++) { #here# }"; a.args.Add("N"); list["fori"]=a; }
	{ auto_code a; a.code="for(int j=0;j<N;j++) { #here# }"; a.args.Add("N"); list["forj"]=a; }
	{ auto_code a; a.code="for(int k=0;k<N;k++) { #here# }"; a.args.Add("N"); list["fork"]=a; }
	{ auto_code a; a.code="for(unsigned int i=0;i<N;i++) { #here# }"; a.args.Add("i"); a.args.Add("N"); list["foru2"]=a; }
	{ auto_code a; a.code="for(unsigned int i=0;i<N;i++) { #here# }"; a.args.Add("N"); list["forui"]=a; }
	{ auto_code a; a.code="for(unsigned int j=0;j<N;j++) { #here# }"; a.args.Add("N"); list["foruj"]=a; }
	{ auto_code a; a.code="for(unsigned int k=0;k<N;k++) { #here# }"; a.args.Add("N"); list["foruk"]=a; }
	{ auto_code a; a.code="while(true) {\n#here#\n}"; list["whilet"]=a; }
	{ auto_code a; a.code="if(cond) {\n\t#here#\n} else {\n\t\n}"; a.args.Add("cond"); list["ifel"]=a; }
	{ auto_code a; a.code="cout<<x<<endl;"; a.args.Add("x"); list["cout"]=a; }
	{ auto_code a; a.code="switch(x) {\ncase #here# :\n\tbreak;\ndefault:\n}"; a.args.Add("x"); list["switch"]=a; }
	{ auto_code a; a.code="class x {\nprivate:\n\t#here#\npublic:\n\tx();\n\t~x();\n};"; a.args.Add("x"); list["class"]=a; }
	{ auto_code a; a.code="class x : public f {\nprivate:\n\t#here#\nprotected:\n\t\npublic:\n\tx();\n\t~x();\n};"; a.args.Add("x"); a.args.Add("f"); list["classh"]=a; }
	{ auto_code a; a.description="dummy text"; 
	a.code=
		"\"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor \"\n"
		"\"incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud \"\n"
		"\"exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute \"\n"
		"\"irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla \"\n"
		"\"pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia \"\n"
		"\"deserunt mollit anim id est laborum.\"";
	list["lorem_ipsum"]=a; }
}

void Autocoder::Clear() {
	list.clear();
	description.Clear();
}

bool Autocoder::LoadFromFile(wxString filename) {
	if (!filename.Len()) filename=DIR_PLUS_FILE(config->home_dir,"autocodes");
	wxTextFile fil(filename);
	if (!fil.Exists()) return false;
	fil.Open(); auto_code ac; wxString name;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (str.StartsWith("#autocode ")) {
			while (ac.code.Last()=='\r'||ac.code.Last()=='\n') ac.code.RemoveLast();
			if (name.Len()) list[name]=ac;
			else description=ac.description;
			ac.Clear();
			int i=10, l=str.Len();
			while (i<l && (str[i]==' '||str[i]=='\t')) i++;
			int li=i; char c;
			while (i<l && (c=str[i]) && ((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_')) i++;
			name=str.Mid(li,i-li);
			while (str[i]==' '||str[i]=='\t') i++;
			if (i<l) {
				if (str[i]=='(') {
					++i;
					while (true) {
						while (str[i]==' '||str[i]=='\t') i++; li=i;
						while(i<l && str[i]!=',' && str[i]!=')') i++;
						wxString par=str.Mid(li,i-li);
						while (par.Len() && (par.Last()==' '||par.Last()=='\t')) par.RemoveLast();
						ac.args.Add(par);
						li=++i;	
						if (i==l || str[i]==')') break;
					}
				}
				if (i+1<l && str[i]=='/' && str[i+1]=='/') {
					i+=2;
					while (str[i]==' '||str[i]=='\t') i++;
					ac.description=str.Mid(i);
				} else {
					ac.code=str.Mid(i);
				}
			}
		} else {
			if (ac.code.Len())
				ac.code<<"\n"<<str;
			else if (str.Len())
				ac.code<<str;
		}
	}
	while (ac.code.Last()=='\r'||ac.code.Last()=='\n') ac.code.RemoveLast();
	if (name.Len()) list[name]=ac;
	return true;
}

void Autocoder::SaveToFile(wxString filename) {
	if (!filename.Len()) filename=DIR_PLUS_FILE(config->home_dir,"autocodes");
	wxTextFile fil(filename);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	fil.AddLine(description);
	fil.AddLine("");
		
	HashStringAutoCode::iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
		wxString head("#autocode ");
		head<<it->first;
		if (it->second.args.GetCount()) {
			head<<"(";
			for (unsigned int i=0;i<it->second.args.GetCount();i++) 
				head<<it->second.args[i]<<",";
			head.RemoveLast(); head<<")";
		}
		if (it->second.description.Len())
			head<<" // "<<it->second.description;
		fil.AddLine(head);
		fil.AddLine(it->second.code);
		fil.AddLine("");
	}
	fil.Write();
	fil.Close();
}


bool Autocoder::Apply(mxSource *src, auto_code *ac, bool args) {
	wxString code=ac->code;
	if (ac->args.GetCount()) {
		if (!args) return false; // faltan los argumentos
		// armar la lista de argumentos
		wxArrayString array;
		wxString str=src->GetTextRange(src->GetTargetStart(),src->GetTargetEnd());
		int i=0, l=str.Len(); str[l-1]=',';
		while (i<l&&str[i]!='(') i++; 
		int parentesis=0,li=++i;
		while (i<l) {
			if (str[i]=='\'') {
				i++; if (str[i]=='\\') i++; i++;
			} else if (str[i]=='\"') {
				i++; 
				while (str[i]!='\"') {
					if (str[i]=='\\') i++; i++;
				}
			} else if (str[i]=='(') {
				parentesis++;
			} else if (str[i]==')') {
				parentesis--;
			} else if (parentesis==0 && str[i]==',') {
				array.Add(str.Mid(li,i-li));li=i+1;
			}
			i++;
		}
		if (array.GetCount()!=ac->args.GetCount()) return false; // cantidad de parametros incorrecta
		// recorrer palabra por palabra de code y tratar de reemplazar
		i=0;l=code.Len();li=0; int r;
		while (i<=l) {
			char c=i<l?code[i]:'*';
			if (!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_')) {
				if (li!=i) {
					r=ac->args.Index(code.Mid(li,i-li));
					if (r!=wxNOT_FOUND) {
						if (li>1&&code[li-1]=='#'&&code[li-2]=='#') { 
							li-=2; 
							code=code.Mid(0,li)+array[r]+code.Mid(i);
							i+=array[r].Len()+2-(i-li);
						} else if (li>0&&code[li-1]=='#') { 
							li--; 
							code=code.Mid(0,li)+"\""+array[r]+"\""+code.Mid(i);
							i+=array[r].Len()+2-(i-li);
						} else {
							code=code.Mid(0,li)+array[r]+code.Mid(i);
							l+=array[r].Len()-(i-li);
							i+=array[r].Len()-(i-li);
						}
					}
				}
				li=i+1;
			}
			i++;
		}
	} else 
		if (args) return false; // sobran los argumentos
	int p=code.Find("#here#");
	if (p!=wxNOT_FOUND) code=code.Mid(0,p)+code.Mid(p+6);
	src->ignore_char_added=true;
	src->ReplaceTarget(src->GetTextRange(src->GetTargetStart(),src->GetTargetEnd())+"\t"); // para que ctrl+z sÃ­ ponga el tab
	src->ReplaceTarget(code);
	src->ignore_char_added=false;
	// ubicar seleccion e indentar si es necesario (solo las linea agregadas, la primera no)
	int l0=src->LineFromPosition(src->GetTargetStart());
	int l1=src->LineFromPosition(src->GetTargetEnd());
	if (p!=wxNOT_FOUND)
		src->SetSelection(src->GetTargetStart()+p,src->GetTargetStart()+p);
	else
		src->SetSelection(src->GetTargetEnd(),src->GetTargetEnd());
	if (src->config_source.syntaxEnable) {
		src->Colourise(src->GetTargetStart(),src->GetTargetEnd());
		if (l0<l1) src->Indent(l0+1,l1);
	}
	return true;
}


bool Autocoder::Apply(mxSource *src) {	
	wxString line=src->GetLine(src->GetCurrentLine());
	int st=src->PositionFromLine(src->GetCurrentLine());
	int i=src->GetCurrentPos()-st-1; int s=i;
	if (i<0) return false;
	char c=line[i]; bool args=false;
	if (c==')') {
		int m=src->BraceMatch(src->GetCurrentPos()-1);
		if (m==wxSTC_INVALID_POSITION) return false;
		i=m-st;	if (i<0) return false; // tiene que estar todo en una sola linea
		if (i<s-1) args=true;
		c=line[--i];
	} 
	if ((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_') {
		int e=i+1;
		while (i>=0 && (c=line[i]) && ((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_')) i--;
		i++; c=line[i]; if (c>='0'&&c<='9') return false;  
//		cerr<<line.Mid(i,e-i)<<endl;
		HashStringAutoCode::iterator it=list.find(line.Mid(i,e-i));
		if (it==list.end()) {
			char cping[6]="pung"; cping[1]='i';
			if (line.Mid(i,e-i)==cping) { // :)
				src->SetTargetStart(st+i); src->SetTargetEnd(st+s+1);
				cping[4]='\t'; cping[5]='\0';
				src->ReplaceTarget(cping);
				src->BeginUndoAction();
				src->SetTargetStart(st+i); src->SetTargetEnd(st+s+2);
				src->ReplaceTarget("pong");
				src->EndUndoAction();
				return true;
			} else if (line.Mid(i,e-i)=="tic") { // X)
				src->SetTargetStart(st+i); src->SetTargetEnd(st+s+1);
				src->ReplaceTarget("tic\t");
				src->BeginUndoAction();
				src->SetTargetStart(st+i); src->SetTargetEnd(st+s+2);
				src->ReplaceTarget("tac");
				src->EndUndoAction();
				return true;
			}
			return false;
		}
		src->SetTargetStart(st+i); src->SetTargetEnd(st+s+1);
		return Apply(src,&(it->second),args);
	}
	return false;
}
