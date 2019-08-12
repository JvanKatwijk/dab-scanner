
----------------------------------------------------------------------
DAB-SCANNER 1.5
-----------------------------------------------------------------------
New in version 1.5

In version 1.5 *device selection* is changed. As in previous versions, the
name of the last selected device is kept between successive invocations
and the device selector is set to that name.
In that case there is *no need to touch the device selector*, the
selected device will automatically be opened at touching one of the
two start buttons. The device will remain open during the program run.
Touching a start button for a next round does not have any effect
on the device or its choice.

------------------------------------------------------------------------
New in version 1.4
------------------------------------------------------------------------

Version 1.4 has - compared to version 1.3 two extensions

* the *directory name*, the name of the selected directory for storing the output will be kept between program invocations and used in the suggestion of
the filename
* in *continuous mode* a table is shown with the channel names and for each
name a marker (either "+" or "-"). Channels for which the marker is "-"
will be skipped while scanning the band. The table is maintained between
program invocations


![dab scanner](/dab-scanner.png?raw=true)

dab-scanner is a tool that is used to scan all channels in a given
DAB band and report the contents of the ensemble(s) found in a txt file
that is formatted in a way that interpretation by e.g. Libre Office is
possible

![dab scanner output](/dab-scanner-output.png?raw=true)

The output shows - per ensemble - next to "standard" output items, the
SNR during reception of that ensemble, and - if it can be identified -
the transmitter identification in terms of a "mainId" and a "subId".

You could consider sending the DAB ensemble description to Gunther Lorenz glorenz@fmlist.org. He maintains a large database with FM stations and DAB ensembles.


-----------------------------------------------------------------------
Scanner use
-----------------------------------------------------------------------

The use of the scanner is extremely simple. The program - when started -
will wait for the user to select a device and touch  one of the two
"start" buttons.

In a previous version, the software would "poll" for the configured
devices to be connected to the computer. However, working with a laptop with one and a half usb connection, I did not realize that people might have different devices connected to the computer and need explicit control
over the selection of the device.

On the top right of the GUI a selector for a device is available.
To keep things simple, the selection is *once only* per program run,
after a device is selected, the selector will not be visible anymore.

If a start button is touched without a device being selected, no actions will follow.

If, however, a device is selected, touching one of the two start buttons will:

* show a menu with which the name of a file can be chosen, the file to which
the output is written. A filename, containing date and time, is suggested
* start the scanning with the lowest channel in the band (5A for Band III).
If the program is convinced that there no DAB signal can be identified, a next channel will be selected within a few seconds. If, however, a DAB signal can be identified, the name
of the ensemble will be shown, as well as the number of services detected in
that channel. After a user defined amount of time (the left spinbox on the GUI)
a next channel will be selected.
* At the end of the cycle the software will determine whether or not to run a
next cycle. In *continuous mode* a next cycle will always start, in *controlled mode*
the software will decrease the value in the cycle counter (a spinbox) by one, and only
if this value is (still) larger than zero, a next cycle will start.

![dab scanner](/dab-summary.png?raw=true)

Note that after starting the scanning process, a *start button* becomes a
*stop button*. Note further that at the end of the scanning process - either by exhaustion or
by touching the (now) *stop button* - the button will become a *start button* again.

The number of cycles in *controlled mode* is determined by the value in the
*cycle counter*. This *cycle counter* is read after each cycle so, during processing
it can be changed.

If, however, wants an "unlimited" amount of cycles, one shouls select the
*start continuous* button. Cycling will just go on until the button is
touches again or - for whatever reason - the program halts.
Since many dx-ers want to cycle for hours, a summary of the output
is written to another textfile. The name of the textfile is related to
the name of the txt file containing the full output.

In *continuous mode* a table is shown with the channel numbers, each marked with a "+" or a "-" sign,
which can be altered by the user (clicking *twice* will change the mark). The software will skip during
scanning those channels marked with a "-" sign.


The GUI further shows during processing a red/green field, where green indicates that at least time synchonization could be achieved, an indicator for the 
Signal Noise Ratio (measured as the ratio between the signals strength
in the datablocks in a DAB frame and the signal strength in the null period
of the frame), and an indicator for the frequency offset required to achieve frequency synchronization.

---------------------------------------------------------------------------
Selecting an input device
---------------------------------------------------------------------------

As with other sdr-j software, the software can be configured with a
variety of devices:
 
* the SDRplay
* the lime SDR
* the hackrf
* the AIRspy
* the good old "RT2832" based dabsticks

As mentioned, a device should be selected prior to touching the start button.

----------------------------------------------------------------------------
The dab-scanner on Windows
----------------------------------------------------------------------------

For use on Windows the releases section of the git repository contains
an installer. The installer, when run, will - as the name suggests -
the software and create - if so desired - a link on the screen.

One issue on Windows is that the suggested name for the output file (a name containing the date and time)
is considered erroneous. 

----------------------------------------------------------------------------
The dab-scanner on Linux and the RPI
----------------------------------------------------------------------------

As usual, users on Linux based systems have to do all the work themselves.

On a Debian based system (i.e. Ubuntu and RPI Stretch) the following
lines could be packed in a script and executed

	sudo apt-get update
	sudo apt-get install qt5-qmake build-essential g++ git 
	sudo apt-get install qt5-default libfftw3-dev 
	sudo apt-get install zlib1g-dev rtl-sdr libusb-1.0-0-dev mesa-common-dev
	sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev 
	sudo apt-get install qtbase5-dev
	sudo apt-get install librtlsdr-dev 
	git clone https://github.com/JvanKatwijk/dab-scanner.git
	cd dab-scanner
	qmake
	make

Obviously assuming that the driver libraries for the device (or devices) to
be used are installed.

-----------------------------------------------------------------------------------------------------------------------
Copyrights
-----------------------------------------------------------------------------------------------------------------------

  	Copyright (C)  2013, 2014, 2015, 2016, 2017, 2018, 2019
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The dab-scanner software is made available under the GPL-2.0.
        The SDR-J software, of which the dab-scanner software is a part,
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.




