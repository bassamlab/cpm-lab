%% This function makes sure that the called MEX file can run out-of-process
% Except for errors or deletion, the process remains active in between calls
% You can delete the process by calling delete vehicleStateListReader.m

% To be able to call this file without errors, you have to replace Matlabs
% own glibcxx with your system's one
% cd /usr/local/MATLAB/.../sys/os/glnxa64
% sudo mkdir old
% sudo mv libstdc++.so.6* old
% sudo ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 libstdc++.so.6 

function result = systemTriggerReader(wait_infinitely_for_msg)
    persistent mh
    if ~(isa(mh,'matlab.mex.MexHost') && isvalid(mh))
        mh = mexhost;
    end
    if exist('wait_infinitely_for_msg', 'var')
        result = feval(mh,"system_trigger_reader", wait_infinitely_for_msg);
    else 
        result = feval(mh,"system_trigger_reader");
    end
end