#include "RenderSystem.h"
#include "rect2.h"
#include "RenderComponent.h"
#include "TransformComponent.h"
#include "SpritesheetPropertiesComponent.h"
#include "DirectionComponent.h"
#include "StateComponent.h"
#include "AnimationComponent.h"
#include "HurtboxComponent.h"
#include "HitstopComponent.h"

void RenderSystem::update(entt::registry& ecs, float timescale) {
    auto animationView = ecs.view<AnimationComponent>();
    for(auto ent : animationView) {
        if(ecs.all_of<HitstopComponent>(ent)) {
            auto hitstopComp = ecs.get<HitstopComponent>(ent);
            if(hitstopComp.hitstopCount < hitstopComp.hitstopCountLimit) continue;
        }
        auto& animationComponent = ecs.get<AnimationComponent>(ent);
        animationComponent.msSinceAnimationStart += timescale * 1000.f;
    }
    auto renderView = ecs.view<RenderComponent>();
    for(auto ent : renderView) {
        // update render quad pos
        auto& transform = ecs.get<TransformComponent>(ent);
        auto& renderComponent = ecs.get<RenderComponent>(ent);
        renderComponent.renderQuad.x = transform.position.x + renderComponent.renderQuadOffset.x;
        renderComponent.renderQuad.y = transform.position.y + renderComponent.renderQuadOffset.y;
    }
}

void RenderSystem::render(SDL_Renderer* renderer, entt::registry& ecs, strb::vec2f renderOffset) {
    auto entities = ecs.view<RenderComponent, TransformComponent>();
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
    for(auto ent : entities) {
        if(ecs.all_of<HurtboxComponent>(ent)) {
            auto hurtbox = ecs.get<HurtboxComponent>(ent);
            if(hurtbox.invulnCount < hurtbox.invulnTime && hurtbox.invulnCount % 3 != 0) {
                continue;
            }
        }
        auto& renderComponent = ecs.get<RenderComponent>(ent);
        SDL_FRect quad = {
            renderComponent.renderQuad.x,
            renderComponent.renderQuad.y,
            renderComponent.renderQuad.w,
            renderComponent.renderQuad.h
        };
        quad.x += renderOffset.x;
        quad.y += renderOffset.y;
        // bounds check before rendering
        if(quad.x + quad.w > 0 && quad.x < _renderBounds.x &&
           quad.y + quad.h > 0 && quad.y < _renderBounds.y) {
            // if entity has spritesheet + required helper components
            if(ecs.all_of<SpritesheetPropertiesComponent>(ent) &&
               ecs.all_of<StateComponent>(ent) &&
               ecs.all_of<DirectionComponent>(ent)) {
                auto& propsComponent = ecs.get<SpritesheetPropertiesComponent>(ent);
                auto& state = ecs.get<StateComponent>(ent).state;
                auto& direction = ecs.get<DirectionComponent>(ent).direction;
                SpritesheetProperties props = propsComponent.getSpritesheetProperties(state, direction);
                propsComponent.spritesheet->setIsAnimated(props.isAnimated);
                propsComponent.spritesheet->setIsLooped(props.isLooped);
                propsComponent.spritesheet->setMsBetweenFrames(props.msBetweenFrames);
                propsComponent.spritesheet->setNumOfFrames(props.numOfFrames);
                if(ecs.all_of<AnimationComponent>(ent)) {
                    auto& animationComponent = ecs.get<AnimationComponent>(ent);
                    auto& state = ecs.get<StateComponent>(ent);
                    // check for change in y index to restart animation counter
                    if(state.state != animationComponent.lastState) {
                        animationComponent.msSinceAnimationStart = 0;
                    }
                    if(props.isLooped) {
                        animationComponent.xIndex = animationComponent.msSinceAnimationStart / props.msBetweenFrames % props.numOfFrames;
                    }
                    else {
                        animationComponent.xIndex = animationComponent.msSinceAnimationStart / props.msBetweenFrames;
                        if(animationComponent.xIndex >= props.numOfFrames) animationComponent.xIndex = props.numOfFrames - 1;
                    }
                    animationComponent.lastState = state.state;
                }
                if(props.isAnimated && ecs.all_of<AnimationComponent>(ent)) {
                    auto& animationComponent = ecs.get<AnimationComponent>(ent);
                    propsComponent.spritesheet->setTileIndex(animationComponent.xIndex, props.yTileIndex);
                    if(ecs.all_of<StateComponent>(ent) &&
                        ecs.all_of<DirectionComponent>(ent)) {
                        auto& state = ecs.get<StateComponent>(ent).state;
                        auto& direction = ecs.get<DirectionComponent>(ent).direction;
                        propsComponent.addSpritesheetProperties(state, direction, props);
                    }
                }
                else {
                    propsComponent.spritesheet->setTileIndex(props.xTileIndex, props.yTileIndex);
                }
                propsComponent.spritesheet->render(
                    quad.x,
                    quad.y,
                    quad.w,
                    quad.h,
                    props.flip,
                    props.angle,
                    props.center
                );
            }
            // else if entity has spritesheet at all
            else if(ecs.all_of<SpritesheetPropertiesComponent>(ent)) {
                auto& propsComponent = ecs.get<SpritesheetPropertiesComponent>(ent);
                SpritesheetProperties props = propsComponent.getPrimarySpritesheetProperties();
                propsComponent.spritesheet->setIsAnimated(props.isAnimated);
                propsComponent.spritesheet->setIsLooped(props.isLooped);
                propsComponent.spritesheet->setMsBetweenFrames(props.msBetweenFrames);
                propsComponent.spritesheet->setNumOfFrames(props.numOfFrames);
                if(props.isAnimated) {
                    propsComponent.spritesheet->setTileIndex(propsComponent.spritesheet->getTileIndex().x, props.yTileIndex);
                }
                else {
                    propsComponent.spritesheet->setTileIndex(props.xTileIndex, props.yTileIndex);
                }
                propsComponent.spritesheet->render(
                    quad.x,
                    quad.y,
                    quad.w,
                    quad.h,
                    props.flip,
                    props.angle,
                    props.center
                );
            }
            else {
                // no spritesheet set, default quad rendered
                SDL_RenderFillRectF(renderer, &quad);
            }
        }
    }
}

void RenderSystem::setRenderBounds(strb::vec2i renderBounds) {
    _renderBounds = renderBounds;
}