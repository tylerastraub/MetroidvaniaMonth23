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
        if(hitstopComp.hitstopCount > hitstopComp.hitstopCountLimit) hitstopComp.hitstopCountLimit = 0;
    }
    // hitbox update
    for(auto ent : hitboxView) {
        auto& hitboxComp = ecs.get<HitboxComponent>(ent);
        auto& hitboxes = hitboxComp.hitboxes;
        auto pos = ecs.get<TransformComponent>(ent).position;
        for(auto& hitbox : hitboxes) {
            hitbox.bounds.x = pos.x + hitbox.offset.x;
            hitbox.bounds.y = pos.y + hitbox.offset.y;
        }
        if(ecs.all_of<HitstopComponent>(ent)) {
            auto hitstopComp = ecs.get<HitstopComponent>(ent);
            if(hitstopComp.hitstopCount < hitstopComp.hitstopCountLimit) continue;
        }
        hitboxComp.doubleHitTimer += timescale * 1000.f;
        if(hitboxComp.doubleHitTimer > hitboxComp.doubleHitTimerLimit && hitboxComp.hits.size()) {
            hitboxComp.hits.clear();
        }
    }
    // hurtbox pos update
    for(auto ent : hurtboxView) {
        auto& hurtboxComp = ecs.get<HurtboxComponent>(ent);
        auto pos = ecs.get<TransformComponent>(ent).position;
        hurtboxComp.bounds.x = pos.x + hurtboxComp.offset.x;
        hurtboxComp.bounds.y = pos.y + hurtboxComp.offset.y;
        if(ecs.all_of<HitstopComponent>(ent)) {
            auto hitstopComp = ecs.get<HitstopComponent>(ent);
            if(hitstopComp.hitstopCount < hitstopComp.hitstopCountLimit) continue;
        }
        hurtboxComp.hitstunCount += timescale * 1000.f;
        hurtboxComp.invulnCount += timescale * 1000.f;
        if(hurtboxComp.hitstunCount > hurtboxComp.hitstunTime) hurtboxComp.hitstunTime = 0;
    }
}

void HitSystem::checkForHitboxCollisions(entt::registry& ecs, float timescale, std::shared_ptr<Audio> audio) {
    auto hitboxView = ecs.view<HitboxComponent>();
    auto hurtboxView = ecs.view<HurtboxComponent>();
    // note: this is currently N^2 + M!! very unoptimized. faster would be to use spacial partition.
    // however, we will see if we actually need it first.
    for(auto attacker : hitboxView) {
        // hitstop check
        if(ecs.all_of<HitstopComponent>(attacker)) {
            auto hitstopComp = ecs.get<HitstopComponent>(attacker);
            if(hitstopComp.hitstopCount < hitstopComp.hitstopCountLimit) continue;
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
            // make sure we aren't double hitting entities
            if(std::find(hitboxComp.hits.begin(), hitboxComp.hits.end(), defender) != hitboxComp.hits.end()) continue;
            // actual hit check
            for(auto hitbox : hitboxes) {
                if(RectUtils::isIntersecting(hitbox.bounds, hurtboxComp.bounds)) {
                    // we have a hit!
                    hitboxComp.hits.push_back(defender);
                    hitboxComp.doubleHitTimer = 0;
                    hurtboxComp.invulnCount = 0;
                    hurtboxComp.hitstunCount = 0;
                    hurtboxComp.hitstunTime = hitboxComp.hitstun;
                    // deduct hp
                    if(ecs.all_of<HealthComponent>(defender)) {
                        ecs.get<HealthComponent>(defender).health -= hitboxComp.damage;
                    }
                    // apply hitstop to attacker
                    if(ecs.all_of<HitstopComponent>(attacker)) {
                        auto& attackerHitstopComp = ecs.get<HitstopComponent>(attacker);
                        attackerHitstopComp.hitstopCount = 0;
                        attackerHitstopComp.hitstopCountLimit = attackerHitstopComp.hitstopDurationOnAttack;
                        // apply hitstop to defender
                        if(ecs.all_of<HitstopComponent>(defender)) {
                            auto& defenderHitstopComp = ecs.get<HitstopComponent>(defender);
                            defenderHitstopComp.hitstopCount = 0;
                            defenderHitstopComp.hitstopCountLimit = attackerHitstopComp.hitstopDurationOnAttack;
                        }
                    }
                    auto attackerHurtbox = ecs.get<HurtboxComponent>(attacker).bounds;
                    float attackerCenter = attackerHurtbox.x + attackerHurtbox.w / 2.f;
                    float defenderCenter = hurtboxComp.bounds.x + hurtboxComp.bounds.w / 2.f;
                    // apply self knockback to attacker
                    if(ecs.all_of<PhysicsComponent>(attacker)) {
                        auto& physics = ecs.get<PhysicsComponent>(attacker);
                        if(physics.touchingGround) physics.velocity.x = 0;
                        physics.velocity.x += hitboxComp.selfKnockback.x;
                        physics.velocity.y = hitboxComp.selfKnockback.y;
                        if(attackerCenter < defenderCenter) physics.velocity.x *= -1.f;
                    }
                    // apply knockback to defender
                    if(ecs.all_of<PhysicsComponent>(defender)) {
                        auto& physics = ecs.get<PhysicsComponent>(defender);
                        physics.velocity = hitboxComp.knockback;
                        if(attackerCenter > defenderCenter) physics.velocity.x *= -1.f;
                    }
                    // then trigger scripts
                    if(hitboxComp.onHitScript) hitboxComp.onHitScript->update(ecs, attacker, timescale, audio);
                    if(hurtboxComp.onHurtScript) hurtboxComp.onHurtScript->update(ecs, attacker, timescale, audio);
                }
            }
        }
    }
}