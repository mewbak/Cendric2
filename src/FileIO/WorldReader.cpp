#include "FileIO/WorldReader.h"
#include "CharacterCore.h"

#ifndef XMLCheckResult
#define XMLCheckResult(result) if (result != tinyxml2::XML_SUCCESS) {g_logger->logError("MapReader", "XML file could not be read, error: " + std::to_string(static_cast<int>(result))); return false; }
#endif

WorldReader::WorldReader() {
	m_triggerProperties.insert({ "hint", &WorldReader::readHintTrigger });
	m_triggerProperties.insert({ "learn spell", &WorldReader::readLearnSpellTrigger });
	m_triggerProperties.insert({ "condition progress", &WorldReader::readConditionProgressTrigger });
	m_triggerProperties.insert({ "reputation progress", &WorldReader::readReputationProgressTrigger });
	m_triggerProperties.insert({ "questcondition progress", &WorldReader::readQuestConditionProgressTrigger });
	m_triggerProperties.insert({ "questdescription progress", &WorldReader::readQuestDescriptionProgressTrigger });
	m_triggerProperties.insert({ "conditions", &WorldReader::readYesConditionsTrigger });
	m_triggerProperties.insert({ "not conditions", &WorldReader::readNotConditionsTrigger });
	m_triggerProperties.insert({ "map entry", &WorldReader::readMapEntryTrigger });
	m_triggerProperties.insert({ "level entry", &WorldReader::readLevelEntryTrigger });
	m_triggerProperties.insert({ "forced map", &WorldReader::readForcedMapTrigger });
	m_triggerProperties.insert({ "set map", &WorldReader::readSetMapTrigger });
	m_triggerProperties.insert({ "set level", &WorldReader::readSetLevelTrigger });
	m_triggerProperties.insert({ "weather", &WorldReader::readWeatherTrigger });
	m_triggerProperties.insert({ "music", &WorldReader::readMusicTrigger });
	m_triggerProperties.insert({ "achievement core", &WorldReader::readAchievementNotifyCoreTrigger });
	m_triggerProperties.insert({ "achievement notify", &WorldReader::readAchievementNotifyTrigger });
	m_triggerProperties.insert({ "quest state", &WorldReader::readQuestStateChangeTrigger });
	m_triggerProperties.insert({ "add item", &WorldReader::readAddItemTrigger });
	m_triggerProperties.insert({ "cutscene", &WorldReader::readCutsceneTrigger });
	m_triggerProperties.insert({ "credits", &WorldReader::readCreditsTrigger });
	m_triggerProperties.insert({ "forced", &WorldReader::readForcedTrigger });
	m_triggerProperties.insert({ "keyguarded", &WorldReader::readKeyguardedTrigger });
	m_triggerProperties.insert({ "persistent", &WorldReader::readPersistentTrigger });
}

void WorldReader::logError(const std::string& error) const {
	g_logger->logError("WorldReader", "Error in world data : " + error);
}

