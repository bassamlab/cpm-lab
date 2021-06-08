%% This function makes sure that the called MEX file can run out-of-process
% Except for errors or deletion, the process remains active in between calls
% You can delete the process by calling delete vehicleStateListReader.m
function result = vehicleStateListReader()
    persistent mh
    if ~(isa(mh,'matlab.mex.MexHost') && isvalid(mh))
        mh = mexhost;
    end
    result = feval(mh,"vehicle_state_list_reader");
end