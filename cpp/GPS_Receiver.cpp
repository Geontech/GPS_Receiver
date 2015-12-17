
/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 	Source: gps_receiver.spd.xml
 	Generated on: Fri Oct 11 10:40:13 EDT 2013
 	REDHAWK IDE
 	Version: 1.8.4
 	Build id: R201305151907

**************************************************************************/

#include "gps_receiver.h"
using namespace boost;

/* For serial interface */
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// NMEA parser provides lat/lon as [deg][min].[sec/60]
// which needs to be converted to decimal degrees.
// http://notinthemanual.blogspot.com/2008/07/convert-nmea-latitude-longitude-to.html
double degMinSec_to_dec(double dms) {
	double frac, result;
	frac = modf(dms / 100.0, &result) / 60.0 * 100.0;
	result = result + frac;
	return result;
}

string int_array_to_char(int int_array[], int size_of_array, string token){
	stringstream returnstring;
	for (int temp = 0; temp < size_of_array; temp++) {
		returnstring << int_array[temp];
		if (temp != size_of_array - 1) returnstring << token;
	}

	return returnstring.str();
}

PREPARE_LOGGING(gps_receiver_i)

gps_receiver_i::gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    gps_receiver_base(devMgr_ior, id, lbl, sftwrPrfl)
{
	_construct();
}

gps_receiver_i::gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    gps_receiver_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
	_construct();
}

gps_receiver_i::gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    gps_receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
	_construct();
}

gps_receiver_i::gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    gps_receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	_construct();
}

gps_receiver_i::~gps_receiver_i()
{
	if (0 < gps_fd)
		close(gps_fd);

	nmea_parser_destroy(&parser);
}

void gps_receiver_i::configure (const CF::Properties& configProperties)
	throw (CF::PropertySet::PartialConfiguration,
		   CF::PropertySet::InvalidConfiguration,
		   CORBA::SystemException)
		   {
	gps_receiver_base::configure(configProperties);
	start();
}

