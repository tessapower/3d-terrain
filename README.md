# 3D Terrain Simulation

An interactive 3D terrain simulation, featuring user controls to move around the scene and an editable terrain mesh that updates in real-time.

<div align="center">

| Real-Time Mesh Editing |
|:---:|
| ![terrain-editing](https://github.com/user-attachments/assets/5a9a353c-fbb8-4825-abf9-dbdc9e239025) |

| Perlin Noise Terrain Generation |
|:---:|
| ![terrain-generation](https://github.com/user-attachments/assets/48bcc307-e780-49d1-abf5-2cdf6ddafba1) |

| Mesh Simplification using Varying Voxel Resolutions |
|:---:|
| ![standford-bunny-voxel](https://github.com/user-attachments/assets/4430cf54-a253-4f05-a04b-be7570e8add0) |

</div>


Technical aspects of this implementation include:

- Terrain mesh generation using Perlin Noise with varying Level of Detail (L.O.D)
- Procedurally generated trees that flow with the terrain as it is edited.
- Procedurally generated clouds using the Marching Cubes algorithm and Perlin Noise.
- Mesh simplification with varying voxel resolutions.
- Ability to edit the terrain mesh by raising or excavating certain areas.
- The terrain displays different textures depending on the height of the mesh at any given point.
- Interactive flying camera that lets the user move around the scene.

## Prerequisites

- OpenGL v3.3
- CMake
- C++11 compiler

The recommended way to build this project is using CMake, so the best structure for the project files is create a dedicated `build` directory in the root of the project.

```bash
git clone https://github.com/tessapower/3d-terrain.git
cd 3d-terrain
mkdir build
cd build
# Replace "Visual Studio XX" with your preferred IDE and version
cmake -G "Visual Studio XX" ..\
```

After following the above instructions, the directory structure will look like this:

### Project Structure

```bash
3d-terrain
├───build
│   ├───bin # Executable directory
│   ├───CMakeFiles
│   ├───ext
│   ├───res
│   ├───src
│   └───x64
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

## Interacting with the Program

### Keyboard/Mouse Controls

| Key | Action |
|:---:|:------:|
| W   | Zoom In (Forwards) |
| S   | Zoom Out (Backwards) |
| A   | Pan Left |
| D   | Pan Right |
| Middle Mouse Click + Hold | Rotate camera |

### GUI Controls

#### Voxel Settings

| Control | Description |
|:-------:|:-----------:|
| Voxel Size | Changes the amount of points sampled of the original mesh |
| IsoLevel | How far away from the mesh a voxel point must be for it to be considered "solid" |
| Smooth Normals | Toggles Smoothed Normals on/off |
| Debugging (Menu) | Allows for visualisation of different steps in the simplification process of the Bunny |
| Cloud Threshold | The threshold the noise value needs to pass to become a solid cloud |
| Cloud Fade out | The distance from the edge of the voxel grid that the cloud starts to fade out on the Y axis to prevent hard edges |

#### Mesh Editing and Terrain

| Control | Description |
|:-------:|:-----------:|
| Radius | Changes Radius of terrain manipulation |
| Strength | Changes Strength of terrain manipulation |
| Grass/Mud Height | Changes at what height the Grass/Mud Textures will appear |
| Mud/Rocks Height | Changes at what height the Mud/Rocks Textures will appear |
| Heightmap Scale | Changes intensity of heightmap |
| Normal Map | Toggles on or off the normal map |
| Raise/Excavate | Switches Mode to either Raise or Excavate Terrain |
| Deform | Raises or Excavates the currently selected terrain |
| Octaves | Number of times the noise values are iterated over |
| Lacunarity | Affects the frequency of the noise |
| Persistence | Affects the amplitude of the noise |
| Height | Scales the height of the heightmap |
| Repeats | Determines how many "tiles" the heightmap can repeat for |
| Seed | Allows the permutation map to become seeded for deterministic randomness |
| Perlin/Flat | Switches Mode to generate either flat or Perlin based Terrain |
| Recreate Terrain | Regenerates Terrain with new values |

#### Tree Settings

| Control | Description |
|:-------:|:-----------:|
| Spooky Mode | Toggles showing tree foliage |
| New Tree | Generates a new base tree |
| Print Tree | Prints a list to stdout of every branch in the tree and the information about that branch |
