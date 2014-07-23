#include "HelpManager.h"

#include <wx/textfile.h>
#include <wx/arrstr.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include "Parser.h"
#include "parserData.h"
#include "mxMainWindow.h"
#include "Language.h"
#include "ProjectManager.h"
#include <vector>
#include "CodeHelper.h"
using namespace std;

HelpManager *help = NULL;

HelpManager::HelpManager() {
//	HashStringString::iterator it;
//	wxTextFile file1(config->Help.quickhelp_index);
//	wxString key, value;
//	file1.Open();
//	if (file1.IsOpened()) {
//		wxString str;
//		for (str=file1.GetFirstLine();!file1.Eof();str=file1.GetNextLine()) {
//			key = str.BeforeFirst(' ');
//			value = str.AfterFirst(' ');
//			if (key.Len() && value.Len()) {
//				it = quick_help_hash.find(key);
//				if (it==quick_help_hash.end())
//					quick_help_hash[key] = value;
//				else
//					it->second<<"\n"<<value;
//			}
//		}
//	}
//	file1.Close();
}

/// funcion auxiliar para las que generan la ayuda rápida a partir de los datos del parser
static void AddDefRef(wxString &content, wxString text, wxString fname, int line) {
	content<<text<<" \"<A href=\"gotoline:"<<fname<<":"<<line<<"\">"<<fname<<"</A>\" "<<LANG(PARSERHELP_DEFINED_IN_POST,"en la linea")<<" "<<line<<"<BR><BR>";	
}

