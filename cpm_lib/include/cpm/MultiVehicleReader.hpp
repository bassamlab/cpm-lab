// MIT License
// 
// Copyright (c) 2020 Lehrstuhl Informatik 11 - RWTH Aachen University
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// This file is part of cpm_lab.
// 
// Author: i11 - Embedded Software, RWTH Aachen University

#pragma once

#include <mutex>
#include <array>
#include <vector>
#include <map>
#include <algorithm>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderParent.hpp"

using namespace std::placeholders;

namespace cpm
{
    /**
     * \brief Class MultiVehicleReader
     * Use this to get a reader for multiple vehicles that works like "Reader", but checks timestamps in the header for all of the vehicles separately
     * This reader always acts in the domain of ParticipantSingleton
     * \ingroup cpmlib
     */
    template<typename T>
    class MultiVehicleReader : public ReaderParent<T>
    {
    private:
        //! Internal mutex for get_samples and copy constructor
        std::mutex m_mutex;
        //! Used as buffer to store vehicle data for each vehicle seperately, gets filled in flush_dds_reader and (partially) cleared in get_samples
        std::vector<std::vector<typename T::type>> vehicle_buffers;
        //! Vehicle IDs to listen for
        std::vector<uint8_t> vehicle_ids;

        /**
         * \brief Callback that is called whenever new data is available in the DDS Reader
         * \param samples Samples read by the reader
         */
        void on_data_available(std::vector<typename T::type> &samples)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto& data : samples)
            {
                uint8_t vehicle = data.vehicle_id();
                long pos = std::distance(vehicle_ids.begin(), std::find(vehicle_ids.begin(), vehicle_ids.end(), vehicle));

                if (pos < static_cast<long>(vehicle_ids.size()) && pos >= 0) {
                    vehicle_buffers.at(pos).push_back(data);
                }
            }
        }

    public:
        /**
         * \brief Constructor
         * \param topic the topic of the communication
         * \param num_of_vehicles The number of vehicles to monitor / read from (from 1 to num_vehicles)
         * \return The MultiVehicleReader, which only keeps the last 2000 msgs for better efficiency (might need to be tweaked)
         */
        MultiVehicleReader(std::string topic, int num_of_vehicles) : 
          ReaderParent<T>(std::bind(&MultiVehicleReader::on_data_available, this, _1), cpm::ParticipantSingleton::Instance(), topic, false, true, false)
        { 
            //Set size for buffers
            vehicle_buffers.resize(num_of_vehicles);

            //Also: Create vehicle id list from 1 to num_of_vehicles
            for (long pos = 0; pos < static_cast<long>(num_of_vehicles); ++pos) {
                vehicle_ids.push_back(pos + 1);
            }
        }

        /**
         * \brief Constructor
         * \param topic the topic of the communication
         * \param _vehicle_ids List of vehicles to monitor / read from
         * \return The MultiVehicleReader, which only keeps the last 2000 msgs for better efficiency (might need to be tweaked)
         */
        MultiVehicleReader(std::string topic, std::vector<uint8_t> _vehicle_ids) : 
          ReaderParent<T>(std::bind(&MultiVehicleReader::on_data_available, this, _1), cpm::ParticipantSingleton::Instance(), topic, false, true, false)
        {             
            //Set size for buffers
            int num_of_vehicles = _vehicle_ids.size();
            vehicle_buffers.resize(num_of_vehicles);

            vehicle_ids = _vehicle_ids;
        }
        
        /**
         * \brief This function returns the newest already valid samples (-> using information from the msg header, Header.idl) 
         * received from each vehicle the reader was set to receive samples from.
         * If a returned sample has a create stamp of 0, a sample age of t_now and is otherwise empty, no sample could be found for that vehicle
         * \param t_now Current time in ns since epoch
         * \param sample_out Map of samples, with vehicle_id -> message / content
         * \param sample_age_out Map of sample ages, with vehicle_id -> age of message
         */
        void get_samples(
            const uint64_t t_now, 
            std::map<uint8_t, typename T::type>& sample_out, 
            std::map<uint8_t, uint64_t>& sample_age_out
        )
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            sample_out.clear();
            sample_age_out.clear();

            for (long i = 0; i < static_cast<long>(vehicle_ids.size()); ++i) {
                typename T::type sample = typename T::type();
                sample.header().create_stamp().nanoseconds(0);
                sample_out[vehicle_ids.at(i)] = sample;
                sample_age_out[vehicle_ids.at(i)] = t_now;
            }

            // select samples
            for (long pos = 0; pos < static_cast<long>(vehicle_ids.size()); ++pos) {
                for (auto& current_sample : vehicle_buffers.at(pos))
                {
                    if(current_sample.header().valid_after_stamp().nanoseconds() > t_now) 
                    {
                        // Data is "in the future", ignore for now
                        continue;
                    }

                    if(sample_out[vehicle_ids.at(pos)].header().create_stamp().nanoseconds() 
                                     <= current_sample.header().create_stamp().nanoseconds())
                    {
                        // Current sample has a higher timestamp, it is newer. Use it.
                        sample_out[vehicle_ids.at(pos)] = current_sample;
                        sample_age_out[vehicle_ids.at(pos)] = 
                            t_now - current_sample.header().valid_after_stamp().nanoseconds();
                    }
                }
            }

            //Clear old data

            //Delete all messages that are older than the currently newest sample
            //Take a look at the create_stamp only for this
            //We do this because we do not need these messages anymore, and as they take up space
            for (long pos = 0; pos < static_cast<long>(vehicle_ids.size()); ++pos) {
                auto it = vehicle_buffers.at(pos).begin();
                while (it != vehicle_buffers.at(pos).end())
                {
                    auto& msg = *it;
                    //Remove the sample only if the currently newest sample is newer regarding its creation
                    if (msg.header().create_stamp().nanoseconds() < sample_out[vehicle_ids.at(pos)].header().create_stamp().nanoseconds())
                    {
                        //Remove the msg, get a new iterator to the next position to proceed
                        it = vehicle_buffers.at(pos).erase(it);
                    }
                    else
                    {
                        //Get the iterator to the next position / proceed to check the age of the next element
                        ++it;
                    }
                }
            }
        }
    };

}