bool WorldReader::checkData(WorldData& data) const {
	if (data.mapSize.x == 0 || data.mapSize.y == 0) {
		logError("map size not set / invalid");
		return false;
	}
	if (data.tileSetPath.empty()) {
		logError("tileset path not set / empty");
		return false;
	}
	for (size_t i = 0; i < data.backgroundTileLayers.size(); ++i) {
		if (data.backgroundTileLayers[i].empty()) {
			logError("background layer " + std::to_string(i) + std::string(" empty"));
			return false;
		}
		if (static_cast<int>(data.backgroundTileLayers[i].size()) != data.mapSize.x * data.mapSize.y) {
			logError("background layer " + std::to_string(i) + std::string(" has not correct size (map size)"));
			return false;
		}
	}
	for (size_t i = 0; i < data.foregroundTileLayers.size(); ++i) {
		if (data.foregroundTileLayers[i].empty()) {
			logError("foreground layer " + std::to_string(i) + std::string(" empty"));
			return false;
		}
		if (static_cast<int>(data.foregroundTileLayers[i].size()) != data.mapSize.x * data.mapSize.y) {
			logError("foreground layer " + std::to_string(i) + std::string(" has not correct size (map size)"));
			return false;
		}
	}
	for (size_t i = 0; i < data.lightedForegroundTileLayers.size(); ++i) {
		if (data.lightedForegroundTileLayers[i].empty()) {
			logError("lighted foreground layer " + std::to_string(i) + std::string(" empty"));
			return false;
		}
		if (static_cast<int>(data.lightedForegroundTileLayers[i].size()) != data.mapSize.x * data.mapSize.y) {
			logError("lighted foreground layer " + std::to_string(i) + std::string(" has not correct size (map size)"));
			return false;
		}
	}
	if (data.collidableTiles.empty()) {
		logError("collidable layer is empty");
		return false;
	}
	if (static_cast<int>(data.collidableTiles.size()) != data.mapSize.x * data.mapSize.y) {
		logError("collidable layer has not correct size (map size)");
		return false;
	}

	return true;
}

bool WorldReader::readLights(tinyxml2::XMLElement* objectgroup, WorldData& data) const {
	tinyxml2::XMLElement* object = objectgroup->FirstChildElement("object");

	while (object != nullptr) {
		int x;
		tinyxml2::XMLError result = object->QueryIntAttribute("x", &x);
		XMLCheckResult(result);

		int y;
		result = object->QueryIntAttribute("y", &y);
		XMLCheckResult(result);

		int width;
		result = object->QueryIntAttribute("width", &width);
		XMLCheckResult(result);

		int height;
		result = object->QueryIntAttribute("height", &height);
		XMLCheckResult(result);

		LightData lightData;
		lightData.radius.x = width / 2.f;
		lightData.radius.y = height / 2.f;
		lightData.center.x = x + lightData.radius.x;
		lightData.center.y = y + lightData.radius.y;

		tinyxml2::XMLElement* properties = object->FirstChildElement("properties");
		if (properties != nullptr) {
			tinyxml2::XMLElement* _property = properties->FirstChildElement("property");
			while (_property != nullptr) {
				const char* textAttr = nullptr;
				textAttr = _property->Attribute("name");
				if (textAttr == nullptr) {
					logError("XML file could not be read, no objectgroup->object->properties->property->name attribute found for light bean.");
					return false;
				}
				std::string name = textAttr;

				if (name == "brightness") {
					float brightness;
					result = _property->QueryFloatAttribute("value", &brightness);
					XMLCheckResult(result);
					if (brightness < 0.f || brightness > 1.f) {
						brightness = 1.f;
						g_logger->logWarning("WorldReader", "Brightness must be between 0 and 1. It was " + std::to_string(brightness) + ", it is now 1");
					}
					lightData.brightness = brightness;
				}
				else {
					logError("XML file could not be read, unknown objectgroup->object->properties->property->name attribute found for light bean.");
					return false;
				}
				_property = _property->NextSiblingElement("property");
			}
		}

		data.lights.push_back(lightData);
		object = object->NextSiblingElement("object");
	}
	return true;
}

