#ifndef PARSER_DATA_H
#define PARSER_DATA_H

/*  La informacion del parser se almacena en estas estructuras. Las estructuras son
nodos para listas enlazadas. Los enlaces se manejan con macros para evitar la
sobrecarga de llamar a funciones.
Al parsear un archivo se crea un pd_file asociado al mismo (previamente se borra el 
anterior si ya habia uno), y luego se le van agregando las cosas con las macros PD_ADD_*.
Estas macros agregan las cosas en las listas correspondientes de classes/funciones/etc
y tambien agregan las referencias en la estructura del archivo, y el item al wxTreeCtrl.
De esta forma, las clases/funciones/etc nunca se eliminan directamente, sino que
como siempre estan asociadas a uno o mas archivo se eliminan mediante el destructor 
del archivo que corresponda (si estan asociadas a dos archivos, se eliminan con el 
destructor del ultimo de ellos que se destruya). Puede darse el caso de que una
clase no este asociada a ningun archivo (por no estar declarada pero si sus miembros, 
o por ser un namespace). En este caso la clase se elimina cuando se elimina el
ultimo de sus miembros o metodos. Respecto a la presencia en el control del arbol, 
cada cosa guarda su itemID. El responsable de insertarlo en el arbol es el parser
mediante las macros PD_ADD_*, pero las cosas se borran solas mediante su destructor. */

//#include "mxMainWindow.h"
#include "Parser.h"

// constantes para manipular con operaciones de bits las propiedades de los metodos y atributos
#define PD_CONST_PUBLIC 			(1<<0)
#define PD_CONST_PRIVATE 			(1<<1)
#define PD_CONST_PROTECTED	 		(1<<2)
#define PD_CONST_VIRTUAL 			(1<<3)
#define PD_CONST_STATIC 			(1<<4)
#define PD_CONST_CONSTRUCTOR 		(1<<5)
#define PD_CONST_DESTRUCTOR		 	(1<<6)
#define PD_CONST_CONST			 	(1<<7)
#define PD_CONST_VOLATILE 			(1<<8)
#define PD_CONST_VIRTUAL_PURE		(1<<9)

#define PD_CONST_ENUM 				(1<<10)
#define PD_CONST_ENUM_CONST 		(1<<11)
#define PD_CONST_MACRO_CONST		(1<<12)
#define PD_CONST_MACRO_FUNC 		(1<<13)
#define PD_CONST_TYPEDEF 			(1<<14)

// macros para cargar el autocompletado estandar


#define CH_REGISTER_FILE(afile,aname) \
	HashStringParserFile::iterator it=parser->h_files.find(aname); \
	if (it==parser->h_files.end()) \
		parser->h_files[aname] = parser->first_file->next = afile=new pd_file(nullptr,aname,wxDateTime(),parser->first_file->next); \
	else \
		afile=it->second;
//	DEBUG_INFO("Archivo: "<<aname);

#define CH_REGISTER_CLASS(afile,aclass,aname) \
	pd_class* &aux=parser->h_classes[aname]; \
	if (!aux) aux=parser->first_class->next = new pd_class(nullptr,afile,0,aname,"",parser->first_class->next); \
	aclass=aux;
//	DEBUG_INFO("   Clase: "<<aname<<"*");

#define CH_REGISTER_INHERIT(afather, aprops, ason) \
	parser->first_inherit->next = new pd_inherit(nullptr,ason,afather,aprops,parser->first_inherit->next); \

// para macros, typedefs, enums (y enums_consts) globales (la diferencia es props)
#define CH_REGISTER_MACRO(afile,aname, aprops) \
	parser->h_macros[aname] = parser->first_macro->next = new pd_macro(nullptr,afile,0,aname,aprops,parser->first_macro->next);

#define CH_REGISTER_MACRO_P(afile,aname,aproto,aprops) \
	parser->h_macros[aname] = parser->first_macro->next = new pd_macro(nullptr,afile,0,aname,aprops,parser->first_macro->next);\
	parser->first_macro->next->proto = aproto;\
//	DEBUG_INFO("   Macro: "<<aname);

#define CH_REGISTER_GLOBAL(afile, atype, aname, aproto, aprops) \
	parser->h_globals[aname] = parser->first_global->next = new pd_var(wxTreeItemId(),nullptr,afile,0,aname,atype,aproto,aprops,parser->first_global->next,nullptr,""); \
//	DEBUG_INFO("   Global:  t:"<<atype<<"   n:"<<aname<<"   p:"<<aproto);

// para atributos y enums/enums_const dentro de clases (la diferencia es props)
#define CH_REGISTER_ATTRIB(afile, aclass, atype, aname, aproto, aprops) \
	aclass->h_attribs[aname] = aclass->first_attrib->next = new pd_var(wxTreeItemId(),nullptr,afile,0,aname,atype,aproto,aprops,aclass->first_attrib->next,aclass,aproto); \
//	DEBUG_INFO("      Attrib:  t:"<<atype<<"   n:"<<aname<<"   p:"<<aproto);

#define CH_REGISTER_FUNCTION(afile, atype, aname, aproto) \
	parser->h_functions[aproto] = parser->first_function->next = new pd_func(wxTreeItemId(), nullptr, aname, aproto, aproto, parser->first_function->next, nullptr); \
	parser->first_function->next->file_dec=afile; \
