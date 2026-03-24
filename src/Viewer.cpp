//
//  Viewer.cpp
//

#include "Viewer.hpp"

Viewer::Viewer() {
	scale = 0.0;
	translation = ofPoint::zero();

	circleResolution = 5;
	lastNumberLidarPoints = 0;
	lastGridScale = -1.0f;
	lastGridOrigin = ofPoint(-99999, -99999);
	lastSensorCount = 0;

	mesh.getVertices().reserve(50000);
	mesh.getColors().reserve(50000);
	mesh.getIndices().reserve(50000);

	initializeCircleMeshes();

	blobFont.setBold();
	blobFont.setSize(14);
	sensorFont.setBold();
	sensorFont.setSize(13);
	filterFont.setBold();
	filterFont.setSize(12);
	cursorFont.setBold();
	cursorFont.setSize(12);
	titleFont.setBold();
	titleFont.setSize(14);
	helpFont.setMedium();
	helpFont.setSize(14);
	saveFont.setBold();
	saveFont.setSize(18);
}

Viewer::~Viewer() {
}

void Viewer::setSpace(const Space& _space) {
	space = _space;
	scale = space.width / (space.areaSize * 1000);

	// invalidate caches that depend on scale/space
	lastGridScale  = -1.0f;
	lastGridOrigin = ofPoint(-99999, -99999);
}

void Viewer::setTranslation(ofPoint _translation) {
	translation = _translation;
}

void Viewer::draw(const vector<Blob>& blobs, const vector<Filter*>& filters, const vector<Sensor*>& sensors) {
	ofPushMatrix();
	ofTranslate(translation);
	drawGrid();
	drawBlobs(blobs);
	drawFilters(filters);
	drawSensors(sensors);
	ofPopMatrix();

	for (const auto& sensor : sensors) {
		if (sensor->showSensorInformation) sensor->draw();
	}

	drawCursorCoordinate();
}

void Viewer::drawCoordinates(vector<LidarPoint>& lidarPoints, int numberLidarPoints) {
	initializeTrianglesMesh(lidarPoints, numberLidarPoints);
	ofPushMatrix();
	ofTranslate(translation);
	mesh.draw();
	ofPopMatrix();
}

void Viewer::rebuildGridMesh() {
	gridMesh.clear();
	gridMesh.setMode(OF_PRIMITIVE_LINES);

	float xScalar = 1000.0f * scale;
	float yScalar = 1000.0f * scale;
	float height  = 3000;
	float width   = 3000;

	for (int i = -30; i < 30; i++) {
		ofColor lineColor = (i % 5 == 0) ? ofColor(75, 75, 75) : ofColor(45, 45, 45);
		float x = i * xScalar + space.origin.x;
		gridMesh.addVertex(ofPoint(x, -height));
		gridMesh.addColor(lineColor);
		gridMesh.addVertex(ofPoint(x, height));
		gridMesh.addColor(lineColor);
	}

	for (int i = -30; i < 30; i++) {
		ofColor lineColor = (i % 5 == 0) ? ofColor(75, 75, 75) : ofColor(45, 45, 45);
		float y = i * yScalar + space.origin.y;
		gridMesh.addVertex(ofPoint(-width, y));
		gridMesh.addColor(lineColor);
		gridMesh.addVertex(ofPoint(width, y));
		gridMesh.addColor(lineColor);
	}

	lastGridScale  = scale;
	lastGridOrigin = space.origin;
}

void Viewer::drawGrid() {
	if (scale != lastGridScale || space.origin != lastGridOrigin) {
		rebuildGridMesh();
	}

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0x3030);
	gridMesh.draw();
	glDisable(GL_LINE_STIPPLE);

	ofSetColor(ofColor::thistle);
	ofFill();
	ofSetCircleResolution(23);
	ofDrawCircle(space.origin, 4);
}

// -----------------------------------------------------------------------------
// Cursor
// -----------------------------------------------------------------------------

void Viewer::drawCursorCoordinate() {
	float x      = ofGetWidth() - 90;
	float y      = ofGetHeight() - 20;
	float length = cursorFont.getStringWidth(cursorString);
	float height = 20;

	ofColor cursorBackgroundColor = ofColor::black;
	cursorBackgroundColor.a = 160;
	ofFill();
	ofSetColor(cursorBackgroundColor);
	ofDrawRectangle(x, y - 15, length, height);

	ofSetColor(ofColor::thistle);
	cursorFont.draw(cursorString, x, y);
}

