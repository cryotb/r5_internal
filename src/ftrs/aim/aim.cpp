#include <include.h>

using namespace features;

bool c_aim::find_target(respawn::c_player* me, cache::entry_t* result, aim::find_context_t& ctx)
{
	ctx.viable_targets = 0;

	ctx.closest_distance = ctx.maximum_distance;
	ctx.closest_fov = ctx.maximum_fov;

	ctx.me = me;

	ctx.my_position = me->origin();
	ctx.my_view = me->view_offset();
	ctx.my_angle = me->eye_angles();

	size_t viable_targets{ };
	aim::find_context_t target_ctx{ };

	for (auto i = 0ull; i < g::p::ent_cache.count(); i++)
	{
		auto entity = g::p::ent_cache.get_neutral(i);

		if (!entity.valid)
			continue;

		if (aim::is_target_viable(entity, ctx))
		{
			*result = entity;
			target_ctx = ctx;

			++viable_targets;
		}
	}

	ctx = target_ctx;

	return viable_targets;
}

bool c_aim::is_weapon_compatible(respawn::c_weapon* wpn) noexcept
{
	auto name_idx = wpn->name_index();

	if (name_idx == respawn::weapon_name_indexes::hands)
		return false;

	return true;
}

void c_aim::create_move(respawn::c_player* me, respawn::user_cmd_t* cmd)
{
	if (!cfg::aim_enabled)
		return;

	if (!respawn::is_playing())
		return;

	if (me == nullptr)
		return;

	if (!respawn::is_alive(me) || respawn::is_knocked(me))
		return;

	auto* const my_wpn = me->get_active_weapon();

	if (my_wpn == nullptr || !is_weapon_compatible(my_wpn))
		return;

	if (cfg::aim_bullet_time && !my_wpn->can_fire(me))
	{
		is_aiming = false;
		return;
	}

	cache::entry_t target{ };
	aim::find_context_t find_ctx{ };

	find_ctx.maximum_distance = cfg::aim_distance;
	find_ctx.maximum_fov = cfg::aim_fov;

	if (cfg::aim_distance <= 0.1f)
		find_ctx.maximum_distance = FLT_MAX;

	if (g::p::cache.local_player.zooming)
		find_ctx.maximum_fov /= 2.5f;

	effective_max_fov_ = find_ctx.maximum_fov;

	if (!find_target(me, &target, find_ctx))
	{
		is_aiming = false;
		return;
	}

	auto bone_id = respawn::bone_id::head;

	if (cfg::aim_random_hitbox)
		bone_id = respawn::bone_id::torso_upper;

	auto my_shoot_pos = me->view_offset();
	auto hit_pos = find_ctx.aim_pos;
	auto aim_ang = math::Angle3{};

	if (cfg::aim_bullet_prediction)
	{
		auto get_distance = [](math::Vector3 target_pos, math::Vector3 our_pos)
		{
			float dist = math::sqrtf((our_pos.x - target_pos.x) * (our_pos.x - target_pos.x)
				+ (our_pos.y - target_pos.y) * (our_pos.y - target_pos.y)
				+ (our_pos.z - target_pos.z) * (our_pos.z - target_pos.z));

			return dist * 0.01905f;
		};

		auto velocity = find_ctx.target_info.velocity;

		if (velocity.Length2d() > 0.f)
		{
			auto world_gravity = 750.f;
			auto distance = get_distance(hit_pos, my_shoot_pos) / 0.01905f;

			auto bullet_speed = my_wpn->projectile_speed();
			auto bullet_gravity = my_wpn->projectile_gravity();

			auto travel_time = (distance / bullet_speed);
			auto prediction_scale = max(cfg::aim_bullet_prediction_scale, bullet_gravity);

			hit_pos.x += (velocity.x * travel_time) * prediction_scale;
			hit_pos.y += (velocity.y * travel_time) * prediction_scale;
			hit_pos.z += (world_gravity * travel_time * travel_time) * prediction_scale;
		}
	}

	math::VectorAngles((hit_pos - my_shoot_pos), aim_ang);

	auto my_angle = me->eye_angles();
	auto my_angle_dyn = (!cfg::no_sway ? me->dyn_angles() : g::p::sway_angles);

	if (cfg::aim_no_recoil > 0)
	{
		auto delta = (my_angle_dyn - my_angle);
		auto length = delta.Length2d();

		if (cfg::aim_no_recoil == 1)
		{
			delta.x = (delta.x / 2.f);
			delta.y = (delta.y / 2.f);
		}

		if (length > 0.f)
		{
			aim_ang.x -= delta.x;
			aim_ang.y -= delta.y;
		}
	}

	if (cfg::aim_smooth_out && cfg::aim_smooth > 0.f)
	{
		auto vCurrentAngle = my_angle;
		auto vAimDelta = (aim_ang - vCurrentAngle);

		math::NormalizeAngle(vCurrentAngle);
		math::NormalizeAngle(vAimDelta);

		const auto flMultiplier = cfg::aim_smooth;
		const auto vActualAimAngle = aim_ang; // back it up so we can compare it later.

		if (vAimDelta.x >= +89.f || vAimDelta.x <= -89.f)
			vAimDelta.x = 0.f;

		if (flMultiplier >= 0.01f)
		{
			aim_ang.x = vCurrentAngle.x + ((vAimDelta.x + FLT_EPSILON) / flMultiplier);
			aim_ang.y = vCurrentAngle.y + ((vAimDelta.y + FLT_EPSILON) / flMultiplier);
			aim_ang.z = 0.f;
		}
	}

	bool should_aim = false;

	if (cfg::aim_ignore_key)
		should_aim = true;
	else if (cfg::aim_auto_fire && g::p::input_mgr.is_button_down(VK_SHIFT))
		should_aim = true;
	else if (cmd->buttons & IN_ATTACK)
		should_aim = true;

	if (cfg::aim_rapid_fire)
	{
		if (cmd->buttons & IN_ATTACK && my_wpn->can_fire(me))
			cvar::time_scale->m_value_float = cfg::aim_rapid_fire_boost;
		else
			cvar::time_scale->m_value_float = 1.f;
	}

	if (should_aim)
	{
		is_aiming = true;
		target_pos_ = hit_pos;
		wish_angle = aim_ang;

		switch (cfg::aim_silent)
		{
		case 0: // plain
			cmd->viewangles = aim_ang;
			me->eye_angles() = aim_ang;
			break;
		case 1: // silent (1)
		case 2: // silent (2)
			cmd->viewangles = aim_ang;
			break;
		default:
			break;
		}

		if (cfg::aim_auto_fire)
			cmd->buttons |= IN_ATTACK;
	}
	else
	{
		is_aiming = false;
		writecmd_silent = false;
		wish_angle = { };
	}
}

