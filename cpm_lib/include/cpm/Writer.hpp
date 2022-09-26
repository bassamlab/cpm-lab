#pragma once

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <memory>

#include "cpm/ParticipantSingleton.hpp"

// #include <experimental/filesystem>

namespace cpm
{
    /**
     * \class Writer
     * \brief Creates a DDS Writer that can be used for writing / publishing messages
     * This encapsulation allows for changes e.g. in the participant or QoS without 
     * the need to change the implementation across the whole project
     * \ingroup cpmlib
     */
    template<class T>
    class Writer
    {
    private:
        //! Data type of the message / used for the topic
        T topic_data_type;
        //! eProsima type support to create the right topic type
        eprosima::fastdds::dds::TypeSupport type_support;
        //! DDS publisher to create the writer on
        std::shared_ptr<eprosima::fastdds::dds::Publisher> publisher;
        //! Topic to communicate over
        std::shared_ptr<eprosima::fastdds::dds::Topic> topic;
        //! Actual internal writer for sending messages over the given topic within the domain of the given participant
        std::shared_ptr<eprosima::fastdds::dds::DataWriter> writer;
        //! DDS participant, defines the domain of messages (and also some QoS like shared memory usage)
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;

        /**
         * \brief Class used to count matched subscribers
         */
        class PubListener : public eprosima::fastdds::dds::DataWriterListener
        {
        public:
            //! Constructor
            PubListener()
                : matched_(0)
            {
            }

            //! Destructor
            ~PubListener() override
            {
            }

            /**
             * \brief Whenever a new publication gets (un)matched, the match count gets updated
             */
            void on_publication_matched(
                    eprosima::fastdds::dds::DataWriter*,
                    const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
            {
                matched_ = info.total_count;
            }

            //! Counts the amount of currently matched publications
            std::atomic_int matched_;

        } listener_; //!< Listener to count currently matched subscripers

        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        Writer(const Writer&&) = delete;
        Writer& operator=(const Writer&&) = delete;

        /**
         * \brief Returns qos for the settings s.t. the constructor becomes more readable
         * \param is_reliable For reliable (true) or best effort (false)
         * \param history_keep_all Keep all samples in the reader until takes gets called (true) or only the last one (false)
         * \param is_transient_local Resend older data to newly joined participants (true) or don't (false)
         * \param is_data_sharing Use shared memory communication (true) or (false)
         */
        eprosima::fastdds::dds::DataWriterQos get_qos(bool is_reliable, bool history_keep_all, bool is_transient_local, bool is_data_sharing)
        {
            auto qos = eprosima::fastdds::dds::DataWriterQos();


            if (is_reliable)
            {
                auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
                qos.reliability(policy);
            }
            else
            {
                //Already implicitly given
                auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
                qos.reliability(policy);
            }

            if (history_keep_all)
            {
                auto policy = eprosima::fastdds::dds::HistoryQosPolicy();
                policy.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
                qos.history(policy);
            }else{
                auto policy = eprosima::fastdds::dds::HistoryQosPolicy();
                policy.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
                qos.history(policy);  
            }

            if (is_transient_local)
            {
                auto policy = eprosima::fastdds::dds::DurabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
                qos.durability(policy);
            }else{
                auto policy = eprosima::fastdds::dds::DurabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
                qos.durability(policy);
            }
            // Can be used to enforce writer side contentfiltering (https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/contentFilteredTopic/writerFiltering.html)
            if(!is_data_sharing){
                auto policy = eprosima::fastdds::dds::DataSharingQosPolicy();
                policy.off();
                qos.data_sharing(policy);
            }

            return qos;
        }


    public:

        /**
         * \brief Constructor for a writer which is communicating within the ParticipantSingleton
         * Allows to set the topic name and some QoS settings
         * \param topic Name of the topic to write in
         * \param reliable Set the writer to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default) (relevant for the reader)
         * \param transient_local Resent messages sent before a new participant joined to that participant (true) or not (false, default)
         */
        Writer(std::string topic_name, bool reliable = false, bool history_keep_all = false, bool transient_local = false)
        : Writer(ParticipantSingleton::Instance(), topic_name, reliable, history_keep_all, transient_local)
        {       
        }

