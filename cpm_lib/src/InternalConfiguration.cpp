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

#include "cpm/InternalConfiguration.hpp"
#include "cpm/init.hpp"
#include "cpm/CommandLineReader.hpp"
#include "cpm/Logging.hpp"
#include "cpm/RTTTool.hpp"

/**
 * \file InternalConfiguration.cpp
 * \ingroup cpmlib
 */

namespace cpm
{
    void init(int argc, char *argv[])
    {
        InternalConfiguration::init(argc, argv);
    }


    void InternalConfiguration::init(int argc, char *argv[])
    {
        InternalConfiguration::the_instance = InternalConfiguration(
            cmd_parameter_int("dds_domain", 0, argc, argv),
            cmd_parameter_string("logging_id", "uninitialized", argc, argv),
            cmd_parameter_string("dds_initial_peer", "", argc, argv)
        );

        // TODO reverse access, i.e. access the config from the logging
        cpm::Logging::Instance().set_id(InternalConfiguration::Instance().get_logging_id());
    }


    InternalConfiguration InternalConfiguration::the_instance;
}