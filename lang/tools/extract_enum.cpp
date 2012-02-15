#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

int main(int argc, char *argv[]) {
	ofstream oe("src/lang_index_macros.h");
	ofstream os("lang/spanish.src");
	ofstream op("lang/spanish.pre");
	op<<"ZINJAI_LANGUAGE_DEFAULT\nspanish\n";
	oe<<"enum LANGUAGE_ENUM {\nLANGUAGE_ZERO = 0,\n";
	vector<string> done;
	for (int u=1;u<argc;u++) {
		cout<<argv[u]<<"..."<<endl;
		ifstream fil(argv[u]);
		string s;
		while (getline(fil,s)) {
			while (s.find("LANG(")!=string::npos) {
				s=s.substr(s.find("LANG(")+5);
				int p=s.find(",");
				string key =s.substr(0,p);
				if (find(done.begin(),done.end(),key)==done.end()) {
					oe<<"LANG_"<<key<<",\n";
					done.push_back(key);
					op<<key<<endl;
					os<<key<<endl;
					p++;
					int p2=p+1;
					string value;
					bool cont=false;
					do {
						cont=false;
						while (s[p2]!='\"') {
							if (s[p2]=='\\') p2++;
							p2++;
						}
						value+=s.substr(p+1,p2-p-1);
						if (p2==s.size()-1) {
							if (getline(fil,s)) {
								p=0;
								while (p<s.size() && (s[p]==' ' || s[p]=='\t')) p++;
								if (p<s.size() && s[p]=='\"') { cont=true; p2=p+1; }
							}
						}
					} while (cont);
					op<<value<<endl;
					os<<value<<endl;
				} else {
					cout<<"WARNING: duplicate "<<key<<endl;
				}
			}
		}
		fil.close();
	}
	op<<"END"<<endl;
	os<<"END"<<endl;
	oe<<"LANGUAGE_MAX\n};\n\n";
	oe.close();
	os.close();
	op.close();
	cout<<"All Ok"<<endl;
	return 0;
}