        /**
         * \brief Constructor for a writer that communicates within another domain
         * Allows to set the topic name and some QoS settings
         * \param _participant The domain (participant) in which to write
         * \param topic Name of the topic to write in
         * \param reliable Set the writer to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default) (relevant for the reader)
         * \param transient_local Resent messages sent before a new participant joined to that participant (true) or not (false, default)
         */
        Writer(
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> _participant, 
            std::string topic_name, 
            bool reliable = false, 
            bool history_keep_all = false, 
            bool transient_local = false,
            bool data_sharing = true
        ) : type_support(new T()), participant_(_participant)
        {
            std::cout << "Creating Writer " << topic_name << " : " << topic_data_type.getName() << std::endl;

            // Check if Type is already registered, create type
            auto find_type_ret = participant_->find_type(topic_data_type.getName());
            std::cout << "Checking if type exists: " << topic_data_type.getName() << std::endl;
            if(find_type_ret.empty()){
                std::cout << "Type does not exist, creating type" << std::endl;
                auto ret = type_support.register_type(participant_.get());
                assert(ret == eprosima::fastdds::dds::TypeSupport::ReturnCode_t::RETCODE_OK);
            }

            assert(participant_->find_type(topic_data_type.getName()).empty() == false);
            std::cout << "Double check: " << participant_->find_type(topic_data_type.getName()).get_type_name() << std::endl;

            // Create Topic
            auto find_topic = participant_->lookup_topicdescription(topic_name);
            if(find_topic == nullptr){
                std::string type_name_str = topic_data_type.getName();
                topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
                    participant_->create_topic(topic_name, type_name_str, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT),
                    [&](eprosima::fastdds::dds::Topic* topic) {
                        if (topic != nullptr)
                        {
                            participant_->delete_topic(topic);
                        }
                    }
                );
            }else{
                topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
                    (eprosima::fastdds::dds::Topic*)find_topic,
                    [&](eprosima::fastdds::dds::Topic* topic) {
                        if (topic != nullptr)
                        {
                            participant_->delete_topic(topic);
                        }
                    }
                );
            }

            std::cout << "Double check topic " <<  topic->get_type_name() << " " << topic->get_name() << std::endl;
            assert(topic);
            assert(participant_->find_type(topic->get_type_name()).empty() == false);

            //Create Publisher
            publisher = std::shared_ptr<eprosima::fastdds::dds::Publisher>(
                participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT),
                [&](eprosima::fastdds::dds::Publisher* publisher) {
                    if (publisher != nullptr)
                    {
                        participant_->delete_publisher(publisher);
                    }
                }
            );

            //Create Writer
            writer = std::shared_ptr<eprosima::fastdds::dds::DataWriter>(
                publisher->create_datawriter(topic.get(), get_qos(reliable, history_keep_all, transient_local, data_sharing), &listener_),
                [&](eprosima::fastdds::dds::DataWriter* writer) {
                    if (writer != nullptr)
                    {
                        publisher->delete_datawriter(writer);
                    }
                }
            );

            assert(writer);                      
        }
        
        /**
         * \brief Send a message in the DDS network using the writer
         * \param msg The message to send
         */
        void write(typename T::type& msg)
        {
            //DDS operations are assumed to be thread safe, so don't use a mutex here
            writer->write(&msg);
        }

        /**
         * \brief Returns number of currently matched readers
         */
        size_t matched_subscriptions_size()
        {
            return static_cast<size_t>(listener_.matched_.load());
        }

        /**
         * \brief Sets a maximum blocking time for the write operation. Only in effect when compiled with -DSTRICT_REALTIME.
         * 
         * The QOS of the wrapped DataWriter in gets updated.
         * This affects the writing operation `write()`.
         * 
         * For further reference see https://fast-dds.docs.eprosima.com/en/latest/fastdds/use_cases/realtime/blocking.html.
         * \param _max_blocking_time The maximum time the writing operation blocks.
         */
        void max_blocking(eprosima::fastrtps::Time_t _max_blocking_time){
            eprosima::fastdds::dds::DataWriterQos data_writer_qos = writer->get_qos();
            auto reliable_policy = data_writer_qos.reliability();
            
            reliable_policy.max_blocking_time = _max_blocking_time;
            data_writer_qos.reliability(reliable_policy);
            writer->set_qos(data_writer_qos);
        }
    };
}