bool WorldReader::readTriggers(tinyxml2::XMLElement* objectgroup, WorldData& data) const {
	tinyxml2::XMLElement* object = objectgroup->FirstChildElement("object");

	while (object != nullptr) {
		TriggerData trigger;
		trigger.worldID = data.id;
		tinyxml2::XMLError result = object->QueryIntAttribute("id", &trigger.objectID);
		XMLCheckResult(result);

		int x;
		result = object->QueryIntAttribute("x", &x);
		XMLCheckResult(result);

		int y;
		result = object->QueryIntAttribute("y", &y);
		XMLCheckResult(result);

		int width;
		result = object->QueryIntAttribute("width", &width);
		XMLCheckResult(result);

		int height;
		result = object->QueryIntAttribute("height", &height);
		XMLCheckResult(result);

		trigger.triggerRect.left = static_cast<float>(x);
		trigger.triggerRect.top = static_cast<float>(y);
		trigger.triggerRect.width = static_cast<float>(width);
		trigger.triggerRect.height = static_cast<float>(height);

		// trigger properties
		tinyxml2::XMLElement* properties = object->FirstChildElement("properties");
		if (properties != nullptr) {
			tinyxml2::XMLElement* _property = properties->FirstChildElement("property");
			while (_property != nullptr) {
				const char* textAttr = nullptr;
				textAttr = _property->Attribute("name");
				if (textAttr == nullptr) {
					logError("XML file could not be read, no property->name attribute found for trigger.");
					return false;
				}

				std::string name = textAttr;

				textAttr = _property->Attribute("value");
				if (textAttr == nullptr) {
					logError("XML file could not be read, " + name  + " value property not found.");
					return false;
				}

				std::string value = textAttr;

				auto entry = m_triggerProperties.find(name);
				if (entry == m_triggerProperties.end()) {
					logError("XML file could not be read, unknown property->name attribute found for trigger: " + name);
					return false;
				}
				
				if (!(*this.*(entry->second))(value, trigger)) {
					logError("XML file could not be read, trigger property invalid\nname: " + name + "\nvalue: " + value);
					return false;
				}

				_property = _property->NextSiblingElement("property");
			}
		}

		data.triggers.push_back(trigger);
		object = object->NextSiblingElement("object");
	}
	return true;
}

bool WorldReader::readBackgroundTileLayer(const std::string& layer, WorldData& data) const {
	std::string layerData = layer;

	size_t pos = 0;
	std::vector<int> backgroundLayer;
	while ((pos = layerData.find(",")) != std::string::npos) {
		backgroundLayer.push_back(std::stoi(layerData.substr(0, pos)));
		layerData.erase(0, pos + 1);
	}
	backgroundLayer.push_back(std::stoi(layerData));

	data.backgroundTileLayers.push_back(backgroundLayer);
	return true;
}

bool WorldReader::readCollidableLayer(const std::string& layer, WorldData& data) const {
	std::string layerData = layer;

	size_t pos = 0;
	size_t index = 0;
	size_t maxindex = data.collidableTiles.size() - 1;
	while ((pos = layerData.find(",")) != std::string::npos && maxindex >= index) {
		if (std::stoi(layerData.substr(0, pos)) != 0) {
			data.collidableTiles.at(index) = true;
		}
		layerData.erase(0, pos + 1);
		index++;
	}
	if (std::stoi(layerData) != 0 && maxindex >= index) {
		data.collidableTiles.at(index) = true;
	}

	return true;
}

bool WorldReader::readForegroundTileLayer(const std::string& layer, WorldData& data) const {
	std::string layerData = layer;

	size_t pos = 0;
	std::vector<int> foregroundLayer;
	while ((pos = layerData.find(",")) != std::string::npos) {
		foregroundLayer.push_back(std::stoi(layerData.substr(0, pos)));
		layerData.erase(0, pos + 1);
	}
	foregroundLayer.push_back(std::stoi(layerData));

	data.foregroundTileLayers.push_back(foregroundLayer);
	return true;
}

bool WorldReader::readLightedForegroundTileLayer(const std::string& layer, WorldData& data) const {
	std::string layerData = layer;

	size_t pos = 0;
	std::vector<int> foregroundLayer;
	while ((pos = layerData.find(",")) != std::string::npos) {
		foregroundLayer.push_back(std::stoi(layerData.substr(0, pos)));
		layerData.erase(0, pos + 1);
	}
	foregroundLayer.push_back(std::stoi(layerData));

	data.lightedForegroundTileLayers.push_back(foregroundLayer);
	return true;
}

bool WorldReader::readTilesetPath(tinyxml2::XMLElement* _property, WorldData& data) const {
	// we've found the property "tilesetpath"
	const char* textAttr = nullptr;
	textAttr = _property->Attribute("value");
	if (textAttr == nullptr) {
		logError("XML file could not be read, no value attribute found (map->properties->property->name=tilesetpath).");
		return false;
	}
	data.tileSetPath = textAttr;
	return true;
}