void HelpManager::HelpFor(pd_class *aclass, wxString &content, wxString &index) {
	int id=index_ref_counter++;
	if (aclass->file) {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<(aclass->is_union?LANG(PARSERHELP_UNION,"Union"):LANG(PARSERHELP_CLASS,"Clase"))<<_T(" <I>")<<aclass->name<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<(aclass->is_union?LANG(PARSERHELP_UNION,"Union"):LANG(PARSERHELP_CLASS,"Clase"))<<_T(" <I><A href=\"#")<<id<<_T("\">")<<aclass->name<<_T("</A></I></B><BR><BR>");
		AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),aclass->file->name,aclass->line);
		if (aclass->templ!="") {
			wxString aux = aclass->templ;
			aux.Replace(_T("<"),_T("&lt;"));
			aux.Replace(_T(">"),_T("&gt;"));
			content<<LANG(PARSERHELP_GENERIC_CLASS,"Clase generica: ")<<aux<<_T("<BR><BR>");
		}
		
		// averiguar la informacion de herencia
		wxArrayString inheritChildren;
		wxArrayString inheritClasses;
		wxArrayString inheritTexts;
		pd_inherit *item=parser->first_inherit;
		while (item->next) {
			item=item->next;
			if (item->son==aclass->name) {
				inheritClasses.Add(item->father);
				switch (item->properties) {
				case PD_CONST_PRIVATE:
					inheritTexts.Add(_T("private "));
					break;
				case PD_CONST_PROTECTED:
					inheritTexts.Add(_T("protected "));
					break;
				case PD_CONST_PUBLIC:
					inheritTexts.Add(_T("public "));
					break;
				default:
					inheritTexts.Add("");
					break;
				}
			} else if (item->father==aclass->name)
				inheritChildren.Add(item->son);
		}
		if (inheritClasses.GetCount()!=0) {
			int in_j = inheritClasses.GetCount()-1;;
			content<<LANG(PARSERHELP_THIS_ONE_INHERITS_FROM,"Hereda de:")<<" "<<inheritTexts[in_j]<<_T("<A href=\"quickhelp:")<<inheritClasses[in_j]<<_T("\">")<<inheritClasses[in_j]<<_T("</A>");
			in_j--;
			while (in_j>0) {
				content<<_T(", ")<<inheritTexts[in_j]<<_T("<A href=\"quickhelp:")<<inheritClasses[in_j]<<_T("\">")<<inheritClasses[in_j]<<_T("</A>");
				in_j--;
			}
			// buscar herencias recursivamente
			if (inheritClasses.GetCount()>1)
				content<<_T(" y ")<<inheritTexts[in_j]<<_T("<A href=\"quickhelp:")<<inheritClasses[in_j]<<_T("\">")<<inheritClasses[in_j]<<_T("</A>");
			content<<_T("<BR>");
			wxArrayString inheritStack;
			wxArrayString inheritMargins;
			for (unsigned int i=0;i<inheritClasses.GetCount();i++) {
				inheritMargins.Add(_T("&nbsp;&nbsp;&nbsp;&nbsp;"));
				inheritStack.Add(inheritClasses[i]);
			}
			while (inheritStack.GetCount()>0) {
				wxString inheritText;
				wxString margin = inheritMargins[inheritStack.GetCount()-1];
				wxString keyword = inheritStack[inheritStack.GetCount()-1];
				inheritMargins.RemoveAt(inheritStack.GetCount()-1);
				inheritStack.RemoveAt(inheritStack.GetCount()-1);
				item=parser->first_inherit;
				while (item->next) {
					item=item->next;
					if (item->son==keyword) {
						inheritStack.Add(item->father);
						inheritMargins.Add(margin+_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
						if (inheritText=="") {
							inheritText=wxString(_T("<A href=\"quickhelp:"))<<item->father<<_T("\">")<<item->father<<_T("</A>");
						} else {
							inheritText=wxString(_T("<A href=\"quickhelp:"))<<item->father<<_T("\">")<<item->father<<_T("</A>, ")+inheritText;
						}
					}
				}
				if (inheritText!="")
					content<<margin<<_T("<A href=\"quickhelp:")<<keyword<<_T("\">")<<keyword<<_T("</A> ")<<LANG(PARSERHELP_OTHER_ONE_INHERIT_FROM,"hereda de")<<" "<<inheritText<<_T("<BR>");
			}
			content<<_T("<BR>");
		}
		if (inheritChildren.GetCount()!=0) {
			if (inheritChildren.GetCount()==1) {
				content<<LANG(PARSERHELP_ONE_CLASS_INHERITS_FROM_PRE,"La clase")<<_T(" <A href=\"quickhelp:")<<inheritChildren[0]<<_T("\">")<<inheritChildren[0]<<_T("</A> ")<<LANG(PARSERHELP_ONE_CLASS_INHERITS_FROM_POST,"hereda de")<<" "<<aclass->name<<_T(".<BR><BR>");
			} else {
				unsigned int in_j=1;
				content<<LANG(PARSERHELP_MANY_CLASSES_INHERIT_FROM_PRE,"Las clases")<<_T(" <A href=\"quickhelp:")<<inheritChildren[0]<<_T("\">")<<inheritChildren[0]<<_T("</A>");
				while (in_j<inheritChildren.GetCount()-1) {
					content<<_T(", <A href=\"quickhelp:")<<inheritChildren[in_j]<<_T("\">")<<inheritChildren[in_j]<<_T("</A>");
					in_j++;
				}
				content<<_T(" y <A href=\"quickhelp:")<<inheritChildren[in_j]<<_T("\">")<<inheritChildren[in_j]<<_T("</A>");
				content<<" "<<LANG(PARSERHELP_MANY_CLASSES_INHERIT_FROM_POST,"heredan de")<<" "<<aclass->name<<_T(".<BR><BR>");
			}							
		}	
		
	} else {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_NAMESPACE,"Espacio de Nombre")<<_T(" <I>")<<aclass->name<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_NAMESPACE,"Espacio de Nombres")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<aclass->name<<_T("</A></I></B><BR><BR>");
	}
	// atributos
	wxString attribs;
	pd_var *avar = aclass->first_attrib->next;
	while (avar!=NULL) {
		if (avar->properties&(PD_CONST_ENUM_CONST||PD_CONST_ENUM)) { avar = avar->next; continue; }
		wxString one_attrib ="<LI>";
		if (!aclass->is_union) {
			if (avar->properties&PD_CONST_PUBLIC) one_attrib<<"public ";
			else if (avar->properties&PD_CONST_PRIVATE) one_attrib<<"private ";
			else if (avar->properties&PD_CONST_PROTECTED) one_attrib<<"protected ";
			if (avar->properties&PD_CONST_STATIC) one_attrib<<"static ";
			if (avar->properties&PD_CONST_VOLATILE) one_attrib<<"volatile ";
		}
		wxString proto = mxUT::ToHtml(avar->proto),link;
		if (help->IsHelpForType(avar->type,link))
			proto.Replace(avar->type,wxString("<A href=\"quickhelp:")<<link<<"\">"<<avar->type<<"</A>",true);
		one_attrib<<proto<<"</LI>";
		attribs=one_attrib+attribs;
		avar = avar->next;
	}
	if (attribs.Len())
		if (aclass->file)
			content<<LANG(PARSERHELP_OWN_ATTRBUTES,"Atributos propios:")<<_T("<BR><UL>")<<attribs<<_T("</UL><BR>");
		else
			content<<LANG(PARSERHELP_VARIABLES,"Variables:")<<_T("<BR><UL>")<<attribs<<_T("</UL><BR>");
	else
		if (aclass->file)
			content<<LANG(PARSERHELP_NO_OWN_ATTRIBUTE_FOUND,"No se declararon atributos propios.")<<_T("<BR><BR>");
	// metodos
	wxString methods;
	pd_func *afunc = aclass->first_method->next;
	while (afunc!=NULL) {
		wxString one_method="<LI>";
		if (afunc->properties&PD_CONST_PUBLIC) one_method<<"public ";
		else if (afunc->properties&PD_CONST_PRIVATE) one_method<<"private ";
		else if (afunc->properties&PD_CONST_PROTECTED) one_method<<"protected ";
		one_method<<"<A href=\"quickhelp:"<<afunc->name<<"\">";
		if (afunc->properties&PD_CONST_STATIC)	one_method<<"static ";
		if (afunc->properties&PD_CONST_VIRTUAL)	one_method<<"virtual ";
		one_method<<mxUT::ToHtml(afunc->proto);
		if (afunc->properties&PD_CONST_CONST) one_method<<" const";
		if (afunc->properties&PD_CONST_VIRTUAL_PURE) one_method<<" = 0";
		one_method<<"</A></LI>";
		afunc = afunc->next;
		methods=one_method+methods;
	}
	if (methods.Len())
		if (aclass->file)
			content<<LANG(PARSERHELP_OWN_METHODS,"Métodos propios:")<<_T("<BR><UL>")<<methods<<_T("</UL><BR>");
		else
			content<<LANG(PARSERHELP_FUNCTIONS,"Funciones:")<<_T("<BR><UL>")<<methods<<_T("</UL><BR>");
	else
		if (aclass->file)
			content<<LANG(PARSERHELP_NO_OWN_METHOD_FOUND,"No se declararon métodos propios.")<<_T("<BR><BR>");
	
	if (ReloadDoxyIndex()) {
		wxString dox, link = GetDoxyInfo(aclass, dox);
		if (link.Len())
			content<<_T("<BR><B><A href=\"doxygen:")<<link<<_T("\">")<<LANG(PARSERHELP_DOXY_DESC,"Informacion en comentarios tipo Doxygen:")<<_T("</A></B><BR>")<<dox<<_T("<BR>");
		else
			content<<_T("<BR>")<<LANG(PARSERHELP_NO_DOXYGEN,"No se encontro informacion de comentarios tipo Doxygen.")<<_T("<BR>");
	}
	
}

