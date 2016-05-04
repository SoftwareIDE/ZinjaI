#include "error_recovery.h"
#include "ProjectManager.h"
#include "Parser.h"
#include "DebugManager.h"
#include <wx/strconv.h>
#include <wx/utils.h>
#include "mxSource.h"
#include "mxMainWindow.h"
#include <csignal>
#include "mxMessageDialog.h"
#include "Language.h"
#include "mxCompiler.h"
using namespace std;

string g_er_dir;
er_source_register *g_er_first_source = nullptr;

#define ERR_REC_LOG_BOOL(what) fil1<<#what<<": "<<((what)?"true":"false")<<endl
#define ERR_REC_LOG_NORM(what) fil1<<#what<<": "<<what<<endl

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

er_source_register::er_source_register(mxSource *s, er_source_register *p, er_source_register *n) {
	if ((next=n)) next->prev=this;
	if ((prev=p)) prev->next=this;
	src=s;
}

er_source_register::~er_source_register() {
	if (prev) prev->next=next;
	if (next) next->prev=prev;
}

void er_register_source(mxSource *src) {
	src->er_register = new er_source_register(src,g_er_first_source,g_er_first_source->next);
}
void er_unregister_source(mxSource *src) {
	delete src->er_register;
}

void er_sigsev(int sig) {
	
//cerr<<"ERROR RECOVERY 1"<<endl;
	
	ofstream fil1((g_er_dir+"error_log").c_str(),ios::ate|ios::app);

	fil1<<endl;

	fil1<<"Error date: "<<wxNow()<<endl;
	fil1<<"signal: "<<sig<<endl;
	
//cerr<<"ERROR RECOVERY 2"<<endl;
	
	if (debug) {
		ERR_REC_LOG_NORM(debug->status);
		ERR_REC_LOG_NORM(debug->last_command);
		ERR_REC_LOG_NORM(debug->last_answer);
	} else
		fil1<<"debug: NULL"<<endl;
	if (parser) {
		ERR_REC_LOG_BOOL(parser->working);
	} else
		fil1<<"parser: NULL"<<endl;
	if (project) 
		ERR_REC_LOG_NORM(project->filename);
	else
		fil1<<"project: NULL"<<endl;
	if (main_window) {
		ERR_REC_LOG_BOOL(!compiler->IsCompiling());
	} else 
		fil1<<"main_window: NULL"<<endl;
	fil1<<endl;
	
	fil1.close();
	
//cerr<<"ERROR RECOVERY 3"<<endl;
	
	ofstream fil2((g_er_dir+"recovery_log").c_str(),ios::trunc);
	fil2<<"Error date: "<<wxNow()<<endl;
	
	er_source_register *sr = g_er_first_source->next;
	char kname[]="kabom-aaa.cpp";
	while (sr) {
		if (kname[6]=='z') {
			kname[6]='a';
			if (kname[7]=='z') {
				kname[7]='a';
				kname[8]++;
			} else
				kname[7]++;
			
		} else
			kname[6]++;
		fil2<<sr->src->page_text<<endl;
		fil2<<sr->src->source_filename.GetFullPath()<<endl;
		fil2<<(g_er_dir+kname).c_str()<<endl;
		sr->src->wxStyledTextCtrl::SaveFile(wxString((g_er_dir+kname).c_str(),wxConvLibc));
		sr = sr->next;
	}
	
//cerr<<"ERROR RECOVERY 4"<<endl;
	
	fil2.close();
	if (project) {
		wxString pfile=project->filename;
		project->filename<<_T(".kaboom");
		project->Save();
		ofstream zpr((g_er_dir+"kboom.zpr").c_str());
		zpr<<DIR_PLUS_FILE(project->path,pfile)<<endl;
		zpr<<pfile<<endl;
		zpr.close();
	}
	
//cerr<<"ERROR RECOVERY 5"<<endl;
	
	if (main_window) {
		mxMessageDialog(main_window,LANG(ERRORRECOVERY_OOPS_MESSAGE,"Ha ocurrido un error grave y ZinjaI se cerrara.\n"
						"Por suerte, se han podido guardar todos sus\n"
						"archivos y proyectos para recuperarlos la\n"
						"proxima vez que inicie el entorno."))
			.Title("OOOPS!!!").IconError().Run();
	}
	exit(sig);
}

void er_init(const char *dir) {
	g_er_dir = dir;
	if (g_er_dir[g_er_dir.size()-1]!='/' && g_er_dir[g_er_dir.size()-1]!='\\') {
#ifdef __WIN32__
		g_er_dir+="\\";
#else
		g_er_dir+="/";
#endif
	}
	g_er_first_source = new er_source_register(nullptr,nullptr,nullptr);
#ifndef _ZINJAI_DEBUG
	signal(SIGSEGV,er_sigsev);
	signal(SIGPIPE,er_sigsev);
#endif
}


void er_uninit() {
	signal(SIGSEGV,nullptr);
	signal(SIGPIPE,nullptr);
}
