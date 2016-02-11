#include <fstream>
#include <cstring>
#include "Language.h"
#include "Cpp11.h"
using namespace std;

char *g_language_buffer = nullptr;
int *g_language_index = nullptr;

/**
* @brief Carga un archivo de lenguaje
*
* Carga los arreglos g_language_buffer y g_language_index, eliminando su contenido
* si es que tenian y utilizando el cache de lenguaje si existe (sino se
* regenera).
* @param lang_in     nombre del archivo de idioma a cargar sin extension
* @param lang_cache  nombre del archivo de cache a verificar sin extension
* @return LANGERR_OK si se carga o regenera sin problema, otro LANGUAGE_ERROR en
*                    otro caso
**/

LANGUAGE_ERROR load_language(string lang_in, string lang_cache) {
	
	if (g_language_buffer) delete [] g_language_buffer;
	if (g_language_index) delete [] g_language_index;
	g_language_buffer=nullptr; g_language_index=nullptr;
	
	LANGUAGE_ERROR le = is_language_compiled(lang_in,lang_cache);
	if (le!=LANGERR_OK) {
		LANGUAGE_ERROR le = compile_language(lang_in,lang_cache);
		if (le==LANGERR_RECOMPILED) return LANGERR_OK;
		else if (le!=LANGERR_CACHE_OK) return le;
	} else if (le==LANGERR_SIGN_NOT_FOUND) return le;
	
	// cargar el buffer con las cadenas en binario y pegadas
	ifstream fin((lang_cache+".buf").c_str(),ios::binary|ios::ate);
	int sz=fin.tellg();
	fin.seekg(0,ios::beg);
	g_language_buffer = new char[sz];
	if (!fin.read(g_language_buffer,sz)) {
		fin.close();
		return load_language(lang_in,"");
	}
	fin.close();
	
	// cargar el buffer con los offsets para armar los punteros
	ifstream fin2((lang_cache+".idx").c_str(),ios::binary|ios::ate);
	sz=fin2.tellg()/sizeof(int);
	fin2.seekg(0,ios::beg);
	g_language_index = new int[sz];
	if (!fin2.read((char*)g_language_index,(sz)*sizeof(int)) || sz!=int(LANGUAGE_MAX)) {
		fin2.close();
		return load_language(lang_in,"");
	}
	fin2.close();
	
	return LANGERR_OK;
	
}

LANGUAGE_ERROR is_language_compiled(string lang_in, string lang_cache) {
	if (!lang_cache.size()) return LANGERR_NO_CACHE_NAME;
	ifstream fc((lang_cache+".sgn").c_str());
	if (!fc.is_open()) return LANGERR_CACHE_NOT_FOUND;
	string sc; getline(fc,sc); fc.close();
	ifstream fi((lang_in+".sgn").c_str());
	if (!fi.is_open()) return LANGERR_SIGN_NOT_FOUND;
	string si; getline(fi,si); fi.close();
	if (sc!=si) return LANGERR_SIGN_DIFFER;
	return LANGERR_OK;
}

LANGUAGE_ERROR compile_language(string lang_in, string lang_cache) {

	// leer las cadenas
	int scount=0, buflen=0;
	ifstream fin((lang_in+".pre").c_str());
	if (!fin.is_open()) return LANGERR_PRE_NOT_FOUND;
	int texts_len = int(LANGUAGE_MAX)+1;
	string *texts = new string[texts_len];
	string tmp;
	getline(fin,tmp);
	while (tmp!="END" && scount<LANGUAGE_MAX) {
		string &tx=texts[scount];
		getline(fin,tx);
		for (unsigned int j=0;j<tx.size();j++) {
			if (tx[j]=='\\' && j+1<tx.size()) {
				if (tx[j+1]=='n') tx.replace(j,2,"\n");
				else if (tx[j+1]=='t') tx.replace(j,2,"\t");
				else tx.erase(j,1);
			}
		}
		buflen+=1+texts[scount++].size();
		getline(fin,tmp);
	}
	if (scount!=LANGUAGE_MAX || tmp!="END") {
		delete [] texts;
		return LANGERR_PRE_WRONG_COUNT;
	}
	fin.close();
	
	// generar el buffer
	g_language_buffer = new char[buflen];
	g_language_index = new int[LANGUAGE_MAX];
	buflen=0; 
	for (int i=0;i<scount;i++) {
		g_language_index[i]=buflen;
		strcpy(g_language_buffer+buflen,texts[i].c_str());
		buflen+=texts[i].size()+1;
	}
	delete [] texts;
	
	// guardar el buffer
	ofstream fout((lang_cache+".buf").c_str(),ios::binary|ios::trunc);
	fout.write(g_language_buffer,buflen);
	fout.close();
	// guardar el indice
	ofstream fout2((lang_cache+".idx").c_str(),ios::binary|ios::trunc);
	fout2.write((char*)g_language_index,(LANGUAGE_MAX)*sizeof(int));
	fout2.close();
	// guardar firma
	ifstream fi((lang_in+".sgn").c_str());
	string si; getline(fi,si); fi.close();
	ofstream fc((lang_cache+".sgn").c_str(),ios::trunc);
	fc<<si<<endl; fc.close();
	
	return LANGERR_RECOMPILED;
}
