#include <iostream>
#include <fstream>
using namespace std;

bool leftcompare(string s1, string s2) {
	if (s2.length()>s1.size()) return false;
	else return s1.substr(0,s2.size())==s2;
}

bool rightcompare(string s1, string s2) {
	if (s2.length()>s1.size()) return false;
	else return s1.substr(s1.size()-s2.size())==s2;
}

string notags(string s) { // saca todo lo que este entre < y >, reemplaza &lt; y &gt;, corta los espacios extra, y saca los :: si no es un metodo
	int d=0,n=s.size(); bool tag=false;
	for (int i=0;i<n;i++) {
		if (tag) {
			if (s[i]=='>') tag=false;
		} else {
			if (s[i]=='<') tag=true;
			else if (!d || (s[i]!=' '||s[d-1]!=' ')) s[d++]=s[i];
		}
	}
	s=s.substr(0,d);
	while (s.find("&lt;")!=string::npos)
		s=s.substr(0,s.find("&lt;"))+"<"+s.substr(s.find("&lt;")+4);
	while (s.find("&gt;")!=string::npos)
		s=s.substr(0,s.find("&gt;"))+">"+s.substr(s.find("&gt;")+4);
	while (s.size() && s[0]==' ') s=s.substr(1);
	if (s.size()>2 && s[0]==':' && s[1]==':') s=s.substr(2);
	return s;
}

int main(int argc, char *argv[]) {
	ifstream f(argv[1]);
	string s;
	string pending; // se guardan en pending los prototipos en lugar de imprimirse directamente, porque el include esta despues
	string lheader;
	while (getline(f,s)) {
		if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
		if (s=="<B><FONT COLOR=\"#FF0000\">Include files</FONT></B><P>") { 
			getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			s=notags(s);
			if (s!=lheader)
			cout<<(lheader=s)<<endl;
		} else if (s=="<HR>") {
			if (pending.size()) cout<<pending; pending="";
			getline(f,s);getline(f,s);getline(f,s);getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			if (leftcompare(s,"<TT>")) s=string("<B>")+s.substr(0,s.find("</TT"))+"<P>"; // algunas macros sin argumentos tienen la descripcion pegada en la misma linea
			if (leftcompare(s,"<B>")) { // las macros empiezan con <B> o <TT> (que se cambia arriba)
				pending+="\t"; pending+=notags(s);
				while (!rightcompare(s,"<P>")) { // si no termina en <P> es porque sigue en la proxima linea
					getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
					pending+=notags(s);
				}
				pending+="\n";
			}
		}
	}
	if (pending.size()) cout<<pending;
	f.close();
	return 0;
}

	
