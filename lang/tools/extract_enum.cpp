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
		string s; int line_num=0;
		while (getline(fil,s)) {
			line_num++;
			while (true) {
				size_t pos0=s.find("LANG(");
				size_t pos1=s.find("LANG1(");
				size_t pos2=s.find("LANG2(");
				size_t pos3=s.find("LANG3(");
				size_t pos=pos0;
				if (pos1!=string::npos && pos1<pos) pos=pos1;
				if (pos2!=string::npos && pos2<pos) pos=pos2;
				if (pos3!=string::npos && pos3<pos) pos=pos3;
				int d=pos==pos0?5:6;
				if (pos==string::npos) break;
				s=s.substr(pos+d);
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
								line_num++;
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

