#ifndef ERROR_RECOVERY_H
#define ERROR_RECOVERY_H
#include <fstream>
using namespace std;

class mxSource;

struct er_source_register {
	mxSource *src;
	er_source_register *next, *prev;
	er_source_register(mxSource *s, er_source_register *p, er_source_register *n);
	~er_source_register();
};

extern er_source_register *g_er_first_source;

extern string g_er_dir;

void er_register_source(mxSource *src);
void er_unregister_source(mxSource *src);

void er_init(const char *dir);
void er_uninit();

void er_sigsev(int sig);
	
#endif

