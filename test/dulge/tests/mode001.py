# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Check the mode of default files in a package"

p = pmpkg("pkg1")
p.files = ["bin/foo",
           "bin/bar"]
self.addpkg(p)

self.args = "-U %s" % p.filename()

self.addrule("PACMAN_RETCODE=0")
for f in p.files:
	self.addrule("FILE_MODE=%s|644" % f)
