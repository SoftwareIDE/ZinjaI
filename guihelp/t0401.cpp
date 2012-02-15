#include <iostream>
using namespace std;

int potencia(int base, int expo) {
	if (expo==0) return 1;
	else return base*potencia(base,expo-1);
}

int main(int argc, char *argv[]) {
	int b,e;
	cout<<"Ingrese la base: ";
	cin>>b;
	cout<<"Ingrese el exponente: ";
	cin>>e;
	cout<<"Resultado: "<<potencia(b,e)<<endl;
	return 0;
}
