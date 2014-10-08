# NMEA Library

**Description:**  This Eclipse project (static C library) is based off source code found from the NMEA Library project on sourceforge.  Unfortunately, it didn't include the necessary files to compile on Linux so I made an Eclipse project to help facilitate that process.

See the ./include/nmea path for necessary header information.  See the original
project website for sample code.

## Installation

Change into the Release path and make install:

    cd ./Release
    make
    sudo make install
    
The library will be installed in `/usr/local`.  To use it in a REDHAWK Eclipse project, add it to your libraries as `/usr/local/lib/libnmea.a`.
