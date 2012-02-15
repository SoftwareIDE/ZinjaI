#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char *argv[]) {
	for (int i=1;i<argc;i++) {
		ifstream fil(argv[i]);
		if (fil.is_open()) {
			string str;
			int l=0;
			while (getline(fil,str)) {
				l++;
				int p = str.find("href",0);
				while (p!=string::npos) {
					if (str[p+5]!='\"') {
						cerr<<argv[i]<<" linea "<<l<<": Falta primer comilla.\n";
					} else {
						int p2 = str.find('\"',p+6);
						if (p2==string::npos) {
							cerr<<argv[i]<<" linea "<<l<<": Falta segunda comilla.\n";
						} else {
							string fname = str.substr(p+6,p2-p-6);
							bool test=true;
							test = ! ( (fname.size()>5 && fname.substr(0,5)=="http:") || (fname.size()>5 && fname.substr(0,7)=="mailto:") ) ;
							if (test) {
								if (fname.find('#',0)!=string::npos) {
									fname=fname.substr(0,fname.find('#',0));
								}
								if (fname.size()) {
									ifstream fil2(fname.c_str());
									if (fil2.is_open()) {
										fil2.close();
									} else {
										cerr<<argv[i]<<" linea "<<l<<": Falta destino \""<<fname<<"\".\n";
									}
								}
							}
						}
					}
					
					p = str.find("href=",p+1);
				}
			}
			fil.close();
		} else {
			cerr<<argv[i]<<": No se pudo abrir.\n";
		}
	}
	return 0;
}

