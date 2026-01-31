Flash arduino_esp.ino to the esp32 using arduino ide

NOTE:
Linux restricts access to USB ports by default. Run this once per session:
sudo chmod 666 /dev/ttyUSB0


make the hover.cpp file in the build directory
use "gz sim alti.sdf" to start the sim in terminal 1
run the hover file in terminal 2
source ros in terminal 3 and use:
ros2 topic pub /drone/target_alt std_msgs/msg/Float64 "{data: 5.0}"
to adjust the height

