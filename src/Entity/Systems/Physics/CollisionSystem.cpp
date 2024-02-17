#include "CollisionSystem.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "CollisionComponent.h"
#include "CrouchComponent.h"

void CollisionSystem::updateLevelCollisionsOnXAxis(entt::registry& ecs, Level level) {
    auto view = ecs.view<TransformComponent, CollisionComponent, PhysicsComponent>();
    for(auto entity : view) {
        auto& transform = ecs.get<TransformComponent>(entity);
        auto& physics = ecs.get<PhysicsComponent>(entity);
        auto& collision = ecs.get<CollisionComponent>(entity);
        int ts = level.getTileSize();

        collision.collisionRect.x = transform.position.x + collision.collisionRectOffset.x;

        strb::vec2i topLeftTile = {static_cast<int>(std::floor(collision.collisionRect.x)) / ts,
            static_cast<int>(std::floor(collision.collisionRect.y)) / ts};
        strb::vec2i bottomRightTile = {static_cast<int>(std::ceil(collision.collisionRect.x + collision.collisionRect.w)) / ts,
            static_cast<int>(std::ceil(collision.collisionRect.y + collision.collisionRect.h - 1)) / ts};

        std::vector<strb::vec2i> tileCollisions;
        if(physics.velocity.x < 0) {
            collision.collidingRight = false;
            // get all tiles to the left
            for(int y = topLeftTile.y; y <= bottomRightTile.y; ++y) {
                Tile t = level.getTileAt(topLeftTile.x, y);
                if(t.type == TileType::SOLID) {
                    tileCollisions.push_back({topLeftTile.x, y});
                }
            }

            if(tileCollisions.size()) {
                // we have a hit!
                strb::vec2i tilePos = *tileCollisions.begin();
                transform.position.x = tilePos.x * ts + ts - collision.collisionRectOffset.x;
                collision.collisionRect.x = transform.position.x + collision.collisionRectOffset.x;
                physics.velocity.x = 0;
                collision.collidingLeft = true;
            }
            else {
                collision.collidingLeft = false;
                if(physics.wallSliding) {
                    physics.wallSliding = false;
                    physics.offWallCount = physics.wallJumpTime;
                    physics.velocity.x = 0.f;
                    transform.position.x = transform.lastPosition.x;
                    collision.collisionRect.x = transform.position.x + collision.collisionRectOffset.x;
                }
            }
        }
        else if(physics.velocity.x > 0) {
            collision.collidingLeft = false;
            // get all tiles to the right
            for(int y = topLeftTile.y; y <= bottomRightTile.y; ++y) {
                Tile t = level.getTileAt(bottomRightTile.x, y);
                if(t.type == TileType::SOLID) {
                    tileCollisions.push_back({bottomRightTile.x, y});
                }
            }

            if(tileCollisions.size()) {
                // we have a hit!
                strb::vec2i tilePos = *tileCollisions.begin();
                transform.position.x = tilePos.x * ts - collision.collisionRect.w - collision.collisionRectOffset.x;
                collision.collisionRect.x = transform.position.x + collision.collisionRectOffset.x;
                physics.velocity.x = 0;
                collision.collidingRight = true;
            }
            else {
                collision.collidingRight = false;
                if(physics.wallSliding) {
                    physics.wallSliding = false;
                    physics.offWallCount = physics.wallJumpTime;
                    physics.velocity.x = 0.f;
                    transform.position.x = transform.lastPosition.x;
                    collision.collisionRect.x = transform.position.x + collision.collisionRectOffset.x;
                }
            }
        }
        else {
            collision.collidingLeft = false;
            collision.collidingRight = false;
        }
    }
}