void HelpManager::HelpFor(pd_func *afunc, wxString &content, wxString &index) {
	int id=index_ref_counter++;
	if (afunc->space) {
		wxString proto = MakeClassLinks(mxUT::ToHtml(afunc->full_proto)), link;
		if (afunc->space->name==afunc->name) {
			index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_CONSTRUCTOR_FROM_CLASS_PRE,"Constructor")<<_T(" <I>")<<mxUT::ToHtml(afunc->proto)<<_T("</I> ")<<LANG(PARSERHELP_CONSTRUCTOR_FROM_CLASS_POST,"de la clase")<<_T(" <I>")<<afunc->space->name<<_T("</I></A></LI>");
			content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_CONSTRUCTOR_FROM_CLASS_PRE,"Constructor")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<mxUT::ToHtml(afunc->proto)<<_T("</A></I> ")<<LANG(PARSERHELP_CONSTRUCTOR_FROM_CLASS_POST,"de la clase")<<_T(" <I><A href=\"quickhelp:")<<afunc->space->name<<_T("\">")<<afunc->space->name<<_T("</A></I></B><BR><BR>");
		} else if (afunc->name[0]=='~') {
			index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_DESTRUCTOR_FROM_CLASS_PRE,"Destructor")<<_T(" <I>")<<mxUT::ToHtml(afunc->proto)<<_T("</I> ")<<LANG(PARSERHELP_DESTRUCTOR_FROM_CLASS_POST,"de la clase")<<_T(" <I>")<<afunc->space->name<<_T("</I></A></LI>");
			content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_DESTRUCTOR_FROM_CLASS_PRE,"Destructor")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<mxUT::ToHtml(afunc->proto)<<_T("</A></I> ")<<LANG(PARSERHELP_DESTRUCTOR_FROM_CLASS_POST,"de la clase")<<_T(" <I><A href=\"quickhelp:")<<afunc->space->name<<_T("\">")<<afunc->space->name<<_T("</A></I></B><BR><BR>");
		} else {
			index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_METHOD_FROM_CLASS_PRE,"Método")<<_T(" <I>")<<mxUT::ToHtml(afunc->proto)<<_T("</I> ")<<LANG(PARSERHELP_METHOD_FROM_CLASS_POST,"de la clase")<<_T(" <I>")<<afunc->space->name<<_T("</I></A></LI>");
			content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_METHOD_FROM_CLASS_PRE,"Método")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<mxUT::ToHtml(afunc->proto)<<_T("</A></I> ")<<LANG(PARSERHELP_METHOD_FROM_CLASS_POST,"de la clase")<<_T(" <I><A href=\"quickhelp:")<<afunc->space->name<<_T("\">")<<afunc->space->name<<_T("</A></I></B><BR><BR>");
		}
		if (afunc->properties&PD_CONST_PUBLIC)
			content<<LANG(PARSERHELP_VISIBILITY_PUBLIC,"Visibilidad: Publico")<<_T("<BR><BR>");
		else if (afunc->properties&PD_CONST_PRIVATE)
			content<<LANG(PARSERHELP_VISIBILITY_PRIVATE,"Visibilidad: Privado")<<_T("<BR><BR>");
		else if (afunc->properties&PD_CONST_PROTECTED)
			content<<LANG(PARSERHELP_VISIBILITY_PROTECTED,"Visibilidad: Protegido")<<_T("<BR><BR>");
		content<<LANG(PARSERHELP_PROTOTYPE,"Prototipo:")<<_T(" <BR><UL><LI>");
		if (afunc->properties&PD_CONST_STATIC) content<<_T("static ");
		if (afunc->properties&PD_CONST_VIRTUAL) content<<_T("virtual ");
		content<<proto<<(afunc->properties&PD_CONST_CONST?" const":"")<<(afunc->properties&PD_CONST_VIRTUAL_PURE?" = 0":"")<<_T("</LI></UL><BR><BR>");
		if (afunc->file_dec) AddDefRef(content,LANG(PARSERHELP_DECLARED_IN_PRE,"Declarado en"),afunc->file_dec->name,afunc->line_dec);
		if (afunc->file_def) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definido en"),afunc->file_def->name,afunc->line_def);
	} else {
		wxString proto = MakeClassLinks(mxUT::ToHtml(afunc->proto)), link;
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_FUNCTION,"Funcion")<<_T(" <I>")<<mxUT::ToHtml(afunc->proto)<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_FUNCTION,"Funcion")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<mxUT::ToHtml(afunc->proto)<<_T("</A></I></B><BR><BR>");
		content<<LANG(PARSERHELP_PROTOTYPE,"Prototipo:")<<_T(" <BR><UL><LI>")<<proto<<(afunc->properties&PD_CONST_CONST?" const":"")<<_T("</LI></UL><BR><BR>");
		if (afunc->file_dec) AddDefRef(content,LANG(PARSERHELP_DECLARED_IN_PRE,"Declarada en"),afunc->file_dec->name,afunc->line_dec);
		if (afunc->file_def) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),afunc->file_def->name,afunc->line_def);
	}
	
	if (ReloadDoxyIndex()) {
		wxString dox, link = GetDoxyInfo(afunc, dox);
		if (link.Len())
			content<<_T("<BR><B><A href=\"doxygen:")<<link<<_T("\">")<<LANG(PARSERHELP_DOXY_DESC,"Informacion en comentarios tipo Doxygen:")<<_T("</A></B><BR>")<<dox<<_T("<BR>");
		else
			content<<_T("<BR>")<<LANG(PARSERHELP_NO_DOXYGEN,"No se encontro informacion de comentarios tipo Doxygen.")<<_T("<BR>");
	}
	
}

