# Execution

## Execution Modes

For ease of editing (restart) internal and boundary values are now output in separate files.  The reason is that the interior values set could be very large and, in any case, far larger than the boundary values. Current editing of standard OpenFOAM files can be awkward and very time consuming.  The interior and boundary values files can be trivially concatenated: a Python simple code is provided to do so.

This version of OpenFOAm allows to output data to:

* standard OpenFOAM files
* HDF5 files
* both standard OpenFOAM files and HDF5 files (not yet impolemented)

Notice that an HDF5 file is not necessarily a file in the normal sense: users can access it as a normal file, through the appropriate HDF5 APIs, but it consists of anumber of "stprage units", called datasets.  Datasets can be collected in groups, and hierarchies of groups can be defined.

In the case of this version of OpenFOAM, the following has been adopted:
1. The top level of the datasets structure is the processor (processor 0, etc.)
1. The names of the datasets are the same as the names of the original OpenFOAM files.
1. The contents of the datasets are byte-identical to the contents of the corresponding OpenFOAM files.
1. Each HDF5 file name includes the time of the dump.

This allows time du ps to be removed easily and cleanly by just deleteing the appropriate hdf5 file.

Notice that if datasets were to be removed from an HDF5 file, reclaiming free space is far from trivial, requiring a full HDF5 file rewrite. The same applies to other large parallel storage systems.  That is the reason to invert the order processor/time dump from the original OpenfFOAM structure.

Once this version of OpenFOAM has been compiled, the user executes it as ihe/she always did, no --changes whatsoever__.  If the user decided to run OpenFOAM without employing the HDF5 facilities, then this version would have the standard OpenFOAM behaviour.

For reasons discussed [elsewhere](https://github.com/stefsal/OeRC_OpenFOAM_HDF5/blob/master/technical_information.md),the user can determine OpenFOAM output byu the environment variable __OF\_OUTPUT\_H5__:

OF\_OUTPUT\_H5 |  Description
-------------- | ------------
0 | Use standard OpenFOAM files
2 | Use HDF5
 

First Header | Second Header
------------ | -------------
Content from cell 1 | Content from cell 2
Content in the first column | Content in the second column


OF_OUTPUT_H5_BUNCHSIZE
