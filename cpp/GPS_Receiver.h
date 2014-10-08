#ifndef GPS_RECEIVER_IMPL_H
#define GPS_RECEIVER_IMPL_H

#include "GPS_Receiver_base.h"

#include <boost/circular_buffer.hpp>
#include <nmea/nmea.h>
using namespace boost;

class GPS_Receiver_i : public GPS_Receiver_base
{
    ENABLE_LOGGING
    public:
        GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPS_Receiver_i();
        int serviceFunction();

        // Frontend GPS Support
        frontend::GPSInfo get_gps_info(const std::string& port_name) { return _gps_info; }
		void set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info) {}
		frontend::GpsTimePos get_gps_time_pos(const std::string& port_name) { return _gps_time_pos; }
		void set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos) {}

		void start() throw(CF::Resource::StartError, CORBA::SystemException);
		void stop() throw(CF::Resource::StopError, CORBA::SystemException);

		void configure_serial_port(const char* oldValue, const char* newValue);

    protected:
        void updateUsageState();

        frontend::GPSInfo _gps_info;
        frontend::GpsTimePos _gps_time_pos;

        // File descriptor for serial stream from GPS
		// Circular buffer for NMEA messages
		// Mutex for reading the buffer
		// Worker thread instance to avoid blocking service_function
		int _gps_fd;
		circular_buffer<unsigned char> _buffer;
		mutex _bufferMutex;
		thread *_worker;

		// For NMEA
		nmeaPARSER _parser;
		nmeaINFO _bufferInfo;

		void _construct();
		void _workerFunction();
};

#endif // GPS_RECEIVER_IMPL_H
