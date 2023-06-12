
# Introduction
This program was developed to control the Fn-F4 key from Lenovo ThinkPad. It turns on the LED whenever the microphone is turned on mute and off when unmuted.

# How to use
Generaly pulseaudio is started when in a desktop gui session what means, the program should run after the user such session get started. Include this program to run in the session application to be started in you desktop session manager.

# Access to the LED class by user
Change access to /sys/class/leds/platform::micmute/brightness:

# ```echo 'SUBSYSTEM=="leds", ACTION=="add", KERNEL=="platform::micmute", RUN+="/bin/chmod a+w /sys/class/leds/%k/brightness"' > /etc/udev/rules.d/82-micmute-led.rules```
