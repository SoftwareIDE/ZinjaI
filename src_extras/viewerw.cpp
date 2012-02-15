#include<windows.h>
#include <string>
using namespace std;

int main(int argc, char *argv[]){
	string cmd;
	for (int i=1;i<argc;i++) {
		cmd+="\"";
		cmd+=argv[i];
		cmd+="\" ";
	}
	ShellExecute(NULL,"open","..\\viewer.exe",cmd.c_str(),"graphviz\\bin",SW_NORMAL);
	return 0;
}


