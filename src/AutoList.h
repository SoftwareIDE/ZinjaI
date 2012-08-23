#ifndef AUTOLISTNODE_H
#define AUTOLISTNODE_H
#include <cstddef>
using namespace std;

#define _autolist_declare_local(T) \
	private:\
		T *local_prev,*local_next, **local_first;\
	public:\
		T *Next() { return local_next; }

#define _autolist_construct_local(_double_pointer_first)\
	local_first=_double_pointer_first;\
	if (*local_first) (*local_first)->local_prev=this; \
	local_prev=NULL; local_next=*local_first;\
	*local_first=this;

#define _autolist_destroy_local\
	if (local_first) {\
		if (local_next) local_next->local_prev=local_prev;\
		if (local_prev) local_prev->local_next=local_next;\
		else *local_first=local_next;\
		local_first=NULL;\
	}

#define _autolist_declare_global(T)\
	private:\
		T *global_prev,*global_next;\
		static T *global_first;\
		static int global_count;\
	public:\
		static int GetGlobalCount() { return global_count; }\
		static T* GetGlobalNext(T* aux) {\
			if (!aux) return global_first;\
			else return aux->global_next;\
		}

#define _autolist_construct_global\
	if (global_first) global_first->global_prev=this;\
	global_prev=NULL;\
	global_next=global_first;\
	global_first=this;\
	global_count++; 

#define _autolist_destroy_global\
	if (global_next) global_next->global_prev=global_prev;\
	if (global_prev) global_prev->global_next=global_next;\
	else global_first=global_next;\
	global_count--;


#define _autolist_initialize_global(T) \
	T* T::global_first=NULL; \
	int T::global_count=0;

template <class T>
void delete_autolist(T first) {
	while (first) {
		T aux=first->Next();
		delete first;
		first=aux;
	}
}

#endif

