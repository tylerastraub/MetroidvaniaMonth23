#include "GameState.h"
#include "RandomGen.h"
#include "LevelParser.h"
#include "Player.h"
// Components
#include "CollisionComponent.h"
#include "HitboxComponent.h"
#include "HurtboxComponent.h"
#include "TransformComponent.h"
// Triggers
#include "TriggerComponent.h"
#include "LevelLoadTriggerComponent.h"
#include "CameraTargetTriggerComponent.h"

#include <chrono>

std::mt19937 RandomGen::randEng{(unsigned int) std::chrono::system_clock::now().time_since_epoch().count()};

/**
 * TODO:
 * ===== ART =====
 * - Add tileset art
 *     - More pyramids/palm trees, marble tiles
 * - Add more player art
 *     - Ground attack, air attack
 *     - Add player particles for attacks (need particle system added)
 * - Add Rockling art
 * - Add Guardian art
 *     - Thinking robotic spider thing with blade that comes out during attack
 * - Add AirDiver art
 * - Add parallax background
 * ===== CODE =====
 * - Tweak AirDiver stats more
 *     - Maybe just need to make it move up a bit after attacks
 * - Add more enemies (not all of these will get done)
 *     - 2nd air enemy that has more unique attack pattern (maybe diving bird like in Mario?)
 *     - Mega-Gravity enemy
 *     - Anti-movement enemy (holds up sign with left or right arrow, when active player can't move that direction)
 *     - Reflector enemy (reflects damage back when shield is up)
 * - Add particle system
 * - Add hazard tile
 * - Add powerup pickups
 * - Add doors with keys/locks
 * - Add minimap (prefably a live one)
*/

bool GameState::init() {
    _level = LevelParser::parseLevelFromTmx(_ecs, getProperty("levelPath"), SpritesheetID::TILESET_DEFAULT, std::stoi(getProperty("playerSpawnID")));
    _debug = std::stoi(getProperty("debug"));
    _player = _level.getPlayerId();
    
    initSystems();

    return true;
}

void GameState::tick(float timescale) {
    _cameraSystem.setCameraGoal(_player);
    _cameraSystem.setCameraSpeed(0.2f);

    _inputSystem.update(_ecs);
    if(_inputSystem.inputPressed(InputEvent::TOGGLE_DEBUG)) _debug = !_debug;

    _scriptSystem.update(_ecs, timescale);

    _physicsSystem.updateX(_ecs, timescale);
    _collisionSystem.updateLevelCollisionsOnXAxis(_ecs, _level);
    _physicsSystem.updateY(_ecs, timescale);
    _collisionSystem.updateLevelCollisionsOnYAxis(_ecs, _level);
    _collisionSystem.checkForCrouchCollision(_ecs, _level);

    _hitSystem.update(_ecs, timescale);
    _hitSystem.checkForHitboxCollisions(_ecs, timescale, getAudioPlayer());

    std::vector<entt::entity> triggers = _collisionSystem.checkForPlayerAndTriggerCollisions(_ecs);
    if(triggers.size()) processTriggers(triggers, timescale);
    
    _cameraSystem.update(_ecs, timescale);
    _renderOffset = _cameraSystem.getCurrentCameraOffset();

    _renderSystem.update(_ecs, timescale);

    // Input updates
    getKeyboard()->updateInputs();
    getController()->updateInputs();
}

void GameState::render() {
    // ImGui::Render();
    SDL_SetRenderDrawColor(getRenderer(), 0x36, 0xbe, 0xd5, 0xFF);
    SDL_RenderClear(getRenderer());

    _level.render(_renderOffset);

    _renderSystem.render(getRenderer(), _ecs, _renderOffset);

    if(_debug) {
        // collision box render
        SDL_SetRenderDrawColor(getRenderer(), 0x00, 0xFF, 0x00, 0x64);
        auto collisionView = _ecs.view<CollisionComponent>();
        for(auto ent : collisionView) {
            auto c = _ecs.get<CollisionComponent>(ent).collisionRect;
            SDL_FRect r = {c.x + _renderOffset.x, c.y + _renderOffset.y, c.w, c.h};
            SDL_RenderDrawRectF(getRenderer(), &r);
        }
        // hitbox render
        SDL_SetRenderDrawColor(getRenderer(), 0xFF, 0x00, 0x00, 0x64);
        auto hitboxView = _ecs.view<HitboxComponent>();
        for(auto ent : hitboxView) {
            auto hitboxes = _ecs.get<HitboxComponent>(ent).hitboxes;
            for(auto hitbox : hitboxes) {
                SDL_FRect r = {hitbox.bounds.x + _renderOffset.x, hitbox.bounds.y + _renderOffset.y, hitbox.bounds.w, hitbox.bounds.h};
                SDL_RenderFillRectF(getRenderer(), &r);
            }
        }
        // hurtbox render
        SDL_SetRenderDrawColor(getRenderer(), 0xFF, 0xFF, 0x00, 0x64);
        auto hurtboxView = _ecs.view<HurtboxComponent>();
        for(auto ent : hurtboxView) {
            auto hurtbox = _ecs.get<HurtboxComponent>(ent);
            SDL_FRect r = {hurtbox.bounds.x + _renderOffset.x, hurtbox.bounds.y + _renderOffset.y, hurtbox.bounds.w, hurtbox.bounds.h};
            SDL_RenderFillRectF(getRenderer(), &r);
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
    _cameraSystem.setCameraBounds({
        std::stof(_level.getProperty("cameraBoundsX")),
        std::stof(_level.getProperty("cameraBoundsY")),
        std::stof(_level.getProperty("cameraBoundsW")),
        std::stof(_level.getProperty("cameraBoundsH"))
    });
    _renderSystem.update(_ecs, 0.f); // just to get player quad aligned
    _cameraSystem.update(_ecs, 0.f);
    _cameraSystem.alignCameraOffsetWithGoal();
}

void GameState::processTriggers(std::vector<entt::entity> triggers, float timescale) {
    std::vector<entt::entity> destroyedTriggers;
    bool hitCameraTrigger = false;
    for(auto trigger : triggers) {
        auto triggerComp = _ecs.get<TriggerComponent>(trigger);
        switch(triggerComp.type) {
            case TriggerType::LEVEL_LOAD: {
                auto levelLoadComp = _ecs.get<LevelLoadTriggerComponent>(trigger);
                GameState* gs = new GameState();
                gs->setProperty("levelPath", levelLoadComp.levelPath);
                gs->setProperty("playerSpawnID", std::to_string(levelLoadComp.playerSpawnID));
                gs->setProperty("debug", std::to_string(_debug));
                /// TODO: add properties here corresponding to player progress/state
                // alternatively, just save progress here then load progress in new GameState
                setNextState(gs);
                break;
            }
            case TriggerType::CAMERA_TARGET: {
                hitCameraTrigger = true;
                strb::vec2f cameraTarget = _ecs.get<CameraTargetTriggerComponent>(trigger).target;
                _cameraSystem.setCameraGoal(entt::null);
                _cameraSystem.setGoalCameraOffset(cameraTarget);
                _cameraSystem.setCameraSpeed(0.075f);
                break;
            }
            default:
                break;
        }
        if(triggerComp.onTriggerScript) triggerComp.onTriggerScript->update(_ecs, trigger, timescale, getAudioPlayer());
        if(triggerComp.triggerOnce) destroyedTriggers.push_back(trigger);
    }

    for(auto trigger : destroyedTriggers) {
        _ecs.destroy(trigger);
    }
}