void gps_receiver_i::_construct() {
	_worker = NULL;
	_buffer.set_capacity(BUFF_SIZE);
	_buffer.clear();

	nmea_parser_init(&parser);
	memset(&bufferInfo, 0, sizeof(bufferInfo));

	// Setup serial port and issue cold start
	struct termios tty;
	memset(&tty, 0, sizeof(tty));

	gps_fd = open(serial_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (gps_fd < 0) {
		LOG_ERROR(gps_receiver_i, "Aborting. Failed to open: " + serial_port);
		return;
	}

	LOG_DEBUG(gps_receiver_i, "Serial port to GPS is now open");

	// From tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
	// Modified with:
	// 1) Launch minicom to init and capture stream
	// 2) kill -9 minicom to avoid a clearing "modem" settings
	// 3) stty -a -F <device> to mirror settings below
	tty.c_cflag = B4800 | CRTSCTS | CS8 | CLOCAL | CREAD;
	tty.c_iflag = IGNPAR | ICRNL;
	tty.c_oflag = 0;
	tty.c_lflag = ICANON;
	tty.c_cc[VINTR]    = 0;     /* Ctrl-c */
	tty.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	tty.c_cc[VERASE]   = 0;     /* del */
	tty.c_cc[VKILL]    = 0;     /* @ */
	tty.c_cc[VEOF]     = 4;     /* Ctrl-d */
	tty.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	tty.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	tty.c_cc[VSWTC]    = 0;     /* '\0' */
	tty.c_cc[VSTART]   = 0;     /* Ctrl-q */
	tty.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	tty.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	tty.c_cc[VEOL]     = 0;     /* '\0' */
	tty.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	tty.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	tty.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	tty.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	tty.c_cc[VEOL2]    = 0;     /* '\0' */
	tcflush (gps_fd, TCIFLUSH);	// Flush modem lines
	tcsetattr(gps_fd, TCSANOW, &tty); // Set attributes

	// Issue SiRF Star IV Cold Restart command to device
	const char *init = "$PSRF101,0,0,0,0,0,0,12,4*10\r\n";
	write(gps_fd, init, sizeof(init));

	LOG_INFO(gps_receiver_i, "Connected to GPS Receiver");
}

void gps_receiver_i::start() throw (CF::Resource::StartError, CORBA::SystemException) {
	// Call base class
	gps_receiver_base::start();

	if (0 >= gps_fd) {
		_worker = NULL;
		LOG_WARN(gps_receiver_i, "Unable to start.  Serial connection to GPS receiver not found.");
		stop();
	}
	else {
		mutex::scoped_lock lock(_bufferMutex);
		_worker = new thread(&gps_receiver_i::_workerFunction, this);
	}
}

void gps_receiver_i::stop() throw (CF::Resource::StopError, CORBA::SystemException) {
	// Call base
	gps_receiver_base::stop();

	mutex::scoped_lock lock(_bufferMutex);
	if (NULL != _worker) {
		_worker->interrupt();
		_worker->join();
		delete _worker;
		_worker = NULL;
	}

	setUsageState(CF::Device::IDLE);
}
/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
        	stream_id = "";
	    	sri = BULKIO::StreamSRI();
	    	sri.hversion = 1;
	    	sri.xstart = 0.0;
	    	sri.xdelta = 0.0;
	    	sri.xunits = BULKIO::UNITS_TIME;
	    	sri.subsize = 0;
	    	sri.ystart = 0.0;
	    	sri.ydelta = 0.0;
	    	sri.yunits = BULKIO::UNITS_NONE;
	    	sri.mode = 0;
	    	sri.streamID = this->stream_id.c_str();

	Time:
	    To create a PrecisionUTCTime object, use the following code:
	        struct timeval tmp_time;
	        struct timezone tmp_tz;
	        gettimeofday(&tmp_time, &tmp_tz);
	        double wsec = tmp_time.tv_sec;
	        double fsec = tmp_time.tv_usec / 1e6;;
	        BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
	        tstamp.tcmode = BULKIO::TCM_CPU;
	        tstamp.tcstatus = (short)1;
	        tstamp.toff = 0.0;
	        tstamp.twsec = wsec;
	        tstamp.tfsec = fsec;
        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type BULKIO::dataShort called short_in
            //  A uses (output) port of type BULKIO::dataFloat called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            BULKIO_dataShort_In_i::dataTransfer *tmp = short_in->getPacket(-1);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (gps_receiver_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        A callback method can be associated with a property so that the method is
        called each time the property value changes.  This is done by calling 
        setPropertyChangeListener(<property name>, this, &gps_receiver::<callback method>)
        in the constructor.
            
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to gps_receiver.cpp
        gps_receiver_i::gps_receiver_i(const char *uuid, const char *label) :
            gps_receiver_base(uuid, label)
        {
            setPropertyChangeListener("scaleValue", this, &gps_receiver_i::scaleChanged);
        }

        void gps_receiver_i::scaleChanged(const std::string& id){
            std::cout << "scaleChanged scaleValue " << scaleValue << std::endl;
        }
            
        //Add to gps_receiver.h
        void scaleChanged(const std::string&);
        
        
************************************************************************************************/
int gps_receiver_i::serviceFunction()
{
    // If buffer has data, parse NMEA messages using NMEA Library.
    std::string temp;
    int msgCount = 0;

    mutex::scoped_lock lock(_bufferMutex);

    if (64 < _buffer.size()) {
    	for (unsigned int i = 0; i < _buffer.size(); i++)
    	{
    		// Ensure NMEA strings end with /r/n per the spec.
    		if (0 < i) {
    			if (('\r' != _buffer[i-1]) && ('\n' == _buffer[i])) {
    				temp.push_back('\r');
    				msgCount++;
    			}
    		}

    		temp.push_back(_buffer[i]);
    	}

    	LOG_TRACE(gps_receiver_i, "Messages: \n" + temp);

    	// Parse and update bufferInfo
		int numProcessed = nmea_parse(&parser, temp.c_str(), temp.size(), &bufferInfo);

		// For debugging
		std::ostringstream dbg;
		dbg << "Messages Good " << numProcessed << " vs. Bad " << msgCount << '\n';

		if (0 < numProcessed) {
			// Update gpsInfo
			_gpsInfo.satellite_count = bufferInfo.satinfo.inview;

			// Update gpsTimePos w/ filter.
			const double NEW = 0.1;
			const double OLD = 0.9;
			_gpsTimePos.position.alt =
					(NEW * bufferInfo.elv) +
					(OLD * _gpsTimePos.position.alt);

			_gpsTimePos.position.lat =
					(NEW * degMinSec_to_dec(bufferInfo.lat)) +
				    (OLD * _gpsTimePos.position.lat);

			_gpsTimePos.position.lon =
					(NEW * degMinSec_to_dec(bufferInfo.lon)) +
					(OLD * _gpsTimePos.position.lon);

			// Convert UTC time
			BULKIO::PrecisionUTCTime tstamp;

			// Whole seconds
			tstamp.twsec = 60.0 * ((60.0 * bufferInfo.utc.hour) + (double) bufferInfo.utc.min)
					+ (double) bufferInfo.utc.sec;

			// "fractional" seconds from 0.0 to 1.0.
			// SiRF Star IV docs show GPS UTC format is in 1/100th seconds (hsec)
			tstamp.tfsec = (double) bufferInfo.utc.hsec / 100.0;
			tstamp.toff = 0.0;
			tstamp.tcmode = 0;
			tstamp.tcstatus = 1;

			_gpsInfo.timestamp = tstamp;
			_gpsTimePos.timestamp = tstamp;

			// Debug output
			dbg << "Satellite count:    " << bufferInfo.satinfo.inview << '\n';
			dbg << "UTC Seconds:        " << tstamp.twsec << '\n';
			dbg << "Latitude:           " << bufferInfo.lat << '\n';
			dbg << "Longitude:          " << bufferInfo.lon << '\n';
			dbg << "Elevation:          " << bufferInfo.elv << '\n';
			LOG_DEBUG(gps_receiver_i, dbg.str());
    	}
		else {
			LOG_DEBUG(gps_receiver_i, "No valid messages processed");
		}

		// Cleanup
    	_buffer.clear();
    }

    // validate data based on satellite count
	_gpsTimePos.position.valid = (3 <= _gpsInfo.satellite_count) ? true : false;

    return NOOP;
}

void gps_receiver_i::_workerFunction() {
	const int SIZE = 128;
	int actualSize = 0;
	unsigned char temp[SIZE];
	memset(temp, 0, SIZE);

	// Brief delay to let the GPS finish cold restart.
	sleep(1);

	while (!_worker->interruption_requested()) {
		actualSize = read(gps_fd, temp, SIZE);
		if (0 < actualSize) {
			mutex::scoped_lock lock(_bufferMutex);
			for (int i = 0; i < actualSize; i++)
				_buffer.push_back(temp[i]);
		}

		// BU-353S4's SiRF Star IV chip has a maximum update
		// rate of 1 second for any message.  No need to hammer
		// through this loop at warp speed.
		usleep(1);
	}
}
