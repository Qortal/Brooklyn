#  Brooklyn R , The Moose V1.0
* V1.0 is based off a Plasma Dektop 
* V1.1a should be based on e17 sicne GPU issue is fixed

D: Raziel A. Crowe aka Scare Crowe
M: Sean Torchy aka Moose on the loose
A: Jason A. Crowe aka Crowetic
X: Netflix & Chill

		 GNUK Token Implementation
		 Source: http://www.gniibe.org

                     SWD port
                        (GND, SWD-CLK, SWD-IO)
    Power port +---------------------+
           Vdd |[]           []()() -------+
           GND |[]                  |      |
               |()() I/O port       | USB  |
               |      (PA2, PA3)    |      |
               |                    -------+
               +---------------------+

http://git.gniibe.org/gitweb/?p=gnuk/fst-01.git

If you rely heavily on VPN to bypass your regional filters then it will be a good idea to have extra entropy in hand. Any OpenSource Entropy stick should work on Brooklyn from now.

You can get a release and flash it directly on your USB 3 hard drive with Balena Etcher. The default image size is meant for a 32GB microSD card. If you have a bigger microSD card then after login do a "sudo raspi-config", head over to Advacned and resize the file system. Same can be done for a GPT partitioned USB 3 hard drive/SSD.  If you have purchased a Raspberry Pi 4 elsewher then you might have to flash the boot loader to do a mass storaeg boot. Ask The Moose on the loose for help.

Want to send some tip for continued development or features / apps ported on Brooklyn?

DOGE: DJTRkmhwhG7W8t7WvAddZBnNkKWML6nHqJ
LTC: LQyhdH9LfL9m5vqMHfeTZdRoUU8qAouZmQ
TRON: TPApU3DDHGGpCXx2xyzKwzbeze9FQ2My9K
Monero: 8AAwtBUuf9qbFttEZevtkpScq1K5XG5iEjTWE5VjAea6gSFvDAiZzNAT6GWSt8bNLSHxvi57j3b2bYzC2KGgfeD664EzRHk
ETH: 0xa69b98d7241006c2f23e895d81fdc7f562a8e378
USDT ERC20 : 0xa69b98d7241006c2f23e895d81fdc7f562a8e378
BTC: 13zp3jdZ5utX5vmZaZiDyJtam8daS4uBpC



Linux Kernel Qortal 5.12.6-accelerated+stable
=============================================

There are several guides for kernel developers and users. These guides can
be rendered in a number of formats, like HTML and PDF. Please read
Documentation/admin-guide/README.rst first.

In order to build the documentation, use ``make htmldocs`` or
``make pdfdocs``.  The formatted documentation can also be read online at:

    https://www.kernel.org/doc/html/latest/

There are various text files in the Documentation/ subdirectory,
several of them using the Restructured Text markup notation.

Please read the Documentation/process/changes.rst file, as it contains the
requirements for building and running the kernel, and information about
the problems which may result by upgrading your kernel.
