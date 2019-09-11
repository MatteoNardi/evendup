# keygrabber

Keygrabber duplicates an input device, making it possibile to use it with more than one
application grabbing it for exclusive use.


Keygrabber will:
- Open the provided input device.
- Issue a IEVIOCGRAB ioctl, so that other applications won't steal it from us.
- Create two new clones and print their path to stdout.
- Loop input events and copy then to all clones

# Example (Kodi + HomeAssistant)

I wrote this simple application because I wanted to control both Kodi and HomeAssistant
with my FLIRC remote. They both run on the same RaspberryPI and they assume they are the
only users of a given input device.

# Installation (From source)

This is how it can be installed from source on a RaspberryPI Buster
```
sudo apt-get install git meson libevdev-dev
git clone https://github.com/MatteoNardi/keygrabber
cd keygrabber
meson build
cd build
ninja
```

# Checking it is working

```
sudo systemctl stop kodi
sudo systemctl stop home-assistant@homeassistant.service
sudo keygrabber /dev/input/by-id/usb-flirc.tv_flirc-if01-event-kbd &
sudo systemctl restart kodi
loginctl enable-linger `whoami`
```


or

echo 'echo "hello world"' > /etc/keybrabber/on_f1
systemctl restart keygrabber


# FAQ

Why not using Kodi keymap? Doesn't allow to control kodi itself or use with different applications, like Steam

• Write Rust program
• SystemD unit file


https://unix.stackexchange.com/questions/400744/how-to-proxy-key-presses
https://github.com/burghardt/esekeyd/blob/master/src/esekeyd.c

Once it is duplicated, you can use it with HomeAssistant AND Kodi
https://www.home-assistant.io/components/keyboard_remote/
sudo apt-get install acl

