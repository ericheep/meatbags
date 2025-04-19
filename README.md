# meatbags
openFrameworks sketch that connects to a Hokuyo UST-LX LIDAR sensor and sends those coordinates out via OSC.

## installation

This project was developed with openFrameowrks v0.12.0, it uses the following openFrameworks addons; all included in the main package.

- ofxNetwork
- ofxGui
- ofxOsc

## external code

This project uses the DBSCAN algorithm for clustering the LiDAR measurements received from Hokuyo sensor. A performant C++ implementaion of DBSCAN by Eleobert is used in the project.

https://github.com/Eleobert/dbscan

A C++11 implementation of C++20's std::span is also used to enable Eleobert's dbscan implementation.

https://github.com/tcbrindle/span

[openFrameworks](https://openframeworks.cc/download/)

![meatbags](https://github.com/user-attachments/assets/573bc162-8214-4989-9b6e-8b0510f38e72)
