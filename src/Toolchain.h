#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H
#include <wx/string.h>
#include <wx/arrstr.h>

#define TOOLCHAIN_MAX_ARGS 5

struct Toolchain {
	
	// common options
	wxString file; ///< toolchain filename
	wxString desc; ///< toolchain name
	
	// options for extern toolchains
	bool is_extern; ///< true for use with tools such as cmake, make, scons; false for zinjai managed compiling process
	wxString build_command; ///< command to run for building the project
	wxString clean_command; ///< command to run for cleaning the project
	wxString arguments[TOOLCHAIN_MAX_ARGS][2]; ///< arguments for extern toolchains ([x][0]=name, [x][1]=value)
	
	// options for gcc-like toolchains
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
	
	void SetProjectArgumets(); ///< applies project toolchain arguments, uso only with current_toolchain
	
	static void LoadToolchains();
	static const Toolchain &GetInfo(wxString fname); ///< used to get info from a toolchain without setting it as active
	static Toolchain &SelectToolchain(); ///< set the project toolchain as current (use default if not project or not found)
	static Toolchain *toolchains; ///< array with all known toolchains
	static int toolchains_count; ///< number of items in toolchains
	static void GetNames(wxArrayString &names, bool exclude_extern=false); ///< populate the array with known toolchains' names
};

extern Toolchain current_toolchain; ///< toolchain in use, setted by Toolchain::SelectToolchain

#endif

