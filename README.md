HOW TO COMPILE PROGRAM:
-
The program contains the following source files:
- main.cpp
- Robot.cpp
- Robot.h
- surfaceModeller.h

in the "textures" folder:
cannon.bmp
snow.bmp

in the "meshes" folder:
cannon_mesh_export.obj

All files are required to run the program. The main.cpp file contains the main function required to run the program, the program was made in Visual Studio. 
To run create a visual studio project with the freeglut and GLEW dependencies and make sure the files are in their corresponding folders.


REQUIREMENTS:
- Starting the game causes 5 robots to walk forward along the ground.
- The defensive cannon is texture mapped.
- Defensive cannon fires projectiles using the space key.
- Projectiles are rendered and animated.
- Robots disappear when hit with a projectile or walking into the defensive cannon.
- Mouse-controlled defensive cannon
- Use of custom mesh generated via assignment 2


MOUSE AND KEY MAPPING:

- Camera -
left mouse button		- Drag to move camera

- Cannon -
space key 			- fire defensive cannon projectile

- Game -
's' key				- Starts the game: summons robots
'r' key				- Resets the game: despawns all active robots
