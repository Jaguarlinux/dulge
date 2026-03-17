# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Sysupgrade with an epoch and ignore on same package"

lp = pmpkg("dummy", "1.0-1")
self.addpkg2db("local", lp)

sp = pmpkg("dummy", "1:1.0-2")
self.addpkg2db("sync", sp)

self.args = "-Su --ignore %s" % lp.name

self.addrule("PACMAN_RETCODE=0")
self.addrule("PKG_VERSION=dummy|1.0-1")
