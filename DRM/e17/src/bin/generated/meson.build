protos = [
  '@0@/unstable/linux-dmabuf/linux-dmabuf-unstable-v1.xml'.format(dir_wayland_protocols),
  '../../protocol/session-recovery.xml',
  '../../protocol/efl-aux-hints.xml',
  '../../protocol/action_route.xml',
  '@0@/unstable/xdg-foreign/xdg-foreign-unstable-v1.xml'.format(dir_wayland_protocols),
  '@0@/unstable/relative-pointer/relative-pointer-unstable-v1.xml'.format(dir_wayland_protocols),
  '@0@/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml'.format(dir_wayland_protocols),
]

proto_c = []
proto_h = []

foreach p: protos
  proto_h += gen_scanner_server.process(p)
  proto_c += gen_scanner_impl.process(p)
endforeach

gadget_loader_protos = [
  '../../protocol/action_route.xml',
  '../../protocol/e-gadget.xml',
]

wayland_proto_c = proto_c
wayland_proto_h = proto_h
