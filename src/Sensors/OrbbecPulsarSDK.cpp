//
//  OrbbecPulsarSDK.cpp
//  meatbags
//

#include "OrbbecPulsarSDK.hpp"
#include "ofMain.h"
#include <cmath>

// SDK default port for network devices - FemtoMega only, not used for LiDAR
// static constexpr uint16_t OB_NET_PORT = 8090;

// -----------------------------------------------------------------------------
// Construction / destruction
// -----------------------------------------------------------------------------

OrbbecPulsarSDK::OrbbecPulsarSDK() {
	model                = "Orbbec Pulsar SDK";
	lidarState           = "disconnected";
	firmwareVersion      = "";
	temperature          = 0.0f;
	currentRotationSpeed = 0;
	lastReconnectAttempt = std::chrono::steady_clock::now() - std::chrono::seconds(10);

	// SL450 default angular resolution at 20Hz = 2700 points per rotation
	angularResolution = 2700;
}

OrbbecPulsarSDK::~OrbbecPulsarSDK() {
	guiMotorSpeed.removeListener(this, &OrbbecPulsarSDK::onMotorSpeedChanged);
	guiFilterLevel.removeListener(this, &OrbbecPulsarSDK::onFilterLevelChanged);
	stopPipeline();
	if(isThreadRunning()) {
		stopThread();
		waitForThread(true);
	}
}

void OrbbecPulsarSDK::onMotorSpeedChanged(int& hz) {
	// pipeline restart required to apply new scan rate — only if already running
	if(pipelineRunning) {
		ofLogNotice("OrbbecSDK") << "Sensor " << index << " motor speed changed to " << hz << " Hz, restarting pipeline";
		stopPipeline();
		// threadedFunction will detect pipelineRunning == false and call startPipeline()
	}
}

void OrbbecPulsarSDK::onFilterLevelChanged(int& level) {
	// filter level can be applied live without restarting the pipeline
	if(obDevice && pipelineRunning) {
		try {
			obDevice->setIntProperty(OB_PROP_LIDAR_TAIL_FILTER_LEVEL_INT, level);
			ofLogNotice("OrbbecSDK") << "Sensor " << index << " filter level set to " << level;
		}
		catch(ob::Error &e) {
			ofLogError("OrbbecSDK") << "Sensor " << index << " filter level error: " << e.what();
		}
	}
}

// -----------------------------------------------------------------------------
// Sensor interface
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::setupParameters() {
	Sensor::setupParameters();
	guiMotorSpeed.set("motor speed", 20, 15, 40);
	guiMotorSpeed.addListener(this, &OrbbecPulsarSDK::onMotorSpeedChanged);
	guiFilterLevel.set("filter level", 0, 0, 5);
	guiFilterLevel.addListener(this, &OrbbecPulsarSDK::onFilterLevelChanged);
}

void OrbbecPulsarSDK::initializeVectors() {
	angularResolution = scanRateToPointCount(hzToScanRate(guiMotorSpeed.get()));
	Sensor::initializeVectors();
}

void OrbbecPulsarSDK::update() {
	updateDistances();
}

// -----------------------------------------------------------------------------
// Connection
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::connect() {
	if(ipAddress.get().empty() || ipAddress.get() == "0.0.0.0") {
		connectionStatus = "No IP address specified";
		return;
	}

	connectionStatus = "Connecting at " + ipAddress.get();

	if(!isThreadRunning()) {
		startThread();
	}
}

void OrbbecPulsarSDK::reconnect() {
	ofLogNotice("OrbbecSDK") << "Force reconnect on sensor " << index;
	stopPipeline();
	isConnected      = false;
	connectionStatus = "Reconnecting...";
	lastReconnectAttempt = std::chrono::steady_clock::now() - std::chrono::seconds(10);
}