//	DEBUG_INFO("   Function:  t:"<<atype<<"   n:"<<aname<<"  p:"<<aproto);

#define CH_REGISTER_METHOD(afile, aclass, atype, aname, aproto) \
	aclass->h_methods[aproto] = aclass->first_method->next = new pd_func(wxTreeItemId(), nullptr, aname, aproto, aproto, aclass->first_method->next, aclass); \
	aclass->first_method->next->properties = PD_CONST_PUBLIC;
//	DEBUG_INFO("   Function:  t:"<<atype<<"   n:"<<aname<<"  p:"<<aproto);



// macros para trabajar con el wxTreeCtrl del arbol de simbolos

#define PD_TREE_CTRL_DELETE(aitem) \
	parser->symbol_tree->Delete(aitem); \


#define PD_TREE_CTRL_ADD_CLASS(aname) \
	parser->symbol_tree->AppendItem(parser->item_classes,aname,mxSTI_CLASS)

#define PD_TREE_CTRL_ADD_SPACE(aname) \
	parser->symbol_tree->AppendItem(parser->item_classes,aname,mxSTI_NAMESPACE)

#define PD_TREE_SET_NAMESPACE(aitem) \
	parser->symbol_tree->SetItemImage(aitem,mxSTI_NAMESPACE);

#define PD_TREE_SET_CLASS(aitem) \
	parser->symbol_tree->SetItemImage(aitem,mxSTI_CLASS);
	
#define PD_TREE_CTRL_ADD_FUNC(aitem, aname) \
	parser->symbol_tree->AppendItem(aitem,aname,mxSTI_FUNCTION)

#define PD_TREE_CTRL_ADD_VAR(aitem, aname, aprops) \
	parser->symbol_tree->AppendItem(aitem,aname, ((aprops)&(PD_CONST_ENUM_CONST|PD_CONST_ENUM))  ? mxSTI_ENUM_CONST : mxSTI_GLOBAL)

#define PD_TREE_CTRL_ADD_MACRO(aname) \
	parser->symbol_tree->AppendItem(parser->item_macros,aname,mxSTI_DEFINE)

#define PD_TREE_CTRL_ADD_ENUM_CONST(aname) \
	parser->symbol_tree->AppendItem(parser->item_macros,aname,mxSTI_ENUM_CONST) 

#define PD_TREE_CTRL_ADD_TYPEDEF(aname) \
	parser->symbol_tree->AppendItem(parser->item_macros,aname,mxSTI_TYPEDEF)

#define PD_TREE_CTRL_SET_ATTRIB(aitem,aprops) \
	if ((aprops)&(PD_CONST_ENUM_CONST|PD_CONST_ENUM)) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_ENUM_CONST); \
	else if ((aprops)&PD_CONST_PUBLIC) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PUBLIC_ATTRIB); \
	else if ((aprops)&PD_CONST_PROTECTED) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PROTECTED_ATTRIB); \
	else if ((aprops)&PD_CONST_PRIVATE) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PRIVATE_ATTRIB); \
	else \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_ATTRIB);

#define PD_TREE_CTRL_SET_METHOD(aitem, aprops) \
	if ((aprops)&PD_CONST_PUBLIC) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PUBLIC_MEMBER); \
	else if ((aprops)&PD_CONST_PROTECTED) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PROTECTED_MEMBER); \
	else if ((aprops)&PD_CONST_PRIVATE) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PRIVATE_MEMBER); \
	else \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_MEMBER);

#define PD_TREE_CTRL_UPDATE_METHOD_IMAGE(aitem, aprops) \
	if ((aprops)|PD_CONST_PUBLIC) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PUBLIC_MEMBER); \
	else if ((aprops)|PD_CONST_PROTECTED) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PROTECTED_MEMBER); \
	else if ((aprops)|PD_CONST_PRIVATE) \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_PRIVATE_MEMBER); \
	else \
		parser->symbol_tree->SetItemImage(aitem,mxSTI_MEMBER);




// macros varias

#define PD_UNREF(what,who) ((what*)(who->ref))

#define PD_INSERT(first,what) \
	if (first->next) \
		first->next->prev=what; \
	first->next=what;

#define PD_DELETE_FILE(aname) {\
	HashStringParserFile::iterator it = parser->h_files.find(aname); \
	if (it!=parser->h_files.end()) { \
		if (it->second->prev) \
			it->second->prev->next=it->second->next; \
		if (it->second->next) \
			it->second->next->prev = it->second->prev; \
		delete it->second; \
		parser->h_files.erase(it); \
	} \
}

#define PD_DELETE_INHERIT(ainh) \
	ainh->prev->next=ainh->next; \
	if (ainh->next) \
		ainh->next->prev=ainh->prev; \
	delete ainh;

#define PD_CLASS_IS_EMPTY(aclass) \
	(aclass->file==nullptr && aclass->first_method->next==nullptr && aclass->first_attrib->next==nullptr)


#define PD_REF_CLEAR(first) { \
	pd_ref *aux; \
	first=first->next; \
	while (first!=nullptr) { \
		aux=first; \
		first=first->next; \
		delete aux; \
	} \
	delete first; \
}


