# Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
self.description = 'download a remote package with -U'
self.require_capability("curl")

url = self.add_simple_http_server({})

self.args = '-Uw {url}/foo.pkg'.format(url=url)

self.addrule('!PACMAN_RETCODE=0')
self.addrule('!CACHE_FEXISTS=foo.pkg')
self.addrule('!CACHE_FEXISTS=foo.pkg.sig')
