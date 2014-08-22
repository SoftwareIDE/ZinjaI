all:
	@echo " "
	@echo use:
	@echo "   make linux           for compiling everything in GNU/Linux or Mac OSX"
	@echo "   mingw32-make win32   for compiling everything in Microsoft Windows"
	@echo in case of error, or missing some dependency, read \"compiling.txt\".
	@echo " "

linux:
	$(MAKE) -f Makefile.lnx -C src
	$(MAKE) -f Makefile.lnx -C parser
	$(MAKE) -f Makefile.lnx -C src_extras

linux-clean:
	$(MAKE) -f Makefile.lnx -C src clean
	$(MAKE) -f Makefile.lnx -C parser clean
	$(MAKE) -f Makefile.lnx -C src_extras clean

win32:
	$(MAKE) -f Makefile.w32 -C src
	$(MAKE) -f Makefile.w32 -C parser
	$(MAKE) -f Makefile.w32 -C src_extras

win32-clean:
	$(MAKE) -f Makefile.w32 -C src clean
	$(MAKE) -f Makefile.w32 -C parser clean
	$(MAKE) -f Makefile.w32 -C src_extras clean

mac:
	$(MAKE) -f Makefile.mac -C src
	$(MAKE) -f Makefile.mac -C parser
	$(MAKE) -f Makefile.mac -C src_extras

mac-clean:
	$(MAKE) -f Makefile.mac -C src clean
	$(MAKE) -f Makefile.mac -C parser clean
	$(MAKE) -f Makefile.mac -C src_extras clean

wine:
	$(MAKE) -f Makefile.wine -C src
	$(MAKE) -f Makefile.wine -C parser
	$(MAKE) -f Makefile.wine -C src_extras

wine-clean:
	$(MAKE) -f Makefile.wine -C src clean
	$(MAKE) -f Makefile.wine -C parser clean
	$(MAKE) -f Makefile.wine -C src_extras clean