// -----------------------------------------------------------------------------
// Blobs
// -----------------------------------------------------------------------------

void Viewer::drawBlobs(const vector<Blob>& blobs) {
	ofNoFill();
	ofSetColor(0, 0, 255);

	for (const auto& blob : blobs) {
		float scaledX = blob.centroid.x * scale + space.origin.x;
		float scaledY = blob.centroid.y * scale + space.origin.y;

		float boxX = blob.bounds.getX() * scale + space.origin.x;
		float boxY = blob.bounds.getY() * scale + space.origin.y;
		float boxW = blob.bounds.getWidth() * scale;
		float boxH = blob.bounds.getHeight() * scale;

		ofDrawRectangle(boxX, boxY, boxW, boxH);

		ofFill();
		ofSetColor(255, 0, 0);
		ofDrawEllipse(scaledX, scaledY, 9, 9);

		// batch label strings rather than 4 separate sprintf/draw calls
		string label = to_string(blob.index) + "\n"
					 + "x: " + ofToString(blob.centroid.x / 1000.0f, 2) + "\n"
					 + "y: " + ofToString(blob.centroid.y / 1000.0f, 2) + "\n"
					 + "pts: " + to_string(blob.numberPoints);

		float labelX = scaledX + 15;
		float lineH  = 14;
		blobFont.draw(to_string(blob.index),                               labelX, scaledY - 21);
		blobFont.draw("x: " + ofToString(blob.centroid.x / 1000.0f, 2),   labelX, scaledY - 7);
		blobFont.draw("y: " + ofToString(blob.centroid.y / 1000.0f, 2),   labelX, scaledY + 7);
		blobFont.draw("points: " + to_string(blob.numberPoints),           labelX, scaledY + 21);

		ofNoFill();
		ofSetColor(0, 0, 255);
	}
}

// -----------------------------------------------------------------------------
// Point cloud mesh - indices rebuilt only when point count changes
// -----------------------------------------------------------------------------

void Viewer::initializeTrianglesMesh(const vector<LidarPoint>& lidarPoints, int numberLidarPoints) {
	if (numberLidarPoints != lastNumberLidarPoints) {
		mesh.clear();
		mesh.setMode(OF_PRIMITIVE_TRIANGLES);

		int totalVerts = numberLidarPoints * (circleResolution + 1);
		mesh.getVertices().assign(totalVerts, ofPoint::zero());
		mesh.getColors().assign(totalVerts, ofColor::black);

		// build index buffer once
		for (int i = 0; i < numberLidarPoints; i++) {
			int base = (circleResolution + 1) * i;
			for (int j = 0; j < circleResolution; j++) {
				mesh.addIndex(base);
				mesh.addIndex(base + j + 1);
				mesh.addIndex(j < circleResolution - 1 ? base + j + 2 : base + 1);
			}
		}

		lastNumberLidarPoints = numberLidarPoints;
	}

	// update only vertex/color data each frame
	auto& verts  = mesh.getVertices();
	auto& colors = mesh.getColors();

	for (int i = 0; i < numberLidarPoints; i++) {
		ofColor pointColor = lidarPoints[i].color;
		float radius;

		if (lidarPoints[i].isInFilter) {
			radius = 2.5f;
			pointColor.a = 255;
		} else {
			radius = 1.5f;
			pointColor.a = 90;
		}

		ofPoint coordinate = lidarPoints[i].coordinate * scale + space.origin;
		int base = (circleResolution + 1) * i;

		verts[base]  = coordinate;
		colors[base] = pointColor;

		for (int j = 0; j < circleResolution; j++) {
			verts[base + j + 1]  = coordinate + circleMesh.getVertex(j) * radius;
			colors[base + j + 1] = pointColor;
		}
	}
}

void Viewer::initializeCircleMeshes() {
	circleMesh.clear();
	float deltaTheta = TWO_PI / float(circleResolution);

	for (float i = 0; i < TWO_PI; i += deltaTheta) {
		circleMesh.addVertex(ofVec3f(cos(i), sin(i), 0));
	}
}

// -----------------------------------------------------------------------------
// Sensors
// -----------------------------------------------------------------------------

