#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>

struct DebugFilter : public eprosima::fastdds::dds::IContentFilter, public eprosima::fastdds::dds::IContentFilterFactory
    {
        bool evaluate(
                const eprosima::fastdds::dds::IContentFilter::SerializedPayload& /*payload*/,
                const eprosima::fastdds::dds::IContentFilter::FilterSampleInfo& /*sample_info*/,
                const eprosima::fastdds::dds::IContentFilter::GUID_t& /*reader_guid*/) const override
        {
            return true;
        }

        eprosima::fastrtps::types::ReturnCode_t create_content_filter(
                const char* /*filter_class_name*/,
                const char* /*type_name*/,
                const eprosima::fastdds::dds::TopicDataType* /*data_type*/,
                const char* filter_expression,
                const eprosima::fastdds::dds::IContentFilterFactory::ParameterSeq& filter_parameters,
                eprosima::fastdds::dds::IContentFilter*& filter_instance) override
        {
            if (nullptr != filter_expression)
            {
                std::string s(filter_expression);
                if (filter_parameters.length() == std::count(s.begin(), s.end(), '%'))
                {
                    filter_instance = this;
                    return eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK;
                }
            }

            return eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        virtual eprosima::fastrtps::types::ReturnCode_t delete_content_filter(
                const char* /*filter_class_name*/,
                eprosima::fastdds::dds::IContentFilter* filter_instance) override
        {
            if (this == filter_instance)
            {
                return eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK;
            }

            return eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

    };