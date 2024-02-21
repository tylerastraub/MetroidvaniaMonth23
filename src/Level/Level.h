#pragma once

#include "Tile.h"
#include "Spritesheet.h"
#include "FloatingPointLightMap.h"

#include <vector>
#include <cstdint>
#include <memory>
#include <entt/entity/registry.hpp>
#include <unordered_map>

class Level {
public:
    Level() = default;
    ~Level() = default;

    void allocateTilemap(int width, int height);
    void render(strb::vec2f renderOffset);

    void setTilemap(std::vector<std::vector<Tile>> tilemap);
    void setLightMap(std::shared_ptr<FloatingPointLightMap> lightMap);
    void setTileSize(int tileSize);
    void setTileAt(int x, int y, Tile tile);
    void setTileset(Spritesheet* tileset);
    void setPlayerId(entt::entity player);
    void setProperty(std::string property, std::string value);

    Tile getTileAt(int x, int y);
    int getTileSize();
    int getTilemapWidth();
    int getTilemapHeight();
    entt::entity getPlayerId();
    std::shared_ptr<FloatingPointLightMap> getLightMap();
    std::string getProperty(std::string property);

private:
    std::shared_ptr<FloatingPointLightMap> _lMap = nullptr;
    std::vector<std::vector<Tile>> _tilemap;
    int _tilemapWidth = 0;
    int _tilemapHeight = 0;
    int _tileSize = 16;
    Spritesheet* _tileset = nullptr;

    std::unordered_map<std::string, std::string> _properties;

    entt::entity _playerId;

};