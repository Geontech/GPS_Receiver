GPS_Receiver
============

FEI-Compliant serial GPS Receiver for REDHAWK.  It has been tested against the USB-attached BU-353S4 GPS receiver that was used on the APG RasPi "RasHawk" project.  This device is an updated and more stable version of that earlier device making this version compatible with REDHAWK 1.10 and FEI 2.0.

## Setup

Give the user that will run this Device access to whatever group owns `/dev/tty????`.  The default in CentOS 6.5 for example is `dailout` and the device shows on `/dev/ttyUSB0`:

    crw-rw----. 1 root dialout 188, 0 Oct  8 07:38 /dev/ttyUSB0

Therefore `dialout` to your user:

    usermod -a -G dialout my_user

## Usage

Configure the `serial_port` property to set the path for the serial interface.   The device will only `start` if it can access that port.  See [Setup](#Setup) if you receive an error about being unable to access an interface that exists in the file system.

