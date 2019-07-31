
----------------------------------------------------------------------
DAB-SCANNER
-----------------------------------------------------------------------

![dab scanner](/dab-scanner.png?raw=true)

dab-scanner is a simple tool that is used to scan all channels in a given
DAB band and report the contents of the ensemble(s) found in a txt file
that is formatted in a way that interpretation by e.g. Libre Office is
possible

![dab scanner output](/dab-scanner-output.png?raw=true)

The output shows - per ensemble - next to "standard" output items, the
SNR during reception of that ensemble, and - if it can be identified -
the transmitter identification in terms of a "mainId" and a "subId".

You could consider sending the DAB ensemble description to Gunther Lorenz glorenz@gmlist.org. He maintains a large database with FM stations and DAB ensembles.



-----------------------------------------------------------------------
Scanner use
-----------------------------------------------------------------------

The use of the scanner is extremely simple. The program - when started -
will wait for the user to touch the "start" button.
Touching the "start" button will 

* show a menu with which the name of a file can be chosen, the file to which
the output is written. A filename, containing date and time, is suggested but while
the name is perfectly legal under Linux, Windows does not accept it, so one has to choose another name.
* start the scanning with the lowest signal in the band. If the program is convinced that there no DAB signal can be identified, a next channel will be selected within a few seconds. If, however, a DAB signal can be identified, the name
of the ensemble will be shown, as well as the number of services detected in
that channel. After a user defined amount of time (the left spinbox on the GUI)
a next channel will be selected.
* At the end of the cycle the software will determine whether or not to run a
next cycle by looking at the spinbox at the right: if the number there is
positive after decreasing it by one, a next cycle will be executed.

Note that at the start of the scanning process, the "start" button becomes a
"stop" button. Note further that at the end of the scanning process the
button will become a "start" button again.

The GUI further shows during processing a red/green field, where green indicates that at least time synchonization could be achieved, an indicator for the 
Signal Noise Ratio, and an indicator for the frequency offset required to achieve frequency synchronization.

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

When started the program will try each of the configured devices and selects
the first one encountered. 

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




