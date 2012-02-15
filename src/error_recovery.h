#ifndef ERROR_RECOVERY_H
using namespace std;
#define ERROR_RECOVERY_H
#include <fstream>

class mxSource;

extern char *error_file;

struct er_source_register {
	mxSource *src;
	er_source_register *next, *prev;
	er_source_register(mxSource *s, er_source_register *p, er_source_register *n);
	~er_source_register();
};

extern er_source_register *er_first_source;

extern string er_dir;

void er_register_source(mxSource *src);
void er_unregister_source(mxSource *src);

void er_init(const char *dir);
void er_uninit();
	
#endif