void Viewer::drawSensors(const vector<Sensor*>& sensors) {
	for (const auto& sensor : sensors) {
		drawSensor(sensor);
	}
}

void Viewer::rebuildConnectionStrings(const vector<Sensor*>& sensors) {
	cachedConnectionStrings.clear();
	cachedConnectionStrings.reserve(sensors.size());
	for (int i = 0; i < sensors.size(); i++) {
		cachedConnectionStrings.push_back("Sensor " + to_string(i + 1) + ": " + sensors[i]->model);
	}
	lastSensorCount = sensors.size();
}

void Viewer::drawSensor(const Sensor* sensor) {
	if (!sensor) return;
	ofPoint point = ofPoint(sensor->position.x, sensor->position.y);

	point *= scale;
	point += space.origin;

	float size     = sensor->position.size;
	float halfSize = sensor->position.halfSize;
	float noseRadius = sensor->noseRadius;
	float noseSize = sensor->nosePosition.size;

	ofSetColor(sensor->sensorColor);
	if (sensor->position.isMouseOver) {
		ofFill();
	} else {
		ofNoFill();
	}

	ofPushMatrix();
	ofTranslate(point.x, point.y);
	ofRotateRad(sensor->sensorRotationRad);
	ofDrawRectangle(-halfSize, -halfSize, size, size);

	if (sensor->isConnected) {
		float time = fmod(ofGetElapsedTimef(), 1.0);
		for (int i = 0; i < 3; i++) {
			float t = 1.0f / 3.0f * i;
			float s = ofMap(fmod((time + t), 1.0f), 0.0f, 1.0f, 0, size);
			ofRectangle r;
			r.setFromCenter(0, 0, s, s);
			ofDrawRectangle(r);
		}
	}

	if (sensor->nosePosition.isMouseOver) {
		ofFill();
	} else {
		ofNoFill();
	}

	ofRectangle nose;
	nose.setFromCenter(0, noseRadius, noseSize / 2.0f, noseSize / 2.0f);
	ofDrawRectangle(nose);
	ofDrawLine(0, halfSize, 0, size + halfSize);

	ofPopMatrix();
}

// -----------------------------------------------------------------------------
// Filters
// -----------------------------------------------------------------------------

void Viewer::drawFilters(const vector<Filter*>& filters) {
	for (auto& filter : filters) {
		drawFilter(filter);
	}
}

void Viewer::drawFilter(Filter* filter) {
	ofColor filterColor = ofColor::magenta;

	if (!filter->isBlobInside) filterColor.a = 150;
	if (filter->isMask)        filterColor = ofColor::lightPink;
	if (!filter->isActive)     filterColor.lerp(ofColor::grey, 0.95);

	ofNoFill();
	ofSetColor(filterColor);
	filter->drawOutline();

	ofColor shapeColor = ofColor(filterColor.r, filterColor.g, filterColor.b, 40);
	ofSetColor(shapeColor);
	ofFill();
	if (filter->isMask) filter->drawShape();

	drawDraggablePoints(filter);
}

void Viewer::drawDraggablePoints(const Filter& bounds) {
	for (const auto& draggablePoint : bounds.draggablePoints) {
		ofPoint point = draggablePoint * 1000.0f * scale + space.origin;

		ofRectangle p;
		float r = draggablePoint.size;
		p.setFromCenter(point.x, point.y, r, r);
		ofNoFill();

		if (draggablePoint.isMouseOver) ofFill();

		ofSetColor(ofColor::magenta);
		ofDrawRectangle(p);

		if (bounds.centroid.isMouseOver) {
			ofPoint centroidPoint = bounds.centroid * 1000.0f * scale + space.origin;
			ofDrawLine(point.x, point.y, centroidPoint.x, centroidPoint.y);
		}
	}

	if (bounds.centroid.isMouseOver) {
		ofPoint centroidPoint = bounds.centroid * 1000.0f * scale + space.origin;

		ofRectangle p;
		float r = bounds.centroid.size;
		p.setFromCenter(centroidPoint.x, centroidPoint.y, r, r);

		ofFill();
		ofSetColor(ofColor::magenta);
		ofDrawRectangle(p);
	}
}

