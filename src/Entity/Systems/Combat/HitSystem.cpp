#include "HitSystem.h"
#include "RectUtils.h"
#include "HitboxComponent.h"
#include "HurtboxComponent.h"
#include "TransformComponent.h"
#include "HitstopComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"

void HitSystem::update(entt::registry& ecs, float timescale) {
    auto hitstopView = ecs.view<HitstopComponent>();
    auto hitboxView = ecs.view<HitboxComponent>();
    auto hurtboxView = ecs.view<HurtboxComponent>();
    // hitstop update
    for(auto ent : hitstopView) {
        auto& hitstopComp = ecs.get<HitstopComponent>(ent);
        hitstopComp.hitstopCount += timescale * 1000.f;
    }
    // hitbox pos update
    for(auto ent : hitboxView) {
        auto& hitboxComp = ecs.get<HitboxComponent>(ent);
        auto hitboxes = hitboxComp.hitboxes;
        if(hitboxes.empty()) continue;
        auto pos = ecs.get<TransformComponent>(ent).position;
        for(auto hitbox : hitboxes) {
            hitbox.bounds.x = pos.x + hitbox.offset.x;
            hitbox.bounds.y = pos.y + hitbox.offset.y;
        }
    }
    // hurtbox pos update
    for(auto ent : hurtboxView) {
        auto& hurtboxComp = ecs.get<HurtboxComponent>(ent);
        auto pos = ecs.get<TransformComponent>(ent).position;
        hurtboxComp.bounds.x = pos.x + hurtboxComp.offset.x;
        hurtboxComp.bounds.y = pos.y + hurtboxComp.offset.y;
        hurtboxComp.invulnCount += timescale * 1000.f;
    }
}

void HitSystem::checkForHitboxCollisions(entt::registry& ecs) {
    auto hitboxView = ecs.view<HitboxComponent>();
    auto hurtboxView = ecs.view<HurtboxComponent>();
    // note: this is currently N^2 + M!! very unoptimized. faster would be to use spacial partition.
    // however, we will see if we actually need it first.
    for(auto attacker : hitboxView) {
        // hitstop check
        if(ecs.all_of<HitstopComponent>(attacker)) {
            auto hitstopComp = ecs.get<HitstopComponent>(attacker);
            if(hitstopComp.hitstopCount < hitstopComp.hitstopDuration) continue;
        }
        // then check we have a valid hitbox component
        auto& hitboxComp = ecs.get<HitboxComponent>(attacker);
        auto hitboxes = hitboxComp.hitboxes;
        if(hitboxes.empty()) continue;
        // now do hurtbox check
        for(auto defender : hurtboxView) {
            // make sure we have a valid hurtbox
            if(attacker == defender) continue;
            auto& hurtboxComp = ecs.get<HurtboxComponent>(defender);
            if(hurtboxComp.invulnCount < hurtboxComp.invulnTime) continue;
            // actual hit check
            for(auto hitbox : hitboxes) {
                if(RectUtils::isIntersecting(hitbox.bounds, hurtboxComp.bounds)) {
                    // we have a hit!
                    hurtboxComp.invulnCount = 0;
                    if(ecs.all_of<HealthComponent>(defender)) {
                        ecs.get<HealthComponent>(defender).health -= hitboxComp.damage;
                    }
                    // apply hitstop
                    if(ecs.all_of<HitstopComponent>(attacker)) {
                        auto& hitstopComp = ecs.get<HitstopComponent>(attacker);
                        hitstopComp.hitstopCount = 0;
                    }
                    // and apply self knockback
                    if(ecs.all_of<PhysicsComponent>(attacker)) {
                        auto& physics = ecs.get<PhysicsComponent>(attacker);
                        auto attackerPos = ecs.get<TransformComponent>(attacker).position;
                        auto defenderPos = ecs.get<TransformComponent>(defender).position;
                        physics.velocity = hitboxComp.selfKnockback;
                        if(attackerPos.x > defenderPos.x) physics.velocity.x *= -1.f;
                    }
                }
            }
        }
    }
}