void HelpManager::HelpForEnum(pd_var *avar, wxString &content, wxString &index) {
	int id=index_ref_counter++;
	if (avar->properties&PD_CONST_ENUM) {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_ENUM,"Tipo Enumerado")<<_T(" <I>")<<avar->proto<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_ENUM,"Tipo Enumerado")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I></B><BR><BR>");
		if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definido en"),avar->file->name,avar->line);
	} else if (avar->properties&PD_CONST_ENUM_CONST) {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_ENUM_CONST,"Constante de Tipo Enumerado")<<_T(" <I>")<<avar->proto<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_ENUM_CONST,"Constante de Tipo Enumerado")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I></B><BR><BR>");
		if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),avar->file->name,avar->line);
	}
}
	
/** Arma el texto de ayuda rápida para variables globales, atributos, y constantes de tipos enumerados **/
void HelpManager::HelpFor(pd_var *avar, wxString &content, wxString &index) {
	if (avar->properties&(PD_CONST_ENUM|PD_CONST_ENUM_CONST)) { HelpForEnum(avar,content,index); return; }
	int id=index_ref_counter++;
	if (avar->properties&PD_CONST_ENUM) {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_ENUM,"Tipo Enumerado")<<_T(" <I>")<<avar->proto<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_ENUM,"Tipo Enumerado")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I></B><BR><BR>");
		if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definido en"),avar->file->name,avar->line);
	} else if (avar->properties&PD_CONST_ENUM_CONST) {
		index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_ENUM_CONST,"Constante de Tipo Enumerado")<<_T(" <I>")<<avar->proto<<_T("</I></A></LI>");
		content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_ENUM_CONST,"Constante de Tipo Enumerado")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I></B><BR><BR>");
		if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),avar->file->name,avar->line);
	} else {
		if (avar->space) {
			index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_ATTRIBUTE_FROM_CLASS_PRE,"Atributo")<<_T(" <I>")<<avar->proto<<_T("</I> ")<<LANG(PARSERHELP_ATTRIBUTE_FROM_CLASS_POST,"de la clase")<<_T(" <I>")<<avar->space->name<<_T("</I></A></LI>");
			content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_ATTRIBUTE_FROM_CLASS_PRE,"Atributo")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I> ")<<LANG(PARSERHELP_ATTRIBUTE_FROM_CLASS_POST,"de la clase")<<_T(" <I><A href=\"quickhelp:")<<avar->space->name<<_T("\">")<<avar->space->name<<_T("</A></I></B><BR><BR>");
			if (avar->properties&PD_CONST_PUBLIC)
				content<<LANG(PARSERHELP_VISIBILITY_PUBLIC,"Visibilidad: Publico")<<_T("<BR><BR>");
			else if (avar->properties&PD_CONST_PRIVATE)
				content<<LANG(PARSERHELP_VISIBILITY_PRIVATE,"Visibilidad: Privado")<<_T("<BR><BR>");
			else if (avar->properties&PD_CONST_PROTECTED)
				content<<LANG(PARSERHELP_VISIBILITY_PROTECTED,"Visibilidad: Protegido")<<_T("<BR><BR>");
			content<<LANG(PARSERHELP_VISIBILITY_DECLARATION,"Declaracion:")<<_T(" <BR><UL><LI>");
			if (avar->properties&PD_CONST_STATIC) content<<_T("static ");
			if (avar->properties&PD_CONST_VOLATILE) content<<_T("volatile ");
			wxString proto = avar->proto,link;
			if (help->IsHelpForType(avar->type,link))
				proto.Replace(avar->type,wxString(_T("<A href=\"quickhelp:"))<<link<<_T("\">")<<avar->type<<_T("</A>"),true);
			content<<proto<<_T("</LI></UL><BR><BR>");
			if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definido en"),avar->file->name,avar->line);
		} else {
			index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<LANG(PARSERHELP_GLOBAL_VAR,"Variable Global")<<_T(" <I>")<<avar->proto<<_T("</I></A></LI>");
			content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<LANG(PARSERHELP_GLOBAL_VAR,"Variable Global")<<_T(" <I><A href=\"#")<<id<<_T("\">")<<avar->proto<<_T("</A></I></B><BR><BR>");
			wxString proto = avar->proto,link;
			if (avar->properties&PD_CONST_VOLATILE) 
				proto=wxString("volatile ")+proto;
			if (help->IsHelpForType(avar->type,link))
				proto.Replace(avar->type,wxString(_T("<A href=\"quickhelp:"))<<link<<_T("\">")<<avar->type<<_T("</A>"),true);
			content<<LANG(PARSERHELP_DECLARATION,"Declaracion:")<<_T(" <BR><UL><LI>")<<proto<<_T("</LI></UL><BR><BR>");
			if (avar->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),avar->file->name,avar->line);
		}
	}
}

