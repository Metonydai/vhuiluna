# vhuiluna
vhuiluna is a personal project aimed at building a fundamental game engine from scrach which is mainly based on Brendan Galea's youtube channel, [Brendan Galea's youtube](https://www.youtube.com/@BrendanGalea).

## Requirements
1. [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (Make sure to **install and set up environment variables** (e.g., `VULKAN_SDK`))
1. [CMake](https://cmake.org/) 
2. [Git](https://git-scm.com/)
3. [Visual Studio](https://visualstudio.microsoft.com/) (2022 recommended)

## Getting Started
Visual Studio 2022 is recommended, focus mainly on a **Windows** build. Mac and Linux support are planned for future updates.

<ins>**1. Downloading the repository:**</ins>

Start by cloning the repository with `git clone --recursive https://github.com/Metonydai/vhuiluna`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

<ins>**2. Step by step build:**</ins>

1. In the project folder, make a new folder named build. Use `mkdir build` to make a directory in command line.

2. `cd build` to change to the build directory.

3. `cmake ../` to configure the cmake file.

4. Build the project by opening the solution file(*.sln) then hit Ctrl + F5 to build for it. Or `cmake --build .`.

5. Make sure to open the .exe file at the project source directory, since all asset paths are relevant to the source directory.

## Keyboard Movement
 1. `w` key : Move camera forward.
    
 2. `s` key : Move camera backward.
    
 3. `a` key : Move camera leftward.
    
 4. `d` key : Move camera rightward.
    
 5. `q` key : Move camera downward.
  
 6. `e` key : Move camera upward.
  
 7. `↑` key : Increase camera's pitch angle.
  
 8. `↓` key : Decrease camera's pitch angle.
  
 9. `←` key : Rotate couterclockwise the camera's yaw angle.
  
10. `→` key : Rotate clockwise the camera's yaw angle.

## Demo film
[Euphonium rendering](https://www.youtube.com/watch?v=dLI2OWWh320)
Some rendering techniques I used :
1. Use Blinn Phong shading.
2. Billboard lighting that the light quad always parallel to the view plane.
3. Shadow mapping with spot light.