#include <include.h>

using namespace features;

respawn::color_t c_glow::get_color(const cache::player_t& player)
{
	auto result = respawn::color_t{};

	const int current_shield = player.m_armor;
	const int current_health = player.m_health;

	const int max_shield = player.m_armor_max;
	const int max_health = player.m_health_max;

	const int total_health = (current_health + current_shield);
	const int split_count = 3;

	const auto frac_shield = ((max_shield + 1) / 3);
	const auto frac_health = ((max_health + 1) / 3);

	if (player.m_bleedout_state == 0)
	{
		if (frac_shield && frac_health)
		{
			const auto shield_state = ((current_shield + 1) / frac_shield);
			const auto health_state = ((current_health + 1) / frac_health);

			const auto total_state = (shield_state + health_state);

			switch (total_state)
			{
			case 0:
			case 1:
				result = respawn::glow_colors::red;
				break;
			case 2:
			case 3:
				result = respawn::glow_colors::yellow;
				break;
			case 4:
			case 5:
			case 6:
				result = respawn::glow_colors::green;
				break;
			default:
				break;
			}
		}
		else
			result = respawn::glow_colors::green;
	}
	else
		result = respawn::glow_colors::white;

	return result;
}

void c_glow::create_move(respawn::c_player* me, respawn::user_cmd_t* cmd)
{
	const auto& local_player_data = g::p::cache.local_player;

	for (auto i = 0ull; i < g::p::ent_cache.count(); i++)
	{
		auto entry = g::p::ent_cache.get_neutral(i);

		if (!entry.valid)
			continue;

		switch (entry.type)
		{
		case cache::types::player:
		case cache::types::dummie:
			break;
		default:
			continue;
		}

		auto data = entry.type_specific.plr;

		if (entry.index == g::p::cache.local_player.index)
		{
			continue;
		}
		else
		{
			if (data.m_teammate && !cfg::esp_show_team_mates)
				continue;
		}

		auto* const pointer = g::p::ent_cache.get_pointer<respawn::c_player*>(i);

		if (pointer)
		{
			if (cfg::esp_enabled && cfg::esp_players && cfg::esp_players_glow)
			{
				pointer->glow_enabled() = true;
				pointer->glow_type() = 0x2;
				pointer->glow_config() = { 101, 102, 46, 96 };
				pointer->glow_color() = get_color(data);
			}
			else
			{
				if (pointer->glow_enabled())
					pointer->glow_enabled() = false;
			}
		}
	}
}
