#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[]) {
	if (argc<3) { cerr<<"ERROR: argc<3"<<endl; return 1; }
	ifstream fin(argv[1]);
	vector<string> v;
	string aux;
	while (getline(fin,aux)) v.push_back(aux);
	fin.close();
	
	ofstream fout;
	
	int state=0;
	for(unsigned int i=0;i<v.size();i++) { 
		string &s=v[i];
		if (state==0) {
			if (s.find("Standard library header ")==0) { 
				string header=s.substr(24); header=header.substr(1,header.size()-2);
//				system("pwd");
//				cerr<<"Opening: \""<<string(argv[2])+"/"+header<<"\"\n";
//				cerr<<(fout.is_open()?1:0)<<endl;
				fout.open((string(argv[2])+"/"+header).c_str(),ios::trunc);
				state=1;
			}
		} else if (state==1) {
			if (s.find("[edit] Synopsis")!=string::npos) state=2;
			else if (s.find("Edit section: Synopsis")!=string::npos) state=2;
		} else if (state==2) {
			if (s.find("   Retrieved from")==0) state=4;
			else if (s.find("[edit] Note")==0) state=3;
			else if (s.find("[edit]")!=0) fout<<s<<endl;
//		} else if (state==3) {
//			if (s.find("   Retrieved from")==0) state=4;
//			else cerr<<argv[1]<<":  "<<s<<endl;
		}
	}
	if (state<2) cerr<<argv[1]<<":  ERROR: state="<<state<<endl;
	
	return 0;
}

