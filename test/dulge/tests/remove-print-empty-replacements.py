# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Print empty replacements when using -Rp"

p = pmpkg("a")
self.addpkg2db("local", p)

self.args = "-Rp --print-format 'foo%%f%%g%%h%%h' %s" % p.name

self.addrule("PACMAN_RETCODE=0")
self.addrule("PACMAN_OUTPUT=^foo$")
