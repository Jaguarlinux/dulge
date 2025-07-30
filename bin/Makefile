-include ../config.mk

SUBDIRS +=	dulge-alternatives
SUBDIRS +=	dulge-create
SUBDIRS +=	dulge-dgraph
SUBDIRS +=	dulge-install
SUBDIRS +=	dulge-pkgdb
SUBDIRS +=	dulge-query
SUBDIRS +=	dulge-reconfigure
SUBDIRS +=	dulge-remove
SUBDIRS +=	dulge-rindex
SUBDIRS +=	dulge-uhelper
SUBDIRS +=	dulge-checkvers
SUBDIRS +=	dulge-fbulk
SUBDIRS +=	dulge-digest
SUBDIRS +=	dulge-fetch

ifeq (${DULGE_OS},linux)
SUBDIRS +=	dulge-uchroot
SUBDIRS +=	dulge-uunshare
endif

include ../mk/subdir.mk
