# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "package name with invalid characters cannot be installed (file)"

p = pmpkg("-foo")
self.addpkg(p)

self.args = "-U -- %s" % p.filename()

self.addrule("!PACMAN_RETCODE=0")
self.addrule("!PKG_EXIST=-foo")
