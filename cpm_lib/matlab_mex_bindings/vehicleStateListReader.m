%% This function makes sure that the called MEX file can run out-of-process
% Except for errors or deletion, the process remains active in between calls
% You can delete the process by calling delete vehicleStateListReader.m

% To be able to call this file without errors, you have to replace Matlabs
% own glibcxx with your system's one
% cd /usr/local/MATLAB/.../sys/os/glnxa64
% sudo mkdir old
% sudo mv libstdc++.so.6* old
% sudo ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 libstdc++.so.6 

function result = vehicleStateListReader(wait_timeout_seconds)
    persistent mh
    if ~(isa(mh,'matlab.mex.MexHost') && isvalid(mh))
        mh = mexhost;
    end
    if exist('wait_timeout_seconds', 'var')
        result = feval(mh,"vehicle_state_list_reader", wait_timeout_seconds);
    else 
        result = feval(mh,"vehicle_state_list_reader");
    end
end