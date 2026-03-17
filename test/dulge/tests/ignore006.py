# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Sync with target in ignore list and say no"

pkg = pmpkg("package1")
self.addpkg2db("sync", pkg)

self.option["IgnorePkg"] = ["package1"]
self.args = "--ask=1 -S %s" % pkg.name

self.addrule("PACMAN_RETCODE=0")
self.addrule("!PKG_EXIST=package1")
