![GitHub top language](https://img.shields.io/github/languages/top/SileYin/2022-MUSI6106)
![GitHub issues](https://img.shields.io/github/issues-raw/SileYin/2022-MUSI6106)
![GitHub last commit](https://img.shields.io/github/last-commit/SileYin/2022-MUSI6106)
![GitHub](https://img.shields.io/github/license/SileYin/2022-MUSI6106)

# MUSI6106 2022: Audio Software Engineering
Template project for assignments and exercises for the class MUSI6106

## Project Structure
```console
|_ 3rdparty: (3rd party dependencies)
  |_ sndlib: sndfile library (3rdparty with ugly code and lots of warnings)
  |_ googletest: googletest framework
|_ cmake.modules: (cmake scripts)
|_ inc: global headers
|_ src: source code
  |_ AudioFileIO: library wrapping sndfile (3rdparty)
  |_ inc: internal headers
  |_ MUSI6106Exec: code for executable binary
  |_ Tests: all code related to tests
	|_ TestData: data for specific tests possibly requiring data
	|_ TestExec: test executable
	|_ Tests: individual test implementations (one file per target)
  |_ Vibrato: library for a vibrato effect
```

## Creating the Project Files with CMake
The project files are generated through [CMake](https://www.cmake.org). Using the latest CMake GUI, 
* point the source code directory to the top-level project directly, then 
* set the build directory to some directory you like (suggestion: sourcedir/bld), 
* hit 'Configure' button until nothing is red, then
* 'Generate' the project and open it with your IDE.
In case there are any problems, try clearing the cache first.

On the command line, try from the sourcedir

```console
cmake -B ./bld/ -DCMAKE_BUILD_TYPE=DEBUG
cmake --build ./bld/ --config Debug
```
Enable ```WITH_TESTS``` to build with GTest support and ```WITH_DOXYGENTARGET``` to add a target for creating a doxygen documentation for your project.

If new files are added, clear the cache and rerun configuration and generation.

