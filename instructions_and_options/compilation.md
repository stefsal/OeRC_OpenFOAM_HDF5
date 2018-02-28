# Compilation

## Options required

The option file src/OpenFoam/Make/options neeed to be tailored to your particular system.

This is required in order to introduce the approriate libraries and include files for

1. MPI (OpenMPI). Because of OF internal mechanisms (link within the source), these need to be included explicitly.
1. HDF5 include files and libraries.


The file  
http://github.com/stefsal/OeRC_OpenFOAM_HDF5/instructions_and_options/src_OpenFoam_Make_options  

https://github.com/stefsal/OeRC_OpenFOAM_HDF5/blob/master/instructions_and_options/src_OpenFoam_Make_options

contains an example of the option file for some specific system.


## Update the sources

The directory in stefsal/OeRC_OpenFOAM_HDF5/source contains all the sources required.  These must copied (overwrite) in the appropriate location of the src//OpenFoam/db directory as indicated:

* OFstream.H  -->   src/OpenFOAM/db/IOstreams/Fstreams/OFstream.H
* OFstream.C  -->   src/OpenFOAM/db/IOstreams/Fstreams/OFstream.C
* OSstream.H  -->   src/OpenFOAM/db/IOstreams/Sstreams/OSstream.H
* OSstream.H  -->   src/OpenFOAM/db/IOstreams/Sstreams/OSstreamI.H
* OSstream.C  -->   src/OpenFOAM/db/IOstreams/Sstreams/OSstream.C
* regIOobjectWrite.C  -->   src/OpenFOAM/db/regIOobject/regIOobjectWrite.C

All the standard OF scripts can then be used for compilation. 

Please notice that if these are the only changes required and if there are **no** unresolved dependencies, you could use the following commands:

```
  wmake libso ~/OpenFOAM/OpenFOAM-4.1/src/OpenFOAM
  cd  ~/OpenFOAM/OpenFOAM-4.1/applications
  Allwmake -j8
```

This will make sure that only the changed contents in db/ will be compiled (of course, with all their dependencies), then the Allwmake command (script) will link the executable.

## IMPORTANT: a Potential Problem and its Solution (Workaround)
Unfortunately, sometimes OpenFOAM put incorrect library addresses (locations) into the file 

```
~/OpenFOAM/OpenFOAM-4.1/platforms/<your platform>/src/OpenFOAM/db/options
```

if that happens, compilation will not succeed.  As that file is {\em created automatically}, it is obviously caused by some bug in OpenFOAM: without going into them, there is little one could do but editing it manually.

Please, do check that, after it is created, it contains the correct addresses of all the libraries needed. If it does not, fix it as required.




OpenFOAM is now ready to use.