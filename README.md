# meatbags
openFrameworks Hokuyo LiDAR blob detection

This project implements the Hokuyo UST-LX LIDAR protocol and clusters data to be used as blob detection for creative applications.

## installation

You can download a built binary from the releases sidebar (Mac OS only) or you can compile the software using the [openFrameworks](https://openframeworks.cc/download/) architecture.

If compiling from source code, I recommend using the [projectGenerator](https://openframeworks.cc/learning/01_basics/create_a_new_project/) software to import the project into your IDE of choice.

This project was developed with openFrameowrks v0.12.0, it uses the following openFrameworks addons; all included in the main package.

- ofxNetwork
- ofxGui
- ofxOsc

## external code

This project uses the DBSCAN algorithm for clustering the LiDAR measurements received from Hokuyo sensor. A performant C++ implementaion of DBSCAN by Eleobert is used in the project.

https://github.com/Eleobert/dbscan

A C++11 implementation of C++20's std::span is also used to enable Eleobert's dbscan implementation.

https://github.com/tcbrindle/span

## tested sensors

- Hokuyo UST-10LX

## todo

- add support for reading multiple sensors simultaneously

![meatbags](https://github.com/user-attachments/assets/573bc162-8214-4989-9b6e-8b0510f38e72)
