#include <include.h>

using namespace features;

void c_esp::draw_bar(respawn::color_t color, FLOAT flX, FLOAT flY, FLOAT flW, FLOAT flH, int current, const int max)
{
	if (current < 0)
		current = 0;

	if (current > max)
		current = max;

	flY -= flH;

	const float base = flH / static_cast<float>(max) * static_cast<float>(current);
	const float delta = (flH - base);

	const float bar_x = flX;

	g::p::render.draw_rect_filled({ 5, 5, 5 }, bar_x, flY, bar_x + 5.0f, flY + flH, 175);
	g::p::render.draw_rect_filled(color, bar_x, flY + delta, bar_x + 5.0f, flY + flH);
	g::p::render.draw_rect({ 5, 5, 5 }, bar_x, flY, bar_x + 5.0f, flY + flH);
}

void c_esp::draw_outline(respawn::color_t color, int opacity, math::Vector3 scr_origin, float bw, float bh)
{
	g::p::render.draw_rect_bordered(color, scr_origin.x - bw, scr_origin.y - bh, scr_origin.x + bw, scr_origin.y + 5.f, opacity);
}

void c_esp::show_debug_ents()
{
	const auto& local_player_data = g::p::cache.local_player;
	const auto my_origin = local_player_data.origin;

	math::Vector3 screen{};

	for (auto i = 0; i < ix::entity_list->get_highest_entity_index(); i++)
	{
		auto* const entity = ix::entity_list->get_client_entity<respawn::c_entity>(i);

		if (entity == nullptr)
			continue;

		if (!respawn::world_to_screen(entity->origin(), screen))
			continue;

		if (!entity->identifier())
			continue;

		const auto distance = entity->origin().DistanceTo(my_origin) / 4096.f;

		if (distance > 512.f)
			continue;

		g::p::render.draw_text_ex(
			respawn::colors::white, 2, 12, screen.x, 
			screen.y, _XS("[%s]\n[%s]"), 
			entity->identifier(),
			entity->model_name()
		);
	}
}

void c_esp::process_player(int index, const cache::entry_t& entry, const cache::player_t& player)
{
	const auto& local_player_data = g::p::cache.local_player;

	if (!player.m_valid)
		return;

	if (index == g::p::cache.local_player.index)
	{
		if (!cfg::esp_show_local_player)
			return;

		if (cvar::third_person->m_value_numeric != 1)
			return;
	}
	else
	{
		if (player.m_teammate && !cfg::esp_show_team_mates)
			return;
	}

	const auto is_ally = (player.m_team_number == local_player_data.team_number);
	const auto is_knocked = (player.m_bleedout_state != 0);

	math::Vector3 pos_head, pos_origin = player.m_origin;
	math::Vector3 scr_head, scr_origin;

	if (player.m_life_state != 0)
		return;

	pos_head = player.m_bones[7];

	if (!respawn::world_to_screen(pos_head, scr_head) ||
		!respawn::world_to_screen(pos_origin, scr_origin))
		return;

	const auto bounds_height = math::fabsf(scr_head.y - scr_origin.y) * 1.5f;
	const auto bounds_width = (bounds_height / 2.5f);

	const auto distance = pos_origin.DistanceTo(local_player_data.origin) / 4096.f;

	if (distance > cfg::esp_object_render_dist)
		return;

	const auto render_text = (distance < cfg::esp_text_render_dist);
	const auto opacity = player.m_visible ? 255 : 125;

	const auto draw_pos_top_x = scr_origin.x + (bounds_width / 2.f);
	const auto draw_pos_top_y = scr_origin.y - (bounds_height + 20.f);
	float draw_pos_top_push = 0.f;

	if (render_text)
	{
		if (cfg::esp_players_name)
		{
			g::p::render.draw_text_centered(
				respawn::colors::white, 2, 14,
				draw_pos_top_x, draw_pos_top_y - draw_pos_top_push,
				player.m_name
			);
			draw_pos_top_push += 15.f;
		}

		if (cfg::esp_players_health > 0 && player.m_health > 0)
		{
			if (cfg::esp_players_health == 1)
			{
				g::p::render.draw_text_centered(
					respawn::colors::green, 2, 14,
					draw_pos_top_x, draw_pos_top_y - draw_pos_top_push,
					_XS("%i HP"), player.m_health
				);
			}
			else if (cfg::esp_players_health == 2)
			{
				g::p::render.draw_text_centered(
					respawn::colors::green, 2, 14,
					draw_pos_top_x, draw_pos_top_y - draw_pos_top_push,
					_XS("%i/%i HP"), player.m_health, player.m_health_max
				);
			}

			draw_pos_top_push += 15.f;
		}

		if (cfg::esp_players_armor > 0 && player.m_armor > 0)
		{
			if (cfg::esp_players_armor == 1)
			{
				g::p::render.draw_text_centered(
					respawn::colors::blue, 2, 14,
					draw_pos_top_x, draw_pos_top_y - draw_pos_top_push,
					_XS("%i AR"), player.m_armor
				);
			}
			else if (cfg::esp_players_armor == 2)
			{
				g::p::render.draw_text_centered(
					respawn::colors::blue, 2, 14,
					draw_pos_top_x, draw_pos_top_y - draw_pos_top_push,
					_XS("%i/%i AR"), player.m_armor, player.m_armor_max
				);
			}
		}
	}

	if(cfg::esp_players_outline)
		draw_outline(player.m_color, opacity, scr_origin, bounds_width, bounds_height);

	if(cfg::esp_players_health_bar && player.m_health_max > 0)
		draw_bar({ 65, 255, 65 }, scr_origin.x - bounds_width - 6.f, scr_origin.y, bounds_width, bounds_height, player.m_health, player.m_health_max);

	if(cfg::esp_players_armor_bar && player.m_armor_max > 0)
		draw_bar({ 65, 65, 255 }, scr_origin.x + bounds_width + 2.f, scr_origin.y, bounds_width, bounds_height, player.m_armor, player.m_armor_max);

	if (cfg::esp_player_bone_ids)
		draw_bone_identifiers(player);
}

