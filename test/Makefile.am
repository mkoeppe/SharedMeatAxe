AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/test
LDADD       = ../src/libmtx.la

# -----> Executables (for tests)

check_HEADERS         = c-args.h c-charpol.h c-ffmat.h c-fileio.h \
	c-grease.h c-kernel.h c-matrix.h c-os.h c-poly.h c-quot.h \
	c-sets.h c-tensor.h c-bitstring.h c-ffio.h c-ffrow.h c-fpoly.h \
	check.h c-matins.h c-matset.h c-perm.h c-pseed.h c-random.h c-stf.h

check_PROGRAMS        = checksum zzztest

zzztest_SOURCES       = zzztest.c c-args.c c-charpol.c c-ffmat.c c-fileio.c \
	c-grease.c c-kernel.c c-matrix.c c-os.c c-poly.c c-quot.c \
	c-sets.c c-tensor.c c-bitstring.c c-ffio.c c-ffrow.c c-fpoly.c \
	c-matins.c c-matset.c c-perm.c c-pseed.c c-random.c c-stf.c

# -----> The test suite

TESTNUMBERS           = t-0100 t-0000 t-0100 t-0100 t-0105 t-0106 t-0107 \
	t-0108 t-0109 t-0110 t-0111 t-0112 t-0113 t-0114 t-0115 t-0116 t-0117 \
	t-0118 t-0200 t-0201 t-0210 t-0211 t-0212 t-0213 t-0214 t-0215

TESTS                 = zzztest $(TESTNUMBERS)

check-local:
	echo $(TESTS)
	@$(MKDIR_P) $(TESTNUMBERS:=_out)

clean-local:
	rm -rf p*.zzz
	rm -rf $(TESTNUMBERS:=_out)
	rm -rf check.*
