#include "cpm/Logging.hpp"
#include <string>

/**
 * \file Logging.cpp
 * \ingroup cpmlib
 */

namespace cpm {

    Logging::Logging(std::string _id, std::filesystem::path _logfile_path) :
    //Logging::Logging() :
        id(_id),
        logfile_path(_logfile_path),
        logger("log", true), log_level_reader(cpm::AsyncReader<LogLevelPubSubType>(
            [this](std::vector<LogLevel>& samples){
                for(auto& data : samples)
                {
                    log_level.store(data.log_level());
                }
            },
            "logLevel",
            true,
            true
        ))
    {

        //Default log level value
        log_level.store(1);

        // Formatted start time for log filename
        char time_format_buffer[100];
        {
            struct tm* tm_info;
            time_t timer;
            time(&timer);
            tm_info = gmtime(&timer);
            strftime(time_format_buffer, 100, "%Y_%m_%d_%H_%M_%S", tm_info);
        }


        filename = "Log_";
        filename += time_format_buffer;
        filename += ".csv";
        std::filesystem::create_directories(logfile_path);
        logfile_path.append(filename);
        file.open(logfile_path, std::ofstream::out | std::ofstream::trunc);
        file << "ID,Level,Timestamp,Content" << std::endl;
        file.close();
    }

    Logging& Logging::InstanceImpl(std::string _id, std::filesystem::path _logfile_path) {
        static Logging instance{_id, _logfile_path};
        return instance;
    }

    Logging& Logging::Instance() {
      return Logging::InstanceImpl();
    }

    void Logging::init(std::string _id, std::filesystem::path _logfile_path){
        Logging::InstanceImpl(_id, _logfile_path);
    }

    uint64_t Logging::get_time() {
        return cpm::get_time_ns();
    }

    void Logging::set_id(std::string _id) {
        //Mutex bc value could be set by different threads at once
        std::lock_guard<std::mutex> lock(log_mutex);

        id = _id;
    }

    std::string Logging::get_filename() {
        return filename;
    }

    std::filesystem::path Logging::get_logfile_path() {
        return logfile_path;
    }

    void Logging::check_id() {
        //Mutex bc value could be set by different threads at once (id could be set with set_id while it is read)
        std::lock_guard<std::mutex> lock(log_mutex);

        if (id == "uninitialized") {
            fprintf(stderr, "Error: Logger ID was never set\n");
            fflush(stderr); 
            exit(EXIT_FAILURE);
        }
    }

}