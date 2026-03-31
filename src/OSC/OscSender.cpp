//
//  OscSender.cpp
//

#include "OscSender.hpp"

OscSender::OscSender() {
	oscSenderAddress.addListener(this, &OscSender::setOscSenderAddress);
	oscSenderPort.addListener(this, &OscSender::setOscSenderPort);

	oscSenderAddress = "127.0.0.1";
	oscSenderPort    = 5322;
	index            = 0;
}

OscSender::~OscSender() {
	oscSenderAddress.removeListener(this, &OscSender::setOscSenderAddress);
	oscSenderPort.removeListener(this, &OscSender::setOscSenderPort);
}

void OscSender::setOscSenderAddress(string& oscSenderAddress) {
	if (oscSenderPort == 0) return;
	oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::setOscSenderPort(int& oscSenderPort) {
	if (oscSenderPort == 0) return;
	oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::sendBlobOsc(vector<Blob>& blobs, const vector<Filter*>& filters) {
	ofxOscMessage blobsActiveMsg;
	blobsActiveMsg.setAddress("/blobsActive");

	ofxOscMessage blobsMsg;
	blobsMsg.setAddress("/blobs");

	for (auto& blob : blobs) {
		ofxOscMessage msg;
		msg.setAddress("/blob");
		msg.addIntArg(blob.index);

		// millimeters to meters
		float x = blob.centroid.x * 0.001f;
		float y = blob.centroid.y * 0.001f;

		msg.addFloatArg(x);
		msg.addFloatArg(y);
		msg.addFloatArg(blob.bounds.width);
		msg.addFloatArg(blob.bounds.height);
		msg.addFloatArg(blob.velocity.x);
		msg.addFloatArg(blob.velocity.y);
		
		oscSender.sendMessage(msg);

		blobsMsg.addIntArg(blob.index);
		blobsMsg.addFloatArg(x);
		blobsMsg.addFloatArg(y);

		blobsActiveMsg.addIntArg(blob.index);
	}

	if (!blobs.empty()) {
		oscSender.sendMessage(blobsMsg);
		oscSender.sendMessage(blobsActiveMsg);
	}
}

void OscSender::sendFilterOsc(const vector<Filter*>& filters) {
	sendFilterStatus(filters);
	sendFilterBlobs(filters);
	sendFilterBlob(filters);
}

void OscSender::sendFilterStatus(const vector<Filter*>& filters) {
	for (auto& filter : filters) {
		ofxOscMessage msg;
		msg.setAddress("/filter");
		msg.addIntArg(filter->index);
		msg.addIntArg(filter->isBlobInside);
		if (filter->isBlobInside) msg.addFloatArg(filter->distanceOfClosestBlob);
		oscSender.sendMessage(msg);
	}
}

void OscSender::sendFilterBlobs(const vector<Filter*>& filters) {
	for (auto& filter : filters) {
		if (!filter->isBlobInside) continue;

		ofxOscMessage msg;
		msg.setAddress("/filterBlobs");
		msg.addIntArg(filter->index);

		for (auto& blob : filter->filterBlobs) {
			msg.addIntArg(blob.index);

			float x = blob.centroid.x * 0.001f;
			float y = blob.centroid.y * 0.001f;

			if (filter->isNormalized) {
				ofPoint n = filter->normalizeCoordinate(x, y);
				x = n.x;
				y = n.y;
			}

			msg.addFloatArg(x);
			msg.addFloatArg(y);
		}

		oscSender.sendMessage(msg);
	}
}

void OscSender::sendFilterBlob(const vector<Filter*>& filters) {
	for (auto& filter : filters) {
		if (!filter->isBlobInside) continue;

		for (auto& blob : filter->filterBlobs) {
			ofxOscMessage msg;
			msg.setAddress("/filterBlob");
			msg.addIntArg(filter->index);
			msg.addIntArg(blob.index);

			float x      = blob.centroid.x * 0.001f;
			float y      = blob.centroid.y * 0.001f;
			float width  = blob.bounds.getWidth()  * 0.001f;
			float height = blob.bounds.getHeight() * 0.001f;

			if (filter->isNormalized) {
				ofPoint nc = filter->normalizeCoordinate(x, y);
				ofPoint ns = filter->normalizeSize(x, y, width, height);
				x      = nc.x;
				y      = nc.y;
				width  = ns.x;
				height = ns.y;
			}

			msg.addFloatArg(x);
			msg.addFloatArg(y);
			msg.addFloatArg(width);
			msg.addFloatArg(height);
			msg.addFloatArg(blob.velocity.x);
			msg.addFloatArg(blob.velocity.y);

			oscSender.sendMessage(msg);
		}
	}
}

void OscSender::sendLogs(const vector<Sensor*> sensors) {
	for (auto& sensor : sensors) {
		string connectionStatus = sensor->logConnectionStatus;
		string status           = sensor->logStatus;
		string mode             = sensor->logMode;

		if (lastConnectionStatus != connectionStatus) {
			lastConnectionStatus = connectionStatus;
			ofxOscMessage msg;
			msg.setAddress("/connectionStatus");
			msg.addIntArg(sensor->index);
			msg.addStringArg(connectionStatus);
			oscSender.sendMessage(msg);
		}

		if (lastStatus != status) {
			lastStatus = status;
			ofxOscMessage msg;
			msg.setAddress("/generalStatus");
			msg.addIntArg(sensor->index);
			msg.addStringArg(status);
			oscSender.sendMessage(msg);
		}

		if (lastMode != mode) {
			lastMode = mode;
			ofxOscMessage msg;
			msg.setAddress("/laserStatus");
			msg.addIntArg(sensor->index);
			msg.addStringArg(mode);
			oscSender.sendMessage(msg);
		}
	}
}
