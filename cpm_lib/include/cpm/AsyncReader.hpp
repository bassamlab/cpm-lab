#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <functional>
#include <vector>
#include <future>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Participant.hpp"
#include "cpm/ReaderParent.hpp"

/**
 * \file AsyncReader.hpp
 */

namespace cpm 
{
    /**
     * \class AsyncReader
     * \brief This class is a wrapper for a data reader that uses an AsyncWaitSet to call a callback function whenever any new data is available
     * Template: Class of the message objects, depending on which IDL file is used
     * \ingroup cpmlib
     */ 
    template<class MessageType> 
    class AsyncReader
    {
    private:
        //! Internal Reader class that takes care of must of the eProsima initialization. Some issues arised when using inheritance w.r.t. destruction order, although they should be fixed now.
        std::shared_ptr<cpm::ReaderParent<MessageType>> reader_parent;
    public:
        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false
        );

        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param participant Domain participant to specify in which domain the reader should operate
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback,
            cpm::Participant& participant, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false
        );

        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param participant Domain participant to specify in which domain the reader should operate
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant,
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false
        );

        /**
         * \brief Returns # of matched writers
         */
        size_t matched_publications_size()
        {
            return reader_parent->matched_publications_size();
        }
    };

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local
    ) : AsyncReader(on_read_callback, cpm::ParticipantSingleton::Instance(), topic_name, is_reliable, is_transient_local)
    {}

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        cpm::Participant& participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local
    ) :  AsyncReader(on_read_callback, participant.get_participant(), topic_name, is_reliable, is_transient_local) {}

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local
    )
    {
        reader_parent = std::make_shared<cpm::ReaderParent<MessageType>>(on_read_callback, participant, topic_name, is_reliable, false, is_transient_local);
    }
}