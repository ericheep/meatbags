//
//  FilterManager.cpp
//  meatbags

#include "FilterManager.hpp"

FilterManager::FilterManager() {
}

FilterManager::~FilterManager() {
	clear();
}

void FilterManager::clear() {
	filterEntries.clear();
}

void FilterManager::initialize() {
	addFilter();
}

void FilterManager::addFilter() {
	addFilter(FilterType::Quad);
}

void FilterManager::addFilter(FilterType type) {
	auto filter = createFilterOfType(type);
	filter->index = filterEntries.size() + 1;

	// default position spread in a circle
	ofPoint center(0, 1.25);
	float ratio = float(filter->index - 1) / 8.0f;
	float cx = cos(ratio * TWO_PI - HALF_PI) * 2.25f + center.x;
	float cy = sin(ratio * TWO_PI - HALF_PI) * 2.25f + center.y;

	for (int i = 0; i < 4; i++) {
		float r = float(i) / 4.0f;
		float x = cos(r * TWO_PI + HALF_PI * 0.5f) * 0.5f + cx;
		float y = sin(r * TWO_PI + HALF_PI * 0.5f) * 0.5f + cy;
		filter->anchorPoints[i] = ofVec2f(x, y);
	}

	filter->update();
	filter->reset();
	filter->updateNormalization();

	filterEntries.push_back({ std::move(filter) });
}

void FilterManager::removeFilter() {
	if (filterEntries.size() > 1) {
		filterEntries.pop_back();
	}
}

void FilterManager::changeFilterType(int index, FilterType newType) {
	if (index < 0 || index >= filterEntries.size()) return;

	auto& entry = filterEntries[index];
	FilterType currentType = getCurrentFilterType(entry.filter.get());
	if (currentType == newType) return;

	Filter* oldFilter = entry.filter.get();
	auto newFilter = createFilterOfType(newType);
	if (!newFilter) return;

	transferFilterState(oldFilter, newFilter.get());
	entry.filter = std::move(newFilter);
}

void FilterManager::update() {
	for (auto& entry : filterEntries) {
		if (entry.filter) entry.filter->update();
	}

	for (int i = 0; i < filterEntries.size(); i++) {
		auto& entry = filterEntries[i];
		if (!entry.nextType.empty()) {
			string type = entry.nextType;
			entry.nextType.clear();
			FilterType newType     = stringToFilterType(type);
			FilterType currentType = getCurrentFilterType(entry.filter.get());
			if (newType != currentType) changeFilterType(i, newType);
		}
	}
}

void FilterManager::transferFilterState(Filter* oldFilter, Filter* newFilter) {
	if (!oldFilter || !newFilter) return;

	newFilter->index      = oldFilter->index;
	newFilter->isMask     = oldFilter->isMask.get();
	newFilter->isNormalized = oldFilter->isNormalized.get();

	for (int i = 0; i < 4; i++) {
		newFilter->anchorPoints[i] = oldFilter->anchorPoints[i].get();
	}

	newFilter->setSpace(oldFilter->space);
	newFilter->setTranslation(oldFilter->translation);
	newFilter->updateNormalization();
}

vector<Filter*> FilterManager::getFilters() {
	vector<Filter*> result;
	for (auto& entry : filterEntries) {
		result.push_back(entry.filter.get());
	}
	return result;
}

std::unique_ptr<Filter> FilterManager::createFilterOfType(FilterType type) {
	switch (type) {
		case FilterType::Ellipse: return std::make_unique<EllipseFilter>();
		case FilterType::Quad:    return std::make_unique<QuadFilter>();
		default:                  return std::make_unique<QuadFilter>();
	}
}

FilterType FilterManager::getCurrentFilterType(Filter* filter) {
	if (dynamic_cast<EllipseFilter*>(filter)) return FilterType::Ellipse;
	if (dynamic_cast<QuadFilter*>(filter))    return FilterType::Quad;
	return FilterType::Quad;
}

FilterType FilterManager::stringToFilterType(const std::string& s) {
	if (s == "ellipse" || s == "Ellipse") return FilterType::Ellipse;
	return FilterType::Quad;
}

