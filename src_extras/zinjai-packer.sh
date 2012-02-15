
if [ "$1" = "prepare" ]; then

  mkdir zinjai
  mkdir zinjai/templates
  mkdir zinjai/autocomp
  mkdir zinjai/colours
  mkdir zinjai/guihelp
  mkdir zinjai/quickhelp
  mkdir zinjai/graphviz
  mkdir zinjai/graphviz/bin
  mkdir zinjai/graphviz/lib
  mkdir zinjai/graphviz/gprof2dot
  mkdir zinjai/parser
  mkdir zinjai/parser/common
  mkdir zinjai/parser/cpp
  mkdir zinjai/parser/cpp/cpplib
  mkdir zinjai/parser/hyper
  mkdir zinjai/parser/misc
  mkdir zinjai/lang
  mkdir zinjai/lang/tools
  mkdir zinjai/lang/tools/mxLangTool
  mkdir zinjai/imgs
  mkdir zinjai/imgs/16
  mkdir zinjai/imgs/24
  mkdir zinjai/imgs/32
  mkdir zinjai/skins
  mkdir zinjai/src
  mkdir zinjai/src_extras/wx_autocomp
  mkdir zinjai/src_extras
  mkdir zinjai/lsdeps

  echo "remember to copy manually graphviz binaries and deps"

elif [ "$1" = "update" ]; then

  scp $2/zinjai/readme.txt				zinjai/
  scp $2/zinjai/compiling.txt				zinjai/
  scp $2/zinjai/zinjai.dir				zinjai/
  scp $2/zinjai/Makefile				zinjai/

  scp $2/zinjai/parser/*				zinjai/parser/
  scp $2/zinjai/parser/common/*				zinjai/parser/common/
  scp $2/zinjai/parser/cpp/*				zinjai/parser/cpp/
  scp $2/zinjai/parser/cpp/cpplib/*			zinjai/parser/cpp/cpplib/
  scp $2/zinjai/parser/hyper/*				zinjai/parser/hyper/
  scp $2/zinjai/parser/misc/*				zinjai/parser/misc/

  scp $2/zinjai/zinjai.zpr				zinjai/
  scp $2/zinjai/src_extras/*				zinjai/src_extras/
  scp $2/zinjai/src_extras/wx_autocomp/*		zinjai/src_extras/wx_autocomp
  scp $2/zinjai/src/*					zinjai/src/

  scp $2/zinjai/imgs/art.svg				zinjai/imgs/
  scp $2/zinjai/imgs/*.png				zinjai/imgs/
  scp $2/zinjai/imgs/*.html				zinjai/imgs/
  scp $2/zinjai/imgs/*.xpm				zinjai/imgs/
  scp $2/zinjai/imgs/*.ico				zinjai/imgs/
  scp $2/zinjai/imgs/*.txt				zinjai/imgs/
  scp $2/zinjai/imgs/16/*				zinjai/imgs/16/
  scp $2/zinjai/imgs/24/*				zinjai/imgs/24/
  scp $2/zinjai/imgs/32/*				zinjai/imgs/32/
  scp -r $2/zinjai/skins/*				zinjai/skins/

  scp $2/zinjai/debug_macros.gdb			zinjai/
  scp $2/zinjai/quickhelp/*				zinjai/quickhelp/
  scp $2/zinjai/colours/*				zinjai/colours/
  scp $2/zinjai/guihelp/*				zinjai/guihelp/
  scp $2/zinjai/autocomp/*				zinjai/autocomp/
  scp -r $2/zinjai/templates/*				zinjai/templates/

  scp $2/zinjai/graphviz/*.cpp				zinjai/graphviz/
  scp $2/zinjai/graphviz/readme.txt			zinjai/graphviz/
  scp $2/zinjai/graphviz/Makefile			zinjai/graphviz/
  scp $2/zinjai/graphviz/gprof2dot/*			zinjai/graphviz/gprof2dot
  
  scp $2/zinjai/lsdeps/*				zinjai/lsdeps/

  scp $2/zinjai/lang/*.txt				zinjai/lang/
  scp $2/zinjai/lang/*.pre				zinjai/lang/
  scp $2/zinjai/lang/*.src				zinjai/lang/
  scp $2/zinjai/lang/*.sgn				zinjai/lang/
  scp $2/zinjai/lang/tools/*.cpp  			zinjai/lang/tools/
  scp $2/zinjai/lang/tools/mxLangTool.bin	  	zinjai/lang/tools/mxLangTool/
  scp $2/zinjai/lang/tools/mxLangTool/Makefile.*	zinjai/lang/tools/mxLangTool/
  scp $2/zinjai/lang/tools/mxLangTool/*.cpp		zinjai/lang/tools/mxLangTool/
  scp $2/zinjai/lang/tools/mxLangTool/*.h		zinjai/lang/tools/mxLangTool/
  scp $2/zinjai/lang/tools/mxLangTool/*.zpr		zinjai/lang/tools/mxLangTool/

elif [ "$1" = "compile" ]; then

  make -C zinjai/src -f Makefile.lnx || exit 1
  make -C zinjai/src_extras -f Makefile.lnx || exit 1
  make -C zinjai/parser -f Makefile.lnx || exit 1

elif [ "$1" = "pack" ]; then

  make -C zinjai/src -f Makefile.lnx || exit 1
  make -C zinjai/src_extras -f Makefile.lnx || exit 1
  make -C zinjai/parser -f Makefile.lnx || exit 1
  
  rm -f zinjai/release.lnx/*
  rmdir -f zinjai/release.lnx
  rm -f zinjai/parser/release.lnx/*
  rmdir -f zinjai/parser/release.lnx
  rm -f zinjai/lang/tools/mxLangTool/release.lnx/*
  rmdir -f zinjai/lang/tools/mxLangTool/release.lnx

  VER=`cat zinjai/src/version.h | head -n 1 | cut -d ' ' -f 3`
  if uname -a | grep x86_64; then ARCH=l64; else ARCH=l32; fi
  if ! tar -czvf zinjai-${ARCH}-${VER}.tgz zinjai; then exit; fi
  echo -n "Done: "
  ls -sh zinjai-${ARCH}-${VER}.tgz

else

  echo Use: 
  echo "    $1 prepare              create directory tree"
  echo "    $1 update \<source\>    update src from \<source\> with scp"
  echo "    $1 compile              build binaries"
  echo "    $1 pack                 rebuild, clean, and make tar file"

fi
