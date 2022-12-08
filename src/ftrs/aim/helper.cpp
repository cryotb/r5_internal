#include <include.h>

using bone_id = respawn::bone_id;
static bone_id bone_array[] = { bone_id::head, bone_id::neck, bone_id::spine, bone_id::torso_center, bone_id::torso_lower, bone_id::torso_upper };

respawn::bone_id aim::get_optimal_bone_id(const cache::player_t& player)
{
	const auto& lp_data = g::p::cache.local_player;

	if (cfg::aim_random_hitbox)
		return bone_id::torso_upper;

	if (cfg::aim_dynamic_hitbox)
	{
		auto closest_bone = bone_id{ };
		auto maximum_fov = FLT_MAX;
		auto closest_fov = maximum_fov;

		const auto& shoot_position = lp_data.shoot_pos;

		for (auto i = 0; i < ARRAYSIZE(bone_array); i++)
		{
			auto id = bone_array[i];
			auto bone_position = player.m_bones[ static_cast<int>(id) ];
			auto fov = math::SE::get_fov_difference(lp_data.eye_angle, shoot_position, bone_position);

			if (fov > maximum_fov)
				continue;

			if (fov < closest_fov)
			{
				closest_bone = static_cast<bone_id>(id);
				closest_fov = fov;

				continue;
			}
		}

		return closest_bone;
	}

	return bone_id::head;
}
