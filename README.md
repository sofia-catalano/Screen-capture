# Screen-capture

How to install wxWidgets:

$ git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
$ mkdir buildgtk
$ cd buildgtk
$ ../configure --with-gtk
$ make
$ sudo make install
$ sudo ldconfig