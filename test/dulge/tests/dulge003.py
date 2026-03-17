# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = "Test command line option (-S --help)"

self.args = "-S --help"

self.addrule("PACMAN_RETCODE=0")
