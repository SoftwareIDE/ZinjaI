#include<cstring>
#include<iostream>
#if defined(__WIN32__)
	#define eternal_nothing while (true) Sleep(10000)
	#include<io.h>
	#include<windows.h>
	#include <conio.h>
#else
	#include<cstdlib>
	#if !defined(__APPLE__)
		#include<wait.h>
	#endif
	#include <termios.h>
	#include <unistd.h>
	#define eternal_nothing while (true) sleep(10)
#endif

#include <fstream>
using namespace std;


const char *lang_debug_finished[] = {
	"<< La depuracion ha finalizado >>",
	"<< Debugging session has finished >>"
};

const char *lang_program_finished[] = {
	"<< El programa ha finalizado: codigo de salida: ",
	"<< Program finished: exit code: "
};

const char *lang_program_finished_abnormal[] = {
	"<< El programa ha finalizado anormalmente: signal ",
	"<< Abnormal program termination: signal "
};

const char *lang_press_key_to_close[] = {
	"<< Presione enter para cerrar esta ventana >>",
	"<< Press enter to close this window >>"
};

const char *lang_error_running[] = {
	"Error ejecuntado: ",
	"Error running: "
};

const char *lang_error_creating_process[] = {
	"Error al crear proceso: ",
	"Error creating process: "
};

int lang_idx=0;

// void on_quit(int sig) {
// 	cerr<<endl<<endl<<lang_debug_finished[lang_idx]<<endl<<lang_press_key_to_close[lang_idx];
// 	cout<<"\033[?25l"<<flush;
// 	struct termios oldt,newt;
// 	tcgetattr( STDIN_FILENO, &oldt );
// 	newt = oldt;
// 	newt.c_lflag &= ~( ICANON | ECHO );
// 	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
// 	char buf[256];
// 	read(STDIN_FILENO,buf,255);
// 	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
// 	exit(0);
// }

#if defined(_WIN32) || defined(__WIN32__)
#else
// esto es para que al enviar una señal en la terminal del runner (por ejemplo ctrl+c) se reenvie al hijo
// hay 2 casos especiales, sigusr2 que la usa zinjai para indicar que debe matar al proceso hijo, cuando
// se hace click en stop, porque si se mata la terminal y los hijos manejan sighup sin salir quedan
// andando huerfanos, y la otra es sighup que se recibe cuando se quiere cerrar la terminal desde el gestor de ventanas
pid_t child_pid=0;
bool received_sighup=false;
void forward_signal(int s) {
//	ofstream of("/home/zaskar/.zinjai/signals.txt",ios::app);
//	of<<endl<<s<<" "<<getppid()<<"  "<<kill(getppid(),0)<<endl;
//	of.close();
	if (s==SIGHUP) {
		received_sighup=true;
		if (child_pid && kill(getppid(),0)) kill(child_pid,SIGKILL);
	}
	//if (child_pid) kill(child_pid,s); // no hace falta, xterm parece que envia a todos los "hijos?"
}
#endif

