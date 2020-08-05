# dab-scanner-1.8 [![Build Status](https://travis-ci.org/JvanKatwijk/dab-scanner.svg?branch=master)](https://travis-ci.org/JvanKatwijk/dab-scanner)


![dab scanner](/dab-scanner-1.7.png?raw=true)

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

On the top right of the GUI a selector for a device is available.
To keep things simple, the selection is *once only* per program run,
after a device is selected, the selector will not be visible anymore.

There are two *start* buttons, one for *continuous* monitoring the band,
the other one for running over the band a specified number of times.

![dab scanner](/dab-summary.png?raw=true)

Note that after starting the scanning process, a *start button* becomes a
*stop button*. Note further that at the end of the scanning process - either by exhaustion or
by touching the (now) *stop button* - the button will become a *start button* again.

The number of cycles in *controlled mode* is determined by the value in the
*cycle counter*. This *cycle counter* is read after each cycle so, during processing
it can be changed.

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
* the Pluto
* the good old "RT2832" based dabsticks

Comment out (or uncomment) the line

	CONFIG += XXXX

where XXXX stands for the devicename, in the ".pro" file to include
or exclude the device in the configuration.

Support for the SDRplay can be configured for both the 2.13 and the 3.06
library.
For support of the 2.14 library, uncomment

	CONFIG += sdrplay_v2

For support of the 3.06 library, uncomment

	CONFIG += sdrplay_v3

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
	sudo apt-get install libsndfile1-dev
	sudo apt-get install zlib1g-dev rtl-sdr libusb-1.0-0-dev mesa-common-dev
	sudo apt-get install libgl1-mesa-dev libqt5opengl5-dev 
	sudo apt-get install qtbase5-dev libqwt-qt5-dev
	git clone https://github.com/JvanKatwijk/dab-scanner.git
	cd dab-scanner
	qmake
	make

Obviously assuming that the driver libraries for the device (or devices) to
be used are installed. (Note that for the Pluto device one needs to
have installed libiio.

The releases section now contains an "appImage". The appImage
is an executable, containing the required libraries *apart from those
for supporting the devices*.

Change the mode of the file to executable.

Note that the appImage is configured for SDRplay (library 2.13), AIRspy
and dabsticks.

------------------------------------------------------------------------------
Copyrights
------------------------------------------------------------------------------

  	Copyright (C)  2018 .. 2020
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The dab-scanner software is made available under the GPL-2.0.
        The SDR-J software, of which the dab-scanner software is a part,
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

