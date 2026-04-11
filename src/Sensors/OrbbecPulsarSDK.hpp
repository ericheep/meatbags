//
//  OrbbecPulsarSDK.hpp
//  meatbags
//
//  Wraps OrbbecSDK v2 for SL450 sensors on firmware 2.x
//  Requires: libobsensor/ObSensor.hpp (OrbbecSDK v2.6.3+)
//

#pragma once

#include "Sensor.hpp"
#include <libobsensor/ObSensor.hpp>
#include <atomic>
#include <mutex>
#include <string>

class OrbbecPulsarSDK : public Sensor {
public:
	OrbbecPulsarSDK();
	virtual ~OrbbecPulsarSDK();

	void update()            override;
	void setupParameters()   override;
	void initializeVectors() override;
	void setMirrorAngles(bool& mirror) override;

	// Called by Sensor::connect() via startThread() — we override to use SDK instead
	void connect();
	void reconnect();

	// Sensor info
	string lidarState;
	string firmwareVersion;
	float  temperature;
	int    currentRotationSpeed;

	ofParameter<int> guiFilterLevel;

private:
	// SDK objects
	ob::Context                   ctx;
	std::shared_ptr<ob::Device>   obDevice;
	std::shared_ptr<ob::Pipeline> obPipeline;
	std::atomic<bool>             pipelineRunning{ false };

	// threadedFunction is minimal — just monitors pipeline health
	void threadedFunction() override;

	void startPipeline();
	void stopPipeline();
	void updateSensorInfo();
	void onMotorSpeedChanged(int& hz);
	void onFilterLevelChanged(int& level);

	// Frame callback — runs on SDK internal thread
	void onFrame(std::shared_ptr<ob::FrameSet> frameSet);

	// Select a LiDAR stream profile matching guiMotorSpeed (Hz)
	std::shared_ptr<ob::StreamProfile> selectLiDARProfile(std::shared_ptr<ob::Pipeline> pipe);

	// Map Hz value to OBLiDARScanRate enum
	static OBLiDARScanRate hzToScanRate(int hz);

	// Map scan rate to point count per rotation (= angularResolution)
	static int scanRateToPointCount(OBLiDARScanRate rate);

	// Reconnection
	std::chrono::steady_clock::time_point lastReconnectAttempt;
	static constexpr int RECONNECT_COOLDOWN_MS = 3000;
};
