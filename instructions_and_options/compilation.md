# Compilation

## Options required

The option file src/OpenFoam/Make/options neeed to be tailored to your particular system.

This is required in order to introduce the apprpriate libraries and include files for

1. MPI (OpenMPI). Because of OF internal mechanisms (link within the source), these need to be included explicitly.
1. HDF5 include files and libraries.


The file  
http://github.com/stefsal/OeRC_OpenFOAM_HDF5/stefsal/OeRC_OpenFOAM_HDF5/instructions_and_options/src_OpenFoam_Make_options  
contains an example of the option file for some specific system.


## Update the sources

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
  wmake libso ~/OpenFOAM/OpenFOAM-4.1/src/OpenFOAM
  cd  ~/OpenFOAM/OpenFOAM-4.1/applications
  Allwmake -j8
```

This will make sure that only the changed contents in db/ will be compiled (of course, with all their dependencies), then the Allwmake command (script) will link the executable.

OpenFOAM is npw ready to use.