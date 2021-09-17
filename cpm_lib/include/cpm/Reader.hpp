#pragma once

#include <iterator>
#include <mutex>
#include <vector>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderParent.hpp"

using namespace std::placeholders;

namespace cpm {
    /**
     * \class Reader
     * \brief Creates a DDS Reader that, on request, returns 
     * the newest valid received sample according to the 
     * timestamp of the DDS type T and the current time. Use this reader if 
     * a synchronous use of samples is desired, within the domain
     * of ParticipantSingleton.
     * This reader uses a buffer that holds the latest
     * samples. Any used DDS type is required to 
     * include the Header.idl: It contains timestamps that specify 
     * from when on a sample is valid (valid_after_stamp) 
     * and a stamp for when a sample was created (create_stamp). 
     * These stamps are used to find the newest 
     * sample (using create_stamp) that 
     * is valid (using valid_after_stamp) according 
     * to the current system time. Once the Reader is 
     * created, it can be used anytime to retrieve the 
     * newest valid sample, if one exist.
     * 
     * IMPORTANT: Only keeps the newest 2000 samples.
     * \ingroup cpmlib
     */
    template<typename T>
    class Reader : public ReaderParent<T>
    {
    private:    
        //! Mutex for access to get_sample and removing old messages
        std::mutex m_mutex;
        //! Internal buffer that stores flushed messages until they are (partially) removed in get_sample
        std::vector<typename T::type> messages_buffer;

        /**
         * \brief Callback that is called whenever new data is available in the DDS Reader
         * \param samples Samples read by the reader
         */
        void on_data_available(std::vector<typename T::type> &samples)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto &sample : samples)
            {
                messages_buffer.push_back(sample);
            }

            //We assume that that data becomes useless with time,
            //so it makes sense to cap the max. amount of data that can be buffered
            //In this case: 2000 messages
            if (messages_buffer.size() > 2000)
            {
                auto diff = messages_buffer.size() - 2000;
                messages_buffer.erase(messages_buffer.begin(), messages_buffer.begin() + diff);
            }
        }

        /**
         * \brief Remove all old messages since the last call to get_samples in the data structure
         * \param current_newest_sample The currently newest sample, as determined by get_newest_sample
         */
        void remove_old_msgs(const typename T::type& current_newest_sample)
        {
            //Delete all messages that are older than the currently newest sample
            //Take a look at the create_stamp only for this
            //We do this because we do not need these messages anymore, and as they take up space
            auto it = messages_buffer.begin();
            while (it != messages_buffer.end())
            {
                auto& msg = *it;
                //Remove the sample only if the currently newest sample is newer regarding its creation
                if (msg.header().create_stamp().nanoseconds() < current_newest_sample.header().create_stamp().nanoseconds())
                {
                    //Remove the msg, get a new iterator to the next position to proceed
                    it = messages_buffer.erase(it);
                }
                else
                {
                    //Get the iterator to the next position / proceed to check the age of the next element
                    ++it;
                }
            }
        }

        /**
         * \brief A function that determines the currently newest sample in the buffer
         * Newest means: Sample with the highest create_stamp that is already valid according to t_now
         * \param t_now Used to determine which samples are already valid
         * \param sample_out Return value, either initialized with zeros if no samples exist, else the currently newest sample in the buffer
         * \param sample_age_out Return value, age of the returned sample (t_now - create_stamp)
         */
        void get_newest_sample(const uint64_t t_now, typename T::type& sample_out, uint64_t& sample_age_out)
        {
            sample_out = typename T::type();
            sample_out.header().create_stamp().nanoseconds(0);
            sample_age_out = t_now;

            // select sample
            for (auto& current_sample : messages_buffer)
            {
                if(current_sample.header().valid_after_stamp().nanoseconds() > t_now) 
                {
                    // Data is "in the future", ignore for now
                    continue;
                }

                if(sample_out.header().create_stamp().nanoseconds() <= current_sample.header().create_stamp().nanoseconds())
                {
                    // Current sample has a higher timestamp, it is newer. Use it.
                    sample_out = current_sample;
                    sample_age_out = t_now - sample_out.header().valid_after_stamp().nanoseconds();
                }
            }
        }

    public:
        Reader(const Reader&) = delete;
        Reader& operator=(const Reader&) = delete;
        Reader(const Reader&&) = delete;
        Reader& operator=(const Reader&&) = delete;
    public:
        /**
         * \brief Constructor using a topic to create a Reader
         * \param topic_name the topic of the communication
         * \param vehicle_id_filter Filter by vehicle ID (if > 0) - only messages with this ID are then passed through
         * \return The DDS Reader
         */
        Reader(std::string topic_name, uint8_t vehicle_id_filter = 0):
          ReaderParent<T>(
            std::bind(&Reader::on_data_available, this, _1),
            ParticipantSingleton::Instance(),
            topic_name, 
            false,
            true,
            false,
            vehicle_id_filter)
        {
            typename T::type topic_type;
            //assert(typeof(topic_type._header().create_stamp().nanoseconds()) == uint64_t);
            static_assert(std::is_same<decltype(topic_type.header().create_stamp().nanoseconds()), uint64_t&>::value, "IDL type must have a Header.");
        }

        /**
         * \brief Returns all samples of the reader, without destroying them (you can still use get_sample afterwards)
         * Should not be used. Is only present for test purposes.
         */
        std::vector<typename T::type> get_all_samples(){
            std::lock_guard<std::mutex> lock(m_mutex);

            return messages_buffer;
        }
             
        /**
         * \brief get the newest valid sample that was received by the reader
         * \param t_now current system time / function call time in nanoseconds
         * \param sample_out the new sample, if one exists
         * \param sample_age_out the age of the returned sample in nanoseconds
         * \return This function does not directly return the sample, it is returned via the parameters
         * This function iterates through all recently received samples (in the buffer) and uses t_now to find out which samples are already valid. Of these samples, the newest one is chosen and returned via the parameters.
         */
        void get_sample(const uint64_t t_now, typename T::type& sample_out, uint64_t& sample_age_out)
        {
            //Lock mutex to make whole get_sample function thread safe
            std::lock_guard<std::mutex> lock(m_mutex);
            
            get_newest_sample(t_now, sample_out, sample_age_out);

            //Delete samples that are older than the selected sample (regarding valid_after)
            //TODO: At reviewer: Should messages that are too old regarding their creation stamp be deleted as well?
            //      If so: A 'timeout' for this could be set in the constructor
            remove_old_msgs(sample_out);
        }
    };

}