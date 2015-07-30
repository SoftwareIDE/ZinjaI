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


#if __cplusplus >= 201103

#define _CPP11_ENABLED 1
	
#else

// missing keywords
#define nullptr NULL
#define override

#endif // cpp11


#endif
