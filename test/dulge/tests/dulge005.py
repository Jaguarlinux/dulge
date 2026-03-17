# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Test invalid combination of command line options (-Qy)"

p = pmpkg("foobar")
self.addpkg2db("local", p)

self.args = "-Qy"

self.addrule("PACMAN_RETCODE=1")
