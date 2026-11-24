Current status:
Created a 3d model of a the maze stage to house the maze, frame and servos.
Created a wiring diagram of the servos connected to an Arduino UNO.
Updated wiring diagram to include a mock thumbstick and added +6v supply.
+6v is for the servos and preventing overdraw from the board.
Created initial code for the Arduino UNO with support for servos and input.
Included wiring schematic for teh Arduino UNO.
Decided to scrap the thumbstick because this dev board sucks and doesn't have a digital to analog signal converter.
Included 2test.py for the MPU5060 testing.
Using servos through the servo controller on the IIC bus with the MPU5060 works.
Created an arduino sketch that controls a servo per axis of movement.
Created startup script on the dev board.
Script autoruns on boot as a systemd service.
Script pinmuxes Pins B11 and B12 for IIC use, loads a precompiled sketch onto the small core.
I'll keep the script off until the project is ready.
Added cs4250startup.sh and autorunSetup.md to project.
Added ProjectThing.ino to project.
Renamed some files to be more accurate.