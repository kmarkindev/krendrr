# krendrr

A bunch of projects aimed to practice graphics programming using different APIs.

At the moment, main develop branch only contains OpenGL code. Other branches contain DX12 code.

## Building

Install MSVC toolkit and run CMake. All dependencies are going to be downloaded automatically during CMake generation.

## Repo Structure

It is very similar to Unreal Engine's modules.

There are nested directories inside Source folder. Each leaf directory is a project (much like UE module). Each project usually contains it's own Source folder together with Content folder.
Content folder gets copied near project's resulting executable.

Projects may be linked by other projects. Usually, executable projects link to library projects. 