bool WorldReader::readMusicPath(tinyxml2::XMLElement* _property, WorldData& data) const {
	// we've found the property "musicpath"
	const char* textAttr = nullptr;
	textAttr = _property->Attribute("value");
	if (textAttr == nullptr) {
		logError("XML file could not be read, no value attribute found (map->properties->property->name=musicpath).");
		return false;
	}
	data.musicPath = textAttr;
	return true;
}

bool WorldReader::readWeather(tinyxml2::XMLElement* _property, WorldData& data) const {
	// we've found the property "weather"
	const char* textAttr = nullptr;
	textAttr = _property->Attribute("value");
	if (textAttr == nullptr) {
		logError("XML file could not be read, no value attribute found (map->properties->property->name=weather).");
		return false;
	}
	data.weather.weather = textAttr;
	return true;
}

bool WorldReader::readDimming(tinyxml2::XMLElement* _property, WorldData& data) const {
	// we've found the property "dimming"
	float dimming = 0.f;
	tinyxml2::XMLError result = _property->QueryFloatAttribute("value", &dimming);
	XMLCheckResult(result);

	if (dimming < 0.0f || dimming > 1.0f) {
		logError("XML file could not be read, dimming value not allowed (only [0,1]).");
		return false;
	}

	data.weather.ambientDimming = dimming;
	return true;
}


bool WorldReader::readBackgroundLayers(tinyxml2::XMLElement* _property, WorldData& data) const {
	// nop, is used by the level only and has no effect for the map.
	return true;
}

bool WorldReader::readTileProperties(tinyxml2::XMLElement* map, WorldData& data) {
	tinyxml2::XMLElement* tileset = map->FirstChildElement("tileset");
	while (tileset != nullptr) {
		int firstGid;
		tinyxml2::XMLError result = tileset->QueryIntAttribute("firstgid", &firstGid);
		XMLCheckResult(result);
		
		if (firstGid != 1) {
			// make sure that we only read tile properties from the first tileset (the level tileset)
			tileset = tileset->NextSiblingElement("tileset");
			continue;
		}

		tinyxml2::XMLElement* tile = tileset->FirstChildElement("tile");
		while (tile != nullptr) {
			AnimatedTileData tileData;
			result = tile->QueryIntAttribute("id", &tileData.tileID);
			XMLCheckResult(result);
			tileData.tileID += firstGid;
			tinyxml2::XMLElement* animation = tile->FirstChildElement("animation");
			if (animation != nullptr) {
				tinyxml2::XMLElement* frame = animation->FirstChildElement("frame");
				while (frame != nullptr) {
					std::pair<int, sf::Time> frameData;
					result = frame->QueryIntAttribute("tileid", &frameData.first);
					XMLCheckResult(result);
					frameData.first += firstGid;
					int milliseconds;
					result = frame->QueryIntAttribute("duration", &milliseconds);
					XMLCheckResult(result);
					frameData.second = sf::milliseconds(milliseconds);
					tileData.frames.push_back(frameData);
					frame = frame->NextSiblingElement("frame");
				}
			}
			if (!tileData.frames.empty()) {
				data.animatedTiles.push_back(tileData);
			}

			// properties can be on two locations in the tmx file because of versions
			// tile->objectgroup->properties or just tile->properties. Check where it is.
			tinyxml2::XMLElement* properties = tile->FirstChildElement("properties");

			if (properties == nullptr) {
				tinyxml2::XMLElement* objectgroup = tile->FirstChildElement("objectgroup");
				if (objectgroup) {
					properties = objectgroup->FirstChildElement("properties");
				}
			}

			if (properties != nullptr) {
				const char* textAttr = nullptr;
				tinyxml2::XMLElement* _property = properties->FirstChildElement("property");

				while (_property != nullptr) {
					textAttr = nullptr;
					textAttr = _property->Attribute("name");
					if (textAttr == nullptr) {
						logError("XML file could not be read, no property->name attribute found.");
						return false;
					}
					std::string name = textAttr;

					textAttr = _property->Attribute("value");
					if (textAttr == nullptr) {
						logError("XML file could not be read, no property->value attribute found.");
						return false;
					}
					std::string value = textAttr;

					if (name == "light") {
						// read light
						LightData lightData;
						if (!LightData::resolveLightString(value, lightData)) return false;

						m_lightTiles.insert({ tileData.tileID, lightData });
					}
					else {
						logError("XML file could not be read, unknown name attribute found in tile properties: " + name);
						return false;
					}

					_property = _property->NextSiblingElement("property");
				}
			}

			tile = tile->NextSiblingElement("tile");
		}
		tileset = tileset->NextSiblingElement("tileset");
	}
	return true;
}

