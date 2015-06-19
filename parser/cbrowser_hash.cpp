#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

vector<string> cpp_keywords = 
{
	"asm",
	"new",
	"catch",
	"try",
	"bool",
	"namespace",
	"throw",
	"this",
	"public",
	"class",
	"private",
	"using",
	"template",
	"protected",
	"inline",
	"operator",
	"virtual",
	"volatile",
	"friend",
	"overload",
	"interface",
	"delete",
	"override",
	"final",
};
vector<string> c_keywords = 
{
	"do",
	"char",
	"if",
	"int",
	"enum",
	"for",
	"case",
	"union",
	"long",
	"void",
	"continue",
	"auto",
	"const",
	"goto",
	"break",
	"while",
	"switch",
	"unsigned",
	"short",
	"float",
	"else",
	"static",
	"return",
	"signed",
	"double",
	"struct",
	"typedef",
	"extern",
	"default",
	"register",
	"sizeof",
};

int get_pos(map<int,int> &codes, string s) { 
	int l = std::min<int>(s.size(),6), h=0;
	for(int j=0;j<l;j++) { 
		h+=codes[(int)s[j]];
	}
	return h;
}

string upper(string s) {
	for(unsigned int i=0;i<s.size();i++) 
		s[i] = toupper(s[i]);
	return s;
}

bool is_cpp(string s) {
	return find(cpp_keywords.begin(),cpp_keywords.end(),s)!=cpp_keywords.end();
}

int main(int argc, char *argv[]) {
	vector<string> keywords = cpp_keywords;
	for(auto s:c_keywords) keywords.push_back(s);
	
	
	srand(time(0));
	map<int,int> codes;
	for(;;){
		for(int i=0;i<256;i++) {
			codes[i]=273;
		}
		for(auto s:keywords) {
			for(unsigned int i=0;i<keywords.size();i++) { 
				int l = std::min<int>(s.size(),6);
				for(int j=0;j<l;j++) {
					if (codes[(int)s[j]]==273) {
						codes[(int)s[j]]=rand()%51;
					}
				}
			}
		}
		set<int> taken;
		for(auto s:keywords) {
			int k = get_pos(codes,s);
			if (k>=273 || taken.find(k)==taken.end())
				taken.insert(k);
			else {
				codes.clear();
				break;
			}
		}
		if (!codes.empty()) break;
	}
	
	
	
	cout << "int c_hash[] = {" << endl;
	for(int i=0;i<16;i++) {
		cout<<"    ";
		for(int j=0;j<16;j++) { 
			cout << setw(3)<<right<<codes[i*16+j]<<",";
		}
		cout <<endl;
	}
	cout <<"};"<<endl;
	cout <<endl;
	
	cout <<"KYW_S  LexKeyWordTab[] = {"<<endl;
	int kmax = 0;
	map<int,string> key_map;
	for(auto s:keywords) {
		kmax = std::max(get_pos(codes,s),kmax);
		key_map[get_pos(codes,s)]=s;
	}
	for(int i=0;i<272;i++) { 
		if (key_map[i]=="") {
			cout << "    { 0 }," << endl;
		} else {
			string s = key_map[i];
			cout << "    { \""<<s<<"\", SN_"<<upper(s)<<", "<<(is_cpp(s)?"CPP":"C")<<"_KEYWORD, "<<s.size()<<" }," << endl;
		}
	}
	cout << "};"<<endl;
	
	return 0;
}

