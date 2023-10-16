# CGRA350: Real-time Rendering

## Group Project: Real-time Renderers

This project is a 3D terrain simulation that features the ability for the user to move around the scene and edit the terrain mesh in real-time. Some interesting aspects of this project include:

- Terrain mesh generation using Perlin Noise with varying L.O.D.
- Procedurally generated trees that flow with the terrain as it is edited.
- Procedurally generated clouds using the Marching Cubes algorithm and Perlin Noise.
- Mesh simplification with varying voxel resolutions.
- Ability to edit the terrain mesh by raising or excavating certain areas.
- The terrain displays different textures depending on the height of the mesh at any given point.  
- Interactive flying camera that lets the user move around the scene.

### Team Members

- Adam Goodyear
- Marshall Scott
- Shekinah Pratap
- Tessa Power
- Timothy Green

## Project Structure

```bash
work:
├───ext           # External libraries, included for simplicity 
├───res           # Asset/Texture files & a self-contained project for shaders
│   ├───assets
│   ├───shaders
│   └───textures
└───src
    ├───cgra      # CGRA350 Framework source code   
    ├───clouds    # Cloud model source code
    ├───mesh      # Code related to editing or simplifying a mesh
    ├───terrain   # Terrain model source code
    ├───trees     # Procedurally generated trees source code
    └───utils     # Utility classes used throughout the project
```

## Prerequisites

- OpenGL v3.3
- CMake
- C++11 compiler
  
The recommended way to build this project is using CMake, so the best structure for the project files is to clone this project into a dedicated directory with the name `work`.

```bash
mkdir cgra350-group-project
cd cgra350-group-project
git clone https://github.com/tessapower/cgra350-group-project.git work
```

Alternatively, download the source code for the project, rename the root directory to `work`, and then place it into a dedicated directory for this project. In this same project directory, create a directory called `build`. The folder structure should now look like this:

```bash
cgra350-group-project:
├───build
└───work
```

## How to Build

### Linux

#### Command Line

The simpliest way to set up a basic project is to run the shell script `f5.sh` which runs the `cmake`, `make` and run commands for you:

```bash
./f5.sh
```

Alternativiely you can run the commands manually:

```bash
cd build
cmake ../work
make
cd ..
```

If the project builds without errors the executable should be located in the `build/bin/` directory and can be run with:

```bash
./build/bin/base [args...]
```

#### Eclipse

Setting up for [Eclipse](https://eclipse.org/) is a little more complicated. Navigate to the build folder and run `cmake` for Eclipse:

```bash
cd build
cmake  -G "Eclipse  CDT4 - Unix  Makefiles" ../work
```

Start Eclipse and go to `File > Import > Existing Projects into Workspace`, browse to and select the `build/` directory as the project. Make sure  the  box `Copy  Projects into Workspace` is unchecked. Once you've imported the project, and are unable run it, do the following:

- Go to `Run > Run  Configurations`.
- On the left side, select `C/C++ Application`.
- In the `Main` tab, make sure your `C/C++ Application` field contains `./bin/base` and that `Enable auto build` is checked.
- Right click on the project and then select, `Run As > C/C++  Application`.

This should setup the default way to start the program, so you can simply run the project any time after that.

### Windows

#### Visual Studio

This project requires at least Visual Studio 2017. You can get the latest, [Visual Studio Community 2017](https://www.visualstudio.com/downloads/), for free from Microsoft.

| Product            | XX |
|:------------------:|:--:|
| Visual Studio 2017 | 15 |

Change to the `build` directory and run the `cmake` command for Visual Studio with the appropriate version number (XX):

```bash
cd build
cmake -G "Visual Studio XX" ..\work
```

Or if you are building for 64-bit systems:

```sh
cmake -G "Visual Studio XX Win64" ..\work
```

After opening the solution (`.sln`) you will need to set some additional variables before running.
- `Solution Explorer > base > [right click] > Set as StartUp Project`
- `Solution Explorer > base > [right click] > Properties > Configuration Properties > Debugging`
- Select `All Configurations` from the configuration drop-down.
- Set `Working Directory` to `$(SolutionDir)../work`.


### OSX

#### XCode

[Xcode](https://developer.apple.com/xcode/) is an IDE that offers a little more than simple text editing. The setup again is very similar to Eclipse.

```bash
cd build
cmake -G "Xcode" ../work
cd ..
```

Once you're setup, you can build your project with Xcode, but have to execute your program with the terminal (making sure you are in the root directory):

```sh
./build/bin/base [args..]
```
## How to Control

### Simple Controls

- W for Zoom In, or Forward 
- S for Zoom Out or Backwards
- A for Pan Left
- D for Pan Right

- Middle Mouse Click+Hold to rotate camera

### ImGUI Controls

#### Voxel Settings
- Voxel Size
    - changes the amount of points sampled of the original mesh
- IsoLevel
    - how far away from the mesh a voxel point must be for it to be considered "solid"
- Smooth Normals
    - Toggles Smoothed Normals on/off
- Debugging (Menu)
    - Allows for visualisation of different steps in the simplification process of the Bunny
- Cloud Threshold
    - The threshold the noise value needs to pass to become a solid cloud.
- Cloud Fade out
    - the distance from the edge of the voxel grid that the cloud starts to fade out on the Y axis to prevent hard edges

#### Mesh Editing and Terrain
- Radius
    - Changes Radius of terrain manipulation
- Strength
    - Changes Strength of terrain manipulation
- Grass/Mud Height
    - Changes at what height the Grass/Mud Textures will appear
- Mud/Rocks Height
    - Changes at what height the Mud/Rocks Textures will appear
- Heighmap Scale
    - Changes intensity of heightmap
- Normal Map (Toggle)
    - Toggles on or off the normal map
- Raise/Excavate (Toggles)
    - Switches Mode to either Raise or Excavate Terrain
- Deform
    - Raises or Excavates the currently selected terrain
- Octaves
    - Number of times the noise values are iterated over
- Lacunarity
    - Affects the frequency of the noise
- Persistence
    - Affects the amplitude of the noise
- Height
    - Scales the height of the heightmap
- Repeats
    - Determines how many "tiles" the heightmap can repeat for
- Seed
    - Allows the permutation map to become seeded for deterministic randomness
- Perlin/Flat (Toggles)
    - Switches Mode to generate either flat or Perlin based Terrain
- Recreate Terrain
    - Regenerates Terrain with new values

#### Tree Settings
- Spooky Mode
    - Turns spooky mode on or off (Disables or Enables showing of foliage)
- New Tree
    - Generates a new base tree
- Print Tree
    - Prints a list of every branch in the tree and the information about that branch
