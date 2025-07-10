# CPM Lab
This repository contains source code for the [CPM Lab](https://cpm.lrt.unibw.de), an open source testbed for connected and automated vehicles. 
The CPM Lab provides a simulation environment and 20 scale-model vehicles for experiments. The software architecture enables the seamless transfer of algorithms tested in simulation to real experiments without any adaptions. Experiments with the 20 vehicles can be extended by unlimited additional simulated vehicles. A Data Distribution Service (DDS) based middleware is responsible for synchronizing all entities and establishes a logical execution time approach. It further allows to change the number of vehicles during experiments. \
The CPM Lab lets researchers as well as students from different disciplines see their ideas develop into reality.
More information is provided in our publication [1], which we kindly ask you to consider citing if you find the CPM Lab helpful for your work.

## Switching between the RTI and FastDDS Versions

The ``install.sh`` script in both RTI and FastDDS versions of the CPM Lab set
some required, but different environment variables in a source file.
When switching back from FastDDS to RTI this can lead to unexpected and hard to debug errors.
Running ``sudo install.sh`` would fix this, but each install takes quite a bit of time.

Contained is a ``quick_install.sh`` script, that sets these variables,
but does not repeat long-running steps of the installer.
Thus, when the CPM Lab is not building properly after switching back from FastDDS,
it is recommended to run ``sudo bash quick_install.sh``, specify the RTI license file,
specify domain ID (usually 21), reboot the computer and then run ``bash build_all.bash``.

## Documentation
The [documentation](https://cpm.lrt.unibw.de/doc/) describes how to set up the simulation environment of the lab and how to set up the complete lab hardware. It also explains its usage with tutorials and documents its components.

## References
1.	M. Kloock, P. Scheffe, O. Greß, and B. Alrifaee, “An Architecture for Experiments in Connected and Automated Vehicles,” IEEE Open Journal of Intelligent Transportation Systems, pp. 1–1, 2023, doi: 10.1109/OJITS.2023.3250951.\
https://youtu.be/PfM0qdzorCc \
https://cpm.lrt.unibw.de/architecture/ 

For a full list of publications related to the CPM Lab see https://cpm.lrt.unibw.de/publications.
