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

//For waiting
#include <chrono>
#include <condition_variable>

#include "cpm/ReaderParent.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;

namespace cpm
{
    /**
     * \class ReaderAbstract
     * \brief Creates a DDS Reader that provides the simple take() function for getting all samples received after the last call of "take()"
     * Abstraction from different DDS Reader implementations
     * Difference to cpm::Reader: That one is supposed to give the latest sample w.r.t. timing information in the header. ReaderAbstract works more general than that.
     * \ingroup cpmlib
     */
    template<typename T>
    class ReaderAbstract : public ReaderParent<T>
    {
    private:   
        //! Mutex for access to get_sample and removing old messages
        std::mutex m_mutex;
        //! Internal buffer that stores flushed messages until they are (partially) removed in get_sample
        std::vector<typename T::type> messages_buffer;

        //! Condition variable for waiting for new data
        std::condition_variable cv;

        //! Must be remembered internally to know if the buffer used must be reset when new data is taken from the internal DDS reader
        bool history_keep_all;

        /**
         * \brief Callback that is called whenever new data is available in the DDS Reader
         * \param samples Samples read by the reader
         */
        void on_data_available(std::vector<typename T::type> &samples)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (! history_keep_all)
                messages_buffer.clear();

            for (auto &sample : samples)
            {
                messages_buffer.push_back(sample);
            }

            cv.notify_all();
        }


    public:
        ReaderAbstract(const ReaderAbstract&) = delete;
        ReaderAbstract& operator=(const ReaderAbstract&) = delete;
        ReaderAbstract(const ReaderAbstract&&) = delete;
        ReaderAbstract& operator=(const ReaderAbstract&&) = delete;
        
        /**
         * \brief Constructor for a ReaderAbstract which is communicating within the ParticipantSingleton
         * Allows to set the topic name and some QoS settings
         * \param topic Name of the topic to read in
         * \param reliable Set the reader to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default)
         * \param transient_local Receive messages sent before joining (true) or not (false, default)
         */
        ReaderAbstract(std::string topic, bool reliable = false, bool history_keep_all = false, bool transient_local = false)
        : ReaderAbstract(cpm::ParticipantSingleton::Instance(), topic, reliable, history_keep_all, transient_local)
        {
        }

        /**
         * \brief Constructor for a ReaderAbstract that communicates within another domain
         * Allows to set the topic name and some QoS settings
         * \param _participant The domain (participant) in which to read
         * \param topic Name of the topic to read in
         * \param reliable Set the reader to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default)
         * \param transient_local Receive messages sent before joining (true) or not (false, default)
         */
        ReaderAbstract(
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> _participant, 
            std::string topic, 
            bool reliable = false, 
            bool _history_keep_all = false, 
            bool transient_local = false
        ) : 
          ReaderParent<T>(std::bind(&ReaderAbstract::on_data_available, this, _1), _participant, topic, reliable, _history_keep_all, transient_local),
          history_keep_all(_history_keep_all)
        {}
        
        /**
         * \brief Get the received messages
         */
        std::vector<typename T::type> take()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto return_buffer = messages_buffer;
            messages_buffer.clear();

            return return_buffer;
        }

        /**
         * \brief Waits for unread messages up to timeout_ms milliseconds.
         * Custom implementation of the eProsma equivalent, 
         * due to different usage of the reader internally.
         * \param timeout_ms Max. time in milliseconds to wait until return
         * \return True if new data is available, else false
         */
        bool wait_for_unread_message(unsigned int timeout_ms)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            cv.wait_for(lock, timeout_ms * 1ms, [this] 
              {
                  //In case of spurious wake up, check if it should still be waiting
                  return messages_buffer.size() > 0;
              }
            );

            //After timeout or wait exits: Return if messages were actually received
            return messages_buffer.size() > 0;
        }
    };
}