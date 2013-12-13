#ifndef ENUMS_H
#define ENUMS_H

/// Determina un tipo de archivo, se utiliza para saber en qué categoría colocarlo dentro del proyecto
enum eFileType { 
	FT_NULL, 		///< undetermined file type
		FT_SOURCE, 	///< source (c,cpp,cxx,c++,etc)
		FT_HEADER, 	///< header (h,hpp,hxx,h++,etc)
		FT_OTHER, 		///< other, neither source nor header
		FT_PROJECT     ///< project file (zpr), not valid for project_file_item->where
};

#endif
