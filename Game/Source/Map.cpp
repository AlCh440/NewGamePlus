#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "../Collider.h"
#include "../Collisions.h"
#include "Scene.h"
//#include "EnemyController.h"

#include "Defs.h"
#include "Log.h"
#include "List.h"

#include <math.h>

Map::Map() : Module(), mapLoaded(false)
{
	name.Create("map");
}

// Destructor
Map::~Map()
{}

int Properties::GetProperty(const char* value, int defaultValue) const
{
	ListItem<Property*>* item = list.start;

	while (item)
	{
		if (item->data->name == value)
			return item->data->value;
		item = item->next;
	}

	return defaultValue;
}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.Create(config.child("folder").child_value());

	return ret;
}

// Draw the map (all requried layers)
void Map::Draw()
{
	if (mapLoaded == false)
	{
		LOG("NOTTTTT");
		return;
	}
	// L04: DONE 5: Prepare the loop to draw all tilesets + DrawTexture()
	ListItem<MapLayer*>* mapLayerItem;
	mapLayerItem = mapData.maplayers.start;

	// L06: TODO 4: Make sure we draw all the layers and not just the first one
	while (mapLayerItem != NULL) {

		if (mapLayerItem->data->properties.GetProperty("Draw") == 1) {

			for (int x = 0; x < mapLayerItem->data->width; x++)
			{
				for (int y = 0; y < mapLayerItem->data->height; y++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0) {

						//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
						//now we always use the firt tileset in the list
						//TileSet* tileset = mapData.tilesets.start->data;
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						app->render->DrawTexture(tileset->texture,
							pos.x,
							pos.y,
							&r);
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}
}

// L04: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	// L05: DONE 1: Add isometric map to world coordinates
	if (mapData.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * mapData.tileWidth;
		ret.y = y * mapData.tileHeight;
	}
	else if (mapData.type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (mapData.tileWidth / 2);
		ret.y = (x + y) * (mapData.tileHeight / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

// L05: DON 2: Add orthographic world to map coordinates
iPoint Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	// L05: DONE 3: Add the case for isometric maps to WorldToMap
	if (mapData.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / mapData.tileWidth;
		ret.y = y / mapData.tileHeight;
	}
	else if (mapData.type == MAPTYPE_ISOMETRIC)
	{

		float half_width = mapData.tileWidth * 0.5f;
		float half_height = mapData.tileHeight * 0.5f;
		ret.x = int((x / half_width + y / half_height) / 2);
		ret.y = int((y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

bool Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	bool ret = false;
	ListItem<MapLayer*>* item;
	item = mapData.maplayers.start;

	for (item = mapData.maplayers.start; item != NULL; item = item->next)
	{
		MapLayer* layer = item->data;

		if (layer->properties.GetProperty("Col", 0) == 0)
			continue;


		uchar* map = new uchar[layer->width * layer->height];
		memset(map, 1, layer->width * layer->height);

		for (int y = 0; y < mapData.height; ++y)
		{
			for (int x = 0; x < mapData.width; ++x)
			{
				int i = (y * layer->width) + x;
				int gid = layer->Get(x, y);

				//TileSet* tileId = GetTilesetFromTileId(gid);

				if (gid != NULL)
				{
					if (gid > 0)
					{
						LOG("YES");
						map[i] = 0;
					}
				}
				else
				{
					map[i] = 1;
				}
			}
		}

		*buffer = map;
		width = mapData.width;
		height = mapData.height;
		ret = true;

		break;
	}

	return ret;
}

// L06: TODO 3: Pick the right Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int id) const
{
	ListItem<TileSet*>* item = mapData.tilesets.start;
	TileSet* set = item->data;

	while (item)
	{
		if (id < item->data->firstgid)
		{
			set = item->prev->data;
			break;
		}
		set = item->data;
		item = item->next;
	}

	return set;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int id) const
{
	SDL_Rect rect = { 0 };

	// L04: TODO 7: Get relative Tile rectangle
	int relativeId = id - firstgid;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relativeId % columns));
	rect.y = margin + ((rect.h + spacing) * (relativeId / columns));

	return rect;
}

// Called before quitting
bool Map::CleanUp()
{
	LOG("Unloading map");

	// L03: DONE 2: Make sure you clean up any memory allocated from tilesets/map
	// Remove all tilesets
	ListItem<TileSet*>* item;
	item = mapData.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	mapData.tilesets.Clear();

	// L04: DONE 2: clean up all layer data
	// Remove all layers
	ListItem<MapLayer*>* item2;
	item2 = mapData.maplayers.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	mapData.maplayers.Clear();

	return true;
}

// Load new map
bool Map::Load(const char* filename)
{
	bool ret = true;
	SString tmp("%s%s", folder.GetString(), filename);

	pugi::xml_document mapFile;
	pugi::xml_parse_result result = mapFile.load_file(tmp.GetString());

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", filename, result.description());
		//ret = false;
	}

	if (ret == true)
	{
		ret = LoadMap(mapFile);
	}

	if (ret == true)
	{
		ret = LoadTileSets(mapFile.child("map"));
	}

	// L04: TODO 4: Iterate all layers and load each of them
	if (ret == true)
	{
		ret = LoadAllLayers(mapFile.child("map"));
	}

	if (ret == true)
	{
		// L03: TODO 5: LOG all the data loaded iterate all tilesets and LOG everything


		// L04: TODO 4: LOG the info for each loaded layer
	}

	// Clean up the pugui tree
	if (mapFile) mapFile.reset();

	ListItem<MapLayer*>* mapLayerItem;
	mapLayerItem = mapData.maplayers.start;

	while (mapLayerItem != NULL) {
		for (int x = 0; x < mapLayerItem->data->width; x++)
		{
			for (int y = 0; y < mapLayerItem->data->height; y++)
			{
				// L04: DONE 9: Complete the draw function
				int gid = mapLayerItem->data->Get(x, y);

				if (mapLayerItem->data->properties.GetProperty("Col", 1) || mapLayerItem->data->properties.GetProperty("Col", 2)) {

					//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
					//now we always use the firt tileset in the list
					//TileSet* tileset = mapData.tilesets.start->data;
					if (gid == 587)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->coll->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::WALL, 0, app->scene);
					}

					if (gid == 586)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->coll->AddCollider({ pos.x, pos.y, 16, 4 }, Collider::Type::PLAT, 0, app->scene);
					}

					if (gid == 618)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->coll->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::LOSE, 0, app->scene);
					}

					if (gid == 619)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->coll->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::WIN, 0, app->scene);
					}

					if (gid == 620)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->coll->AddCollider({ pos.x, pos.y, 1, 16 }, Collider::Type::CAM, 0, app->scene);
					}

					if (gid == 791)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->enemy->AddEnemy(pos.x, pos.y, 0, false);
					}


					if (gid == 1002)
					{
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->enemy->AddEnemy(pos.x, pos.y, 1, false);
					}
				}
			}
		}
		mapLayerItem = mapLayerItem->next;
	}

	mapLoaded = ret;

	return ret;
}

