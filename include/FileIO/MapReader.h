#pragma once

#include "global.h"
#include "Logger.h"
#include "tinyxml2/tinyxml2.h"

#include "Structs/MapData.h"
#include "Enums/MapDynamicTileID.h"
#include "FileIO/WorldReader.h"

class MapReader final : public WorldReader {
public:
	bool readWorld(const std::string& filename, MapData& data, const CharacterCore* core);

private:
	void logError(const std::string& error) const override;

	bool readMapProperties(tinyxml2::XMLElement* map, WorldData& data) const override;
	bool readFirstGridIDs(tinyxml2::XMLElement* map, MapData& data);
	bool readCollidableTiles(tinyxml2::XMLElement* firstTile);
	bool readBackgroundTileLayer(const std::string& layer, MapData& data) const;
	bool readLayers(tinyxml2::XMLElement* map, MapData& data) const;
	
	bool readObjects(tinyxml2::XMLElement* map, MapData& data) const;
	bool readNPCs(tinyxml2::XMLElement* objects, MapData& data) const;
	bool readDynamicTiles(tinyxml2::XMLElement* objects, MapData& data) const;
	bool readCollidableObjectLayer(tinyxml2::XMLElement* objects, MapData& data) const;

	// check map bean for validity before loading the map
	bool checkData(MapData& data) const;

	// this is the width in tiles of the dynamic tile tileset
	// and used to calculate the skin of the dynamic tile.
	static const int DYNAMIC_TILE_COUNT;

	int m_firstGidDynamicTiles;

	std::map<int, std::vector<sf::FloatRect>> m_tileColliderMap;
};