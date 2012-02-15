all:
	@echo " "
	@echo use:
	@echo "   make linux           for compiling everything in GNU/Linux"
	@echo "   make linux           for compiling everything in Mac OS"
	@echo "   mingw32-make win32   for compiling everything in Microsoft Windows"
	@echo in case of error, or missing some dependency, read \"compiling.txt\".
	@echo " "

linux:
	make -f Makefile.lnx -C src
	make -f Makefile.lnx -C parser
	make -f Makefile.lnx -C src_extras

linux-clean:
	make -f Makefile.lnx -C src clean
	make -f Makefile.lnx -C parser clean
	make -f Makefile.lnx -C src_extras clean

win32:
	mingw32-make -f Makefile.w32 -C src
	mingw32-make -f Makefile.w32 -C parser
	mingw32-make -f Makefile.w32 -C src_extras

win32-clean:
	mingw32-make -f Makefile.w32 -C src clean
	mingw32-make -f Makefile.w32 -C parser clean
	mingw32-make -f Makefile.w32 -C src_extras clean

mac:
	make -f Makefile.mac -C src
	make -f Makefile.mac -C parser
	make -f Makefile.mac -C src_extras

mac-clean:
	make -f Makefile.mac -C src clean
	make -f Makefile.mac -C parser clean
	make -f Makefile.mac -C src_extras clean

wine:
	make -f Makefile.wine -C src
	make -f Makefile.wine -C parser
	make -f Makefile.wine -C src_extras

wine-clean:
	make -f Makefile.wine -C src clean
	make -f Makefile.wine -C parser clean
	make -f Makefile.wine -C src_extras clean
