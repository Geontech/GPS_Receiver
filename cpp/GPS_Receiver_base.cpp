#include "GPS_Receiver_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the device class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

GPS_Receiver_base::GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl),
    ThreadedComponent()
{
    construct();
}

GPS_Receiver_base::GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    ThreadedComponent()
{
    construct();
}

GPS_Receiver_base::GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    ThreadedComponent()
{
    construct();
}

GPS_Receiver_base::GPS_Receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    ThreadedComponent()
{
    construct();
}

GPS_Receiver_base::~GPS_Receiver_base()
{
    delete gps;
    gps = 0;
}

void GPS_Receiver_base::construct()
{
    loadProperties();

    gps = new frontend::InGPSPort("gps", this);
    addPort("gps", gps);

}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void GPS_Receiver_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Device_impl::start();
    ThreadedComponent::startThread();
}

void GPS_Receiver_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Device_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void GPS_Receiver_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the device running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Device_impl::releaseObject();
}

void GPS_Receiver_base::loadProperties()
{
    addProperty(device_kind,
                "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                "device_kind",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(device_model,
                "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                "device_model",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(stream_id,
                "stream_id",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(serial_port,
                "/dev/ttyUSB0",
                "serial_port",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


