#include<iostream>
#include "zockets.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
using namespace std;

int main (int argc, char *argv[]) {
	int iarg=0, proxy_port=80;
	bool proxy=false;
	string proxy_dir, pname, postfix, child;
	bool child_mode = false;
	int lver=0;
	while (++iarg<argc) {
		string sarg(argv[iarg]);
		if (sarg=="--help") {
		} else if (sarg=="--postfix") {
			postfix=argv[++iarg];
		} else if (sarg=="--version") {
			lver=atoi(argv[++iarg]);
		} else if (sarg=="--child") {
			child=argv[++iarg]; child_mode=true;
		} else if (sarg=="--proxy") {
			proxy=true; proxy_dir=argv[++iarg];
			size_t p=proxy_dir.find(":",0);
			if (p!=string::npos) {
				proxy_port=atoi(proxy_dir.substr(p+1).c_str());
				proxy_dir=proxy_dir.substr(0,p);
			}
		} else {
			pname=sarg;
		}
	}
	if (!pname.length()) return 1;
	string web=pname+".sourceforge.net";
	ZOCKET z = zocket_llamar(proxy_port,(proxy?proxy_dir:web).c_str());
	if (z==ZOCKET_ERROR) {
		if (child_mode) {
			ofstream fout(child.c_str(),ios::trunc);
			fout<<"error"<<endl;
			fout.close();
		} else
			cout<<"No se pudo contactar el servidor."<<endl;
		return 1;
	}
	string request("GET ");
	if (proxy) { request+="http://"; request+=web; }
	request+="/version HTTP/1.1\nhost:";
	request+=web+"\n\n";
	zocket_escribir(z,request.c_str(),request.size());
	char data[1024]; int c=1024;
	string all_data, site_ver;
	for (int i=0;i<100;i++) {
		c=1024; Sleep(150);
		zocket_leer(z,data,c);
		data[c]='\0';
		all_data+=data;
		size_t p = all_data.find("ZVERSION=",0);
		if (p!=string::npos && p+17<=all_data.size()) {
			site_ver = all_data.substr(p+9,8);
			break;
		}
	}
	if (child_mode) {
		ofstream fout(child.c_str(),ios::trunc);
		if (site_ver.size()) {
			if (!lver) {
				ifstream f("version");
				if (f.is_open()) f>>lver;
				f.close();
			}
			if (lver) {
				int sver=0; 
				stringstream ss;
				ss<<site_ver;
				ss>>sver;
				if (sver<=lver) {
					fout<<"nonews"<<endl;
				} else if (sver>lver) {
					fout<<"update"<<endl;
					fout<<site_ver<<endl;
				}
			} else
				fout<<"error"<<endl;
		} else {
			fout<<"error"<<endl;
		}
		fout.close();
	} else {
		if (site_ver.size()) {
			if (!lver) {
				ifstream f("version");
				if (f.is_open()) f>>lver;
				f.close();
			}
			if (!lver) {
				cout<<"No se pudo determinar la version instalada."<<endl;
				cout<<"- La version oficial mas reciente es "<<site_ver<<endl;
			} else {
				int sver=0; 
				stringstream ss;
				ss<<site_ver;
				ss>>sver;
				if (sver<=lver) {
					cout<<"No hay actualizaciones disponibles."<<endl;
					cout<<"- La version oficial mas reciente es "<<site_ver<<endl;
					cout<<"- La version que esta utilizando es "<<lver<<endl;
				} else if (sver>lver) {
					cout<<"Hay una actualizacion disponible."<<endl;
					cout<<"- La version oficial mas reciente es "<<site_ver<<endl;
					cout<<"- La version que esta utilizando es "<<lver<<endl;
				}
			}
		} else {
			cout<<"No se pudo contactar el servidor."<<endl;
		}
	}
	zocket_cerrar(z);
	return 0;
}

