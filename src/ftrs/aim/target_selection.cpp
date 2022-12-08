#include <include.h>

bool aim::is_target_viable(cache::entry_t& entity, aim::find_context_t& ctx)
{
	auto& info = ctx.target_info;

	switch (entity.type)
	{
	case cache::types::dummie:
	case cache::types::player:
	{
		if (cfg::aim_players)
		{
			auto& player = entity.type_specific.plr;

			if (player.m_teammate && !cfg::aim_friendly_fire)
				return false;

			if (player.m_life_state != LIFE_ALIVE)
				return false;

			if (!player.m_visible)
				return false;

			ctx.aim_pos = player.m_bones[static_cast<int>(get_optimal_bone_id(player))];

			info.velocity = player.m_velocity;
		}

		break;
	}
	case cache::types::npc:
	{
		switch (entity.entity_type)
		{
		case respawn::entity_types::PROWLER:

			if (cfg::aim_prowler)
			{
				auto& npc = entity.type_specific.npc;

				if (npc.m_life_state != LIFE_ALIVE)
					return false;

				ctx.aim_pos = npc.m_center;

				info.velocity = { };

				break;
			}
		}

		break;
	}
	case cache::types::dynamic_prop:
	case cache::types::script_prop:
	{
		const auto& prop = entity.type_specific.prop_dynamic;

		using prop_type = cache::prop_dynamic_types;

		switch (prop.m_type)
		{
		case prop_type::shooting_range_target:

			if (cfg::aim_firing_range_targets)
			{
				ctx.aim_pos = prop.m_center;

				info.velocity = { };
			}

			break;
		}

		break;
	}
	default:
		return false;
	}

	//
	// At this point the current target has been classified as viable.
	// Calculate aim angles and such, and then perform some checks.
	//

	auto entity_pos = ctx.aim_pos;
	auto arbitrary_aim_angle = math::Angle3{};

	math::VectorAngles((entity_pos - ctx.my_position), arbitrary_aim_angle);

	auto distance = entity_pos.DistanceTo(ctx.my_position) / 4096.f;
	auto fov = math::SE::get_fov_difference(ctx.my_angle, ctx.my_view, entity_pos);

	if (distance > ctx.maximum_distance)
		return false;

	if (fov > ctx.maximum_fov)
		return false;

	if (fov < ctx.closest_fov)
	{
		ctx.closest_fov = fov;
		++ctx.viable_targets;

		return true;
	}

	return false;
}
