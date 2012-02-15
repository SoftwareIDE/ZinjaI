
draw (needed for generation of project graphs and class hierarchy graphs in GNU/Linux)
  wrapper for running GraphViz binaries with its special LD_LIBRARY_PATH

viewer (recommended for viewing project graphs and class hierarchy graphs in all platforms)
  CImg based image viewer, for displaying graphviz results

viewerw (recommended for viewing project graphs and class hierarchy graphs in Windows)
  wrapper for calling viewer in Windows

mac-terminal.sh
  script for openning a new terminal window and running some command (will be runner) in MacOs

runner (needed for executing any project from ZinjaI on all platforms)
  this is the program who actually runs your program when you hit "run" in ZinjaI. It sets working directory and shows exit code and waits for the key after execution.

shellexecute (needed for opening web-pages and some other files in Windows)
  executable wrapper for winapi funcion shellexecute, for openning files with their associated programs

lsdeps (needed for displaying dependencies in executable's properties in Windows)
  Windows tool similar to ldd in linux, to list executable dependencies (itlib and peliberr are part of this program, see lsdeps.txt for details)

updatem (needed for searching for updates in all platforms)
  tool that connects to ZinjaI's website and compare last available version from site with current version from installed ZinjaI's executable (when called from ZinjaI)


zinjai-packer.sh
  script for building ZinjaI tgz for GNU/Linux from my Ubuntu virtual machine

make_dmg
  bash script for copying files and generating mac dmg packages from a GNU/Linux system with cross-compiling

make_wine
  bash script for building binaries with wine, copying files from a GNU/Linux ZinjaI installation to a set of folders that zinjai.nsh will use and building the final win32 installer (you need to add mingw, wx and openl manually)

zinjai.nsh
  nsis-installer script for building Windows installer (you need to arrange files in folders zinjai, fuentes, mingw, opengl and wx to run it, use make_wine for the first two)


autocomp_wx
  bash scripts and cpp files used to generate wxWidgets autocompletion index from its html reference (call do.sh with html's directory as the only argument and will generate the index file wxWidgets)