// -----------------------------------------------------------------------------
// Thread — monitors pipeline health, handles reconnection
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::threadedFunction() {
	startPipeline();

	while(isThreadRunning()) {
		if(!pipelineRunning) {
			auto now     = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastReconnectAttempt).count();
			if(elapsed >= RECONNECT_COOLDOWN_MS) {
				ofLogNotice("OrbbecSDK") << "Sensor " << index << " pipeline not running, attempting reconnect";
				startPipeline();
				lastReconnectAttempt = std::chrono::steady_clock::now();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	stopPipeline();
}

// -----------------------------------------------------------------------------
// Pipeline
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::startPipeline() {
	stopPipeline();

	try {
		ofLogNotice("OrbbecSDK") << "Sensor " << index << " searching for device at " << ipAddress.get();

		// LiDAR devices are discovered via queryDeviceList, not createNetDevice
		// (createNetDevice is FemtoMega/GigE only)
		auto deviceList = ctx.queryDeviceList();

		if(deviceList->getCount() == 0) {
			connectionStatus = "No devices found";
			ofLogWarning("OrbbecSDK") << "Sensor " << index << " — no devices found on network";
			return;
		}

		// Find device matching our IP address
		std::string targetIP = ipAddress.get();
		for(uint32_t i = 0; i < deviceList->getCount(); i++) {
			auto candidate = deviceList->getDevice(i);

			// Read the device's LiDAR IP address
			try {
				uint32_t dataSize = 32;
				uint8_t  data[32] = { 0 };
				candidate->getStructuredData(OB_RAW_DATA_LIDAR_IP_ADDRESS, data, &dataSize);
				std::string devIP = std::to_string(data[3]) + "." + std::to_string(data[2])
								  + "." + std::to_string(data[1]) + "." + std::to_string(data[0]);

				if(devIP == targetIP) {
					obDevice = candidate;
					break;
				}
			}
			catch(...) {
				// Not a LiDAR or doesn't support this property — skip
				continue;
			}
		}

		if(!obDevice) {
			connectionStatus = "Device not found at " + targetIP;
			ofLogWarning("OrbbecSDK") << "Sensor " << index << " — no LiDAR found at " << targetIP;
			return;
		}

		// Read firmware version and IP
		auto info       = obDevice->getDeviceInfo();
		firmwareVersion = info->getFirmwareVersion();

		// Disable tail filter by default — apply guiFilterLevel instead
		try {
			obDevice->setIntProperty(OB_PROP_LIDAR_TAIL_FILTER_LEVEL_INT, guiFilterLevel.get());
		}
		catch(...) {}

		// Create pipeline and config
		obPipeline = std::make_shared<ob::Pipeline>(obDevice);
		auto config = std::make_shared<ob::Config>();

		// Select the LiDAR stream profile matching guiMotorSpeed
		auto profile = selectLiDARProfile(obPipeline);
		if(!profile) {
			ofLogError("OrbbecSDK") << "Sensor " << index << " — no matching LiDAR profile for " << guiMotorSpeed.get() << " Hz";
			connectionStatus = "No matching profile";
			return;
		}

		config->enableStream(profile);

		// Only deliver framesets that contain all enabled stream types
		config->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ANY_SITUATION);

		// Start with callback
		obPipeline->start(config, [this](std::shared_ptr<ob::FrameSet> frameSet) {
			onFrame(frameSet);
		});

		pipelineRunning  = true;
		isConnected      = true;
		connectionStatus = "Connected (" + std::string(info->getName()) + " fw:" + firmwareVersion + ")";

		// Update angularResolution to match actual profile
		auto lidarProfile = profile->as<ob::LiDARStreamProfile>();
		angularResolution = scanRateToPointCount(lidarProfile->getScanRate());
		initializeVectors();

		ofLogNotice("OrbbecSDK") << "Sensor " << index << " connected — "
			<< angularResolution << " pts/rotation @ " << guiMotorSpeed.get() << " Hz";
	}
	catch(ob::Error &e) {
		ofLogError("OrbbecSDK") << "Sensor " << index << " connect error: " << e.what();
		connectionStatus = "Error: " + std::string(e.what());
		isConnected      = false;
		pipelineRunning  = false;
		obPipeline.reset();
		obDevice.reset();
	}
	catch(std::exception &e) {
		ofLogError("OrbbecSDK") << "Sensor " << index << " exception: " << e.what();
		connectionStatus = "Exception: " + std::string(e.what());
		isConnected      = false;
		pipelineRunning  = false;
	}
}

void OrbbecPulsarSDK::stopPipeline() {
	if(pipelineRunning && obPipeline) {
		try {
			obPipeline->stop();
		}
		catch(...) {}
	}
	pipelineRunning  = false;
	isConnected      = false;
	obPipeline.reset();
	obDevice.reset();
}

// -----------------------------------------------------------------------------
// Frame callback — runs on SDK internal thread
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::onFrame(std::shared_ptr<ob::FrameSet> frameSet) {
	if(!frameSet) return;

	auto frame = frameSet->getFrame(OB_FRAME_LIDAR_POINTS);
	if(!frame) return;

	auto lidarFrame = frame->as<ob::LiDARPointsFrame>();
	if(!lidarFrame) return;

	auto format = lidarFrame->getFormat();

	// We use OB_FORMAT_LIDAR_SCAN: OBLiDARScanPoint { float angle; float distance; }
	// angle in degrees, distance in mm — directly compatible with our coordinate pipeline
	if(format != OB_FORMAT_LIDAR_SCAN) {
		ofLogWarning("OrbbecSDK") << "Sensor " << index << " unexpected format, expected LIDAR_SCAN";
		return;
	}

	auto     points     = reinterpret_cast<const OBLiDARScanPoint*>(lidarFrame->getData());
	uint32_t pointCount = static_cast<uint32_t>(lidarFrame->getDataSize() / sizeof(OBLiDARScanPoint));

	if(pointCount == 0) return;

	// Resize vectors if resolution changed
	if((int)pointCount != angularResolution) {
		angularResolution = (int)pointCount;
		initializeVectors();
	}

	{
		std::lock_guard<std::mutex> lock(distancesMutex);
		for(uint32_t i = 0; i < pointCount; i++) {
			distances[i] = points[i].distance;  // mm, same unit as existing OrbbecPulsar
		}
	}

	// Update angles from scan points (angle in degrees → radians handled in setMirrorAngles)
	// Override angles array directly from SDK data since scan points carry per-point angles
	for(uint32_t i = 0; i < pointCount && i < (uint32_t)angles.size(); i++) {
		float angleDeg = points[i].angle;
		float angleRad = angleDeg * DEG_TO_RAD;
		angles[i]      = (mirrorAngles ? -angleRad : angleRad);
	}

	// Update sensor info for overlay display
	lidarState          = "normal";
	connectionStatus    = "Connected";
	logStatus           = lidarState;
	logConnectionStatus = connectionStatus;

	{
		std::lock_guard<std::mutex> lock(distancesAvailableMutex);
		newDistancesAvailable = true;
	}

	lastFrameTime = ofGetElapsedTimef();

	updateSensorInfo();
}

// -----------------------------------------------------------------------------
// Sensor info
// -----------------------------------------------------------------------------

void OrbbecPulsarSDK::updateSensorInfo() {
	sensorInfoLines.clear();
	sensorInfoLines.emplace_back("Model: " + model);
	sensorInfoLines.emplace_back("IP: " + ipAddress.get());
	sensorInfoLines.emplace_back("Firmware: " + firmwareVersion);
	sensorInfoLines.emplace_back("State: " + lidarState);
	sensorInfoLines.emplace_back("Connection: " + connectionStatus);
	sensorInfoLines.emplace_back("Points/scan: " + ofToString(angularResolution));
	sensorInfoLines.emplace_back("Speed: " + ofToString(guiMotorSpeed.get()) + " Hz");
}

// -----------------------------------------------------------------------------
// Profile selection
// -----------------------------------------------------------------------------

std::shared_ptr<ob::StreamProfile> OrbbecPulsarSDK::selectLiDARProfile(std::shared_ptr<ob::Pipeline> pipe) {
	try {
		auto profileList = pipe->getStreamProfileList(OB_SENSOR_LIDAR);
		OBLiDARScanRate targetRate = hzToScanRate(guiMotorSpeed.get());

		// Find profile matching target scan rate and LIDAR_SCAN format
		for(uint32_t i = 0; i < profileList->getCount(); i++) {
			auto profile      = profileList->getProfile(i);
			auto lidarProfile = profile->as<ob::LiDARStreamProfile>();
			if(lidarProfile->getScanRate() == targetRate &&
			   profile->getFormat() == OB_FORMAT_LIDAR_SCAN) {
				return profile;
			}
		}

		// Fallback: any LIDAR_SCAN profile
		for(uint32_t i = 0; i < profileList->getCount(); i++) {
			auto profile = profileList->getProfile(i);
			if(profile->getFormat() == OB_FORMAT_LIDAR_SCAN) {
				ofLogWarning("OrbbecSDK") << "Sensor " << index
					<< " no profile for " << guiMotorSpeed.get() << " Hz, using first LIDAR_SCAN profile";
				return profile;
			}
		}

		// Last resort: first available profile
		if(profileList->getCount() > 0) {
			ofLogWarning("OrbbecSDK") << "Sensor " << index << " no LIDAR_SCAN profile found, using first available";
			return profileList->getProfile(0);
		}
	}
	catch(ob::Error &e) {
		ofLogError("OrbbecSDK") << "selectLiDARProfile error: " << e.what();
	}
	return nullptr;
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

OBLiDARScanRate OrbbecPulsarSDK::hzToScanRate(int hz) {
	switch(hz) {
		case 15: return OB_LIDAR_SCAN_15HZ;
		case 20: return OB_LIDAR_SCAN_20HZ;
		case 25: return OB_LIDAR_SCAN_25HZ;
		case 30: return OB_LIDAR_SCAN_30HZ;
		case 40: return OB_LIDAR_SCAN_40HZ;
		default: return OB_LIDAR_SCAN_20HZ;
	}
}

int OrbbecPulsarSDK::scanRateToPointCount(OBLiDARScanRate rate) {
	switch(rate) {
		case OB_LIDAR_SCAN_15HZ: return 3600;
		case OB_LIDAR_SCAN_20HZ: return 2700;
		case OB_LIDAR_SCAN_25HZ: return 2160;
		case OB_LIDAR_SCAN_30HZ: return 1800;
		case OB_LIDAR_SCAN_40HZ: return 1350;
		default:                 return 2700;
	}
}
