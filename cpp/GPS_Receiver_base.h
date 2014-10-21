#ifndef GPS_RECEIVER_IMPL_BASE_H
#define GPS_RECEIVER_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>

#include <frontend/frontend.h>
#include "struct_props.h"

class GPS_Receiver_base : public Device_impl, protected ThreadedComponent, public virtual frontend::gps_delegation
{
    public:
        GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPS_Receiver_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        float endian_bug;
        position_struct position;

        // Ports
        frontend::InGPSPort *gps;

    private:
        void construct();
};
#endif // GPS_RECEIVER_IMPL_BASE_H