void Viewer::drawDraggablePoints(const Filter* filter) {
	ofColor filterColor = ofColor::magenta;
	if (!filter->isBlobInside) filterColor.a = 150;
	if (filter->isMask)        filterColor = ofColor::lightPink;
	if (!filter->isActive)     filterColor.lerp(ofColor::grey, 0.95);

	for (auto& draggablePoint : filter->draggablePoints) {
		ofPoint point = draggablePoint * 1000.0f * scale + space.origin;

		ofRectangle p;
		float r = draggablePoint.size;
		p.setFromCenter(point.x, point.y, r, r);
		ofNoFill();

		if (draggablePoint.isMouseOver) ofFill();

		ofSetColor(filterColor);
		ofDrawRectangle(p);

		if (filter->centroid.isMouseOver) {
			ofPoint centroidPoint = filter->centroid * 1000.0f * scale + space.origin;
			ofDrawLine(point.x, point.y, centroidPoint.x, centroidPoint.y);
		}
	}

	ofSetColor(filterColor);
	ofPoint indexPoint = filter->centroid * 1000.0f * scale + space.origin;
	filterFont.draw(to_string(filter->index), indexPoint.x - 3, indexPoint.y + 3);

	if (filter->centroid.isMouseOver) {
		ofPoint centroidPoint = filter->centroid * 1000.0f * scale + space.origin;

		ofRectangle p;
		float r = filter->centroid.size;
		p.setFromCenter(centroidPoint.x, centroidPoint.y, r, r);

		ofFill();
		ofSetColor(filterColor);
		ofDrawRectangle(p);
	}
}

// -----------------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------------

void Viewer::setCursorString(const ofPoint& mousePoint) {
	ofPoint point = (mousePoint - space.origin - translation) / scale * 0.001f;

	char buffer[32];
	sprintf(buffer, "%.3f %.3f", point.x, point.y);
	cursorString = buffer;
}

void Viewer::onMouseMoved(ofMouseEventArgs& mouseArgs) {
	setCursorString(ofPoint(mouseArgs.x, mouseArgs.y));
}

void Viewer::onMouseDragged(ofMouseEventArgs& mouseArgs) {
	setCursorString(ofPoint(mouseArgs.x, mouseArgs.y));
}

// -----------------------------------------------------------------------------
// Help / save UI
// -----------------------------------------------------------------------------

void Viewer::drawHelpText() {
	ofSetColor(ofColor::thistle);

	titleFont.draw("meatbags v" + version, 15, 20);
	helpFont.draw("headless mode", 15, 40);

	helpFont.draw("(h) toggle help file", 15, 80);
	helpFont.draw("(m) hold and move mouse to translate grid", 15, 100);
	helpFont.draw("(f) press while over the center of a filter to toggle mask/filter", 15, 120);
	helpFont.draw("(t) press while over the center of a filter to toggle active/inactive", 15, 140);
	helpFont.draw("(ctrl/cmd + s) press to save", 15, 160);

	titleFont.draw("blob OSC format", 15, 200);
	helpFont.draw("/blob index x y width length laserIntensity filterIndex1 filterIndex2 ...", 15, 220);
	helpFont.draw("/blobsActive index1 index2 ...", 15, 240);

	titleFont.draw("filter OSC format", 15, 280);
	helpFont.draw("/filter index isAnyBlobInside blobDistanceToCentroid", 15, 300);
	helpFont.draw("/filterBlob filterIndex blobIndex x y width height", 15, 320);
	helpFont.draw("/filterBlobs filterIndex blobIndex1 x1 y1 blobIndex2 x2 y2 ...", 15, 340);

	titleFont.draw("logging OSC format", 15, 380);
	helpFont.draw("/generalStatus sensorIndex status", 15, 400);
	helpFont.draw("/connectionStatus sensorIndex status", 15, 420);
	helpFont.draw("/laserStatus sensorIndex status", 15, 440);
}

void Viewer::drawSaveNotification() {
	string saveText = "configuration saved";
	float stringWidth = saveFont.getStringWidth(saveText);
	ofRectangle saveRectangle;
	saveRectangle.setFromCenter(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f - 5, stringWidth + 50, 35);

	ofSetColor(ofColor::black);
	ofDrawRectangle(saveRectangle);
	ofSetColor(ofColor::thistle);
	ofNoFill();
	ofDrawRectangle(saveRectangle);
	saveFont.draw(saveText, ofGetWidth() * 0.5f - stringWidth * 0.5f, ofGetHeight() * 0.5f);
}
