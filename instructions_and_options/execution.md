# Execution

This document only relates to the execution of this version of OpenFOAM. For further details, the reasons for using HDF5 etc. we refer the user to the document [technical\_information](https://github.com/stefsal/OeRC_OpenFOAM_HDF5/blob/master/technical_information.md) in this repository.

## Execution Modes

For ease of editing (restart) internal and boundary values are now output in separate files.  The reason is that the interior values set could be very large and, in any case, far larger than the boundary values. Current editing of standard OpenFOAM files can be awkward and very time consuming.  The interior and boundary values files can be trivially concatenated: a Python simple code is provided to do so.

This version of OpenFOAm allows to output data to:

* standard OpenFOAM files
* HDF5 files
* both standard OpenFOAM files and HDF5 files (not yet implemented)

## Output structure

Notice that an HDF5 file is not necessarily a file in the normal sense: users can access it as a normal file, through the appropriate HDF5 APIs, but it consists of anumber of "stprage units", called datasets.  Datasets can be collected in groups, and hierarchies of groups can be defined.

In the case of this version of OpenFOAM, the following has been adopted:
1. The top level of the datasets structure is the processor (processor 0, etc.)
1. The names of the datasets are the same as the names of the original OpenFOAM files.
1. The contents of the datasets are byte-identical to the contents of the corresponding OpenFOAM files.
1. Each HDF5 file name includes the time of the dump.

This allows time dumps to be removed easily and cleanly by just deleteing the appropriate hdf5 file.

Notice that if datasets were to be removed from an HDF5 file, reclaiming free space is far from trivial, requiring a full HDF5 file rewrite. The same applies to other large parallel storage systems.  That is the reason to invert the order processor/time dump from the original OpenfFOAM structure.

## Execution and User's Options (via Environment Variables)

Once this version of OpenFOAM has been compiled, the user executes it as ihe/she always did, no --changes whatsoever__.  If the user decided to run OpenFOAM without employing the HDF5 facilities, then this version would have the standard OpenFOAM behaviour.

For reasons discussed [elsewhere](https://github.com/stefsal/OeRC_OpenFOAM_HDF5/blob/master/technical_information.md),the user can determine OpenFOAM output by means of environment variables. 
__The user must make sure that the environment variables are set for all processes.__ The ways to do so vary may be local to the specific HPC system, scheduler etc. and we cannot provide here specific information

Two environment variables can be used.  The first defines whether HDF5 is used or standard OpenFOAM:

| __OF\_OUTPUT\_H5__ |  Description | Status
|:--------------:|:------------ |:------:|
| 0 | Use standard OpenFOAM files | available
| 2 | Use HDF5 | available
| 1 | both OpenFoam files and HDF5 | _to be implemented_
| not set | Use standard OpenFOAM | available

The second environment variable allows _bunching_ the output from groups of processes. This can be useful for very large numbers of processes. the processes are divided into  bunches, an HDF5 file being created for each time and each bunch.

If _P_, _T_, _F_ are the number of processes, of time dumps required, of files for each process and each time dump, respectively, and _B_ is the bunch size, the 
table below gives the number of files created and applies it to the case _P=10000_, _T=100_, _F=10_, _B=100_

Standard OpenFOAM | HDF5 no bunches | HDF5 with _B_ bunches
|:-----:|:-----:|:------:|
_T_\*_P_\*_F_ | _T_ | _T_\*_P_/_B_
10,000,000 | 100 | 10,000

Clearly, using standard OpenFOAM would not be feasiblefor this problem as the number of files created would probably exceed the availble capabilities of the HPC system. Using HDF5 would be possible either with or without bunching.

The environment variable defining bunching is:

OF\_OUTPUT_H5\_BUNCHSIZE |  Description | Status
-------------- | ------------ | ------
<= 1 | No bunching | available
\>1 | Use HDF5 bunching with the given size | available
not defined | no bunching | available

If standard OpenFOAM is used, then this environment variable is ignored.
