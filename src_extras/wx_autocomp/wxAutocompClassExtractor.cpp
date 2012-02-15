// /mnt/ex/Descargas/out_of_memory/docs/mshtml/wx/htmls/wx_wxlistbook.html
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
	string s,cname, ocname;
	bool members=false,events=false,styles=false;
	while (getline(f,s)) {
		if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
		if (s=="<B><FONT COLOR=\"#FF0000\">Derived from</FONT></B><P>") { 
			getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			while (!s.size() || s[0]!='<' || leftcompare(s,"<A HREF=")) { // solo considerar los links, las clases que no tienen link no tiene sentido incluirlas porque igual no se las conoce, asi que no aportan nada, y se pueden confundir con otros textos como "No base clase" o "None."
				if (leftcompare(s,"<A HREF=")) { cname+=" public "; cname+=notags(s); }
				getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			}
		}
		if (!cname.size() && leftcompare(s,"<H2>")) { 
			ocname=cname=notags(s); // guardar el nombre de la clase, no se muestra ahora porque pueden venir macros y enumns que van fuera
		} else if (s=="<B><FONT COLOR=\"#FF0000\">Include files</FONT></B><P>") { 
			getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			s=notags(s); if (s.find(">")!=string::npos) s=s.substr(0,s.find(">")+1); // en algunos casos hay comentarios que sacar en la misma linea
			cout<<s<<endl;
		} if (s=="<B><FONT COLOR=\"#FF0000\">Window styles</FONT></B><P>") {
			members=false; events=false; styles=true;
		} if (s=="<B><FONT COLOR=\"#FF0000\">Event handling</FONT></B><P>") {
			members=false; events=true; styles=false;
		} if (s=="<B><FONT COLOR=\"#FF0000\">Members</FONT></B><P>" || s=="<B><FONT COLOR=\"#FF0000\">Function groups</FONT></B><P>") {
			members=true; events=false; styles=false;
			cout<<"\tclass "<<cname<<endl;
		} else if ( (styles||events) && leftcompare(s,"<TR><TD VALIGN=TOP")) {
			getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
			cout<<"\t"<<notags(s)<<endl;
		} else if (!members) { // algunos no tienen el titulo "member functions, como wxboxsizer, entonces me entero cuando veo el constructor
			if (s==string("<H3>")+ocname+"::"+ocname+"</H3>") {
				members=true;
				cout<<"\tclass "<<cname<<endl;
			}
		} else if (members && leftcompare(s,"<B>")) { // los metodos estan en negrita
			if (s.find("</B>()")!=string::npos || s.find("</B>(<B>")!=string::npos) { // si no tiene el parentesis de los metodos puede que solo sea un comentario o warning
				cout<<"\t\t"<<notags(s);
				while (!rightcompare(s,"<P>"))  {
					getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
					cout<<notags(s);
				}
				cout<<endl;
			}
//		} else if (members && s=="<HR>") { // esta forma no agarra constructores ni sobrecargas
//			getline(f,s);getline(f,s);
//			if (notags(s).find("::")!=string::npos) {
//				getline(f,s);getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
//				cout<<"\t\t"<<notags(s);
//				while (!rightcompare(s,"<P>")) {
//					getline(f,s); if (s.size()&&s[s.size()-1]=='\r') s=s.substr(0,s.size()-1);
//					cout<<notags(s);
//				}
//				cout<<endl;
//			}
		}
	}
	if (!members && ocname.size())
		cout<<"\tclass "<<cname<<endl;
	f.close();
	return 0;
}

	
