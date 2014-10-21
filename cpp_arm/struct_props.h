#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>

struct position_struct {
    position_struct ()
    {
        latitude = 0;
        longitude = 0;
        endian_bug = 1.0;
    };

    static std::string getId() {
        return std::string("position");
    };

    double latitude;
    double longitude;
    double endian_bug;
};

inline bool operator>>= (const CORBA::Any& a, position_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("position::latitude", props[idx].id)) {
            if (!(props[idx].value >>= s.latitude)) return false;
        }
        else if (!strcmp("position::longitude", props[idx].id)) {
            if (!(props[idx].value >>= s.longitude)) return false;
        }
        else if (!strcmp("endian_bug", props[idx].id)) {
            if (!(props[idx].value >>= s.endian_bug)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const position_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("position::latitude");
    props[0].value <<= s.latitude;
    props[1].id = CORBA::string_dup("position::longitude");
    props[1].value <<= s.longitude;
    props[2].id = CORBA::string_dup("endian_bug");
    props[2].value <<= s.endian_bug;
    a <<= props;
};

inline bool operator== (const position_struct& s1, const position_struct& s2) {
    if (s1.latitude!=s2.latitude)
        return false;
    if (s1.longitude!=s2.longitude)
        return false;
    if (s1.endian_bug!=s2.endian_bug)
        return false;
    return true;
};

inline bool operator!= (const position_struct& s1, const position_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
