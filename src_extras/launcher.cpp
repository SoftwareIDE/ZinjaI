#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <dlfcn.h>
using namespace std;

const char *binname="bin/zinjai.bin";

const char *my_getenv(const char *varname) {
	char empty[]="";
	const char *res = getenv(varname);
	return res?res:empty;
}

void ReplaceEnvVar(const char *varname, const char *new_value) {
	string zvname=string("ZINJAI_EV_")+varname;
	const char *old_value = getenv(varname);
	setenv(zvname.c_str(),old_value?old_value:"ZINJAI_UNSET",1);
	setenv(varname,new_value,1);
}

void fix_argv(char *argv[]) {
	int n=strlen(argv[0])-1;
	char *argv0=new char[n+15];
	while (n>0&&argv[0][n]!='/') n--;
	if (argv[0][n]=='/') strncpy(argv0,argv[0],++n);
	strcpy(argv0+n,binname);
	argv[0]=argv0;
}

void FixUbuntuMenuTweaks() {
	
	// check if ~/.zinjai/ubuntu file exists, this indicates if we should 
	// disable ubuntu's especial menues and toolbars or not
	string home_path = my_getenv("HOME");
	if (!home_path.empty() && !home_path[home_path.size()-1]!='/') 
		home_path+="/";
	home_path+=".zinjai/ubuntu";
	if( access(home_path.c_str(),F_OK) == -1 ) {
		// disable unity's customizations
		ReplaceEnvVar("UBUNTU_MENUPROXY","");
		ReplaceEnvVar("LIBOVERLAY_SCROLLBAR","0");
	}
}

void FixLocaleSettingsForScintilla() {
	setenv("LANG","",1); // workaround for the problem with ansi-wxstyledtext and utf8 locale
	setenv("LC_CTYPE","",1); // workaround for the problem with ansi-wxstyledtext and utf8 locale
}

void FixMissingLibPNG12(string zinjai_path) {
	void *handle = dlopen ("libpng12.so", RTLD_LAZY);
	if (!handle) {
		string ld_library_path = my_getenv("LD_LIBRARY_PATH");
		if (!ld_library_path.empty()) ld_library_path+=":";
		if (zinjai_path.find('/')==string::npos) zinjai_path="";
		else zinjai_path=zinjai_path.substr(0,zinjai_path.rfind('/')+1);
		ld_library_path+=zinjai_path+"libs";
		setenv("LD_LIBRARY_PATH",ld_library_path.c_str(),1);
	} else {
		dlclose(handle);
	}
}

int main(int argc, char *argv[]) {

	FixUbuntuMenuTweaks();
	FixLocaleSettingsForScintilla();
	FixMissingLibPNG12(argv[0]);
	
	fix_argv(argv);
	pid_t pid=fork();
	if (pid==0) {
		execvp(argv[0],argv);
		// ver si es porque no tiene permiso de ejecuci�n, en cuyo caso intentamos cambiarlos
		struct stat sb;
		if (stat(argv[0],&sb)!=-1 && !(sb.st_mode&S_IXUSR)) {
			string cmd("chmod a+x \""); cmd+=argv[0]; cmd+="\"";
			system(cmd.c_str());
			execvp(argv[0],argv);
			cerr<<"Necesita darle permisos de ejecuci�n al archivo zinjai.bin."<<endl;
			cerr<<"Puede intentar hacerlo con el comando \"sudo chmod a+x zinjai.bin\"."<<endl;
		}
	} else {
		int ret=0;
		waitpid(pid,&ret,0);
		if (ret) { // si algo fall� en la ejecuci�n, ver si faltan bibliotecas
			string s;
			s+="ldd \""; s+=argv[0]; s+="\""; 
			s+=" | grep \"not found\" >/dev/null";
			if (system(s.c_str())==0) {
				cerr<<endl<<endl;
				cerr<<"Puede que su sistema no tenga todas las bibliotecas necesarias para ejecutar ZinjaI."<<endl;
				cerr<<"Instale las bibliotecas faltantes con el gestor de paquetes de su distribución."<<endl;
				cerr<<"Las bibliotecas faltantes son:"<<endl;
				string s; s+="ldd \""; s+=argv[0]; s+="\""; 
				s+=" | grep \"not found\" | cut -d . -f 1";
				system(s.c_str());
				cerr<<endl;
				cerr<<"Por ejemplo, si la biblioteca faltante es \"libpng12.so.0\", puede instalarla con:"<<endl;
				cerr<<"    En Debian/Ubuntu y derivados, el comando: \"sudo apt-get install libpng12-0\""<<endl;
				cerr<<"    En Arch Linux, el comando: \"sudo pacman -S libpng12\""<<endl;
				cerr<<"    En Fedora, el comando: \"yum install libpng12"<<endl;
				cerr<<"    En Suse/OpenSuse, el comando: \"zypper install libpng12-0\""<<endl;
			}
		}
		
	}
	return 0;
}

