# evendup

Evendup duplicates an input device, making it possibile to use it with multiple
applications (Even if they grab it for exclusive use). Evendup will:
- Open the provided input device.
- Issue a IEVIOCGRAB ioctl, so that other applications won't steal it from us.
- Create two clones.
- Loop input events and copy them to all clones

## Motivation (Kodi + HomeAssistant)

I wrote this simple application because I wanted to control both Kodi and HomeAssistant
with my FLIRC remote. They both run on the same RaspberryPI and they assume they are the
only users of a given input device.

Basically, evendup is a workaround against unnecessary IEVIOCGRAB ioctl calls.

## Installation from source and example usage

This is how it can be installed from source on a RaspberryPI Buster
```
sudo apt-get install git meson libevdev-dev
git clone https://github.com/MatteoNardi/evendup
cd evendup
meson build
cd build
ninja
sudo ninja install
sudo systemctl enable evendup@usb-flirc.tv_flirc-if01-event-kbd.service
```

## Manual start

```
sudo systemctl stop kodi
sudo systemctl stop home-assistant@homeassistant.service
sudo evendup /dev/input/by-id/usb-flirc.tv_flirc-if01-event-kbd &
sudo systemctl start home-assistant@homeassistant.service
sudo systemctl start kodi
```

## Running without root permissions

You'll need to fix uinput permissions:
```
sudo apt-get install acl
ExecStartPre = sudo setfacl -m u:%u:rw /dev/uinput
```

