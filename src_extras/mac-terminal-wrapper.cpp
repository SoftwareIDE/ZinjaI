#include <iostream>
#include <sys/stat.h>
#include <fstream>
using namespace std;

int main(int argc, char *argv[]) {
	char path[2048],cwd[1024];
	getcwd(cwd,1024);
	strcpy(path,"open ");
	strcat(path,getenv("HOME"));
	strcat(path,"/.zinjai/runner.command");
	ofstream f(path+5,ios::trunc);
	f<<"echo -e \\\\E]0\\;'Zinjai - Consola de Ejecucion'\\\\a && clear && cd \"";
	f<<cwd<<"\" &&";
	for (int i=1;i<argc;i++)
		f<<" \""<<argv[i]<<"\"";
	f.close();
	chmod(path+5,448);
	system(path);
	return 0;
}

