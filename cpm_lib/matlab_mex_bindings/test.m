% MIT License
% 
% Copyright (c) 2020 Lehrstuhl Informatik 11 - RWTH Aachen University
% 
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
% 
% The above copyright notice and this permission notice shall be included in all
% copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
% SOFTWARE.
% 
% This file is part of cpm_lab.
% 
% Author: i11 - Embedded Software, RWTH Aachen University

function main(vehicle_id)
    % Get current path
    clc
    script_directoy = fileparts([mfilename('fullpath') '.m']);

    % Add all libraries to the path
    setenv("LD_RUN_PATH", [getenv('LD_RUN_PATH'), ':/home/leon/dev/software/cpm_lib/build/', ':/usr/local/lib/']);
    setenv("LD_LIBRARY_PATH", [getenv('LD_LIBRARY_PATH'), ':/home/leon/dev/software/cpm_lib/build/', ':/usr/local/lib/']);
    % disp(getenv('LD_LIBRARY_PATH'));

    %%WARNING: THIS DOES NOT WORK - INSTEAD, CALL THIS BEFORE STARTING MATLAB (replace with your own file locations)
    %export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6:/home/leon/dev/software/cpm_lib/build/libcpm.so:/usr/local/lib/libfastcdr.so::/usr/local/lib/libfastrtps.so"

    % disp('Calling init.');
    % mex_test_eprosima('create');

    % disp('Waiting...');
    % pause(1);

    % disp('Calling write');
    % mex_test_eprosima('write');

    % disp('Calling delete');
    % mex_test_eprosima('delete');

    ready_status = ReadyStatus('hlc_1', 2021);

    % Testing the ready signal
    ready_status_writer(ready_status);

    % What happens if no system trigger was yet sent?
    % system_trigger = SystemTrigger;
    % system_trigger = system_trigger_reader(system_trigger);
    % disp(system_trigger);
    % -> As you can see: I added a value called is_valid to the system trigger, which is false if no msg was received

    % Test if the specified data type works
    state_list = VehicleStateList;

    % Now wait for a msg
    % system_trigger = SystemTrigger;
    % system_trigger = system_trigger_reader(system_trigger, true);
    % disp(system_trigger);

    % Clear mex files etc. from system memory
    % Else: The transient local ready signal etc. are still being sent
    clear ready_status_writer
    clear system_trigger_reader
end