void HelpManager::HelpFor(pd_macro *amacro, wxString &content, wxString &index) {
	if (amacro->props&(PD_CONST_ENUM_CONST|PD_CONST_ENUM)) {
		wxString s;
		pd_var aux(wxTreeItemId(),NULL,NULL,0,s,s,s,0,NULL);
		aux.file=amacro->file;
		aux.line=amacro->line;
		aux.properties=amacro->props;
		aux.proto=amacro->proto;
		HelpForEnum(&aux,content,index);
		return;
	}
	int id=index_ref_counter++;
	index<<_T("<LI><A href=\"#")<<id<<_T("\">")<<(amacro->props&PD_CONST_TYPEDEF?LANG(PARSERHELP_TYPEDEF,"Typedef"):LANG(PARSERHELP_MACRO,"Macro"))<<_T(" <I>")<<amacro->name<<_T("</I></A></LI>");
	content<<_T("<A name=\"")<<id<<_T("\"><HR></A><B>")<<(amacro->props&PD_CONST_TYPEDEF?LANG(PARSERHELP_TYPEDEF,"Typedef"):LANG(PARSERHELP_MACRO,"Macro"))<<_T(" <I><A href=\"#")<<id<<_T("\">")<<amacro->name<<_T("</A></I></B><BR><BR>");
	if (amacro->file) AddDefRef(content,LANG(PARSERHELP_DEFINED_IN_PRE,"Definida en"),amacro->file->name,amacro->line);
	
	// incluir el contenido
	if (amacro->props&PD_CONST_TYPEDEF) {
		content<<LANG(PARSERHELP_DEFINED_AS,"Definida como:")<<_T(" <BR>&nbsp;&nbsp;&nbsp;&nbsp;")<<
			mxUT::ToHtml(wxString("typedef ")<<amacro->cont<<" "<<amacro->name)<<"<BR>";
	} else {
		wxTextFile fil(amacro->file->name);
		if (!fil.Exists()) return;
		fil.Open();
		fil.GoToLine(amacro->line-2);
		wxString line=fil.GetNextLine();
		unsigned int i=0;
		while (i<line.Len() && (line[i]==' '||line[i]=='\t') )
			i++;
		if (line.Len()<i+7) return;
		if (line.Mid(i,7)==_T("#define")) { // si todavia hay un define en esa linea
			i+=8;
			while (i<line.Len() && (line[i]==' '||line[i]=='\t') )
				i++;
			if (line.Len()<i+amacro->name.Len()) return;
			if (line.Mid(i,amacro->name.Len())==amacro->name) { // si es el define que buscamos
				i+=amacro->name.Len();
				if (line.Len()>i && (line[i]==' ' || line[i]=='(' || line[i]=='\t') ) {
					content<<LANG(PARSERHELP_DEFINED_AS,"Definida como:")<<_T(" <BR><BR>&nbsp;&nbsp;&nbsp;&nbsp;")<<mxUT::ToHtml(line)<<_T("<BR>");
					while(line[line.Len()-1]=='\\' && !fil.Eof()) {
						content<<_T("&nbsp;&nbsp;&nbsp;&nbsp;")<<mxUT::ToHtml(line=fil.GetNextLine())<<_T("<BR>");
					}
				}
			}
		}
		fil.Close();
	}
}