void c_esp::process_prowler(int index, const cache::entry_t& entry, const cache::npc_t& npc)
{
	const auto& local_player_data = g::p::cache.local_player;

	if (!npc.m_valid)
		return;

	math::Vector3 scr_origin, pos_origin = npc.m_origin;

	if (!respawn::world_to_screen(pos_origin, scr_origin))
		return;

	const auto distance = pos_origin.DistanceTo(local_player_data.origin) / 4096.f;

	if (distance > 4096.f)
		return;

	g::p::render.draw_text(respawn::colors::yellow, 2, 20, scr_origin.x, scr_origin.y, _XS("PROWLER"));
	g::p::render.draw_text_ex(
		respawn::colors::white, 2, 20, 
		scr_origin.x, scr_origin.y + 22, 
		_XS("%i HP"),
		npc.m_health
	);
}

void c_esp::process_loot(int index, const cache::entry_t& entry, const cache::loot_t& loot)
{
	const auto& local_player_data = g::p::cache.local_player;

	if (!loot.m_valid)
		return;

	math::Vector3 scr_origin, pos_origin = loot.m_origin;

	if (!respawn::world_to_screen(pos_origin, scr_origin))
		return;

	const auto distance = pos_origin.DistanceTo(local_player_data.origin) / 4096.f;

	if (distance > 1024.f)
		return;

	using lid = respawn::loot_ids;

	bool found = true;
	std::pair<const char*, respawn::color_t> ldef;

	switch ( static_cast<lid>(loot.m_script_id) )
	{
	/*case lid::SHIELD_BLUE: ldef = std::make_pair(_XS("SH"), respawn::colors::blue); break;
	case lid::SHIELD_PURP: ldef = std::make_pair(_XS("SH"), respawn::colors::purple); break;
	case lid::SHIELD_GOLD: ldef = std::make_pair(_XS("SH"), respawn::colors::yellow); break;

	case lid::BACKPACK_BLUE: ldef = std::make_pair(_XS("BP"), respawn::colors::blue); break;
	case lid::BACKPACK_PURP: ldef = std::make_pair(_XS("BP"), respawn::colors::purple); break;
	case lid::BACKPACK_GOLD: ldef = std::make_pair(_XS("BP"), respawn::colors::yellow); break;

	case lid::HELMET_BLUE: ldef = std::make_pair(_XS("H"), respawn::colors::blue); break;
	case lid::HELMET_PURP: ldef = std::make_pair(_XS("H"), respawn::colors::purple); break;
	case lid::HELMET_GOLD: ldef = std::make_pair(_XS("H"), respawn::colors::yellow); break;*/

	case lid::WEAPON_CHARGE_RIFLE: ldef = std::make_pair(_XS("CR"), respawn::colors::blue); break;
	default:
		found = false;
		break;
	}

	if (found)
	{
		g::p::render.draw_text(ldef.second, 2, 14, scr_origin.x, scr_origin.y, ldef.first);
	}
	else if (cfg::esp_loot_unknown)
	{
		g::p::render.draw_text_ex(
			respawn::colors::yellow, 2, 14, 
			scr_origin.x, scr_origin.y, _XS("ID: %i"), 
			loot.m_script_id
		);
	}
}

void process_generic(const math::Vector3& origin, const char* name, const respawn::color_t color)
{
	const auto& local_player_data = g::p::cache.local_player;

	math::Vector3 scr_origin, pos_origin = origin;

	if (!respawn::world_to_screen(pos_origin, scr_origin))
		return;

	const auto distance = pos_origin.DistanceTo(local_player_data.origin) / 4096.f;

	if (distance > 4096.f)
		return;

	g::p::render.draw_text(color, 2, 20, scr_origin.x, scr_origin.y, name);
}

