#include <iostream>
#include <iomanip>
using namespace std;

struct registro {
	int n1,n2;
	double d;
};

void mostrar(registro *arreglo, int n) {
	for (int i=0;i<n;i++)
		cout<<i<<right
		<<setw(5)<<arreglo[i].n1<<" "
		<<setw(4)<<arreglo[i].n2<<" "
		<<arreglo[i].d<<endl;
}

int main(int argc, char *argv[]) {
	registro lista[10];
	for (int i=0;i<10;i++) {
		lista[i].n1=rand()%1000;
		lista[i].n2=rand()%1000;
		lista[i].d=double(rand()%1000)/1000;
	}
	mostrar(lista,10);
	return 0;
}


