#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

const char *binname="zinjai.bin";

void fix_argv(char *argv[]) {
	int n=strlen(argv[0])-1;
	char *argv0=new char[n+15];
	while (n>0&&argv[0][n]!='/') n--;
	if (argv[0][n]=='/') strncpy(argv0,argv[0],++n);
	strcpy(argv0+n,binname);
	argv[0]=argv0;
}


int main(int argc, char *argv[]) {
	setenv("UBUNTU_MENUPROXY","",1);
	setenv("LIBOVERLAY_SCROLLBAR","0",1);
	fix_argv(argv);
	execvp(argv[0],argv);
	return 0;
}

