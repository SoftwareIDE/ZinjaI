#ifndef CPP11_H
#define CPP11_H

/// crappy workaround for the need of lambda functions (mac port use some old pre c++11 gcc)
class GenericAction {
public:
	virtual void Do()=0;
	virtual ~GenericAction(){};
};

#define _LAMBDA_0(Name,Action) \
	class Name : public GenericAction {\
	public: void Do() Action };

#define _LAMBDA_1(Name,Type,Arg,Action) \
	class Name : public GenericAction {\
	public: Name(Type arg) : Arg(arg) {} \
	public: void Do() Action \
	private: Type Arg; };

/// crappy workaround for the need of lambda functions (mac port use some old pre c++11 gcc)
template<typename T>
class GenericActionEx {
public:
	virtual void Do(T arg)=0;
	virtual ~GenericActionEx(){};
};

#define _LAMBDAEX_0(Name,Type,ArgName,Action) \
	class Name : public GenericActionEx<Type> {\
	public: void Do(Type ArgName) Action };

#define _LAMBDAEX_1(Name,Type,ArgName,AType1,Attrib1,Action) \
	class Name : public GenericActionEx<Type> {\
	public: Name(AType1 at1) : Attrib1(at1) {} \
	public: void Do(Type ArgName) Action \
	private: AType1 Attrib1; };

#define _LAMBDAEX_2(Name,Type,ArgName,AType1,Attrib1,AType2,Attrib2,Action) \
	class Name : public GenericActionEx<Type> {\
	public: Name(AType1 at1, AType2 at2) : Attrib1(at1), Attrib2(at2) {} \
	public: void Do(Type ArgName) Action \
	private: AType1 Attrib1; AType2 Attrib2; };


#define _CAPTURELIST_4(LType,LName,VType1,VName1,VType2,VName2,VType3,VName3,VType4,VName4) \
	struct LType { VType1 VName1; VType2 VName2; VType3 VName3; VType4 VName4; } LName; \
	LName.VName1=VName1; LName.VName2=VName2; LName.VName3=VName3; LName.VName4=VName4;

#define _CAPTURELIST_5(LType,LName,VType1,VName1,VType2,VName2,VType3,VName3,VType4,VName4,VType5,VName5) \
	struct LType { VType1 VName1; VType2 VName2; VType3 VName3; VType4 VName4; VType5 VName5; } LName; \
	LName.VName1=VName1; LName.VName2=VName2; LName.VName3=VName3; LName.VName4=VName4; LName.VName5=VName5;


#if __cplusplus >= 201103

#define _CPP11_ENABLED 1
	
#else

// missing keywords
#define nullptr NULL
#define override

#endif // cpp11


#endif
