#Compilation#

##Options required##

The option file src/OpenFoam/Make/options neeed to be tailored to your particular system.

This is required in order to introduce the apprpriate libraries and include files for

1. MPI (OpenMPI). Because of OF internal mechanisms (link within the source), these need to be included explicitly.
1. HDF5 include files and libraries.


The file stefsal/OeRC_OpenFOAM_HDF5/stefsal/OeRC_OpenFOAM_HDF5instructions_and_options/src_OpenFoam_Make_options contains an example of the option file for some specific system.


##Update the sources##

The directory in stefsal/OeRC_OpenFOAM_HDF5/source contains all the sources required:

* OFstream.H
* OFstream.C
* OSstream.H
* OSstream.C
* regIOobjectWrite.C

This must copied (overwrite) in the appropriate location of the src//OpenFoam/db directory.

All the standard OF scripts can then be used for compilation. 

Please notice that if these are the only changes required and if there are **no** unresolved dependencies, you could use the following commands:

```
cloud cuckoo land
```



1. Pippo
1. PLuto
1. Paperino
