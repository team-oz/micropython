.. _p2_quickref:

Quick reference for the P2 (Propeller 2)
========================================

.. image:: img/HubRAM.gif
    :alt: Hub memory of the P2 (animated)
    :width: 427px

The team-oz P2 Reference Board

Below is a quick referece for Propeller 2 (P2) based boards.  P2 has unique features including smart peripherals built into every pin, there is an overview here

.. toctree::
   :maxdepth: 1
   
   general.rst
   tutorial/intro.rst
     
Installing MicroPython
----------------------

See the corresponding section of tutorial: :ref:'p2_intro' to flash your P2 board with native MicroPython firmware

General board control
---------------------

The MicroPython REPL is at baudrate 115200. 
Tab-completion is useful to find out what methods an object has. 
Paste mode (control-E) is useful to paste a large slab of Python code into the REPL. 

The :mod:'machine' module:: 

