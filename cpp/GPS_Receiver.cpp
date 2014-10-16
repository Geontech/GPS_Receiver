/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "GPS_Receiver.h"

PREPARE_LOGGING(GPS_Receiver_i)

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl)
{
	this->addPropertyChangeListener(
			"position",
			this,
			&GPS_Receiver_i::configure_position);
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
	this->addPropertyChangeListener(
			"position",
			this,
			&GPS_Receiver_i::configure_position);
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
	this->addPropertyChangeListener(
			"position",
			this,
			&GPS_Receiver_i::configure_position);
}

GPS_Receiver_i::GPS_Receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    GPS_Receiver_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	this->addPropertyChangeListener(
			"position",
			this,
			&GPS_Receiver_i::configure_position);
}

GPS_Receiver_i::~GPS_Receiver_i()
{
}

void GPS_Receiver_i::configure_position(
		const position_struct* oldValue,
		const position_struct* newValue) {
	this->_gps_time_pos.position.valid = true;
	this->_gps_time_pos.position.lat = newValue->latitude;
	this->_gps_time_pos.position.lon = newValue->longitude;
	this->_gps_info.satellite_count = 42;
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
	this->_gps_info.timestamp = bulkio::time::utils::now();
	this->_gps_time_pos.timestamp = bulkio::time::utils::now();
    return NOOP;
}
