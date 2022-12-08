#include <include.h>

void aim::psilent(respawn::c_player* me, respawn::c_weapon* weapon)
{
	const auto net_chan = respawn::net_chan();

	if (net_chan == nullptr)
		return;

	if (!ix::engine_client->is_in_game())
		return;

	const auto choked_packets = net_chan->num_choked_packets();
	const auto last_out_cmd_nr = respawn::last_out_cmd_nr();

	if (f::aim.is_aiming)
	{
		g::choke_this_tick = (choked_packets <= cfg::aim_choke_cap);

		auto* const silent_cmd = ix::input->get_user_cmd(last_out_cmd_nr - 1);

		if (silent_cmd && silent_cmd->command_number)
		{
			silent_cmd->buttons |= IN_ATTACK;
			silent_cmd->viewangles = f::aim.wish_angle;
		}
	}
}
