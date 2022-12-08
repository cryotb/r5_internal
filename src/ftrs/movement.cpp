#include <include.h>

using namespace features;

void bunnyhop(bool& hopping, respawn::c_player* me, respawn::user_cmd_t* cmd)
{
	if (!cfg::movement_bunny_hop)
		return;

	if (hopping)
	{
		if (!(cmd->buttons & IN_JUMP))
		{
			hopping = false;
			return;
		}

		if (!(me->flags() & FL_ONGROUND))
			cmd->buttons &= ~IN_JUMP;
	}
	else
	{
		if (cmd->buttons & IN_DUCK)
			hopping = true;
	}
}

void freecam(respawn::c_player* me)
{
	static bool is_toggled{ };

	g::in_freecam = cfg::movement_freecam && is_toggled;

	if (si::create_move.was_button_down(VK_NUMPAD7))
		is_toggled = !is_toggled;

	me->observer_mode() = g::in_freecam ? 7 : 0;
}

void thirdperson()
{
	static bool previous_enabled{ };
	static bool is_toggled{ };
	static int initial_value{ };

	if (!initial_value)
		initial_value = cvar::third_person->m_value_numeric;

	const auto maintain_value = []()
	{
		if (cfg::movement_thirdperson && is_toggled)
			cvar::third_person->m_value_numeric = 1;
		else
			cvar::third_person->m_value_numeric = initial_value;
	};

	if (previous_enabled != cfg::movement_thirdperson)
	{
		maintain_value();
		previous_enabled = cfg::movement_thirdperson;
	}

	if (si::create_move.was_button_down(VK_XBUTTON1))
	{
		is_toggled = !is_toggled;
		maintain_value();
	}
}

void airstuck()
{
	static bool is_toggled{ };
	static int initial_value{ };

	if (!initial_value)
		initial_value = cvar::move_use_dt->m_value_numeric;

	const auto maintain_value = []()
	{
		if (is_toggled)
			cvar::move_use_dt->m_value_numeric = 0;
		else
			cvar::move_use_dt->m_value_numeric = initial_value;
	};

	if (si::create_move.was_button_down(VK_NUMPAD4))
	{
		is_toggled = !is_toggled;
		maintain_value();
	}
}

void fakelag(respawn::c_player* me)
{
	static bool is_toggled{ };

	if (!cfg::fake_lag)
		return;

	const auto my_index = g::p::cache.local_player.index;
	const auto my_team_number = g::p::cache.local_player.team_number;

	const auto choked_packets = g::p::cache.net_chan.num_choked_packets;

	if (cfg::fake_lag == 1)
	{
		//
		// adaptive, based on enemy sight.
		//
		size_t num_visible_opponents{};

		for (auto i = 0ull; i < ix::global_vars->MaxClients; i++)
		{
			if (i == my_index)
				continue;

			auto player_data = g::p::ent_cache.get_player(i);

			if (!player_data.m_valid)
				continue;

			if (player_data.m_team_number == my_team_number)
				continue;

			if (!player_data.is_alive() || !player_data.m_visible)
				continue;

			++num_visible_opponents;
		}

		if (num_visible_opponents > 0)
			g::choke_this_tick = (choked_packets < cfg::fake_lag_choke_cap);
	}
	else if (cfg::fake_lag == 2)
	{
		//
		// full pulled, all time.
		// TODO: once we figure out xbutton keys for input handler,
		// make an option like on aimbot called ignore key 
		// and make the default one require a key like on the external.
		//

		if ( (!cfg::fake_lag_ignore_key && g::p::input_mgr.is_button_down(VK_XBUTTON2)) || cfg::fake_lag_ignore_key)
		{
			g::choke_this_tick = (choked_packets < cfg::fake_lag_choke_cap);
		}
	}
}

void charge_rifle(respawn::c_player* me)
{
	if (!cfg::other_infinite_charge_rifle)
		return;

	static bool toggled = false;

	if (si::create_move.was_button_down(VK_F12))
	{
		toggled = !toggled;

		if (!toggled)
		{
			cvar::time_scale->m_value_float = 1.f;
		}
	}

	auto my_wpn = me->get_active_weapon();

	if (my_wpn)
	{
		if (my_wpn->projectile_speed() == 1.f)
		{
			if (toggled)
			{
				cvar::time_scale->m_value_float = 0.000000000000000000000000000000001f;
			}
		}
	}
}

void c_movement::create_move(respawn::c_player* me, respawn::user_cmd_t* cmd)
{
	if (respawn::is_playing())
	{
		bunnyhop(m_hopping, me, cmd);
		charge_rifle(me);
	}

	freecam(me);
	fakelag(me);
	thirdperson();
	airstuck();
}
