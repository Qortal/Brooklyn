Introduction
============


What's Gnuk?
------------

Gnuk is an implementation of USB cryptographic token for GNU Privacy
Guard.  Gnuk supports OpenPGP card protocol version 2, and it runs on
STM32F103 processor.

This document explains about Gnuk 1.2, which comes with ECC algorithm.


Cryptographic token and feature of Gnuk
---------------------------------------

Cryptographic token is a store of private keys and it computes cryptographic
functions on the device.

The idea is to separate important secrets to independent device, 
from where nobody can extract them.


Development Environment
-----------------------

See :doc:`development` for development environment for Gnuk.
Gnuk is developed on the environment where there are only Free Software.


Target boards for running Gnuk
------------------------------

Hardware requirement for Gnuk is the micro controller STM32F103.
In version 1.2, Gnuk supports following boards.

* FST-01 (Flying Stone Tiny ZERO-ONE)

* Olimex STM32-H103

* ST Nucleo F103

* Nitrokey Start


Host prerequisites for using Gnuk Token
---------------------------------------

* GNU Privacy Guard (GnuPG)

* libusb

* [Optional] SSH: openssh

* [experimental] Web: scute, firefox


Usages
------

* Sign with GnuPG
* Decrypt with GnuPG
* Use with OpenSSH through gpg-agent (as ssh-agent)
* [experimental] Use with Firefox through Scute for X.509 client certificate authentication
