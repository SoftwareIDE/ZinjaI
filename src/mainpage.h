
// Para generar esta documentación: Herramientas->Generar Documentación->Generar
// Hay que tener Doxygen instalado (www.doxygen.org) y configurado (pestaña 
// "Rutas 2" del diálogo de Preferencias).

/**
* 
* @mainpage ZinjaI - Documentación del Código Fuente
* 
* @section Introduccion Introducción
* 
* Esta es la documentación del código fuente del entorno. Actualmente se encuentra
* incompleta, por lo que puede que muchos elementos no presenten su descripción.
* 
* Para ver la versión de ZinjaI que generó estos documentos consulte #VERSION.
*
* Para obtener actualizaciones visite http://zinjai.sourceforge.net
*
* Para mayor información zaskar_@yahoo.com.ar
* 
* Copyleft 2007-2016, por Pablo Novara
* 
* @section code-styling Coding Style
* 
* This code-styling guide was included after almost 8 years of development without 
* one, so most of the code does not repect this conventions. It's included now
* here to be used in new code, when refactorying and doing other modifications.
* I hope to see most of ZinjaI's code adjusted to this guide in the long run.
* 
* @subsection naming Naming
*  - SourceFileName.cpp
*  - HeaderFileName.hpp
*  - mxGuiClassName
*  - RegularClassName
*  - StructName
*  - RegularMemberFunctionName
*  - StaticMemberFunctionName
*  - globalFunctionName
*  - g_global_variable
*  - struct_member_var
*  - m_class_member_var
*  - const_static_member_var
*  - local_var_or_function_argument
*  - _preprocessor_function
*  - PREPROCESSOR_CONSTANT
*  - t_typedef
*  - EnumClassType::EnumValue
* 
* @subsection braces Braces
* @code
*   - FooClass::FooClass(...) : m_v(...) {
*         ...
* 	}
* @endcode
*  or
* @code
*   - FooClass::FooClass(...)
* 	    : m_v1(...),m_v2(...),...
* 		  m_v9(...),m_vN(...)
* 	{
* 	    ...
* 	}
* @endcode
* 
* @subsection guidelines Miscelaneous guidelines
* 
*   - Use struct for stuff without behaviour/invariants (registers) 
*     and class for everything else.
* 
*   - Do not use "using namespace bla".... 
*     use std::string, std::vector, std::whatever instead.
* 
*   - Try to avoid naked pointers.
* 
*   - Write relatively short functions, use single-responasibility-principle and 
*     single-level-of-abstraction-principle.
* 
*   - Use representative names (avoid not obvious abreviations).
*   
*   - Indentation: 1 tab = 4 spaces
*
*
* @page ChangeLog Registro de cambios
* @htmlinclude ChangeLog.html
*
**/
