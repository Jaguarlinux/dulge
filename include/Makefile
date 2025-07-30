-include ../config.mk

INCS =	dulge.h

.PHONY: all
all:
	sed -e "s|@@VERSION@@|${VERSION}|g" ${INCS}.in > ${INCS}

.PHONY: install
install:
	install -d $(DESTDIR)$(INCLUDEDIR)/dulge
	install -m 644 $(INCS) $(DESTDIR)$(INCLUDEDIR)
	for f in array bool data dictionary number object string; do \
		install -m 644 dulge/dulge_$${f}.h $(DESTDIR)$(INCLUDEDIR)/dulge; \
	done

.PHONY: uninstall
uninstall:
	-rm -f $(DESTDIR)$(INCLUDEDIR)/$(INCS)

.PHONY: clean
clean:
	-rm -f ${INCS}
