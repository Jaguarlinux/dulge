# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Get package list from sync dbs"

sp = pmpkg("dummy")
sp.files = ["bin/dummy",
            "usr/man/man1/dummy.1"]
self.addpkg2db("sync", sp)

self.args = "-Sl"

self.addrule("PACMAN_RETCODE=0")
self.addrule("PACMAN_OUTPUT=^sync %s" % sp.name)
