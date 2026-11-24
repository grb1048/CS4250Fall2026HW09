Project:    Marble Maze

Expected parts list:
-2 S51 servo module
-PCA9685 servo driver module
-5V power supply
-MPU5060
-Marble
-oz64

Description:
A small marble maze controlled from a MPU5060.
The oz64 receives input from the MPU5060 and translates to an output of servos.
The servos and oz64 will be in a game frame while the MPU5060 is connected via wire.
The frame and maze are seperate because I want to be able to create different mazes and drop them inplace.
Ideally the mazes will be 3d printed for ease of access.

Physical setup:
1 servo per axis (X,Y) to reduce required material. May use 2 per axis depending on stability.
The maze will be loose meaning it will drop inplace and be ready to play.

Scrapped/On-Hold:
Wiimote controller - (Scrapped). Needs bluetooth.
Thumbstick (scrapped) - analog not supported.

Timeline:
This week - oz64 testing
1 week - Creating physical object based on 3D model (test fitting etc.)
2 week - Assembly and code tweaks