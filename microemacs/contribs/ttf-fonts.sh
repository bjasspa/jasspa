#!/bin/sh
if [ -d ~/.local/share/fonts ]; then
    (sleep 5 && xset +fp ~/.local/share/fonts && xset fp rehash && echo "TTF fonts loaded!")
fi