// L03: TODO: Load map general properties
bool Map::LoadMap(pugi::xml_node mapFile)
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE 3: Load map general properties
		mapData.height = map.attribute("height").as_int();
		mapData.width = map.attribute("width").as_int();
		mapData.tileHeight = map.attribute("tileheight").as_int();
		mapData.tileWidth = map.attribute("tilewidth").as_int();

		// L05: DONE 1: Add formula to go from isometric map to world coordinates
		mapData.type = MAPTYPE_UNKNOWN;
		if (strcmp(map.attribute("orientation").as_string(), "isometric") == 0)
		{
			mapData.type = MAPTYPE_ISOMETRIC;
		}
		if (strcmp(map.attribute("orientation").as_string(), "orthogonal") == 0)
		{
			mapData.type = MAPTYPE_ORTHOGONAL;
		}


	}

	return ret;
}


// L03: DONE 4: Implement the LoadTileSet function to load the tileset properties
bool Map::LoadTileSets(pugi::xml_node mapFile) {

	bool ret = true;

	pugi::xml_node tileset;
	for (tileset = mapFile.child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();
		if (ret == true) ret = LoadTilesetDetails(tileset, set);
		if (ret == true) ret = LoadTilesetImage(tileset, set);
		mapData.tilesets.Add(set);
	}

	return ret;
}

// L03: TODO 4: Load Tileset attributes
bool Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;

	// L03: DONE 4: Load Tileset attributes
	set->name.Create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	set->tilecount = tileset_node.attribute("tilecount").as_int();
	set->columns = tileset_node.attribute("columns").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	return ret;
}

// L03: DONE 4: Load Tileset image
bool Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE 4: Load Tileset image
		SString tmp("%s%s", folder.GetString(), image.attribute("source").as_string());
		set->texture = app->tex->Load(tmp.GetString());
	}

	return ret;
}

// L04: DONE 3: Implement a function that loads a single layer layer
bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	//Load the attributes
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();

	//L06: TODO 6 Call Load Propoerties
	LoadProperties(node, layer->properties);

	//Reserve the memory for the tile array
	layer->data = new uint[layer->width * layer->height];
	memset(layer->data, 0, layer->width * layer->height);

	//Iterate over all the tiles and assign the values
	pugi::xml_node tile;
	int i = 0;
	for (tile = node.child("data").child("tile"); tile && ret; tile = tile.next_sibling("tile"))
	{
		layer->data[i] = tile.attribute("gid").as_int();
		i++;
	}

	return ret;
}

// L04: TODO 4: Iterate all layers and load each of them
bool Map::LoadAllLayers(pugi::xml_node mapNode) {
	bool ret = true;
	for (pugi::xml_node layerNode = mapNode.child("layer"); layerNode && ret; layerNode = layerNode.next_sibling("layer"))
	{
		//Load the layer
		MapLayer* mapLayer = new MapLayer();
		ret = LoadLayer(layerNode, mapLayer);

		//add the layer to the map
		mapData.maplayers.Add(mapLayer);
	}

	return ret;
}

void Map::LoadenemiesLvl()
{
	bool ret = false;
	ListItem<MapLayer*>* item;
	item = mapData.maplayers.start;

	for (item = mapData.maplayers.start; item != NULL; item = item->next)
	{
		MapLayer* layer = item->data;

		if (layer->properties.GetProperty("En", 0) == 0)
			continue;


		for (int y = 0; y < mapData.height; ++y)
		{
			for (int x = 0; x < mapData.width; ++x)
			{
				int i = (y * layer->width) + x;
				int gid = layer->Get(x, y);

				if (gid == 791)
				{
					TileSet* tileset = GetTilesetFromTileId(gid);

					SDL_Rect r = tileset->GetTileRect(gid);
					iPoint pos = MapToWorld(x, y);

					//app->enemy->AddEnemy(pos.x, pos.y, 0, false);
				}


				if (gid == 1002)
				{
					TileSet* tileset = GetTilesetFromTileId(gid);

					SDL_Rect r = tileset->GetTileRect(gid);
					iPoint pos = MapToWorld(x, y);

					//app->enemy->AddEnemy(pos.x, pos.y, 1, false);
				}
			}
		}


		break;
	}


}

// L06: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
	{
		Properties::Property* p = new Properties::Property();
		p->name = propertieNode.attribute("name").as_string();
		p->value = propertieNode.attribute("value").as_int();

		properties.list.Add(p);
	}

	return ret;
}