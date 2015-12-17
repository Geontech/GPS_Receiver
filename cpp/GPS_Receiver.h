#ifndef GPS_RECEIVER_IMPL_H
#define GPS_RECEIVER_IMPL_H

#include <boost/circular_buffer.hpp>
#include <nmea/nmea.h>
#include "gps_receiver_base.h"

using namespace boost;

class gps_receiver_i;

const int BUFF_SIZE = 256;

class gps_receiver_i : public gps_receiver_base
{
    ENABLE_LOGGING
    public:
        gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        gps_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~gps_receiver_i();
        int serviceFunction();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);
        void stop() throw (CF::Resource::StopError, CORBA::SystemException);
        void configure (const CF::Properties& configProperties) throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

        FRONTEND::GPSInfo* gpsInfo() { return new FRONTEND::GPSInfo(_gpsInfo); }
        FRONTEND::GpsTimePos* gpsTimePos() { return new FRONTEND::GpsTimePos(_gpsTimePos); }


    private:
        FRONTEND::GPSInfo _gpsInfo;
        FRONTEND::GpsTimePos _gpsTimePos;

        // File descriptor for serial stream from GPS
        // Circular buffer for NMEA messages
        // Mutex for reading the buffer
        // Worker thread instance to avoid blocking service_function
        int gps_fd;
        circular_buffer<unsigned char> _buffer;
        mutex _bufferMutex;
        thread *_worker;

        // For NMEA
    	nmeaPARSER parser;
		nmeaINFO bufferInfo;

        void _construct();
        void _workerFunction();
};

#endif
