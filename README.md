# GCS-LiCAS
Simple Ground Control Station (GCS) program in C++ for sending LiCAS operation codes through UDP socket. Each operation code is an integer number corresponding to a task implemented by the LiCAS Control Program. This GCS also sends keep alive messages so the LiCAS can detect connection loss.

Before launching the LiCAS Control Program, run the GCS first to have proper control over the arms.

# Installation
Clone this repository in your LiCAS workspace folder. To compile, provide execution permits to the compile script and then run this file:

chmod +x compile.sh
./compile.sh

# Run the GCS
To run the GCS you need to specify the IP address of the computer where the LiCAS Control Program is executed, along with the UDP port for sending the GCS data packets. Use port 22000 preferably. Examples:

./gcs localhost 22000

./gcs 192.168.100.31 22000

The operation codes for the LiCAS arms can be customized, but these are the most common:

* 1: move arms to rest pose
* 2: move arms to operation pose
* 11: identify joint angles by moving the arms manually
* 49: disable joint torque control in servo actuators (free motion)
* 50: enable joint torque control in servo actuators (position control)
* 101: external control through External Control Interface (ECI)
  
