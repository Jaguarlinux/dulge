# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Read a package DB with several PGP signatures"

for i in range(1000):
	sp = pmpkg("pkg%03d" % i)
	sp.desc = "test description for package %d" % i
	sp.pgpsig = "asdfasdfsdfasdfsdafasdfsdfasd"
	self.addpkg2db("sync", sp)

self.args = "-Ss"

self.addrule("PACMAN_RETCODE=0")