#define PD_ADD_REF(first, counter, where) \
	first->next = new pd_ref(first, where, counter, first->next); \
	if (first->next->next) \
		first->next->next->prev = first->next; \

#define PD_DEL_REF(item) \
	if (item->next) \
		item->next->prev = item->prev; \
	item->prev->next = item->next; \
	delete item;

//#define PD_FILE_EXISTS(aname) h_files.find(aname)!=h_files.end()
//#define PD_CLASS_EXISTS(aname) h_classes.find(aname)!=h_classes.end()
//#define PD_FUNCTION_EXISTS(aname) h_functions.find(aname)!=h_functions.end()


#define PD_RENAME_FILE(oname,aname) \
	HashStringParserFile::iterator it = h_files.find(oname); \
	if (it!=h_files.end()) { \
		it->second->name=aname; \
		h_files[aname]= it->second; \
		h_files.erase(it); \
	}

#define PD_REGISTER_FILE(afile,aname,atime) \
	HashStringParserFile::iterator it = h_files.find(aname); \
	if (it!=h_files.end()) { \
		afile = it->second; \
		afile->time = atime; \
	} else {\
		h_files[aname] = afile = new pd_file(last_file, aname, atime, last_file->next); \
		PD_INSERT(last_file,afile); \
	} \
	afile->counter++;

#define PD_REGISTER_INHERIT(afile,aname,afather,aprops) { \
	pd_inherit *pd_aux = new pd_inherit(first_inherit,aname,afather,aprops,first_inherit->next); \
	if (first_inherit->next) \
		first_inherit->next->prev=pd_aux; \
	first_inherit->next=pd_aux; \
	PD_ADD_REF(afile->first_inherit,afile->counter,pd_aux); \
}

#define PD_REGISTER_GLOBAL(afile,aline,atype,aname,aprops) { \
	wxString aproto = atype+" "+aname; \
	HashStringParserVariable::iterator it = h_globals.find(aproto); \
	pd_var *pd_aux; \
	if (it!=h_globals.end() && it->second->prev) { \
		pd_aux = it->second; \
		wxString ext_new=(afile->name).AfterLast('.'); \
		wxString ext_old=(pd_aux->file->name).AfterLast('.'); \
		bool was_header= ext_old.size()>0&&(ext_old[0]|32)=='h';\
		bool is_header = ext_new.size()>0&&(ext_new[0]|32)=='h';\
		if (is_header || !was_header) { \
			PD_DEL_REF(pd_aux->ref); \
			PD_ADD_REF(afile->first_global,afile->counter,pd_aux); \
			pd_aux->ref = afile->first_global->next; \
			pd_aux->file = afile; \
			pd_aux->line = aline; \
			pd_aux->ref->counter = afile->counter; \
			pd_aux->properties = aprops; \
		}\
	} else {\
		h_globals[aproto] = pd_aux = new pd_var(item_globals, last_global, afile, aline, aname, atype, aproto, aprops, last_global->next); \
		PD_INSERT(last_global,pd_aux); \
		PD_ADD_REF(afile->first_global,afile->counter,pd_aux); \
		pd_aux->ref = afile->first_global->next; \
		pd_aux->line = aline; \
		pd_aux->ref->counter = afile->counter; \
	} \
}

#define PD_REGISTER_MACRO(afile,aline,aname,acont,aparams,atype) { \
	HashStringParserMacro::iterator it = h_macros.find(aname); \
	pd_macro *pd_aux; \
	if (it!=h_macros.end() && it->second->prev) { \
		pd_aux = it->second; \
		if (pd_aux->file!=afile) { \
			PD_DEL_REF(pd_aux->ref); \
			PD_ADD_REF(afile->first_macro,afile->counter,pd_aux); \
			pd_aux->ref = afile->first_macro->next; \
			pd_aux->file = afile; \
		} \
	} else {\
		h_macros[aname] = pd_aux = new pd_macro(last_macro, afile, aline, aname, atype, last_macro->next); \
		PD_INSERT(last_macro,pd_aux); \
		PD_ADD_REF(afile->first_macro,afile->counter,pd_aux); \
		pd_aux->ref = afile->first_macro->next; \
	} \
	pd_aux->cont = acont;\
	if (atype&PD_CONST_MACRO_FUNC) \
		pd_aux->proto = aname+"("+aparams+")";\
	else \
		pd_aux->proto = aname;\
	pd_aux->line = aline; \
	pd_aux->ref->counter = afile->counter; \
}

#define PD_REGISTER_FUNCTION_DEC(afile,aline,aname,aproto,afullproto) { \
	HashStringParserFunction::iterator it = h_functions.find(aproto); \
	pd_func *pd_aux; \
	if (it!=h_functions.end() && it->second->prev) { \
		pd_aux = it->second; \
		pd_aux->full_proto = afullproto; \
	} else { \
		h_functions[aproto] = pd_aux = new pd_func(item_functions, last_function, aname, aproto, afullproto, last_function->next, nullptr); \
		PD_INSERT(last_function,pd_aux); \
	} \
	if (pd_aux->file_dec==afile) { \
		pd_aux->line_dec = line; \
		pd_aux->ref_dec->counter = afile->counter; \
	} else { \
		if (pd_aux->file_dec) { \
			PD_DEL_REF(pd_aux->ref_dec); \
		} \
		pd_aux->file_dec = afile; \
		pd_aux->line_dec = aline; \
		PD_ADD_REF(afile->first_func_dec,afile->counter,pd_aux); \
		pd_aux->ref_dec = afile->first_func_dec->next; \
	} \
}


