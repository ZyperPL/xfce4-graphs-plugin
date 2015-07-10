#!/bin/sh

main=graphs

gcc -std=c11 -shared -o lib${main}.so -fPIC ${main}.c `pkg-config --cflags --libs libxfce4panel-2.0 gtk+-2.0`
sudo cp lib${main}.so /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/lib${main}.so 
sudo cp ${main}.desktop /usr/share/xfce4/panel/plugins/${main}.desktop

rm lib${main}.so
