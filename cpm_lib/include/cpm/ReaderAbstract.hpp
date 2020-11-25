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

#include "cpm/AsyncReader.hpp"

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
    class ReaderAbstract : public eprosima::fastdds::dds::DataReaderListener
    {
    private:
        AsyncReader<T>* reader;

        //! Internal DDS reader that is abstracted by this class
        dds::sub::DataReader<T> dds_reader;

        void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;

        // this is actually never called...fix Writer API
        static void dummyCallback(std::vector<typename T::type> trigger){}


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
        {
            dds::domain::DomainParticipant& p = cpm::ParticipantSingleton::Instance();
            ReaderAbstract(p, topic, reliable, history_keep_all, transient_local); 
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
            dds::domain::DomainParticipant & _participant, 
            std::string topic, 
            bool reliable = false, 
            bool history_keep_all = false, 
            bool transient_local = false
        )
        {
          reader = new cpm::AsyncReader<T>(&dummyCallback, _participant, topic, reliable, transient_local, history_keep_all, this);
        }
        
        /**
         * \brief Get the received messages
         */
        std::vector<typename T::type> take()
        {
            //Only take() could be a cause for not being thread-safe, but the DDS APIs should be implemented thread-safe (is the case for RTI DDS)
            std::vector<typename T::type> samples;

            eprosima::fastdds::dds::SampleInfo info;
            typename T::type data;
            while(reader->get_reader()->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK) {
              if (info.instance_state == eprosima::fastdds::dds::ALIVE)
              {
                samples.push_back(data);
              }
            }
            return samples;
        }

        /**
         * \brief Returns # of matched writers, needs template parameter for topic type
         */
        size_t matched_publications_size()
        {
            return 0;
            //auto matched_pub = dds::sub::matched_publications(dds_reader);
            //return matched_pub.size();
        }
    };
}