void c_esp::overlay_hud()
{
	float push = 0.f;
	constexpr float start_x = 150.f, start_y = 5.f;

	//
	//
	//

	if (cfg::other_debug_overlay)
	{
		g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("is_ingame= %i"), g::is_in_game);
		push += 15.f;

		g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("engine time= %i"), (int)*gv::engine_time);
		push += 15.f;

		g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("current time= %i"), (int)ix::global_vars->Curtime);
		push += 15.f;

		g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("sway angles= (%i, %i, %i)"), 
			(int)g::p::sway_angles.x, (int)g::p::sway_angles.y, (int)g::p::sway_angles.z);
		push += 15.f;

		if (g::is_in_game)
		{
			g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("local_player= %p"), g::p::cache.local_player.address);
			push += 15.f;

			g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("local_weapon= %p"), g::p::cache.local_weapon.address);
			push += 15.f;

			if (g::p::cache.local_weapon.address)
			{
				g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("  reloading= %i"), g::p::cache.local_weapon.reloading);
				push += 15.f;

				g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("  next_primary_attack= %i"), (int)g::p::cache.local_weapon.next_primary_attack);
				push += 15.f;

				g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("  next_ready_time= %i"), (int)g::p::cache.local_weapon.next_ready_time);
				push += 15.f;

				g::p::render.draw_text_ex(respawn::colors::white, 2, 14, start_x, start_y + push, _XS("  can_fire= %i"), (int)g::p::cache.local_weapon.can_fire);
				push += 15.f;
			}
		}
	}

	if (g::unprotected_game)
	{
		g::p::render.draw_text(respawn::colors::red, 2, 14, start_x, start_y + push, _XS("*UNPROTECTED GAME*"));
		push += 15.f;
	}

	if (cfg::aim_enabled)
	{
		if (cfg::aim_silent == 0 || cfg::aim_silent == 1)
		{
			g::p::render.draw_text(respawn::colors::red, 2, 14, start_x, start_y + push, _XS("*UNSAFE AIM*"));
			push += 15.f;
		}
	}

	if (cfg::custom_crosshair_enabled)
	{
		if (respawn::is_playing())
		{
			const auto screen_size = g::p::screen;

			const float x_center = static_cast<float>(screen_size.w) / 2.0f;
			const float y_center = static_cast<float>(screen_size.h) / 2.0f;

			const float width = cfg::custom_crosshair_size;

			g::p::render.draw_rect_filled(respawn::colors::white, x_center - width, y_center - 1.0f, x_center + width, y_center + 1.0f);
			g::p::render.draw_rect_filled(respawn::colors::white, x_center - 1.0f, y_center - width, x_center + 1.0f, y_center + width);

			if (cfg::custom_crosshair_outline)
			{
				g::p::render.draw_rect(respawn::colors::black, x_center - width, y_center - 1.0f, x_center + width, y_center + 1.0f);
				g::p::render.draw_rect(respawn::colors::black, x_center - 1.0f, y_center - width, x_center + 1.0f, y_center + width);
			}
		}
	}
}

void c_esp::info_panel()
{

}

void c_esp::draw_bone_identifiers(const cache::player_t& player)
{
	math::Vector3 pos, screen;

	for (auto i = 0ull; i < 20; i++)
	{
		pos = player.m_bones[i];

		if (!respawn::world_to_screen(pos, screen))
			continue;

		char text[64];

		util::format_string(text, sizeof(text), _XS("%i"), i);

		g::p::render.draw_text({ 65, 255, 65 }, 2, 15, screen.x, screen.y, text);
	}
}

void c_esp::paint()
{
	if (!cfg::esp_enabled)
		return;

	if (cfg::esp_show_debug_ents)
	{
		show_debug_ents();
		return;
	}

	if (respawn::is_playing())
	{
		for (auto i = 0ull; i < g::p::ent_cache.count(); i++)
		{
			auto data = g::p::ent_cache.get_neutral(i);

			if (!data.valid)
				continue;

			switch (data.type)
			{
			case cache::types::player:
			case cache::types::dummie:
			{
				if (cfg::esp_players)
					process_player(i, data, data.type_specific.plr);

				break;
			}
			case cache::types::npc:
			{
				using ent_type = respawn::entity_types;

				switch (data.entity_type)
				{
				case ent_type::PROWLER:

					if(cfg::esp_prowler)
						process_prowler(i, data, data.type_specific.npc);

					break;
				}

				break;
			}
			case cache::types::loot:
			{
				if (cfg::esp_loot)
					process_loot(i, data, data.type_specific.loot);

				break;
			}
			case cache::types::dynamic_prop:
			case cache::types::script_prop:
			{
				using pd_types = cache::prop_dynamic_types;

				switch (data.type_specific.prop_dynamic.m_type)
				{
				case pd_types::shooting_range_target:
					
					if(cfg::esp_firing_range_targets)
						process_generic(data.type_specific.prop_dynamic.m_center, _XS("X"), respawn::colors::red);

					break;
				case pd_types::trap_gas:
					
					if(cfg::esp_traps)
						process_generic(data.type_specific.prop_dynamic.m_center, _XS("GAS-T"), respawn::colors::yellow);

					break;
				}

				break;
			}
			default:
				break;
			}
		}
	}

	overlay_hud();
	info_panel();
}
