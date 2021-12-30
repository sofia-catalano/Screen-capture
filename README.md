# Screen-capture

How to install wxWidgets:
$ sudo apt install libgtk-3-dev
$ git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
$ cd wxWidgets
$ mkdir buildgtk
$ cd buildgtk
$ ../configure --with-gtk
$ make
$ sudo make install
$ sudo ldconfig