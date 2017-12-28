ACLOCAL_AMFLAGS = -I m4
ZZZ = 0
SUBDIRS = src

EXTRA_DIST = etc/Doxyfile etc/meataxe-footer.html

@DX_RULES@

.PHONY: doc

doc: $(docdir) doxygen-doc

$(docdir):
	@echo "Creating" $(docdir)
	@$(MKDIR_P) "$(docdir)"

uninstall-hook:
	@$(RM) -r $(docdir)
