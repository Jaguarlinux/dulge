# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "local package name with invalid characters can be removed"

sp = pmpkg("-foo")
self.addpkg2db("local", sp)

self.args = "-R -- %s" % sp.name

self.addrule("PACMAN_RETCODE=0")
self.addrule("!PKG_EXIST=-foo")