void CollisionSystem::updateLevelCollisionsOnYAxis(entt::registry& ecs, Level level) {
    auto view = ecs.view<TransformComponent, CollisionComponent, PhysicsComponent>();
    for(auto entity : view) {
        auto& transform = ecs.get<TransformComponent>(entity);
        auto& physics = ecs.get<PhysicsComponent>(entity);
        auto& collision = ecs.get<CollisionComponent>(entity);
        int ts = level.getTileSize();

        collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;

        strb::vec2i topLeftTile = {static_cast<int>(std::floor(collision.collisionRect.x)) / ts,
            static_cast<int>(std::floor(collision.collisionRect.y)) / ts};
        strb::vec2i bottomRightTile = {static_cast<int>(std::ceil(collision.collisionRect.x + collision.collisionRect.w - 1)) / ts,
            static_cast<int>(std::ceil(collision.collisionRect.y + collision.collisionRect.h)) / ts};

        std::vector<strb::vec2i> tileCollisions;
        if(physics.velocity.y < 0) {
            collision.collidingDown = false;
            physics.wallSliding = false;
            // get all tiles above us
            for(int x = topLeftTile.x; x <= bottomRightTile.x; ++x) {
                Tile t = level.getTileAt(x, topLeftTile.y);
                if(t.type == TileType::SOLID) {
                    tileCollisions.push_back({x, topLeftTile.y});
                }
            }

            if(tileCollisions.size()) {
                // we have a hit!
                strb::vec2i tilePos = *tileCollisions.begin();
                transform.position.y = tilePos.y * ts + ts - collision.collisionRectOffset.y;
                collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                physics.velocity.y = 0;
                collision.collidingUp = true;
            }
            else {
                collision.collidingUp = false;
            }
        }
        else if(physics.velocity.y > 0) {
            collision.collidingUp = false;
            // get all tiles below us
            for(int x = topLeftTile.x; x <= bottomRightTile.x; ++x) {
                Tile t = level.getTileAt(x, bottomRightTile.y);
                if(t.type == TileType::SOLID) {
                    tileCollisions.push_back({x, bottomRightTile.y});
                }
                else if(t.type == TileType::PLATFORM) {
                    if(transform.lastPosition.y + collision.collisionRectOffset.y + collision.collisionRect.h - 1 < bottomRightTile.y * ts) {
                        tileCollisions.push_back({x, bottomRightTile.y});
                    }
                }
            }

            if(tileCollisions.size()) {
                // we have a hit!
                strb::vec2i tilePos = *tileCollisions.begin();
                transform.position.y = tilePos.y * ts - collision.collisionRect.h - collision.collisionRectOffset.y;
                collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                physics.velocity.y = 0;
                collision.collidingDown = true;
                physics.touchingGround = true;
                physics.jumping = false;
                physics.wallJumping = false;
                physics.wallSliding = false;
                collision.onPlatform = (level.getTileAt(tilePos.x, tilePos.y).type == TileType::PLATFORM);
            }
            else {
                collision.collidingDown = false;
                physics.touchingGround = false;
                collision.onPlatform = false;
            }
        }
        else {
            collision.collidingUp = false;
            collision.collidingDown = false;
        }
    }
}

void CollisionSystem::checkForCrouchCollision(entt::registry& ecs, Level level) {
    auto view = ecs.view<CollisionComponent, CrouchComponent>();
    for(auto entity : view) {
        auto collision = ecs.get<CollisionComponent>(entity).collisionRect;
        auto& crouch = ecs.get<CrouchComponent>(entity);
        int ts = level.getTileSize();

        crouch.canUncrouch = true;
        if(!crouch.crouching) return;
        
        float crouchOffset = crouch.standingHeight - crouch.crouchingHeight;
        strb::rect2f crouchCollider = {collision.x, collision.y - crouchOffset, collision.w, 1};
        strb::vec2i topLeftTile = {static_cast<int>(std::floor(crouchCollider.x)) / ts,
            static_cast<int>(std::floor(crouchCollider.y)) / ts};
        strb::vec2i bottomRightTile = {static_cast<int>(std::ceil(crouchCollider.x + crouchCollider.w - 1)) / ts,
            static_cast<int>(std::ceil(crouchCollider.y + crouchCollider.h)) / ts};
            
        // check tiles above crouch collider
        for(int x = topLeftTile.x; x <= bottomRightTile.x; ++x) {
            Tile t = level.getTileAt(x, topLeftTile.y);
            if(t.type == TileType::SOLID) {
                crouch.canUncrouch = false;
                break;
            }
        }
    }
}