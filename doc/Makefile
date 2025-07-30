-include ../config.mk

# Makefile to build the libdulge API documentation.
#
DOXYF	?=	dulge_api_doxyfile
FORMAT	?=	png
FILES	=	dulge_transaction_dictionary.$(FORMAT)
FILES	+=	dulge_pkgdb_dictionary.$(FORMAT)
FILES	+=	dulge_pkg_props_dictionary.$(FORMAT)
FILES	+=	dulge_pkg_files_dictionary.$(FORMAT)
FILES	+=	dulge_binary_pkg_content.$(FORMAT)

.PHONY: all
all: $(FILES) doxyfiletrans doxygendocs

$(FILES): %.$(FORMAT): %.dot
	dot -T$(FORMAT) $< -o images/$@

doxyfiletrans: $(DOXYF).in
ifdef DISTVER
	sed -e "s|@@PROJECT_NUMBER@@|${DISTVER}|" $< > $(DOXYF)
else
	sed -e "s|@@PROJECT_NUMBER@@|$$(date +%Y%m%d)|" $< > $(DOXYF)
endif

doxygendocs: $(FILES) doxyfiletrans
	doxygen $(DOXYF)

.PHONY: clean
clean:
	-rm -f images/*.$(FORMAT)
	-rm -rf ../api
	-rm -f $(DOXYF)

.PHONY: install
install:
	install -d $(DESTDIR)$(SHAREDIR)/doc/dulge
	cp -r ../api $(DESTDIR)$(SHAREDIR)/doc/dulge