bool WorldReader::readMapProperties(tinyxml2::XMLElement* map, WorldData& data) const {
	// check if renderorder is correct
	const char* textAttr = nullptr;
	textAttr = map->Attribute("renderorder");
	if (textAttr == nullptr) {
		logError("XML file could not be read, no renderorder attribute found.");
		return false;
	}
	std::string renderorder = textAttr;
	if (renderorder != "right-down") {
		logError("XML file could not be read, renderorder is not \"right-down\".");
		return false;
	}

	// check if orientation is correct
	textAttr = nullptr;
	textAttr = map->Attribute("orientation");
	if (textAttr == nullptr) {
		logError("XML file could not be read, no orientation attribute found.");
		return false;
	}
	std::string orientation = textAttr;
	if (orientation != "orthogonal") {
		logError("XML file could not be read, renderorder is not \"orthogonal\".");
		return false;
	}

	// read map width and height
	tinyxml2::XMLError result = map->QueryIntAttribute("width", &data.mapSize.x);
	XMLCheckResult(result);
	result = map->QueryIntAttribute("height", &data.mapSize.y);
	XMLCheckResult(result);

	// pre-load collidable layer
	data.collidableTiles.clear();
	for (int i = 0; i < data.mapSize.x * data.mapSize.y; ++i) {
		data.collidableTiles.push_back(false);
	}

	// read tile size
	sf::Vector2i tileSize;
	result = map->QueryIntAttribute("tilewidth", &tileSize.x);
	XMLCheckResult(result);
	result = map->QueryIntAttribute("tileheight", &tileSize.y);
	XMLCheckResult(result);
	if (tileSize.x != TILE_SIZE || tileSize.y != TILE_SIZE) {
		logError("The tilesize for level and map must be " + std::to_string(TILE_SIZE));
		return false;
	}

	return true;
}

bool WorldReader::layerConditionsFulfilled(tinyxml2::XMLElement* layer) const {
	tinyxml2::XMLElement* properties = layer->FirstChildElement("properties");
	if (properties == nullptr) {
		return true;
	}

	tinyxml2::XMLElement* _property = properties->FirstChildElement("property");

	while (_property != nullptr) {
		const char* textAttr = nullptr;
		textAttr = _property->Attribute("name");
		if (textAttr == nullptr) {
			logError("XML file could not be read, no property->name attribute found.");
			continue;
		}

		std::string name = textAttr;
		bool isNotCondition = false;
		if (name == "not conditions") {
			isNotCondition = true;
		}
		else if (name != "conditions") {
			_property = _property->NextSiblingElement("property");
			continue;
		}

		textAttr = _property->Attribute("value");
		if (textAttr == nullptr) {
			g_logger->logWarning("WorldReader", "property 'conditions' or 'not conditions' on layer properties has no content.");
			continue;
		}

		std::string conditions = textAttr;

		size_t pos = 0;

		while (!conditions.empty()) {
			if ((pos = conditions.find(",")) == std::string::npos) {
				logError("Layer conditions could not be read, conditions must be two strings separated by a comma (conditionType,conditionName)*");
				return false;
			}

			std::string conditionType = conditions.substr(0, pos);
			conditions.erase(0, pos + 1);
			std::string conditionName;

			if ((pos = conditions.find(",")) != std::string::npos) {
				conditionName = conditions.substr(0, pos);
				conditions.erase(0, pos + 1);
			}
			else {
				conditionName = conditions;
				conditions.clear();
			}

			if (isNotCondition) {
				if (m_core->isConditionFulfilled(conditionType, conditionName)) {
					return false;
				}
			}
			else {
				if (!m_core->isConditionFulfilled(conditionType, conditionName)) {
					return false;
				}
			}
		}

		_property = _property->NextSiblingElement("property");
	}
	return true;
}

