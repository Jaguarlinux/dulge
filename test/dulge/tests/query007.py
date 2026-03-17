# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Query ownership of file in root"

sp = pmpkg("dummy")
sp.files = ["etc/config"]
self.addpkg2db("local", sp)

self.filesystem = ["config"]

self.args = "-Qo /config"

self.addrule("PACMAN_RETCODE=1")
