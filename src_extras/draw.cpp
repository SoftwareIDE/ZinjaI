#include <string>
#include <cstdlib>
#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
using namespace std;

int main(int argc, char *argv[]) {
#if defined(_WIN32) || defined(__WIN32__)		
	SetCurrentDirectory("graphviz\\bin");
	string cmd=argv[1];
	cmd+=".exe";
#else
	setenv("LD_LIBRARY_PATH","/usr/lib:/usr/X11/lib:graphviz/lib",1);
	string cmd("./graphviz/bin/");
	cmd+=argv[1];
#endif
	int i=2;
	while (i<argc) {
		cmd+=" \"";
		cmd+=argv[i];
		cmd+="\"";
		i++;
	}
	return system(cmd.c_str());
}

