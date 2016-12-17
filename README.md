# onsen-watch
Onsen Watch is a Onsen.ag radio notification, download and listening tool.

音泉Watchは音泉ラジオ通知＆タウンロードツールです。

## Download

Windows (32-bit): https://github.com/z411/onsen-watch/releases/download/v0.1.1/onsen-watch-win32.7z

## Screenshot

![Alt text](/screenshot.jpg?raw=true "Onsen Watch")

## Compiling

You need these dependencies:

* libxml2
* QtCore5
* QtGui5
* QtNetwork5
* QtWidgets5

These are optional:

* QtMultimedia5 (if you want sound alerts and the internal media player)

Then clone repo and go into directory:

    cd onsen-watch
    qmake
    make
    ./onsen-watch

