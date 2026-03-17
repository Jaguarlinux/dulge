# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "package name with invalid characters cannot be installed"

sp = pmpkg("-foo")
self.addpkg2db("sync", sp)

self.args = "-S -- %s" % sp.name

self.addrule("!PACMAN_RETCODE=0")
self.addrule("!PKG_EXIST=-foo")
