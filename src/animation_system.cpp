#include "animation_system.hpp"

void AnimationSystem::step(float elapsed_ms)
{
	// TODO: add conditional for game over -> game state?

	for (auto& entity : registry.animations.entities)
	{
		Animation& animation = registry.animations.get(entity);
		SpriteSheetImage& sprite_sheet = registry.spriteSheetImages.get(entity);

		// check if frame should be updated
		if (animation.time_since_last_frame >= animation.time_per_frame)
		{
			// we've reached the last frame going forwards
			if (animation.forwards && sprite_sheet.current_frame + 1 > animation.end_frame)
			{
				if (animation.loop == ANIM_LOOP_TYPES::NO_LOOP)
				{
					// animation is over since no loop, remove the entity from game
					registry.remove_all_components_of(entity);
				}
				else if (animation.loop == ANIM_LOOP_TYPES::PING_PONG)
				{
					// go backwards through the frames
					sprite_sheet.current_frame--;
					animation.forwards = !animation.forwards;
				}
				else if (animation.loop == ANIM_LOOP_TYPES::LOOP)
				{
					sprite_sheet.current_frame = animation.start_frame;
				}
			}
			// we've reached the last frame going backwards
			else if (!animation.forwards && sprite_sheet.current_frame - 1 < animation.start_frame)
			{
				if (!animation.forwards && animation.loop == ANIM_LOOP_TYPES::NO_LOOP)
				{
					registry.remove_all_components_of(entity);
				}
				else if (animation.loop == ANIM_LOOP_TYPES::PING_PONG)
				{
					// go forwards through the frames
					sprite_sheet.current_frame++;
					animation.forwards = !animation.forwards;
				}
				else if (animation.loop == ANIM_LOOP_TYPES::LOOP)
				{
					sprite_sheet.current_frame = animation.end_frame;
				}
			}
			else if (animation.forwards)
			{
				sprite_sheet.current_frame++;
			}
			else
			{
				sprite_sheet.current_frame--;
			}

			animation.time_since_last_frame = 0;
		}
		else
		{
			animation.time_since_last_frame += elapsed_ms;
		}
	}
}


void changeAnimationFrames(Entity entity, int start_frame, int end_frame)
{
	Animation& animation = registry.animations.get(entity);
	animation.start_frame = start_frame;
	animation.end_frame = end_frame;
}