int HelpManager::GetParserHelp(wxString keyword, wxString &content) {
	wxString index;
	wxString ret;
	wxString others;
	int cont=0;

	ReloadDoxyIndex();
	
	// buscar en las macros
	pd_macro *amacro = parser->last_macro->next;
	while (amacro!=NULL) {
		if (amacro->name==keyword) {
			HelpFor(amacro,ret,index);
			cont++;
		}
		amacro=amacro->next;
	}
	// buscar en las variable globales
	pd_var *avar = parser->last_global->next;
	while (avar!=NULL) {
		if (avar->name==keyword) {
			HelpFor(avar,ret,index);
			cont++;
		}
		avar=avar->next;
	}
	// buscar en las funciones
	pd_func *afunc = parser->last_function->next;
	while (afunc!=NULL) {
		if (afunc->name==keyword) {
			HelpFor(afunc,ret,index);
			cont++;
		}
		afunc=afunc->next;
	}

	// buscar en las clases
	pd_class *aclass = parser->last_class->next;
	while (aclass!=NULL) {
		// ver si es la clase
		if (aclass->name==keyword) {
			HelpFor(aclass,ret,index);
			cont++;
		}
		// buscar en los atributos
		afunc = aclass->first_method;
		while (afunc!=NULL) {
			if (afunc->name==keyword || (afunc->name[0]=='~' && aclass->name==keyword)) {
				HelpFor(afunc,ret,index);
				cont++;
			}
			afunc=afunc->next;
		}
		// buscar en los metodos
		avar = aclass->first_attrib;
		while (avar!=NULL) {
			if (avar->name==keyword) {
				HelpFor(avar,ret,index);
				cont++;
			}
			avar=avar->next;
		}
		aclass=aclass->next;
	}
	if (cont>1)
		content<<_T("<B>")<<LANG(PARSERHELP_SYMBOLS_FOUND,"Simbolos encontrados:")<<_T("<UL>")<<index<<_T("</UL><BR></B>");
	content<<ret<<_T("<HR><BR>");
	return cont;
}

int HelpManager::GetStandardHelp(wxString keyword, wxString &content) {
//	HashStringString::iterator it;
//	if ((it=quick_help_hash.find(keyword))!=quick_help_hash.end()) {
//		int count=0;
//		wxString files = it->second+"\n";
//		while (files.Len()) {
//			wxString fname = files.BeforeFirst('\n');
//			files = files.AfterFirst('\n');
//			if (wxFileName::FileExists(DIR_PLUS_FILE(config->Help.quickhelp_dir,fname))) {
//				wxTextFile fil(DIR_PLUS_FILE(config->Help.quickhelp_dir,fname));
//				fil.Open();	
//				wxString lin;
//				for (lin=fil.GetFirstLine();!fil.Eof();lin=fil.GetNextLine())
//					content+=lin;
//				content+=_T("<HR>");
//				fil.Close();
//				count++;
//			}
//		}
//		return count;
//	}
	return 0;
}

wxString HelpManager::GetQuickHelp(wxString keyword) {

	wxString ret("<HTML><HEAD><TITLE>ZinjaI Quick Help</TITLE></HEAD><BODY>");
	wxString content;
	int count=GetStandardHelp(keyword,content)+GetParserHelp(keyword,content);
	
	if (count)
		ret+=content;
	else {
		ret+="<BR>";
		ret+=LANG1(PARSERHELP_NO_RESULTS,"No se encontro ayuda rápida para la palabra seleccionada: <{1}>",keyword);
		ret+="<BR><HR><BR>";
	}
	
	ret+="<A href=\"cppreference:"+keyword+"\">";
	ret+=LANG1(PARSERHELP_GOTO_CPPREFENCE,"Buscar \"<{1}>\" en la referencia C/C++...",keyword);
	ret+="</A><BR>";
	
	ret+"</BODY></HTML>";
	return ret;
	
}

