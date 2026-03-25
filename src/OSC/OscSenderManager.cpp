//
//  OscSenderManager.cpp
//  meatbags

#include "OscSenderManager.hpp"

OscSenderManager::OscSenderManager() {
}

void OscSenderManager::addOscSender() {
	auto oscSender = std::make_unique<OscSender>();
	oscSender->index = oscSenderEntries.size() + 1;
	oscSenderEntries.push_back({ std::move(oscSender) });
}

void OscSenderManager::removeOscSender() {
	if (oscSenderEntries.size() > 1) {
		oscSenderEntries.pop_back();
	}
}

vector<OscSender*> OscSenderManager::getOscSenders() {
	vector<OscSender*> result;
	for (auto& entry : oscSenderEntries) {
		result.push_back(entry.oscSender.get());
	}
	return result;
}

void OscSenderManager::send(vector<Blob>& blobs, const vector<Sensor*> sensors, const vector<Filter*>& filters) {
	for (auto& entry : oscSenderEntries) {
		if (entry.oscSender->sendBlobsActive)   entry.oscSender->sendBlobOsc(blobs, filters);
		if (entry.oscSender->sendFiltersActive) entry.oscSender->sendFilterOsc(filters);
		if (entry.oscSender->sendLogsActive)    entry.oscSender->sendLogs(sensors);
	}
}

void OscSenderManager::initialize() {
	addOscSender();
}

void OscSenderManager::loadOscSenders(int n, ofJson& config) {
	for (int i = 0; i < n; i++) addOscSender();

	for (int i = 0; i < oscSenderEntries.size(); i++) {
		string key = "osc_sender_" + to_string(i + 1);
		if (!config.contains(key)) continue;
		ofJson& s = config[key];
		auto& sender = oscSenderEntries[i].oscSender;

		if (s.contains("address"))       sender->oscSenderAddress  = s["address"].get<string>();
		if (s.contains("port"))          sender->oscSenderPort     = s["port"].get<int>();
		if (s.contains("send_blobs"))    sender->sendBlobsActive   = s["send_blobs"].get<bool>();
		if (s.contains("send_filters"))  sender->sendFiltersActive = s["send_filters"].get<bool>();
		if (s.contains("send_logs"))     sender->sendLogsActive    = s["send_logs"].get<bool>();
	}
}

void OscSenderManager::load(ofJson config) {
	if (config.contains("number_osc_senders")) {
		int n = config["number_osc_senders"];
		loadOscSenders(n, config);
	} else {
		addOscSender();
	}
}

void OscSenderManager::saveTo(ofJson& config) {
	config["number_osc_senders"] = oscSenderEntries.size();
	for (int i = 0; i < oscSenderEntries.size(); i++) {
		string key = "osc_sender_" + to_string(i + 1);
		auto& sender = oscSenderEntries[i].oscSender;
		config[key]["address"]      = sender->oscSenderAddress.get();
		config[key]["port"]         = sender->oscSenderPort.get();
		config[key]["send_blobs"]   = sender->sendBlobsActive.get();
		config[key]["send_filters"] = sender->sendFiltersActive.get();
		config[key]["send_logs"]    = sender->sendLogsActive.get();
	}
}
