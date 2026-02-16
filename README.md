# cadsh: Command Line CAD

Presents the Manifold library in a command line program.  The interface is all command line parameters, acted-on sequentially on a mesh list. 3MF files can be loaded and saved at any point in the parameter sequence, and most of the Manifold primitives can be created.  Operators work on the last mesh in the list, and aggregators work in Manifold-fashion on the list.

The Manifold library and its documentation can be found here: https://github.com/elalish/manifold

The following would load a 3MF file into the mesh list, simplify the last mesh, and save the mesh list:

    cadsh load:mesh.3mf simplify:0.1 save:simpler-mesh.3mf

If the 3MF file contains multiple meshes, you would union them to simplify all of them:

    cadsh load:mesh.3mf union simplify:0.1 save:simpler-aggregated-mesh.3mf

primitives can be created to make parts:

    cadsh cube:10,1,1,ctr sphere:2,20 union save:part.3mf

cadsh by default acts silently; add 'verbose' as a parameter anywhere in the parameter string to turn on per-command verbosity.

All numeric parameters can be expressed as simple math expressions, e.g., ```scale:1/87```.  If verbose is on, parameters are reported as the result of the expression.

extrude and revolve use polygon files defined as text files, one comma-separated point per line:
```
0.000,0.000
5.929,0.026
5.404,1.443
3.856,2.807
2.728,6.270
1.600,7.582
0.000,8.448
```

## Usage: 

    cadsh [cmd ...]

In the following command descriptions, [] are used to segregate optional parameters.  Refer to the manifold documentation for parameter semantics.

cadsh starts with an empty mesh list; load/save and primitive commands add meshes to the list.  Transforms work only on the last mesh in the list.  Aggregators work with the entire mesh list in the same semantics as the corresponding Manifold BatchBoolean/Hull methods

Commands:
- input/output:
   - load:filename
   - save:filename
- primitives:
   - cube:x,y,z[,'ctr']
   - cylinder:h,rl[,rh[,seg[,'ctr']]]
   - sphere:r[,seg]
   - tetrahedron
   - isocahedron
   - extrude:polyfilename,height[,div[,twistdeg[,scaletop]]]
   - revolve:polyfilename,segments,degrees
   - heightmap:heightmapfilename
- operators (work on only last mesh):
   - translate:x,y,z
   - rotate:x,y,z
   - scale:s|x,y,z
   - simplify:s
- aggregators:
   - union
   - subtract
   - intersect
   - hull

## Building

cadsh requires at least C++17, and the Manifold library.  cmake is used to configure the build system; there is one option: BUILD_MANIFOLD.  This option has three possible values:

1. GITHUB - Clones the Manifold library, builds it, and installs it to a local external/usr/ tree in the build directory.
2. SRCPKG - Downloads a Manifold source package, builds it, and installs it to a local external/usr/ tree in the build directory.
3. pkgfile - Unpacks a previously downloaded source package, builds it, and installs it to a local external/usr/ tree in the build directory.

Here's the nominal build sequence:

```
git clone https://github.com/butcherg/cadsh.git
cd cadsh
mkdir build
cd build
cmake -DBUILD_MANIFOLD=GITHUB ..
make
```

## Acknowledgements

- miniz: Public Domain
- rapidxml: Copyright (c) 2006, 2007 Marcin Kalicinsk, MIT License

## To-Do

1. Add other Manifold primitives: Minkowski
2. Add other Manifold operations: Refine, Smooth
3. Structure a mechanism to allow operators to be applied to other than just the last mesh

