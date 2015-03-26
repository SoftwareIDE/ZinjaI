#ifndef RAII_H
#define RAII_H

#include "Cpp11.h"

/**
* @brief RAII wrapper for objects allocated on heap (new), but with a life-cycle
* as if it where on the stack (attached to a local scope)
**/
template<class T>
class RaiiDelete {
	T *&p;
public:
	RaiiDelete(T *&ptr) : p(ptr) {}
	~RaiiDelete() { delete p; }
};

// fms stands for faked-move-semantics, this project should still compile in pre c++11 compilers
template<class T> void fms_move(T *&des, T *&src) { des=src; src=nullptr; }
template<class T> T *fms_move(T *&src) { T *des=src; src=nullptr; return des; }
template<class T> T *&fms_delete(T *&des) { if (des) delete des; des=nullptr; return des; }

class BoolFlag;

/**
* RAII wrapper for handling flags, set a flag on creation and reset it on deletion
**/
template<typename T, typename U=bool, U SET_VALUE=true, U RESET_VALUE=false>
class FlagGuard {
	T *flag;
public:
	FlagGuard(T &f, bool force):flag(&f) { *flag=SET_VALUE; }
	FlagGuard(T &f):flag(&f) { if (*flag==SET_VALUE) { flag=nullptr; } else *flag=SET_VALUE; }
	bool IsOk() { return flag!=nullptr; }
	void Release() { if (flag) *flag=RESET_VALUE; flag=nullptr; }
	~FlagGuard() { if (flag) *flag=RESET_VALUE; }
};

/**
* Class for bool flags (used mostly for masking events) that enforce use with FlagGuard, and ensure correct initialization
**/
class BoolFlag {
public:
	BoolFlag() : value(false) {}
	operator bool() { return value; }
private:
	template<typename T, typename U, U V1, U V2>
	friend class FlagGuard;
	BoolFlag &operator=(bool b) { value=b; return *this; }
	bool value;
};

typedef FlagGuard<BoolFlag,bool,true,false> BoolFlagGuard;
typedef FlagGuard<bool,bool,true,false> boolFlagGuard;


/**
* Wrapper class for ensuring class member variables initialization
**/
template<class T>
class NullInitializedPtr {
	T *ptr;
public:
	NullInitializedPtr() : ptr(nullptr) {}
	operator T*&() { return ptr; }
	operator const T*&() const { return ptr; }
	T *&operator=(T *other) { ptr=other; return ptr; }
};

#endif