#define PD_REGISTER_METHOD_DEC(aclass,afile,aline,atype,aspace,aname,aparams,afullparams,aprops) { \
	wxString aproto = ((props&(PD_CONST_DESTRUCTOR|PD_CONST_CONSTRUCTOR))?"":atype + " ") + aname + " (" + aparams + ")"; \
	wxString afullproto = ((props&(PD_CONST_DESTRUCTOR|PD_CONST_CONSTRUCTOR))?"":atype + " ") + aspace + "::" + aname + " (" + afullparams + ")"; \
	if (!aclass || aclass->name!=aspace) { \
		HashStringParserClass::iterator pd_it = h_classes.find(aspace); \
		if (pd_it!=h_classes.end() && pd_it->second->prev) { \
			aclass = pd_it->second; \
		} else { \
			aclass = new pd_class(last_class, nullptr, 0, aspace, "", last_class->next); \
			PD_INSERT(last_class,aclass); \
			h_classes[aspace] = aclass; \
		} \
	} \
	HashStringParserFunction::iterator it = aclass->h_methods.find(aproto); \
	pd_func *pd_aux; \
	if (it!=aclass->h_methods.end()) { \
		pd_aux = it->second; \
		pd_aux->full_proto = afullproto; \
	} else { \
		aclass->h_methods[aproto] = pd_aux = new pd_func(aclass->item, aclass->first_method, aname, aproto, afullproto, aclass->first_method->next, aclass); \
		PD_INSERT(aclass->first_method,pd_aux); \
	} \
	if (pd_aux->file_dec==afile) { \
		pd_aux->line_dec = line; \
		pd_aux->ref_dec->counter = afile->counter; \
	} else { \
		if (pd_aux->file_dec) { \
			PD_DEL_REF(pd_aux->ref_dec); \
		} \
		pd_aux->file_dec = afile; \
		pd_aux->line_dec = aline; \
		PD_ADD_REF(afile->first_func_dec,afile->counter,pd_aux); \
		pd_aux->ref_dec = afile->first_func_dec->next; \
	} \
	pd_aux->properties = (pd_aux->props_dec=aprops)|pd_aux->props_def; \
	PD_TREE_CTRL_SET_METHOD(pd_aux->item, pd_aux->properties); \
}



#define PD_REGISTER_METHOD_DEF(aclass,afile,aline,atype,aspace,aname,aparams,afullparams,aprops) { \
	wxString aproto = ((props&(PD_CONST_DESTRUCTOR|PD_CONST_CONSTRUCTOR))?"":atype + " ") + aname + " (" + aparams + ")"; \
	wxString afullproto = ((props&(PD_CONST_DESTRUCTOR|PD_CONST_CONSTRUCTOR))?"":atype + " ") + aspace + "::" + aname + " (" + afullparams + ")"; \
	if (!aclass || aclass->name!=aspace) { \
		HashStringParserClass::iterator pd_it = h_classes.find(aspace); \
		if (pd_it!=h_classes.end() && pd_it->second->prev) { \
			aclass = pd_it->second; \
		} else { \
			aclass = new pd_class(last_class, nullptr, 0, aspace, "", last_class->next); \
			PD_INSERT(last_class,aclass); \
			h_classes[aspace] = aclass; \
		} \
	} \
	HashStringParserFunction::iterator it = aclass->h_methods.find(aproto); \
	pd_func *pd_aux; \
	if (it!=aclass->h_methods.end()) { \
		pd_aux = it->second; \
		pd_aux->full_proto = afullproto; \
	} else { \
		aclass->h_methods[aproto] = pd_aux = new pd_func(aclass->item, aclass->first_method, aname, aproto, afullproto, aclass->first_method->next, aclass); \
		PD_INSERT(aclass->first_method,pd_aux); \
	} \
	if (pd_aux->file_def==afile) { \
		pd_aux->line_def = line; \
		pd_aux->ref_def->counter = afile->counter; \
	} else { \
		if (pd_aux->file_def) { \
			PD_DEL_REF(pd_aux->ref_def); \
		} \
		pd_aux->file_def = afile; \
		pd_aux->line_def = aline; \
		PD_ADD_REF(afile->first_func_def,afile->counter,pd_aux); \
		pd_aux->ref_def = afile->first_func_def->next; \
	} \
	pd_aux->properties = (pd_aux->props_def=aprops)|pd_aux->props_dec; \
	PD_TREE_CTRL_SET_METHOD(pd_aux->item, pd_aux->properties); \
}

