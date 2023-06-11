Change access to /sys/class/leds/platform::micmute/brightness:

# echo 'SUBSYSTEM=="leds", ACTION=="add", KERNEL=="platform::micmute", RUN+="/bin/chmod a+w /sys/class/leds/%k/brightness"' > /etc/udev/rules.d/82-micmute-led.rules

