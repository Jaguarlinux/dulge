# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Remove a package with --dbonly, no files touched"

p = pmpkg("dummy")
p.files = ["bin/dummy",
            "usr/man/man1/dummy.1"]
self.addpkg2db("local", p)

self.args = "-R --dbonly %s" % p.name

self.addrule("PACMAN_RETCODE=0")
self.addrule("!PKG_EXIST=dummy")
for f in p.files:
	self.addrule("FILE_EXIST=%s" % f)