#define PD_REGISTER_FUNCTION_DEF(afile,aline,aname,aproto,afullproto) { \
	HashStringParserFunction::iterator it = h_functions.find(aproto); \
	pd_func *pd_aux; \
	if (it!=h_functions.end() && it->second->prev) { \
		pd_aux = it->second; \
		pd_aux->full_proto = afullproto; \
	} else { \
		h_functions[aproto] = pd_aux = new pd_func(item_functions, last_function, aname, aproto, afullproto, last_function->next, nullptr); \
		PD_INSERT(last_function,pd_aux); \
	} \
	if (pd_aux->file_def==afile) { \
		pd_aux->line_def = line; \
		pd_aux->ref_def->counter = afile->counter; \
	} else { \
		if (pd_aux->file_def) { \
			PD_DEL_REF(pd_aux->ref_def); \
		} \
		pd_aux->file_def = afile; \
		pd_aux->line_def = aline; \
		PD_ADD_REF(afile->first_func_def,afile->counter,pd_aux); \
		pd_aux->ref_def = afile->first_func_def->next; \
	} \
}



#define PD_REGISTER_ATTRIB(aclass,afile,aline,aspace,atype,aname,aprops) { \
	wxString aproto = atype + " " + aname; \
	wxString afullproto = atype + " " + aspace + "::" + aname; \
	if (!aclass || aclass->name!=aspace) { \
		HashStringParserClass::iterator pd_it = h_classes.find(aspace); \
		if (pd_it!=h_classes.end() && pd_it->second->prev) { \
			aclass = pd_it->second; \
		} else { \
			aclass = new pd_class(last_class, nullptr, 0, aspace, "", last_class->next); \
			PD_INSERT(last_class,aclass); \
			h_classes[aspace] = aclass; \
		} \
	} \
	HashStringParserVariable::iterator it = aclass->h_attribs.find(aproto); \
	pd_var *pd_aux; \
	if (it!=aclass->h_attribs.end()) { \
		pd_aux = it->second; \
		pd_aux->properties = (aprops); \
	} else { \
		aclass->h_attribs[aproto] = pd_aux = new pd_var(aclass->item, aclass->first_attrib, afile, aline, aname, atype, aproto, aprops, aclass->first_attrib->next,aclass,afullproto); \
		PD_INSERT(aclass->first_attrib,pd_aux); \
		PD_ADD_REF(afile->first_attrib,afile->counter,pd_aux); \
		pd_aux->ref = afile->first_attrib->next; \
	} \
	if (pd_aux->file==afile) { \
		pd_aux->line = line; \
		pd_aux->ref->counter = afile->counter; \
	} else { \
		if (pd_aux->ref) { \
			PD_DEL_REF(pd_aux->ref); \
		} \
		pd_aux->file = afile; \
		pd_aux->line = aline; \
		PD_ADD_REF(afile->first_attrib,afile->counter,pd_aux); \
		pd_aux->ref = afile->first_attrib->next; \
	} \
	PD_TREE_CTRL_SET_ATTRIB(pd_aux->item, aprops); \
}

#define PD_REGISTER_CLASS_DEF(pd_aux, afile, aline, aname, atempl, ais_union) { \
	if (!pd_aux || pd_aux->name!=aname) { \
		HashStringParserClass::iterator pd_it = h_classes.find(aname); \
		if (pd_it==h_classes.end() || !pd_it->second->prev) {\
			pd_aux = new pd_class(last_class, afile, aline, aname, atempl, last_class->next); \
			PD_INSERT(last_class,pd_aux); \
			PD_ADD_REF(afile->first_class, afile->counter, pd_aux); \
			pd_aux->ref = afile->first_class->next; \
			h_classes[name] = pd_aux; \
		} else { \
			pd_aux = pd_it->second; \
			pd_aux->templ=atempl; \
		} \
	} \
	if (pd_aux->file==afile) { \
		pd_aux->line = aline; \
		pd_aux->ref->counter = afile->counter; \
	} else { \
		if (pd_aux->ref) { \
			PD_DEL_REF(pd_aux->ref); \
		} else \
			PD_TREE_SET_CLASS(pd_aux->item); \
		pd_aux->file = afile; \
		pd_aux->line = aline; \
		PD_ADD_REF(afile->first_class, afile->counter, pd_aux); \
		pd_aux->ref = afile->first_class->next; \
	} \
	pd_aux->is_union=ais_union; \
}

// elimina una clase del hash, de la lista, y de la memoria
#define PD_DELETE_CLASS(ahash, aclass) { \
	ahash.erase(ahash.find(aclass->name)); \
	if (aclass->next) \
		aclass->next->prev = aclass->prev; \
	aclass->prev->next = aclass->next; \
	delete aclass; \
}

#define PD_DELETE_FUNCTION(ahash, afunc) { \
	ahash.erase(ahash.find(afunc->proto)); \
	if (afunc->next) \
		afunc->next->prev = afunc->prev; \
	afunc->prev->next = afunc->next; \
	delete afunc; \
}

#define PD_DELETE_METHOD(ahash, afunc) { \
	ahash.erase(ahash.find(afunc->proto)); \
	if (afunc->next) \
		afunc->next->prev = afunc->prev; \
	afunc->prev->next = afunc->next; \
	if (PD_CLASS_IS_EMPTY(afunc->space) ) { \
		pd_class *aux = afunc->space; \
		delete afunc; \
		PD_DELETE_CLASS(parser->h_classes,aux); \
	} else\
		delete afunc; \
}


