#ifndef GPROFDATA_H
#define GPROFDATA_H

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <algorithm>
using namespace std;

struct GprofData {
	int cmp_id;
	void cut(const string &s, string *v, int cant) {
		int p=0,l=s.size(),i=0;
		while (true) {
			while(p<l && (s[p]==' '||s[p]=='\t')) p++;
			if (i+1==cant) { v[i]=s.substr(p); break; }
			int p0=p;
			while(p<l && (s[p]!=' '&& s[p]!='\t')) p++;
			v[i]=s.substr(p0,p-p0);
			i++;
		}
	}
	void fix(string *v, int m=4) {
		for(int i=0;i<m-1;i++) { 
			bool is_ok = true;
			for(size_t j=0;j<v[i].size();j++) { 
				if ((v[i][j]<'0'||v[i][j]>'9')&&v[i][j]!='.'&&v[i][j]!='+'&&v[i][j]!='/'&&v[i][j]!='['&&v[i][j]!=']')
					{ is_ok = false;  break; }
			}
			if (!is_ok) {
				// concatenar desde i hacia adelante (puede haber cortado una funcion en los espacios de los argumentos)
				for(int j=i+1;j<m;j++)
					if (!v[j].empty()) { v[i]+=" "; v[i]+=v[j]; }
				// amontonar los strings no vacios al final
				for(int n=m-1;n>=0;n--,i--) v[n] = i>=0?v[i]:"";
				break;
			}
		}
	}
	struct tm_item {
		float percent_time,cumulative_seconds,self_seconds,self_s_calls,total_s_calls;
		int calls;
		string name;
		bool match(const char *s) {
			size_t i=name.find(s,0);
			if (i==string::npos) return false;
			size_t j=name.find('(',0);
			return (j==string::npos || i<j);
		}
	};
	struct call_item {
		string name;
		float percent_time,self,children;
		string called;
	};
	struct graph_node {
		string name;
		vector<call_item> calls_to, called_by;
	};
	vector<tm_item> table;
	vector<graph_node> graph;
	GprofData(const char *fname) {
		ifstream fin(fname);
		string s;
		int status=0; // 1=1er tabla, 2=calls_pre, 3=calls_post
		size_t name_col;
		graph_node node;
		while (getline(fin,s)) {
			if (status==0) {
				// cabecera de la primer tabla
				name_col=s.find("name");
				if (s.find("s/call")!=string::npos && name_col!=string::npos) status=1;
			} else if (status==1) {
				// gran tabla por funciones
				if (s.size()>name_col) {
					tm_item t; t.name=s.substr(name_col);
					while (t.name.size()&&t.name[0]==' ') t.name=t.name.substr(1); // a veces una columna tiene un numero (contador de llamadas) muy grande y creo que por eso esta queda desfazada
					s.erase(name_col);
					string vals[6]; 
					cut(s,vals,6);
					t.percent_time=atof(vals[0].c_str());
					t.cumulative_seconds=atof(vals[1].c_str());
					t.self_seconds=atof(vals[2].c_str());
					t.calls=atoi(vals[3].c_str());
					t.self_s_calls=atof(vals[4].c_str());
					t.total_s_calls=atof(vals[5].c_str());
					table.push_back(t);
				} else status=2;
			} else if (status==2) {
				// cabecera del grafo de llamadas
				if (s.find("% time")!=string::npos) status=3;
			} else if (status==3) {
				// se termina el grafo
				if (s.size()<5) {
					if (node.name.size()) graph.push_back(node);
					break;
				}
				if (s[0]=='[') {
					// la linea de la propia función del nodo
					string vals[5];
					cut(s,vals,5);
					string who=vals[4];
					if (who.size() && (who[0]>='0'&&who[1]<='9')) {
						cut(who,vals,2);
						who=vals[1];
					}
					node.name=who;
					status=4;
				} else {
					// alguien llama al nodo actual
					string vals[4];
					cut(s,vals,4);
					fix(vals); // corrige cuando hay campos en blanco (llamadas espontanes, sin info, recursivas, etc)
					if (vals[3].size()) {
						call_item ci;
						ci.self=atof(vals[0].c_str());
						ci.children=atof(vals[1].c_str());
						ci.called=vals[2].c_str();
						ci.name=vals[3];
						node.called_by.push_back(ci);
					}
				}
			} else if (status==4) {
				if (s.substr(0,5)=="-----") {
					if (node.name.size()) graph.push_back(node);
					// termina este nodo
					node.name=""; node.calls_to.clear(); node.called_by.clear(); status=3;
				} else {
					// el nodo actual llama a alguien
					string vals[4];
					cut(s,vals,4);
					fix(vals); // corrige cuando hay campos en blanco (llamadas espontanes, sin info, recursivas, etc)
					if (vals[3].size()) {
						call_item ci;
						ci.self=atof(vals[0].c_str());
						ci.children=atof(vals[1].c_str());
						ci.called=vals[2].c_str();
						ci.name=vals[3];
						node.calls_to.push_back(ci);
					}
				}
			}
		}
	}
	
	static bool cmp_percent_time(const tm_item &i1, const tm_item &i2){ return i1.percent_time<i2.percent_time; }
	static bool cmp_cumulative_seconds(const tm_item &i1, const tm_item &i2){ return i1.cumulative_seconds<i2.cumulative_seconds; }
	static bool cmp_self_seconds(const tm_item &i1, const tm_item &i2){ return i1.self_seconds<i2.self_seconds; }
	static bool cmp_self_s_calls(const tm_item &i1, const tm_item &i2){ return i1.self_s_calls<i2.self_s_calls; }
	static bool cmp_total_s_calls(const tm_item &i1, const tm_item &i2){ return i1.total_s_calls<i2.total_s_calls; }
	static bool cmp_calls(const tm_item &i1, const tm_item &i2){ return i1.calls<i2.calls; }
	static bool cmp_name(const tm_item &i1, const tm_item &i2){ return i1.name<i2.name; }
	void Sort(int col, bool asc) {
		if (col==0) sort(table.begin(),table.end(),cmp_percent_time);
		if (col==1) sort(table.begin(),table.end(),cmp_cumulative_seconds);
		if (col==2) sort(table.begin(),table.end(),cmp_self_seconds);
		if (col==3) sort(table.begin(),table.end(),cmp_calls);
		if (col==4) sort(table.begin(),table.end(),cmp_self_s_calls);
		if (col==5) sort(table.begin(),table.end(),cmp_total_s_calls);
		if (col==6) sort(table.begin(),table.end(),cmp_name);
		if (!asc) reverse(table.begin(),table.end());
	}
};

#endif

