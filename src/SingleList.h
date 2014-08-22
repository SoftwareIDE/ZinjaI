#ifndef SINGLELIST_H
#define SINGLELIST_H
#include <cstddef>

/** 
* SingleList is a simple array based container, similar to std::vector, but here doesn't care about maintaining the order.
*
* So Remove can just swap the element to be removed with last one to make it faster.
**/
template<class T>
class SingleList {
protected:
	T *m_vec; ///< the actual data, stored in a dinamyc array
	int m_size; ///< actual number of user elements in the array (<=capacity)
	int m_capacity; ///< amount of memory reserved in m_vec (usually more than needed for the actual number of elements)
	SingleList(const SingleList &other); ///< it is not allowed to duplicate a SingleList
	void CreateVector() {
		T *new_vec=new T[m_capacity];
		for(int i=0;i<m_size;i++) new_vec[i]=m_vec[i];
		delete []m_vec; m_vec=new_vec;
	}
	void GrowMem() {
		if (!m_vec) m_capacity=10; 
		else m_capacity*=2; 
		CreateVector();
	}
	void EnsureMemFor(int num_elems) {
		if (num_elems>=m_capacity) return;
		if (!m_capacity) { m_capacity=10; }
		while (m_capacity<num_elems) m_capacity*=2;
		CreateVector();
	}
public:
	static int NotFound() { return -1; }
	SingleList():m_vec(NULL),m_size(0),m_capacity(0) {
		
	}
	SingleList(int size):m_size(size),m_capacity(size) {
		m_vec=new T[size];
	}
	SingleList(SingleList *list) {
		StealFrom(list);
	}
	void StealFrom(SingleList *list) {
		m_vec=list->m_vec;
		m_size=list->m_size;
		m_capacity=list->m_capacity;
		list->m_vec=NULL;
		list->m_size=list->m_capacity=0;
	}
	int GetSize() const { 
		return m_size;
	}
	int Add(const T &data) {
		if (m_size==m_capacity) GrowMem();
		m_vec[m_size]=data;
		return m_size++;
	}
	/// inserta elementos vacios (valor por default/basura) en la pos indicada
	void MakeRoomForMultipleInsert(int pos, int cant) {
		EnsureMemFor(m_size+cant);
		for(int i=m_size;i>pos;i--) 
			m_vec[i+cant-1] = m_vec[i-1];
		m_size+=cant;
	}
	/// redimensionamiento lógico, no se pierden datos
	void Resize(int new_size) {
		EnsureMemFor(new_size);
		m_size=new_size;
	}
	void Swap(int i, int j) {
		T aux=m_vec[i]; m_vec[i]=m_vec[j]; m_vec[j]=aux;
	}
	T &operator[](int i) { 
		return m_vec[i];
	}
	const T &operator[](int i) const { 
		return m_vec[i];
	}
	/// reset completo, libera la memoria eliminando todos los datos
	void Reset() {
		delete []m_vec;
		m_vec=NULL;
		m_capacity=m_size=0;
	}
	/// reset logico, no libera memoria
	void Clear() {
		m_size=0;
	}
	int Find(const T &data) {
		for(int i=0;i<m_size;i++) { 
			if (m_vec[i]==data) return i;
		}
		return NotFound();
	}
	bool Contains(const T &data) {
		for(int i=0;i<m_size;i++) { 
			if (m_vec[i]==data) return true;
		}
		return false;
	}
	void Remove(int pos) {
		--m_size;
		for(int i=pos;i<m_size;i++) { 
			m_vec[i]=m_vec[i+1];
		}
	}
	void FastRemove(int pos) {
		m_vec[pos]=m_vec[--m_size];
	}
	~SingleList() {
		delete [] m_vec;
	}
};

template<class T> class LocalList;
template<class T> class LocalListNode;

/** 
* Auxiliar class for LocalListNode, should not be used directly.
*
* I use this class instead of a raw pointer for pointing nodes in a LocalList (that will be some 
* kind of SingleList) to avoid the problem of loosing references when the array inside these
* classes need to be rebuild (when adding elements and size==capacity)
**/
template<class T> 
struct LocalListNodePtr {
	LocalList<T> *m_list;
	int m_pos;
	LocalListNodePtr() : m_list(NULL) {}
	LocalListNodePtr(LocalList<T> *list, int pos): m_list(list), m_pos(pos) {}
	bool IsNull() const { return m_list==NULL; }
	void SetNull() { m_list=NULL; }
	LocalListNode<T>* operator->();
};

/** 
* Auxiliar class for LocalList and GlobalList, should not be used directly outside these containers.
*
* LocalList will own these elements in its own array, but they elements also have links as list node
* to be linked in the GlobalList. So, GlobalList list is actually a list whose nodes are keept in
* several LocalLists, and LocalLists are just arrays of nodes for GlobalList.
**/
template<class T>
struct LocalListNode {
	T m_data;
	LocalListNodePtr<T> m_next;
	LocalListNodePtr<T> m_prev;
};

