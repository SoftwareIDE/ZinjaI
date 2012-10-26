#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H
#include <wx/string.h>
#include <wx/arrstr.h>

struct Toolchain {
	wxString name; ///< toolchain_name
	bool is_extern; ///< true for use with tools such as cmake, make, scons; false for zinjai managed compiling process
	wxString c_compiler; ///< c compiler command
	wxString cpp_compiler; ///< c++ compiler command
	wxString linker; ///< linker command
	wxString c_compiling_options; ///< forced compiler arguments for compiling c files
	wxString cpp_compiling_options; ///< forced compiler arguments for compiling c++ files
	wxString c_linker_options; ///< forced linker arguments
	wxString cpp_linker_options; ///< forced linker arguments
	wxString dynamic_lib_linker; ///< command for linking dynamic libraries
	wxString static_lib_linker; ///< command for linking static libraries
	Toolchain(); ///< loads default values for current platform
	static void LoadToolchains();
	static bool SelectToolchain();
	static Toolchain *toolchains; ///< array with all known toolchains
	static int toolchains_count; ///< number of items in toolchains
	static void GetNames(wxArrayString &names, bool exclude_extern=false); ///< populate the array with known toolchains' names
};

extern Toolchain current_toolchain; ///< toolchain in use, setted by Toolchain::SelectToolchain

#endif

