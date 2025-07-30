-include ../config.mk

PCFILE = libdulge.pc

.PHONY: all
all:
	sed -e "s|@@VERSION@@|${VERSION}|g" \
		-e "s|@@PREFIX@@|${PREFIX}|g" \
		-e "s|@@EPREFIX@@|${EPREFIX}|g" \
		${PCFILE}.in > ${PCFILE}

.PHONY: install
install:
	install -d $(DESTDIR)$(MANDIR)/man5
	install -m644 dulge.d.5 $(DESTDIR)$(MANDIR)/man5
	install -d $(DESTDIR)$(MANDIR)/man7
	install -m644 dulge.7 $(DESTDIR)$(MANDIR)/man7
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m644 $(PCFILE) $(DESTDIR)$(PKGCONFIGDIR)
	install -d $(DESTDIR)/$(DBDIR)/keys
	install -m644 *.plist $(DESTDIR)/$(DBDIR)/keys
	install -d $(DESTDIR)/$(SHAREDIR)/dulge.d
	install -m644 repod-main.conf $(DESTDIR)/$(SHAREDIR)/dulge.d/00-repository-main.conf
	install -m644 dulge.conf $(DESTDIR)/$(SHAREDIR)/dulge.d/dulge.conf
	install -d $(DESTDIR)/$(SHAREDIR)/zsh/site-functions
	install -m644 _dulge* $(DESTDIR)/$(SHAREDIR)/zsh/site-functions
	install -d $(DESTDIR)/$(SHAREDIR)/bash-completion/completions
	install -m644 dulge.bash $(DESTDIR)/$(SHAREDIR)/bash-completion/completions/dulge
	for file in dulge-checkvers dulge-create dulge-dgraph dulge-install \
		dulge-pkgdb dulge-query dulge-reconfigure dulge-remove dulge-rindex; do \
		ln -sf dulge $(DESTDIR)/$(SHAREDIR)/bash-completion/completions/$$file; \
	done

.PHONY: uninstall
uninstall:
	-rm -f $(DESTDIR)$(PKGCONFIGDIR)/$(PCFILE)
	-rm -f $(DESTDIR)$(SHAREDIR)/dulge.d/00-repository-main.conf
	-rm -f $(DESTDIR)$(SHAREDIR)/dulge.d/dulge.conf


.PHONY: clean
clean:
	-rm -f $(PCFILE)
