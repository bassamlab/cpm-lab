#pragma once

#include "cpm/dds/VehicleState.h"

namespace cpm
{
    /**
     * \class VehicleIDFilter
     * \brief Filters vehicle IDs (more or less deprecated)
     * \ingroup cpmlib
     */
    template<class T>
    struct VehicleIDFilter
    {
        //! ID to filer by
        uint8_t vehicle_id_;

        //! Constructor
        VehicleIDFilter(uint8_t vehicle_id) : vehicle_id_(vehicle_id) {}

        //! Filter function, just tells if the given value's ID matches the filter
        bool filter(typename T::type& val){
          return (val.vehicle_id() == vehicle_id_);
        }
    };
}