void WorldReader::updateData(WorldData& data) const {
	int x = 0;
	int y = 0;

	std::vector<bool> xLine;

	// calculate collidable tiles
	for (std::vector<bool>::iterator it = data.collidableTiles.begin(); it != data.collidableTiles.end(); ++it) {

		xLine.push_back((*it));
		if (x + 1 >= data.mapSize.x) {
			x = 0;
			y++;
			data.collidableTilePositions.push_back(xLine); // push back creates a copy of that vector.
			xLine.clear();
		}
		else {
			x++;
		}
	}

	// calculate map rect
	data.mapRect.left = 0;
	data.mapRect.top = 0;
	data.mapRect.height = TILE_SIZE_F * data.mapSize.y;
	data.mapRect.width = TILE_SIZE_F * data.mapSize.x;

	// calculate lights from tiles
	readLightsFromLayers(data, data.backgroundTileLayers);
	readLightsFromLayers(data, data.foregroundTileLayers);
	readLightsFromLayers(data, data.lightedForegroundTileLayers);
}

void WorldReader::readLightsFromLayers(WorldData& data, std::vector<std::vector<int>>& layers) const {
	for (auto& layer : layers) {
		int x = 0;
		int y = 0;
		for (size_t i = 0; i < layer.size(); ++i) {
			if (m_lightTiles.find(layer.at(i)) != m_lightTiles.end()) {
				LightData light = m_lightTiles.at(layer.at(i));
				light.center.x += TILE_SIZE_F * x;
				light.center.y += TILE_SIZE_F * y;
				data.lights.push_back(light);
			}

			if (x + 1 >= data.mapSize.x) {
				x = 0;
				++y;
			}
			else {
				++x;
			}
		}
	}
}