void c_aim::cl_move(respawn::c_player* me)
{
	//
	// ...
	//
}

void c_aim::paint()
{
	if (!respawn::is_playing())
		return;

	if (!cfg::aim_enabled)
		return;

	if(cfg::aim_visualize_range)
		draw_range();

	if (is_aiming)
	{
		math::Vector3 target_scr{ };

		if (respawn::world_to_screen(target_pos_, target_scr))
		{
			const float x_center = target_scr.x;
			const float y_center = target_scr.y;

			const float width = 5.0f;

			g::p::render.draw_rect_filled(respawn::colors::red, x_center - width, y_center - 1.0f, x_center + width, y_center + 1.0f);
			g::p::render.draw_rect_filled(respawn::colors::red, x_center - 1.0f, y_center - width, x_center + 1.0f, y_center + width);

			g::p::render.draw_rect(respawn::colors::red, x_center - width, y_center - 1.0f, x_center + width, y_center + 1.0f);
			g::p::render.draw_rect(respawn::colors::red, x_center - 1.0f, y_center - width, x_center + 1.0f, y_center + width);
		}
	}
}

void c_aim::draw_range()
{
	auto screen = g::p::screen;

	float local_fov = 110.f;
	float valid_fov = effective_max_fov_ / local_fov;

	float local_range = (local_fov * static_cast<float>(M_PI) / 180.f);
	float valid_range = (valid_fov * static_cast<float>(M_PI) / 180.f);

	float radius = (math::tanf((valid_fov / 2.f)) / math::tanf((local_fov / 2.f)) * screen.w * 25.f);

	float scenter_x = (screen.w / 2);
	float scenter_y = (screen.h / 2);

	g::p::render.draw_circle(respawn::colors::white, scenter_x, scenter_y, radius, 100, 125);
}
