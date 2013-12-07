basic_micro
===========

Implementation of the classic ‘Simon Says’ game using the PICAXE-08M2 microcontroller.

The lack of output pins on the 08M2 really limits what you can do. If you're planning on building this circuit, or anything of equal or greater complexity, I would recommend using something bigger.

Usage
=====
* Build the circuit using the basic_micro.sch [Eagle](http://www.cadsoftusa.com/) file.
* Download the code to the PICAXE. The download must be in progress when the circuit is powered on. This is due to C.0/Serial Out being used as an output.