bool WorldReader::readHintTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::Hint);
	content.s1 = value;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readLearnSpellTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::LearnSpell);
	content.i1 = std::stoi(value);
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readConditionProgressTrigger(const std::string& value, TriggerData& trigger) const {
	std::string conditionProgress = value;

	size_t pos = 0;
	if ((pos = conditionProgress.find(",")) == std::string::npos) {
		logError("XML file could not be read, condition progress value must be two strings, seperated by a comma.");
		return false;
	}

	TriggerContent content(TriggerContentType::ConditionProgress);
	content.s1 = conditionProgress.substr(0, pos);
	conditionProgress.erase(0, pos + 1);
	content.s2 = conditionProgress;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readReputationProgressTrigger(const std::string& value, TriggerData& trigger) const {
	std::string reputationProgress = value;

	size_t pos = 0;
	if ((pos = reputationProgress.find(",")) == std::string::npos) {
		logError("XML file could not be read, reputation progress value must be two strings, seperated by a comma.");
		return false;
	}

	TriggerContent content(TriggerContentType::ReputationProgress);
	content.s1 = reputationProgress.substr(0, pos);
	FractionID fractionID = resolveFractionID(content.s1);
	if (fractionID == FractionID::VOID) return false;
	content.i1 = static_cast<int>(fractionID);
	reputationProgress.erase(0, pos + 1);
	content.i2 = std::stoi(reputationProgress);
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readQuestConditionProgressTrigger(const std::string& value, TriggerData& trigger) const {
	std::string conditionProgress = value;

	size_t pos = 0;
	if ((pos = conditionProgress.find(",")) == std::string::npos) {
		logError("XML file could not be read, questcondition progress value must be two strings, seperated by a comma.");
		return false;
	}

	TriggerContent content(TriggerContentType::QuestConditionProgress);
	content.s1 = conditionProgress.substr(0, pos);
	conditionProgress.erase(0, pos + 1);
	content.s2 = conditionProgress;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readQuestDescriptionProgressTrigger(const std::string& value, TriggerData& trigger) const {
	std::string conditionProgress = value;

	size_t pos = 0;
	if ((pos = conditionProgress.find(",")) == std::string::npos) {
		logError("XML file could not be read, questdescription progress value must be two strings, seperated by a comma.");
		return false;
	}

	TriggerContent content(TriggerContentType::QuestDescriptionProgress);
	content.s1 = conditionProgress.substr(0, pos);
	conditionProgress.erase(0, pos + 1);
	content.i1 = std::stoi(conditionProgress);
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readConditionsTrigger(const std::string& value, TriggerData& trigger, bool isNotConditions) const {
	std::string conditions = value;

	size_t pos = 0;

	while (!conditions.empty()) {
		if ((pos = conditions.find(",")) == std::string::npos) {
			logError("Trigger conditions could not be read, conditions must be two strings separated by a comma (conditionType,conditionName)*");
			return false;
		}

		std::string conditionType = conditions.substr(0, pos);
		conditions.erase(0, pos + 1);
		std::string conditionName;

		if ((pos = conditions.find(",")) != std::string::npos) {
			conditionName = conditions.substr(0, pos);
			conditions.erase(0, pos + 1);
		}
		else {
			conditionName = conditions;
			conditions.clear();
		}

		Condition cond;
		cond.negative = isNotConditions;
		cond.name = conditionName;
		cond.type = conditionType;
		trigger.conditions.push_back(cond);
	}
	return true;
}

bool WorldReader::readYesConditionsTrigger(const std::string& value, TriggerData& trigger) const {
	return readConditionsTrigger(value, trigger, false);
}

bool WorldReader::readNotConditionsTrigger(const std::string& value, TriggerData& trigger) const {
	return readConditionsTrigger(value, trigger, true);
}

bool WorldReader::readWorldEntryTrigger(const std::string& value, TriggerData& trigger, TriggerContentType type) const {
	std::string worldEntry = value;

	size_t pos = 0;
	if ((pos = worldEntry.find(",")) == std::string::npos) {
		logError("XML file could not be read, world entry value must be a string (world id) and the x and y coords, seperated by commas.");
		return false;
	}

	TriggerContent content(type);
	content.s1 = worldEntry.substr(0, pos);
	worldEntry.erase(0, pos + 1);

	if ((pos = worldEntry.find(",")) == std::string::npos) {
		logError("XML file could not be read, world entry value must be a string (world id) and the x and y coords, seperated by commas.");
		return false;
	}
	content.i1 = std::atoi(worldEntry.substr(0, pos).c_str());
	worldEntry.erase(0, pos + 1);

	content.i2 = std::atoi(worldEntry.c_str());
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readMapEntryTrigger(const std::string& value, TriggerData& trigger) const {
	return readWorldEntryTrigger(value, trigger, TriggerContentType::MapEntry);
}

bool WorldReader::readLevelEntryTrigger(const std::string& value, TriggerData& trigger) const {
	return readWorldEntryTrigger(value, trigger, TriggerContentType::LevelEntry);
}

bool WorldReader::readForcedMapTrigger(const std::string& value, TriggerData& trigger) const {
	return readWorldEntryTrigger(value, trigger, TriggerContentType::SetForcedMap);
}

bool WorldReader::readSetMapTrigger(const std::string& value, TriggerData& trigger) const {
	return readWorldEntryTrigger(value, trigger, TriggerContentType::SetMap);
}

bool WorldReader::readSetLevelTrigger(const std::string& value, TriggerData& trigger) const {
	return readWorldEntryTrigger(value, trigger, TriggerContentType::SetLevel);
}

bool WorldReader::readWeatherTrigger(const std::string& value, TriggerData& trigger) const {
	std::string weatherString = value;

	size_t pos = 0;
	if ((pos = weatherString.find(",")) == std::string::npos) {
		logError("XML file could not be read, weather trigger value must be a string (world id) and the dimming (int between 0 and 100) and the weather (possibly empty), separated by commas.");
		return false;
	}

	TriggerContent content(TriggerContentType::Weather);
	content.s1 = weatherString.substr(0, pos);
	weatherString.erase(0, pos + 1);

	if ((pos = weatherString.find(",")) == std::string::npos) {
		logError("XML file could not be read, weather trigger value must be a string (world id) and the dimming (int between 0 and 100) and the weather (possibly empty), separated by commas.");
		return false;
	}
	content.i1 = std::atoi(weatherString.substr(0, pos).c_str());
	if (content.i1 < 0 || content.i2 > 100) {
		logError("XML file could not be read, weather trigger dimming must be an int between 0 and 100");
		return false;
	}

	weatherString.erase(0, pos + 1);
	content.s2 = weatherString;

	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readMusicTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::Music);
	content.s1 = value;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readAchievementNotifyCoreTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::AchievementNotifyCore);
	content.s1 = value;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readAchievementNotifyTrigger(const std::string& value, TriggerData& trigger) const {
	std::string achId = value;

	size_t pos = 0;
	if ((pos = achId.find(",")) == std::string::npos) {
		logError("XML file could not be read, achievment notify value must be two comma separated strings (achievement id, message)");
		return false;
	}

	TriggerContent content(TriggerContentType::AchievementNotify);
	content.s1 = achId.substr(0, pos);
	achId.erase(0, pos + 1);
	content.s2 = achId;

	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readQuestStateChangeTrigger(const std::string& value, TriggerData& trigger) const {
	std::string questID = value;

	size_t pos = 0;
	if ((pos = questID.find(",")) == std::string::npos) {
		logError("XML file could not be read, quest state trigger value must be two comma separated strings (quest id, quest state)");
		return false;
	}

	TriggerContent content(TriggerContentType::QuestStateChange);
	content.s1 = questID.substr(0, pos);
	questID.erase(0, pos + 1);

	QuestState state = resolveQuestState(questID);
	if (state <= QuestState::VOID || state >= QuestState::MAX) {
		logError("Quest State not resolved: " + std::to_string(static_cast<int>(state)));
	}

	content.i1 = static_cast<int>(state);

	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readAddItemTrigger(const std::string& value, TriggerData& trigger) const {
	std::string item = value;

	size_t pos = 0;
	if ((pos = item.find(",")) == std::string::npos) {
		logError("XML file could not be read, add item trigger value must be two comma separated strings (item id, amount)");
		return false;
	}

	TriggerContent content(TriggerContentType::ItemChange);
	content.s1 = item.substr(0, pos);
	item.erase(0, pos + 1);

	content.i1 = static_cast<int>(std::stoi(item));

	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readCreditsTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::Credits);
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readCutsceneTrigger(const std::string& value, TriggerData& trigger) const {
	TriggerContent content(TriggerContentType::Cutscene);
	content.s1 = value;
	trigger.content.push_back(content);
	return true;
}

bool WorldReader::readForcedTrigger(const std::string& value, TriggerData& trigger) const {
	trigger.isForced = true;
	return true;
}

bool WorldReader::readKeyguardedTrigger(const std::string& value, TriggerData& trigger) const {
	trigger.isKeyGuarded = true;
	return true;
}

bool WorldReader::readPersistentTrigger(const std::string& value, TriggerData& trigger) const {
	trigger.isPersistent = true;
	return true;
}