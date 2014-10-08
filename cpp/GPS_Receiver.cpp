/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "GPS_Receiver.h"

PREPARE_LOGGING(GPS_Receiver_i)

/* For serial interface */
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

const int BUFF_SIZE = 256;

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

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	this->addPropertyChangeListener("serial_port", this, &GPS_Receiver_i::configure_serial_port);
}

GPS_Receiver_i::~GPS_Receiver_i()
{
}

void GPS_Receiver_i::_construct() {
	_worker = NULL;
	_buffer.set_capacity(BUFF_SIZE);
	_buffer.clear();

	nmea_parser_init(&_parser);
	memset(&_bufferInfo, 0, sizeof(_bufferInfo));

	// Setup serial port and issue cold start
	struct termios tty;
	memset(&tty, 0, sizeof(tty));

	_gps_fd = open(serial_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_gps_fd < 0) {
		LOG_ERROR(GPS_Receiver_i, "Aborting. Failed to open: " + serial_port);
		return;
	}

	LOG_DEBUG(GPS_Receiver_i, "Serial port to GPS is now open");

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
	tcflush (_gps_fd, TCIFLUSH);	// Flush modem lines
	tcsetattr(_gps_fd, TCSANOW, &tty); // Set attributes

	// Issue SiRF Star IV Cold Restart command to device
	const char *init = "$PSRF101,0,0,0,0,0,0,12,4*10\r\n";
	write(_gps_fd, init, sizeof(init));

	LOG_INFO(GPS_Receiver_i, "Connected to GPS Receiver");
}

void GPS_Receiver_i::configure_serial_port(const char* oldValue, const char* newValue) {
	serial_port = newValue;
	if (this->_started) {
		this->stop();
	}
	this->start();
}

/**************************************************************************

    This is called automatically after allocateCapacity or deallocateCapacity are called.
    Your implementation should determine the current state of the device:

       setUsageState(CF::Device::IDLE);   // not in use
       setUsageState(CF::Device::ACTIVE); // in use, with capacity remaining for allocation
       setUsageState(CF::Device::BUSY);   // in use, with no capacity remaining for allocation

**************************************************************************/
void GPS_Receiver_i::updateUsageState()
{
	if (this->_started) {
		setUsageState(CF::Device::ACTIVE);
	}
	else {
		setUsageState(CF::Device::IDLE);
	}
}

void GPS_Receiver_i::start() throw (CF::Resource::StartError, CORBA::SystemException) {
	// Call base class and construct
	GPS_Receiver_base::start();

    _construct();

	if (0 >= _gps_fd) {
		_worker = NULL;
		LOG_WARN(GPS_Receiver_i, "Unable to start.  Serial connection to GPS receiver not found.");
		stop();
	}
	else {
		mutex::scoped_lock lock(_bufferMutex);
		_worker = new thread(&GPS_Receiver_i::_workerFunction, this);
	}
}

void GPS_Receiver_i::stop() throw (CF::Resource::StopError, CORBA::SystemException) {
	// Call base
	GPS_Receiver_base::stop();

	mutex::scoped_lock lock(_bufferMutex);
	if (NULL != _worker) {
		_worker->interrupt();
		_worker->join();
		delete _worker;
		_worker = NULL;
	}

    if (0 < _gps_fd)
	    close(_gps_fd);

    nmea_parser_destroy(&_parser);
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
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS or dataVITA49  port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the device has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the device base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
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

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the device developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (GPS_Receiver_base).
    
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
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &GPS_Receiver_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to GPS_Receiver.cpp
        GPS_Receiver_i::GPS_Receiver_i(const char *uuid, const char *label) :
            GPS_Receiver_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &GPS_Receiver_i::scaleChanged);
        }

        void GPS_Receiver_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }
            
        //Add to GPS_Receiver.h
        void scaleChanged(const float* oldValue, const float* newValue);
        
        
************************************************************************************************/
int GPS_Receiver_i::serviceFunction()
{
    // If buffer has data, parse NMEA messages using NMEA Library.
    std::string temp;
    int msgCount = 0;
    int retval = NOOP;

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

    	LOG_TRACE(GPS_Receiver_i, "Messages: \n" + temp);

    	// Parse and update bufferInfo
		int numProcessed = nmea_parse(&_parser, temp.c_str(), temp.size(), &_bufferInfo);

		// For debugging
		std::ostringstream dbg;
		dbg << "Messages Good " << numProcessed << " vs. Bad " << msgCount << '\n';

		if (0 < numProcessed) {
			retval = NORMAL;
			// Update gpsInfo
			_gps_info.satellite_count = _bufferInfo.satinfo.inview;

			// Update gpsTimePos w/ filter.
			const double NEW = 0.1;
			const double OLD = 0.9;
			_gps_time_pos.position.alt =
					(NEW * _bufferInfo.elv) +
					(OLD * _gps_time_pos.position.alt);

			_gps_time_pos.position.lat =
					(NEW * degMinSec_to_dec(_bufferInfo.lat)) +
				    (OLD * _gps_time_pos.position.lat);

			_gps_time_pos.position.lon =
					(NEW * degMinSec_to_dec(_bufferInfo.lon)) +
					(OLD * _gps_time_pos.position.lon);

			// Convert UTC time
			BULKIO::PrecisionUTCTime tstamp;

			// Whole seconds
			tstamp.twsec = 60.0 * ((60.0 * _bufferInfo.utc.hour) + (double) _bufferInfo.utc.min)
					+ (double) _bufferInfo.utc.sec;

			// "fractional" seconds from 0.0 to 1.0.
			// SiRF Star IV docs show GPS UTC format is in 1/100th seconds (hsec)
			tstamp.tfsec = (double) _bufferInfo.utc.hsec / 100.0;
			tstamp.toff = 0.0;
			tstamp.tcmode = 0;
			tstamp.tcstatus = 1;

			_gps_info.timestamp = tstamp;
			_gps_time_pos.timestamp = tstamp;

			// Debug output
			dbg << "Satellite count:    " << _bufferInfo.satinfo.inview << '\n';
			dbg << "UTC Seconds:        " << tstamp.twsec << '\n';
			dbg << "Latitude:           " << _bufferInfo.lat << '\n';
			dbg << "Longitude:          " << _bufferInfo.lon << '\n';
			dbg << "Elevation:          " << _bufferInfo.elv << '\n';
			LOG_DEBUG(GPS_Receiver_i, dbg.str());
    	}
		else {
			LOG_DEBUG(GPS_Receiver_i, "No valid messages processed");
		}

		// Cleanup
    	_buffer.clear();
    }

    // validate data based on satellite count
	_gps_time_pos.position.valid = (3 <= _gps_info.satellite_count) ? true : false;

    return retval;
}

void GPS_Receiver_i::_workerFunction() {
	const int SIZE = 128;
	int actualSize = 0;
	unsigned char temp[SIZE];
	memset(temp, 0, SIZE);

	// Brief delay to let the GPS finish cold restart.
	sleep(1);

	while (!_worker->interruption_requested()) {
		actualSize = read(_gps_fd, temp, SIZE);
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