template<class T> class LocalList;
template<class T> class GlobalListIterator;

/** 
* A GlobalList keeps track of elements from a set of LocalLists. Elements should be inserted and 
* deleted from Local ones, and not directly here. 
*
* This class is for just traversing them, using GlobalListIterator, or to erase an item without
* knowing in wich LocalList is it.
**/
template<class T>
class GlobalList {
	friend class GlobalListIterator<T>;
	LocalListNodePtr<T> m_first;
	LocalListNodePtr<T> m_last;
public:
	GlobalList() {}
	GlobalListIterator<T> Find(T data);
	void Remove(GlobalListIterator<T> it);
	void FindAndRemove(T data);
private:
	friend class LocalList<T>; ///< private methods are made to be called from LocalList
	void Add(LocalListNodePtr<T> node) {
		node->m_prev=m_last;
		node->m_next.SetNull();;
		if (m_last.IsNull()) {
			m_first=node;
		} else {
			m_last->m_next=node;
		}
		m_last=node;
	}
	void Remove(LocalListNodePtr<T> node) {
		if (node->m_next.IsNull()) {
			m_last=node->m_prev;
		} else {
			node->m_next->m_prev=node->m_prev;
		}
		if (node->m_prev.IsNull()) {
			m_first=node->m_next;
		} else {
			node->m_prev->m_next=node->m_next;
		}
	}
	void UpdatePos(LocalListNode<T>* node, int pos) {
		if (node->m_next.IsNull()) m_last.m_pos=pos;
		else node->m_next->m_prev.m_pos=pos;
		if (node->m_prev.IsNull()) m_first.m_pos=pos;
		else node->m_prev->m_next.m_pos=pos;
	}
	void UpdateList(LocalListNode<T>* node, LocalList<T> *list) {
		if (node->m_next.IsNull()) m_last.m_list=list;
		else node->m_next->m_prev.m_list=list;
		if (node->m_prev.IsNull()) m_first.m_list=list;
		else node->m_prev->m_next.m_list=list;
	}	
};

template<class T> class LocalListIterator;

