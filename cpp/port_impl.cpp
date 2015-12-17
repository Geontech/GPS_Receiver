/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    Source: gps_receiver.spd.xml

*******************************************************************************************/

#include "gps_receiver.h"

// ----------------------------------------------------------------------------------------
// FRONTEND_GPS_In_i definition
// ----------------------------------------------------------------------------------------
FRONTEND_GPS_In_i::FRONTEND_GPS_In_i(std::string port_name, gps_receiver_base *_parent) : 
Port_Provides_base_impl(port_name)
{
    parent = static_cast<gps_receiver_i *> (_parent);
}

FRONTEND_GPS_In_i::~FRONTEND_GPS_In_i()
{
}

FRONTEND::GPSInfo* FRONTEND_GPS_In_i::gps_info()
{
    boost::mutex::scoped_lock lock(portAccess);
    FRONTEND::GPSInfo* tmpVal = this->parent->gpsInfo();

    return tmpVal;
}

void FRONTEND_GPS_In_i::gps_info(const FRONTEND::GPSInfo &data)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}

FRONTEND::GpsTimePos* FRONTEND_GPS_In_i::gps_time_pos()
{
    boost::mutex::scoped_lock lock(portAccess);
    FRONTEND::GpsTimePos* tmpVal = this->parent->gpsTimePos();

    return tmpVal;
}

void FRONTEND_GPS_In_i::gps_time_pos(const FRONTEND::GpsTimePos &data)
{
    boost::mutex::scoped_lock lock(portAccess);
    // TODO: Fill in this function
}