#define PD_DELETE_ATTRIB(ahash, avar) { \
	ahash.erase(ahash.find(avar->proto)); \
	if (avar->next) \
		avar->next->prev = avar->prev; \
	avar->prev->next = avar->next; \
	if (PD_CLASS_IS_EMPTY(avar->space) ) { \
		pd_class *aux = avar->space; \
		delete avar; \
		PD_DELETE_CLASS(parser->h_classes,aux); \
	} else\
		delete avar; \
}

#define PD_DELETE_GLOBAL(ahash, avar) { \
	ahash.erase(ahash.find(avar->full_proto)); \
	if (avar->next) \
		avar->next->prev = avar->prev; \
	avar->prev->next = avar->next; \
	delete avar; \
}

#define PD_DELETE_MACRO(ahash, amacro) { \
	ahash.erase(ahash.find(amacro->name)); \
	if (amacro->next) \
		amacro->next->prev = amacro->prev; \
	amacro->prev->next = amacro->next; \
	delete amacro; \
}

struct pd_file;
struct pd_class;

struct pd_ref { // para la lista de cosas asociadas a un archivo o clase
	pd_ref *prev, *next; // porque esto es un nodo de una lista enlazada
	void *ref; // cosa a la que hace referencia
	char counter;
	pd_ref(pd_ref *aprev, void *aref, char acounter, pd_ref *anext) {
		prev=aprev;
		next=anext;
		ref=aref;
		counter=acounter;
	}
};


struct pd_func { // funciones y metodos (las func no tienen por space una clase, space==nullptr || space->file_def==nullptr, el segundo es namespace)
	HashStringParserFunction::iterator hash_it; // hay un hash por nombre de funcion para encontrar rapido si ya estan definidas o no
	wxTreeItemId item; // este es el item del arbol de simbolos para borrarlo/actualizarlo rapidamente, o para recuperar
	wxTreeItemId image; // es el icono que va en el arbol
	pd_func *prev,*next; // porque esto es un nodo de una lista enlazada
	pd_file *file_dec; // archivo donde se declaro
	int line_dec; // linea donde se declaro
	pd_ref *ref_dec; // la referencia que la apunta (dentro del archivo)
	pd_file *file_def; // archivo donde se definio
	int line_def; // linea donde se definio
	pd_ref *ref_def; // la referencia que la apunta (dentro del archivo)
	unsigned short props_def; // visibilidad/static/virtual/¿common/etc?
	unsigned short props_dec; // visibilidad/static/virtual/¿common/etc?
	unsigned short properties;
	wxString name; // nombre pelado
	wxString proto; // prototipo completo
	wxString full_proto; // prototipo completo
	pd_class *space;
	pd_func(wxTreeItemId fitem, pd_func *aprev, wxString &aname, wxString &aproto, wxString afullproto, pd_func *anext, pd_class *aspace=nullptr){
//		if ( (space=aspace) )
//			full_proto=afullproto;
//		else
//			full_proto=proto;
		space=aspace;
		full_proto=afullproto;
		if (aprev)
			item = PD_TREE_CTRL_ADD_FUNC(fitem,aproto);
		properties=props_def=props_dec=0;
		name=aname;
		proto=aproto;
		next=anext;
		prev=aprev;
		file_dec=file_def=nullptr;
		ref_dec=ref_def=nullptr;
//		if (prev) DEBUG_INFO(wxString("Creada Funcion ")<<name);
	}
	~pd_func() {
		if (prev) PD_TREE_CTRL_DELETE(item);
//		if (prev) DEBUG_INFO(wxString("Destruida Funcion ")<<name);
	}
};

struct pd_macro {
	wxTreeItemId item;
	pd_macro *prev,*next; // porque esto es un nodo de una lista enlazada
	wxString name;
	wxString cont;
	wxString proto;
	pd_file *file;
	pd_ref *ref;
	int line;
	int props; // PD_CONST_MACRO_CONST o PD_CONST_MACRO_FUNC o PD_CONST_TYPEDEF o PD_CONST_ENUM o PD_CONST_ENUM_CONST (uno solo de estos)
	pd_macro(pd_macro *aprev, pd_file *afile, int aline, wxString &aname, int aprops, pd_macro *anext) {
		props=aprops;
		if (aprev) {
			if (props&PD_CONST_TYPEDEF)
				item = PD_TREE_CTRL_ADD_TYPEDEF(aname);
			else if (props&(PD_CONST_ENUM|PD_CONST_ENUM_CONST))
				item = PD_TREE_CTRL_ADD_ENUM_CONST(aname);
			else
				item = PD_TREE_CTRL_ADD_MACRO(aname);
		}
		file=afile;
		line=aline;
		prev=aprev;
		next=anext;
		name=aname;
//		if (prev) DEBUG_INFO("Creada Macro "<<name);
	}
	~pd_macro(){
		if (prev) PD_TREE_CTRL_DELETE(item);
//		if (prev) DEBUG_INFO("Destruida Macro "<<name);
	}
};

