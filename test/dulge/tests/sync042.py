# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Install a sync package conflicting with a local one"

sp = pmpkg("pkg1")
sp.conflicts = ["pkg2"]
self.addpkg2db("sync", sp);

lp = pmpkg("pkg2")
self.addpkg2db("local", lp);

self.args = "-S %s" % sp.name

self.addrule("PACMAN_RETCODE=1")
self.addrule("!PKG_EXIST=pkg1")
self.addrule("PKG_EXIST=pkg2")
