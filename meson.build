project('evendup', 'c', license: 'MIT')

evdev = dependency('libevdev')

executable('evendup', 'src/main.c', dependencies: [ evdev ], install: true)

# Systemd unit
systemd = dependency('systemd', required: false)
if systemd.found()
  bindir = join_paths(get_option('prefix'), get_option('bindir'))
  subs = configuration_data()
  subs.set('bindir', bindir)

  unit_file = configure_file(
    input: 'data/evendup@.service.in',
    output: 'evendup@.service',
    configuration: subs
  )

  unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
  install_data(unit_file, install_dir: unitdir)
endif