struct pd_var {
	wxTreeItemId item;
	pd_var *prev,*next; // porque esto es un nodo de una lista enlazada
	pd_class *space;
	wxString name;
	wxString type;
	wxString proto;
	wxString full_proto;
	pd_file *file;
	pd_ref *ref;
	unsigned short properties;
	int line;
	pd_var(wxTreeItemId fitem, pd_var *aprev, pd_file *afile, int aline, wxString &aname, wxString &atype, wxString &aproto, int aproperties, pd_var *anext, pd_class *aspace = nullptr, wxString afullproto="") {
		space=aspace;
		proto=aproto;
		properties=aproperties;
		if (space)
			full_proto = afullproto;
		else
			full_proto = proto;
		if (aprev) 
			item = PD_TREE_CTRL_ADD_VAR(fitem, proto, properties);
		file=afile;
		line=aline;
		prev=aprev;
		next=anext;
		name=aname;
		type=atype;
//		if (prev) DEBUG_INFO("Creada Variable "<<name);
//		if (prev) DEBUG_INFO("FullProto: "<<full_proto);
	}
	~pd_var(){
		if (prev) 
			PD_TREE_CTRL_DELETE(item);
//		if (prev) DEBUG_INFO("Destruida Variable "<<name);
	}
};

struct pd_inherit {
	wxString father;
	wxString son;
	short properties;
	pd_inherit *next;
	pd_inherit *prev;
	pd_inherit(pd_inherit *aprev, wxString afather, wxString ason, int aproperties ,pd_inherit *anext) {
		father=afather;
		son=ason;
		properties=aproperties;
		next=anext;
		prev=aprev;
	}
};

struct pd_class { // clases, namespaces y uniones (para el uso son exactamente lo mismo, is_union separa entre clases y uniones, namespaces no tienen definicion, solo contienen miembros)
	pd_ref *ref; // la referencia que la apunta (dentro del archivo)
	wxTreeItemId item;
	wxTreeItemId image;
	int line;
	bool is_union;
	pd_class *prev,*next; // porque esto es un nodo de una lista enlazada
	wxString name;
	wxString templ;
//	pd_class *space;
	pd_file *file;
	pd_func *first_method;
	HashStringParserFunction h_methods;
	HashStringParserVariable h_attribs;
	pd_var *first_attrib;
//	wxArrayString fathers;
	pd_class(pd_class *aprev, pd_file *afile, int aline, wxString &aname, wxString atempl, pd_class *anext) {
		ref=nullptr;
		line=aline;
		templ=atempl;
		name=aname;
		prev=aprev;
		next=anext;
		file=afile;
//		space=nullptr;
		is_union=false;
		wxString s;
		first_method=new pd_func(wxTreeItemId(), nullptr,s,s,s,nullptr);
		first_attrib=new pd_var(wxTreeItemId(), nullptr,nullptr,0,s,s,s,0,nullptr);
		if (prev) {
			if (file)
				item = PD_TREE_CTRL_ADD_CLASS(name);
			else
				item = PD_TREE_CTRL_ADD_SPACE(name);
		}
//		if (prev) DEBUG_INFO(wxString("Creado Clase ")<<name);
	}
	~pd_class() {
		if (prev) PD_TREE_CTRL_DELETE(item);
		delete first_method;
		delete first_attrib;
//		if (prev) DEBUG_INFO(wxString("Destruido Clase ")<<name);
	}
	

};

struct pd_file { // archivos, para que el parser sepa que borrar
	bool hide_symbols; ///< symbols from this file should be hidden for certain operations (as "goto class/function/method" dialog, for files generated by wxbf, where we usually want to go to the inherited classes instead)
	char counter; // marca en las referencias para saber cuales son nuevas y cuales viejas
	//HashStringParserFile::iterator hash_it; // hay un hash por nombre de archivo para ubicarlos rapido
	pd_file *prev,*next; // porque esto es un nodo de una lista enlazada
	int ref_count; // cuantas veces se llama a este archivo (puefullprotode estar incluido varias veces)
	wxString name; // nombre completo del archivo (absoluto para includes del proyecto, entre < y > para includes del sistema)
	wxDateTime time; // momento en que se actualizo por ultima vez la informacion de este archivo
	// listas de cosas definidas en ese archivo
	pd_ref *first_class;
	pd_ref *first_method_dec;
	pd_ref *first_method_def;
	pd_ref *first_func_dec;
	pd_ref *first_func_def;
	pd_ref *first_global;
	pd_ref *first_attrib;
	pd_ref *first_macro;
	pd_ref *first_include;
	pd_ref *first_inherit;
	pd_file (pd_file *aprev,wxString aname, wxDateTime atime, pd_file *anext) : hide_symbols(false) {
		counter=0;
		ref_count=1;
		name=aname;
		time=atime;
		next=anext;
		prev=aprev;
		first_class=new pd_ref(nullptr,nullptr,0,nullptr);
		first_inherit=new pd_ref(nullptr,nullptr,0,nullptr);
		first_func_def=new pd_ref(nullptr,nullptr,0,nullptr);
		first_func_dec=new pd_ref(nullptr,nullptr,0,nullptr);
		first_global=new pd_ref(nullptr,nullptr,0,nullptr);
		first_macro=new pd_ref(nullptr,nullptr,0,nullptr);
		first_include=new pd_ref(nullptr,nullptr,0,nullptr);
		first_method_def=first_func_def;//new pd_ref(nullptr,nullptr,0,nullptr);
		first_method_dec=first_func_dec;//new pd_ref(nullptr,nullptr,0,nullptr);
		first_attrib=first_global;//new pd_ref(nullptr,nullptr,0,nullptr);
//		if (prev) DEBUG_INFO(wxString("Creado Archivo ")<<name);
	}
	
