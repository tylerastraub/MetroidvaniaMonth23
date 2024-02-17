#include "GameState.h"
#include "RandomGen.h"
#include "LevelParser.h"
#include "Player.h"
#include "CollisionComponent.h"

#include <chrono>

std::mt19937 RandomGen::randEng{(unsigned int) std::chrono::system_clock::now().time_since_epoch().count()};

bool GameState::init() {
    _level = LevelParser::parseLevelFromTmx(_ecs, "res/tiled/test_level.tmx", SpritesheetID::TILESET_DEFAULT);
    _player = _level.getPlayerId();
    
    initSystems();

    return true;
}

void GameState::tick(float timescale) {
    _inputSystem.update(_ecs);
    if(_inputSystem.inputPressed(InputEvent::TOGGLE_DEBUG)) _debug = !_debug;

    _scriptSystem.update(_ecs, timescale);

    _physicsSystem.updateX(_ecs, timescale);
    _collisionSystem.updateLevelCollisionsOnXAxis(_ecs, _level);
    _physicsSystem.updateY(_ecs, timescale);
    _collisionSystem.updateLevelCollisionsOnYAxis(_ecs, _level);
    _collisionSystem.checkForCrouchCollision(_ecs, _level);
    
    _cameraSystem.update(_ecs, timescale);
    _renderOffset = _cameraSystem.getCurrentCameraOffset();

    _renderSystem.update(_ecs, timescale);

    // Input updates
    getKeyboard()->updateInputs();
    getController()->updateInputs();
}

void GameState::render() {
    // ImGui::Render();
    SDL_SetRenderDrawColor(getRenderer(), 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(getRenderer());

    _level.render(_renderOffset);

    _renderSystem.render(getRenderer(), _ecs, _renderOffset);

    if(_debug) {
        SDL_SetRenderDrawColor(getRenderer(), 0x00, 0xFF, 0x00, 0xAF);
        auto view = _ecs.view<CollisionComponent>();
        for(auto ent : view) {
            auto c = _ecs.get<CollisionComponent>(ent).collisionRect;
            SDL_FRect r = {c.x + _renderOffset.x, c.y + _renderOffset.y, c.w, c.h};
            SDL_RenderDrawRectF(getRenderer(), &r);
        }
    }
    
    // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(getRenderer());
}

void GameState::handleControllerButtonInput(SDL_Event e) {
    getController()->updateButtonInputs(e);
}

void GameState::handleControllerAxisInput(SDL_Event e) {
    getController()->updateAxisInputs(e);
}

void GameState::handleMouseInput(SDL_Event e) {
    getMouse()->updateInput(e, _renderOffset.x, _renderOffset.y);
}

void GameState::initSystems() {
    _inputSystem.init(getKeyboard(), getController(), getSettings());

    _renderSystem.setRenderBounds(getGameSize());

    _scriptSystem.init(getAudioPlayer());

    _cameraSystem.setCameraGoal(_player);
    _cameraSystem.setGameSize(getGameSize());
    _cameraSystem.setLevelSize({
        _level.getTilemapWidth() * _level.getTileSize(),
        _level.getTilemapHeight() * _level.getTileSize()
    });
}