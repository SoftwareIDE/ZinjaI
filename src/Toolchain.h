#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H
#include <wx/string.h>
#include <wx/arrstr.h>

#define TOOLCHAIN_MAX_ARGS 5

/// type of toolchain
enum TC_TYPE { 
	TC_GCC, ///< real gcc, zinjai will simplify output and append some arguments for controling error messages format
	TC_CLANG, ///< real clang, zinjai will for controling error messages format
	TC_GCC_LIKE, ///< some that acts like gcc, but its not, zinjai wont add argument and wont try to simplify errors
	TC_EXTERN, ///< the whole building process is managed by some extern tool, such as make, cmake, scons, or some script
	TC_COUNT ///< should not happend
};

struct Toolchain {
	
	long version_c; /// compiler version (if gcc 4.8.x => version_c=4008)
	long version_cpp; /// compiler version (if g++ 4.8.x => version_cpp=4008)
	
	// common options
	wxString file; ///< toolchain filename
//	wxString desc; ///< toolchain name
	
	// options for extern toolchains
	wxString build_command; ///< command to run for building the project
	wxString clean_command; ///< command to run for cleaning the project
	wxString arguments[TOOLCHAIN_MAX_ARGS][2]; ///< arguments for extern toolchains ([x][0]=name, [x][1]=value)
	
	// options for gcc-like toolchains
	TC_TYPE type; ///< apply ad-hoc arguments and output parsing (control diagnostics format and apply error simplifications)
	wxString c_compiler; ///< c compiler command
	wxString cpp_compiler; ///< c++ compiler command
	wxString linker; ///< linker command
	wxString c_compiling_options; ///< forced compiler arguments for compiling c files
	wxString cpp_compiling_options; ///< forced compiler arguments for compiling c++ files
	wxString c_linker_options; ///< forced linker arguments
	wxString cpp_linker_options; ///< forced linker arguments
	wxString dynamic_lib_linker; ///< command for linking dynamic libraries
	wxString static_lib_linker; ///< command for linking static libraries
	wxString base_path; ///< path for compiler folder, will be used to replace ${MINGW_DIR} and ${COMPILER_DIR} in project's settings
	wxString bin_path; ///< path for binaries, to be added to env PATH variable to be able to launch the compiler from anywhere
	
	wxString mingw_dir; ///< calculated field, to replace ${MINGW_DIR}, based on base_path, but made absolute 
	
	Toolchain(); ///< loads default values for current platform
	
	void SetPaths(); ///< configure PATH variable for finding current compiler (this method is called automatically from SelectToolchain)
	void SetArgumets(); ///< applies project toolchain arguments, uso only with current_toolchain (this method is called automatically from SelectToolchain)
	
	wxString GetExtraCompilingArguments(bool cpp); ///< get extra arguments to deal with difference in output formatting in recent gcc versions (clang-like messages)
	wxString FixArgument(bool cpp, wxString arg); ///< if compiler is too old, some arguments may be different
	bool CheckVersion(bool cpp, int _v, int _s); ///< returns true if compiler version is greater than or equal to _v._s
	
	void Save(const wxString &fname); ///< saves a single toolchain
	static void LoadToolchains(); ///< loads all available toolchains
	static const Toolchain &GetInfo(const wxString &fname); ///< used to get info from a toolchain without setting it as active
	static Toolchain &SelectToolchain(); ///< set the project toolchain as current (use default if not project or not found)
	static Toolchain *toolchains; ///< array with all known toolchains
	static int toolchains_count; ///< number of items in toolchains
	static void GetNames(wxArrayString &names, bool exclude_extern=false); ///< populate the array with known toolchains' names
	static Toolchain *GetToolchain(const wxString &name);
	
	bool IsExtern() const { return type>=TC_EXTERN; }
};

extern Toolchain current_toolchain; ///< toolchain in use, setted by Toolchain::SelectToolchain

#endif