int main(int argc, char *argv[]) {
	
	bool tty=false;
	string tty_fout;
	int waitkey=0;
	string dir;
	bool dir_done=false;
	int cmd_start=1;
	
	for (int i=1;i<argc;i++) {
		if (strcmp(argv[i],"-tty")==0) {
			tty=true;
			tty_fout=argv[++i];
		} else if (strcmp(argv[i],"-waitkey-onerror")==0) {
			waitkey=1;
		} else if (strcmp(argv[i],"-waitkey")==0) {
			waitkey=2;
		} else if (strcmp(argv[i],"-lang")==0) {
			i++;
			if (strcmp(argv[i],"english")==0)
				lang_idx=1;
		} else if (dir_done) {
			cmd_start=i;
			break;
		} else {
			dir_done=true;
			dir=argv[i];
		}
	}

	if (tty) { // guarda la direccion de la terminal en un archivo y entra en loop infinito (terminal para depuracion)
//		if (waitkey) signal(2,on_quit);
		system((string("tty >")+tty_fout).c_str());
		eternal_nothing;
	}
	
	if (cmd_start) {
		chdir(dir.c_str());
		
#if defined(_WIN32) || defined(__WIN32__)

		string command;
		command+="\"";
		command+=argv[cmd_start];
		command+="\" ";
		for (int i=cmd_start+1;i<argc;i++) {
			command+="\"";
			command+=argv[i];
			command+="\" ";
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		
		// Start the child process.
		if( !CreateProcess( NULL, // No module name (use command line).
			(char*)command.c_str(), // Command line.
			NULL,             // Process handle not inheritable.
			NULL,             // Thread handle not inheritable.
			FALSE,            // Set handle inheritance to FALSE.
			0,                // No creation flags.
			NULL,             // Use parent's environment block.
			NULL,             // Use parent's starting directory.
			&si,              // Pointer to STARTUPINFO structure.
			&pi )             // Pointer to PROCESS_INFORMATION structure.
			)
		{
			cerr<<lang_error_creating_process[lang_idx]<<command<<endl;
			cin.get();
			return 1;
		}
		
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		
		DWORD ret;
		GetExitCodeProcess(pi.hProcess, &ret);
		
		// Close process and thread handles.
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		if (waitkey==2 || (waitkey==1 && ret!=0)) {
			cerr<<endl<<endl<<lang_program_finished[lang_idx]<<ret<<" >>"<<endl<<lang_press_key_to_close[lang_idx];
			getch();
		}
#else

		int child_status,margc=0;
		
		char **margv= new char*[argc-cmd_start+1];
		for (int i=cmd_start;i<argc;i++) {
			margv[margc++]=argv[i];
		}
		margv[margc]=NULL;
		
//		pid_t child_pid;
		child_pid=fork();
		if (child_pid==0) {
			execvp(margv[0],margv);
			cerr<<lang_error_running[lang_idx]<<margv[0]<<endl;
			cin.get();
			return 1;
		}
		
		int ret=0;
for(int i=0;i<32;i++) { signal(i,forward_signal); }
		waitpid(child_pid,&child_status,0);
for(int i=0;i<32;i++) { signal(i,NULL); }
		if (WIFEXITED(child_status)) {
			ret=WEXITSTATUS(child_status);
			if (waitkey==2 || (waitkey==1 && ret!=0)) {
				cerr<<endl<<endl<<lang_program_finished[lang_idx]<<ret<<" >>"<<endl<<lang_press_key_to_close[lang_idx];
				waitkey=2;
			}
		} else {
			if (waitkey>0) {
				const char signals[][15] = { "", // 0
					"SIGHUP","SIGINT","SIGQUIT","SIGILL","SIGTRAP", // 1-5
					"SIGABRT","SIGBUS","SIGFPE","SIGKILL","SIGUSR", // 6-10
					"SIGSEGV","SIGUSR2","SIGPIPE","SIGALRM","SIGTERM", // 11-15
					"SIGSTKFLT","SIGCHLD","SIGCONT","SIGSTOP","SIGTSTP", //16-20
					"SIGTTIN","SIGTTOU","SIGURG","SIGXCPU","SIGXFSZ", // 21-25
					"SIGVTALRM","SIGPROF","SIGWINCH","SIGIO","SIGPWR", // 26-29
					"SIGSYS","SIGRTMIN","SIGRTMIN+1","SIGRTMIN+2","SIGRTMIN+3", // 33-37
					"SIGRTMIN+4","SIGRTMIN+5","SIGRTMIN+6","SIGRTMIN+7","SIGRTMIN+8", // 38-42
					"SIGRTMIN+9","SIGRTMIN+10","SIGRTMIN+11","SIGRTMIN+12","SIGRTMIN+13", // 43-47
					"SIGRTMIN+14","SIGRTMIN+15","SIGRTMAX-14","SIGRTMAX-13","SIGRTMAX-12", // 48-52
					"SIGRTMAX-11","SIGRTMAX-10","SIGRTMAX-9","SIGRTMAX-8","SIGRTMAX-7", // 53-57
					"SIGRTMAX-6","SIGRTMAX-5","SIGRTMAX-4","SIGRTMAX-3","SIGRTMAX-2", // 58-62
					"SIGRTMAX-1","SIGRTMAX" }; // 63-64
				int sig=WTERMSIG(child_status);
				if (sig>0 && sig<=64)
					cerr<<endl<<endl<<lang_program_finished_abnormal[lang_idx]<<signals[sig]<<"("<<sig<<") >>"<<endl<<lang_press_key_to_close[lang_idx];
				else
					cerr<<endl<<endl<<lang_program_finished_abnormal[lang_idx]<<sig<<" >>"<<endl<<lang_press_key_to_close[lang_idx];
				waitkey=2;
			}
		}
		if (!received_sighup&&waitkey==2) {
			cout<<"\033[?25l"<<flush;
			struct termios oldt,newt;
			tcgetattr( STDIN_FILENO, &oldt );
			newt = oldt;
			newt.c_lflag &= ~( ICANON | ECHO );
			tcsetattr( STDIN_FILENO, TCSANOW, &newt );
			char buf[256];
			read(STDIN_FILENO,buf,255);
			tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
		}
#endif

//		int ret = system(command.c_str());
//#if !defined(_WIN32) && !defined(__WIN32__)
//		if (ret&0xFF) {
//			if (waitkey) cerr<<endl<<endl<<"<< El programa ha finalizado anormalmente: signal "<<int(ret&0xFF)<<" >>";
//		} else {
//			ret>>=8;
//#endif
//			if (waitkey) cerr<<endl<<endl<<"<< El programa ha finalizado: codigo de salida: "<<ret<<" >>"<<endl<<"<< Presione enter para cerrar esta ventana >>";
//#if !defined(_WIN32) && !defined(__WIN32__)

		return ret;
	}
	return 0;
}