bool HelpManager::IsHelpForType(wxString what, wxString &link) {
	what = code_helper->UnMacro(what);
	while (what.Len() && (what.Last()=='*' || what.Last()==']' || what.Last()==' ')) {
		if (what.Last()==']') {
			while (what.Len() && what.Last()!='[')
				what.RemoveLast();
			if (what.Len())
				what.RemoveLast();
		} else
			what.RemoveLast();
	}
	link=what;
	pd_class *aclass = parser->last_class->next;
	while (aclass!=NULL) {
		if (aclass->name==what) 
			return true;
		aclass = aclass->next;
	}
//	return quick_help_hash.find(what)!=quick_help_hash.end();
	return false;
}
wxString HelpManager::MakeClassLinks(wxString what) {
	int len = what.Len();
	int p=0, p1=0;
	wxString link;
	while (p<len) {
		char c=what[p];
		while ( p<len && ! ( (c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_' ) ) {
			p++; c=what[p];
		}
		if (p<len) {
			p1=p;
			while ( ( p<len && (c>='A' && c<='Z') ) || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_' ) {
				p++;
				c=what[p];
			}
		}
		if (IsHelpForType(what.SubString(p1,p-1),link)) {
			what = what.SubString(0,p1-1)<<_T("<A href=\"quickhelp:")<<link<<_T("\">")<<what.SubString(p1,p-1)<<_T("</A>")<<what.Mid(p);
			int ll = len;
			len = what.Len();
			p += len-ll;
		}
			
	}
	return what;
//	
//	wxArrayString types;
//	mxUT::Split(afunc->proto,types);
//	types.Sort();
//	for (unsigned int i=0;i<types.GetCount();i++) {
//		if (types[i].Len()) {
//			if (types[i][0]=='(') 
//				types[i].erase(0,1);
//			if (types[i].Last()==')') 
//				types[i].RemoveLast();
//			if (i==0 || types[i]!=types[i-1]) {
//				if (IsHelpForType(types[i],link)) {
//					proto.Replace(types[i],wxString(_T("<A href=\"quickhelp:"))<<link<<_T("\">")<<types[i]<<_T("</A>"),true);
//				}
//			}
//		}
//	}
}

struct xml_doxy_item {
	wxString name,file,anchor/*,args*/;
	char kind;
	xml_doxy_item(char k):kind(k){}
};

bool HelpManager::ReloadDoxyIndex() {
	// ver si hay que usar el indice
	wxString file = doxygen_configuration::get_tag_index();
	if (!file.Len()) return false;
	// ver si existe el indice
	wxFileName fname ( file );
	if (!fname.FileExists()) return false;
	// releer si fue modificado
	if (fname.GetModificationTime()!=doxy_index_stamp) {
		bool on_class=false;
		doxy_index.clear();
		doxy_index_stamp=fname.GetModificationTime();
		wxTextFile fil(file);
		if (!fil.Exists()) return false;
		fil.Open();
		wxString str,tag;
		vector<xml_doxy_item> pila;
		pila.push_back(xml_doxy_item('*'));
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			// el nivel de anidamiento se lee por espacios aprovechando que doxygen es prolijo
			xml_doxy_item &top = pila[pila.size()-1];
			tag = str.BeforeFirst('>').AfterFirst('<');
			if (on_class && tag.StartsWith(_T("/compound"))) {
				xml_doxy_item di=pila[pila.size()-1];
				pila.pop_back();
				if (di.kind=='c') 
					doxy_index[wxString("+class+")<<di.name]=di.file;
			} else if (tag.StartsWith(_T("compound "))) {
				if (tag.Contains(_T("kind=\"class\"")) || tag.Contains(_T("kind=\"struct\"")))
					pila.push_back(xml_doxy_item('c'));
				else
					pila.push_back(xml_doxy_item('u'));
				on_class=true;
			} else if (tag.StartsWith(_T("member "))) {
				if (tag.Contains(_T("kind=\"function\"")))
					pila.push_back(xml_doxy_item('f'));
				else
					pila.push_back(xml_doxy_item('u'));
			} else if (tag.StartsWith(_T("filename"))) {
				top.file=str.AfterFirst('>').BeforeLast('<');
				if (!top.file.EndsWith(_T(".html")))
					top.file<<_T(".html");
//				cerr<<top.file<<endl;
			} else if (tag.StartsWith(_T("name"))) {
				top.name=str.AfterFirst('>').BeforeLast('<');
				if (top.name.Contains("::"))
					top.name=top.name.AfterLast(':');
			} else if (tag.StartsWith(_T("anchor"))) {
				top.anchor=str.AfterFirst('>').BeforeLast('<');
			} else if (tag.StartsWith(_T("/class"))) {
				pila.pop_back();
			} else if (tag.StartsWith(_T("/member"))) {
				int i=pila.size()-1;
				while (i && !pila[i].file.Len()) i--;
				if (i && top.anchor.Len() && top.name.Len() && top.kind=='f') {
					wxString link=pila[i].file+_T("#")+top.anchor;
					wxString func=top.name;/*+ParseDoxyArgs(top.args);*/
					pila.pop_back();
					xml_doxy_item &ntop = pila[pila.size()-1];
					if (ntop.kind=='c')
						func = ntop.name+_T("::")+func;
					HashStringString::iterator it = doxy_index.find(func);
					while (it!=doxy_index.end()) {
						func<<"*";
						it = doxy_index.find(func);
					}
					doxy_index[func]=link;
				} else 
					pila.pop_back();
			}
		}
		fil.Close();
	}
	return true;
}


wxString HelpManager::GetDoxyInfo(pd_class *aclass, wxString &desc) {
	desc.Clear();
	HashStringString::iterator it;
	wxString key = aclass->name;
	it = doxy_index.find(wxString("+class+")<<key);
	if (it!=doxy_index.end()) {
		wxTextFile fil(DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,DIR_PLUS_FILE("html",it->second))));
		if (!fil.Exists()) return "";
		fil.Open();
		wxString str;
		bool on_desc=false;
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			if (on_desc) {
				if (str.Contains("<h2")) break; 
				///  @todo: ver que hacer con el utf8 para que muestre acentos
				if (str.Len()&&str.FromUTF8(str.char_str()).Len()) // trae problemas en algunos linuxs 
					str = str.FromUTF8(str.char_str());
				str.Replace(_T("</a>"),_T("</aZZZ>"));
				str.Replace(_T("<a "),_T("<aZZZ "));
				desc+=str+"\n";
//				cerr<<"DESC: "<<str<<endl;
			} else if ( str.Contains("name=\"_details\"")||str.Contains("name=\"details\"")) on_desc=true;
		}
		fil.Close();
		return DIR_PLUS_FILE(DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,_T("html"))),it->second);
	}
	return "";	
}