/**
* A LocalList is a list that contains elements that are also registered in a GlobalList. This class is 
* responsible for keeping the global one synchronized, and is the real owner of the elements.
**/
template<class T>
class LocalList : private SingleList<LocalListNode<T> > {
	friend class LocalListNodePtr<T>;
	friend class LocalListIterator<T>;
	GlobalList<T> *m_global_list; ///< a GlobalList that will contain references to elements in this list (among others)
	LocalList(const LocalList &other) {} ///< it is not allowed to duplicate a LocalList
public:
	/// creates an not-initializaed local list (no global list attached, should call Init before using it)
	LocalList():m_global_list(NULL) { }
	/// initialize a list created with the default null constructor (attachs a global list)
	void Init(GlobalList<T> *global_list) {
		m_global_list=global_list;
	}
	LocalList(LocalList<T> *local_list):SingleList<LocalListNode<T> >(local_list) {
		for(int i=0;i<local_list->m_size;i++)
			m_global_list->UpdateList(SingleList<LocalListNode<T> >::m_vec+i,this);
	}
	/// steals data from other LocalList (and update global list references, *this should be an empty list)
	void StealFrom(LocalList<T> *local_list) {
		SingleList<LocalListNode<T> >::StealFrom(local_list);
		for(int i=0;i<SingleList<LocalListNode<T> >::m_size;i++)
			m_global_list->UpdateList(SingleList<LocalListNode<T> >::m_vec+i,this);
	}
	/// creates an emtpy LocalList (attached to the given global list)
	LocalList(GlobalList<T> *global_list):m_global_list(global_list) {
		
	}
	/// return the number of elements in the local list
	int GetSize() const { 
		return SingleList<LocalListNode<T> >::GetSize();
	}
	/// adds an element to the list (adds to both local and global lists)
	void Add(const T &data) {
		LocalListNode<T> node; node.m_data=data;
		int pos=SingleList<LocalListNode<T> >::Add(node);
		m_global_list->Add(LocalListNodePtr<T>(this,pos));
	}
	/// adds an element to the list (removes from both local and global lists)
	void Remove(int pos) {
		m_global_list->Remove(LocalListNodePtr<T>(this,pos)); // remove from global
		SingleList<LocalListNode<T> >::FastRemove(pos); // remove from local (this will make a swap)
		int size=SingleList<LocalListNode<T> >::m_size;
		if (size!=pos) m_global_list->UpdatePos(SingleList<LocalListNode<T> >::m_vec+pos,pos); // udate position in references to the swapped element)
	}
	/// swaps positions for two elements
	void Swap(int i, int j) {
//		LocalListNode<T> aux=SingleList<LocalListNode<T> >::m_vec[i]; 
//		SingleList<LocalListNode<T> >::m_vec[i]=SingleList<LocalListNode<T> >::m_vec[j]; 
//		SingleList<LocalListNode<T> >::m_vec[j]=aux;
//		m_global_list->UpdatePos(SingleList<LocalListNode<T> >::m_vec+j,j); // udate position in references to the swapped element)
//		m_global_list->UpdatePos(SingleList<LocalListNode<T> >::m_vec+i,i); // udate position in references to the swapped element)
		T aux=SingleList<LocalListNode<T> >::m_vec[i].m_data;
		SingleList<LocalListNode<T> >::m_vec[i].m_data=SingleList<LocalListNode<T> >::m_vec[j].m_data; 
		SingleList<LocalListNode<T> >::m_vec[j].m_data=aux;
	}
	/// retrieves and elemento from the list
	T &operator[](int i) { 
		return SingleList<LocalListNode<T> >::m_vec[i].m_data;
	}
	/// retrieves and elemento from the list (for consts lists)
	const T &operator[](int i) const { 
		return SingleList<LocalListNode<T> >::m_vec[i].m_data;
	}
	/// erase all elements from the list (cleans both local and global lists, also frees internal reserved memory)
	void Reset() { 
		for(int i=0;i<SingleList<LocalListNode<T> >::m_size;i++)
			m_global_list->Remove(LocalListNodePtr<T>(this,i));
		SingleList<LocalListNode<T> >::Reset();
	}
	/// erase all elements from the list (cleans both local and global lists, but keeps the internal reserved memory)
	void Clear() { 
		for(int i=0;i<SingleList<LocalListNode<T> >::m_size;i++)
			m_global_list->Remove(LocalListNodePtr<T>(this,i));
		SingleList<LocalListNode<T> >::Clear();
	}
	/// find and element in the list, return value is the zero-base position, or the special value given by NotFound() method
	int Find(const T &data) {
		for(int i=0;i<SingleList<LocalListNode<T> >::m_size;i++) { 
			if (SingleList<LocalListNode<T> >::m_vec[i].m_data==data) return i;
		}
		return NotFound();
	}
	/// gives an special position used to mark invalid positions (example, when a Find call cannot find the element)
	static int NotFound() { return SingleList<LocalListNode<T> >::NotFound(); }
	/// remove all element (also remove their references from global list)
	~LocalList() { 
		Clear();
	}
};

template<class T>
LocalListNode<T>* LocalListNodePtr<T>::operator->() { return m_list->m_vec+m_pos; }

/**
* Special class for traversing elements in a GlobalList. 
*
* User should create an instance using an existing GlobalList as argument, and
* traverse elements with Next() and GetData() until IsValid() returns false.
**/ 
template<class T>
class GlobalListIterator {
	LocalListNodePtr<T> m_node;
	friend class GlobalList<T>;
public:
	GlobalListIterator() {}
	GlobalListIterator(GlobalList<T> *g) : m_node(g->m_first) {}
	bool IsValid () { return !m_node.IsNull(); }
	bool Next() { m_node=m_node->m_next; return !m_node.IsNull(); }
	T &operator*() { return m_node->m_data; }
	const T &operator*() const { return m_node->data; }
	T &GetData() { return m_node->m_data; }
	const T &GetData() const { return m_node->data; }
	T &operator->() { return m_node->m_data; }
	const T &operator->() const { return m_node->data; }
};


template<class T>
GlobalListIterator<T> GlobalList<T>::Find(T data) {
	GlobalListIterator<T> it(this);
	while (it.IsValid()) {
		if (*it==data) return it;
		it.Next();
	}
	return it;
}

template<class T>
void GlobalList<T>::Remove(GlobalListIterator<T> it) {
	it.m_node.m_list->Remove(it.m_node.m_pos);
}

template<class T>
void GlobalList<T>::FindAndRemove(T data) {
	GlobalListIterator<T> it=Find(data);
	if (it.IsValid()) Remove(it);
}

template<class T>
class LocalListIterator {
	LocalListNodePtr<T> m_ptr;
public:
	LocalListIterator() {}
	LocalListIterator(LocalList<T> *list) : m_ptr(list,0) {}
	bool IsValid() { return m_ptr.m_list && m_ptr.m_pos<m_ptr.m_list->m_size; }
	void Next() { m_ptr.m_pos++; }
	T &GetData() { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
	const T &GetData() const { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
	T &operator*() { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
	const T &operator*() const { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
	T &operator->() { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
	const T &operator->() const { return (m_ptr.m_list->m_vec+m_ptr.m_pos)->m_data; }
};
#endif

