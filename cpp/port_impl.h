#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

class gps_receiver_base;
class gps_receiver_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include <FRONTEND/GPS.h>

// ----------------------------------------------------------------------------------------
// FRONTEND_GPS_In_i declaration
// ----------------------------------------------------------------------------------------
class FRONTEND_GPS_In_i : public POA_FRONTEND::GPS, public Port_Provides_base_impl
{
    public:
        FRONTEND_GPS_In_i(std::string port_name, gps_receiver_base *_parent);
        ~FRONTEND_GPS_In_i();

        FRONTEND::GPSInfo* gps_info();
        void gps_info(const FRONTEND::GPSInfo &data);
        FRONTEND::GpsTimePos* gps_time_pos();
        void gps_time_pos(const FRONTEND::GpsTimePos &data);

    protected:
        gps_receiver_i *parent;
        boost::mutex portAccess;
};
#endif