wxString HelpManager::GetDoxyInfo(pd_func *afunc, wxString &desc) {
	desc.Clear();
	HashStringString::iterator it;
	wxString key = ( afunc->space ? afunc->space->name+_T("::")+afunc->name : afunc->name ); 
	it = doxy_index.find(key);
	wxString args = afunc->proto.AfterFirst('(');
	args.RemoveLast();
	args.Replace(_T(" ["),_T("["));
	while (it!=doxy_index.end()) {
		wxString auxp = ParseDoxyText(it->second,desc);
		if (auxp==args)
			return DIR_PLUS_FILE(DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,_T("html"))),it->second);
		key<<"*";
		it = doxy_index.find(key);
	}
	return "";	
}

wxString HelpManager::ParseDoxyText(wxString link, wxString &desc) {
	desc.Clear();
//	cerr<<"FILE:  "<<link<<endl;
	wxString file = DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,_T("html")),link.BeforeLast('#')));
	if (!file.Len()) return "";
	if (!wxFileName(file).FileExists()) return "";
	wxString anchor=link.AfterLast('#');
	wxString anchor_label=_T("class=\"anchor\"");
	wxString anchor_key=wxString(_T("name=\""))<<anchor<<"\"";
	wxString anchor_key2=wxString(_T("id=\""))<<anchor<<"\"";
	wxTextFile fil(file);
	int divcounter=0;
	fil.Open();
	wxString str, proto;
	bool on_anchor=false, on_proto=false, on_desc=false;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (str.Contains(anchor_label)) {
			if (on_anchor) {
				// ya no deberia pasar por aca, el final de una funcion se determina con su div mas abajo, donde concatena la descripcion
				fil.Close();
				if (proto.Len()) proto.RemoveLast();
				return proto;
			} else {
				if (str.Contains(anchor_key) || str.Contains(anchor_key2)) 
				on_anchor=true;
			}
		} else if (on_anchor) {
			if (str.Contains(_T("class=\"memproto\""))) {
				on_proto=true;
			} else if (str.Contains(_T("class=\"memdoc\""))) {
				on_proto=false; 
				on_desc=true;
			} else {
				if (on_proto) {
					int i=str.Find(_T("class=\"paramtype\""));
					if (i!=wxNOT_FOUND) {
						str = str.Mid(i).AfterFirst('>');
						wxString str2 = fil.GetNextLine();
						if (str2.Find('[')!=wxNOT_FOUND)
							str<<_T("[")<<str2.AfterFirst('[');
						wxString arg(str);
						int ir=0, i=0, par=0, l=str.Len();
						bool comillas=false;
						while (i<l && str[i]!=',') {
							char c=str[i];
							if (comillas) {
								if (c=='\"') comillas=false;
							} else {
								if (c=='\"') comillas=true;
								else if (c=='<') par++;
								else if (c=='>') par--;
								else if (par==0) arg[ir++]=c;
							}
							i++;
						}
						arg = arg.Mid(0,ir);
						arg.Replace(_T("&#160;"),"");
						arg.Replace(_T("&;"),"");
						arg.Replace(_T("&nbsp;"),"");
						arg.Replace(_T("&gt;"),_T(">"));
						arg.Replace(_T("&lt;"),_T("<"));
						arg.Replace(_T("&amp;"),_T("&"));
						arg.Replace(_T("  ")," ");
						arg.Replace(_T(" <"),_T("<"));
						arg.Replace(_T("< "),_T("<"));
						arg.Replace(_T("> "),_T(">"));
						arg.Replace(_T(" >"),_T(">"));
						if (arg.EndsWith(_T(">&"))) {
							arg.Last()=' ';
							arg<<"&";
						}
						proto+=arg+",";
//						cerr<<"**ARGS: "<<arg<<endl;
					}
				} else if (on_desc) {
					if (str.Contains("<div")) divcounter++;
					else if (str.Contains("</div")) divcounter--;
					if (divcounter<0) {
						fil.Close();
						if (proto.Len()) proto.RemoveLast();
						return proto;
					}
					///  @todo: ver que hacer con el utf8 para que muestre acentos
					if (str.Len()&&str.FromUTF8(str.char_str()).Len()) // trae problemas en algunos linuxs 
						str = str.FromUTF8(str.char_str());
					str.Replace(_T("</a>"),_T("</aZZZ>"));
					str.Replace(_T("<a "),_T("<aZZZ "));
					desc+=str+"\n";
//					cerr<<"DESC: "<<str<<endl;
				}
			}
		}
	}
	fil.Close();
	return "";
}

void HelpManager::Initialize ( ) {
	help = new HelpManager();
}

