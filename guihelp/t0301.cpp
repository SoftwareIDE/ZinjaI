#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	unitbuf(cout);
	int cant,n;
	cout<<"Cantidad de datos: ";
	cin>>cant;
	double sum=0;
	for (int i=0;i<cant;i++) {
		cout<<"Ingrese el dato "<<i<<": ";
		cin>>n;
		sum+=n;
	}
	double prom = sum/cant;
	cout<<"Promedio: "<<prom;
	return 0;
} 