	void Purge() { // borra todo lo que ya no sirve o no le pertenece mas
//DEBUG_INFO("Purgando "<<name<<" ... ");
		pd_ref *item, *aux;
		
		// purgar clases
//DEBUG_INFO("Purgando clases... ");
		item = first_class->next;
		while(item!=nullptr) {
			aux=item->next;
			if (PD_UNREF(pd_class,item)->file==this && item->counter!=counter) {
				PD_UNREF(pd_class,item)->file=nullptr;
				if (PD_CLASS_IS_EMPTY(PD_UNREF(pd_class,item))) {
					PD_DELETE_CLASS(parser->h_classes,PD_UNREF(pd_class,item));
				} else
					PD_UNREF(pd_class,item)->ref=nullptr;
				PD_DEL_REF(item);
			} else if (PD_UNREF(pd_class,item)->file!=this) {
				PD_DEL_REF(item);
			}
			item = aux;
		}
		
		// purgar funciones y metodos
		
//DEBUG_INFO("Purgando funciones (def)... ");
		item = first_func_def->next;
		while(item!=nullptr) {
			aux=item->next;
			if (PD_UNREF(pd_func,item)->file_def==this && item->counter!=counter) {
				if (PD_UNREF(pd_func,item)->file_dec!=nullptr) {
						PD_UNREF(pd_func,item)->file_def=nullptr;
				} else {
					if (PD_UNREF(pd_func,item)->space) {
						PD_DELETE_METHOD(PD_UNREF(pd_func,item)->space->h_methods,PD_UNREF(pd_func,item));
					} else {
						PD_DELETE_FUNCTION(parser->h_functions,PD_UNREF(pd_func,item));
					}
				}
				PD_DEL_REF(item);
			} else if (PD_UNREF(pd_func,item)->file_def!=this) {
				PD_DEL_REF(item);
			}
			item = aux;
		}		

//DEBUG_INFO("Purgando funciones (dec)... ");
		item = first_func_dec->next;
		while(item!=nullptr) {
			aux=item->next;
			if (PD_UNREF(pd_func,item)->file_dec==this && item->counter!=counter) {
				if (PD_UNREF(pd_func,item)->file_def!=nullptr) {
					PD_UNREF(pd_func,item)->file_dec=nullptr;
				} else {
					if (PD_UNREF(pd_func,item)->space) {
						PD_DELETE_METHOD(PD_UNREF(pd_func,item)->space->h_methods,PD_UNREF(pd_func,item));
					} else {
						PD_DELETE_FUNCTION(parser->h_functions,PD_UNREF(pd_func,item));
					}
				}
				PD_DEL_REF(item);
			} else if (PD_UNREF(pd_func,item)->file_dec!=this) {
				PD_DEL_REF(item);
			}
			item = aux;
		}
		
//DEBUG_INFO("Purgando macros... ");
		// purgar macros
		item = first_macro->next;
		while(item!=nullptr) {
			aux=item->next;
			if (PD_UNREF(pd_macro,item)->file==this && item->counter!=counter) {
				PD_DELETE_MACRO(parser->h_macros,PD_UNREF(pd_macro,item));
				PD_DEL_REF(item);
			} else if (PD_UNREF(pd_macro,item)->file!=this) {
				PD_DEL_REF(item);
			}
			item = aux;
		}		
		
//DEBUG_INFO("Purgando variables... ");
		// purgar variables y atributos
		item = first_global->next;
		while(item!=nullptr) {
			aux=item->next;
			if (PD_UNREF(pd_var,item)->file==this && item->counter!=counter) {
				if (PD_UNREF(pd_var,item)->space) {
					PD_DELETE_ATTRIB(PD_UNREF(pd_var,item)->space->h_attribs,PD_UNREF(pd_var,item));
				} else {
					PD_DELETE_GLOBAL(parser->h_globals,PD_UNREF(pd_var,item));
				}
				PD_DEL_REF(item);
			} else if (PD_UNREF(pd_var,item)->file!=this) {
				PD_DEL_REF(item);
			}
			item = aux;
		}

		// purgar herencias
		item = first_inherit->next;
		while(item!=nullptr) {
			aux=item->next;
			if (item->counter!=counter) {
				PD_DELETE_INHERIT(PD_UNREF(pd_inherit,item));
				PD_DEL_REF(item);
			}
			item = aux;
		}
		
		
//DEBUG_INFO("OK");
	}
	
	~pd_file() { // borrar todos lo que hay asociado a este archivo
		counter++;
		Purge();
		delete first_class;
		delete first_func_def;
		delete first_func_dec;
		delete first_global;
		delete first_macro;
		delete first_include;
		delete first_inherit;
//if (prev) DEBUG_INFO(wxString("Destruido archivo ")<<name);
	}
};

#endif
