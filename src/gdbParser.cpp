#include "gdbParser.h"


void GdbParse_SkipList(const wxString &s, int &i, int l) {
	int balance=0;
	do {
		if (s[i]=='\"'||s[i]=='\'') GdbParse_SkipString(s,i,l);
		else if (s[i]=='['||s[i]=='{'||s[i]=='(') balance++;
		else if (s[i]==']'||s[i]=='}'||s[i]==')') balance--;
	} while (balance && ++i<l);
}

void GdbParse_SkipEmpty(const wxString &s, int &i, int l) {
	while(i<l && (s[i]==' '||s[i]=='\t'||s[i]=='\n')) i++;
}

bool GdbParse_IsList(const wxString &s, int &i, int l) {
	if (l==-1) l=s.Len(); 
	GdbParse_SkipEmpty(s,i,l);
	return i<l && s[i]=='{';
}

bool GdbParse_GetPair(const wxString &s, int &i, int &pos_st, int &pos_eq, int &pos_end, int l) {
	if (l==-1) l=s.Len(); 
	GdbParse_SkipEmpty(s,i,l);
	if (i>=l || s[i]=='}' || s[i]==']' || s[i]==')') return false;
	pos_st=i; pos_eq=-1;
	while(i<l && s[i]!='}' && s[i]!=']' && s[i]!=')' && s[i]!=',') {
		if (s[i]=='=') {
			if (pos_eq==-1) pos_eq=i;
		} else if (s[i]=='{'||s[i]=='('||s[i]=='[') {
			GdbParse_SkipList(s,i,l);
		} else if (s[i]=='\"'||s[i]=='\'') {
			GdbParse_SkipString(s,i,l); 
		}
		i++;
	}
	pos_end=i;
	return true;
}

bool GdbParse_GetNext(const wxString &s, int &i, wxString &key, wxString &val) {
	int l=s.Len();
	while (i<l && (s[i]<='a'||s[i]>='z')) i++;
	while(true) {
		int p0=i; // posicion donde empieza el nombre del "campo"
		while (i<l && s[i]!=']' && s[i]!='}' && s[i]!='=') i++;
		int p1=i; // posicion del igual
		if (++i>=l || s[i]!='\"') { // si no era un campo, termino la entrada
			i=-1; return true;
		}
		while (++i<l && s[i]!='\"') { if (s[i]=='\\') i++; }
		int p2=i; // posicion de la comilla que cierra
		key=s.Mid(p0,p1-p0);
		val=s.Mid(p1+2,p2-p1-2);
		while (++i<l && s[i]!=',' && s[i]!=']' && s[i]!='}'); 
		if (i==l || s[i]==']' || s[i]=='}') 
			return true; 
		else { 
			i++; return false;
		}
	}
}