string FilterManager::filterTypeToString(FilterType type) {
	switch (type) {
		case FilterType::Ellipse: return "ellipse";
		case FilterType::Quad:    return "quad";
		default:                  return "quad";
	}
}

void FilterManager::checkBlobs(std::vector<Blob>& blobs) {
	for (auto& entry : filterEntries) {
		entry.filter->checkBlobs(blobs);
	}
}

void FilterManager::setSpace(Space& space) {
	for (auto& entry : filterEntries) {
		entry.filter->setSpace(space);
	}
}

void FilterManager::setTranslation(ofPoint translation) {
	for (auto& entry : filterEntries) {
		entry.filter->setTranslation(translation);
	}
}

bool FilterManager::onMouseMoved(ofMouseEventArgs& mouseArgs) {
	for (auto& entry : filterEntries) {
		if (entry.filter->onMouseMoved(mouseArgs)) return true;
	}
	return false;
}

bool FilterManager::onMousePressed(ofMouseEventArgs& mouseArgs) {
	for (auto& entry : filterEntries) {
		if (entry.filter->onMousePressed(mouseArgs)) return true;
	}
	return false;
}

bool FilterManager::onMouseDragged(ofMouseEventArgs& mouseArgs) {
	for (auto& entry : filterEntries) {
		if (entry.filter->onMouseDragged(mouseArgs)) return true;
	}
	return false;
}

bool FilterManager::onMouseReleased(ofMouseEventArgs& mouseArgs) {
	for (auto& entry : filterEntries) {
		if (entry.filter->onMouseReleased(mouseArgs)) return true;
	}
	return false;
}

bool FilterManager::onKeyPressed(ofKeyEventArgs& keyArgs) {
	for (auto& entry : filterEntries) {
		if (entry.filter->onKeyPressed(keyArgs)) return true;
	}
	return false;
}

// -----------------------------------------------------------------------------
// Save / Load
// -----------------------------------------------------------------------------

void FilterManager::saveTo(ofJson& config) {
	config["number_filters"] = filterEntries.size();
	for (int i = 0; i < filterEntries.size(); i++) {
		string key = "filter_" + to_string(i + 1);
		Filter* f  = filterEntries[i].filter.get();

		config[key]["type"]       = filterTypeToString(getCurrentFilterType(f));
		config[key]["is_mask"]    = f->isMask.get();
		config[key]["normalized"] = f->isNormalized.get();

		for (int p = 0; p < 4; p++) {
			config[key]["anchor_" + to_string(p)]["x"] = f->anchorPoints[p].get().x;
			config[key]["anchor_" + to_string(p)]["y"] = f->anchorPoints[p].get().y;
		}
	}
}

void FilterManager::loadFilters(int n, ofJson& config) {
	for (int i = 0; i < n; i++) addFilter();

	for (int i = 0; i < filterEntries.size(); i++) {
		string key = "filter_" + to_string(i + 1);
		if (!config.contains(key)) continue;
		ofJson& fc = config[key];
		Filter* f  = filterEntries[i].filter.get();

		if (fc.contains("type")) {
			FilterType newType = stringToFilterType(fc["type"].get<string>());
			if (newType != getCurrentFilterType(f)) changeFilterType(i, newType);
			f = filterEntries[i].filter.get(); // refresh pointer after potential type change
		}
		if (fc.contains("is_mask"))    f->isMask      = fc["is_mask"].get<bool>();
		if (fc.contains("normalized")) f->isNormalized = fc["normalized"].get<bool>();

		for (int p = 0; p < 4; p++) {
			string pk = "anchor_" + to_string(p);
			if (fc.contains(pk)) {
				float ax = fc[pk].value("x", 0.0f);
				float ay = fc[pk].value("y", 0.0f);
				f->anchorPoints[p] = ofVec2f(ax, ay);
			}
		}

		f->update();
		f->updateNormalization();
	}
}

void FilterManager::load(ofJson config) {
	if (config.contains("number_filters")) {
		int n = config["number_filters"];
		loadFilters(n, config);
	} else {
		addFilter();
	}
}
