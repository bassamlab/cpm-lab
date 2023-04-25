#pragma once
#include <cstddef>
#include <filesystem>
namespace cpm {
    struct Constants{
        /**
        * \brief Maximum number of vehicles.
        * \ingroup cpm
        */
        static const size_t MAX_NUM_VEHICLES = 30;
        inline static const std::filesystem::path CPM_LOG_PATH{"/tmp/cpm_lab_recordings/"};
    };
}