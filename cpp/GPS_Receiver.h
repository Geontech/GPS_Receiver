#ifndef GPS_RECEIVER_IMPL_H
#define GPS_RECEIVER_IMPL_H

#include "GPS_Receiver_base.h"

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

		void configure_position(const position_struct* oldValue, const position_struct* newValue);

    protected:
        void updateUsageState();

        frontend::GPSInfo _gps_info;
        frontend::GpsTimePos _gps_time_pos;
};

#endif // GPS_RECEIVER_IMPL_H
