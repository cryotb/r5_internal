#include <include.h>

using namespace features;

void force_heirloom(respawn::c_entity* vm, int model_index)
{
	if (!vm || !model_index)
		return;

	vm->set_model_index(model_index);
}

bool is_valid_sequence(int heirloom_idx, int sequence_nr)
{
	int blacklisted_sequence_nr = 0;

	switch (heirloom_idx)
	{
	case HEIRLOOM_IDX_WRAITH:
	case HEIRLOOM_IDX_BLOODHOUND:
		blacklisted_sequence_nr = 60;
		break;
	case HEIRLOOM_IDX_LIFELINE:
		blacklisted_sequence_nr = 65;
		break;
	case HEIRLOOM_IDX_PATHFINDER_GLOVES:
		blacklisted_sequence_nr = 64;
		break;
	case HEIRLOOM_IDX_OCTANE_KNIFE:
		blacklisted_sequence_nr = 57;
		break;
	case HEIRLOOM_IDX_MIRAGE:
		blacklisted_sequence_nr = 53;
		break;
	case HEIRLOOM_IDX_GIBRALTAR:
		blacklisted_sequence_nr = 71;
		break;
	case HEIRLOOM_IDX_BANGALORE:
		blacklisted_sequence_nr = 79;
		break;
	default:
		//
		// No validation for this specific heirloom.
		//
		break;
	}

	return (sequence_nr != blacklisted_sequence_nr);
}

void reset_sequence(int heirloom_idx, uint32_t* sequence_nr)
{
	uint32_t default_sequence = 0;

	switch (heirloom_idx)
	{
	case HEIRLOOM_IDX_WRAITH:
	case HEIRLOOM_IDX_BLOODHOUND:
		default_sequence = 46;
		break;
	case HEIRLOOM_IDX_LIFELINE:
		default_sequence = 48;
		break;
	case HEIRLOOM_IDX_PATHFINDER_GLOVES:
		default_sequence = 51;
		break;
	case HEIRLOOM_IDX_OCTANE_KNIFE:
		default_sequence = 42;
		break;
	case HEIRLOOM_IDX_MIRAGE:
		default_sequence = 43;
		break;
	case HEIRLOOM_IDX_GIBRALTAR:
		default_sequence = 57;
		break;
	case HEIRLOOM_IDX_BANGALORE:
		default_sequence = 59;
		break;
	default:
		//
		// No rule for this specific heirloom.
		//
		break;
	}

	if (default_sequence != 0)
		*sequence_nr = default_sequence;
}

void c_skin_changer::create_move(respawn::c_player* me, respawn::user_cmd_t* cmd)
{

}

bool is_hands(respawn::c_entity* entity)
{
	auto model_name = entity->model_name();

	if (model_name)
	{
		if (stl::strlen(model_name) == 45)
		{
			if (model_name[12] == 'e' && model_name[15] == 't' && model_name[18] == 'h')
				return true;
		}
	}

	return false;
}

void c_skin_changer::frame_stage_notify(int stage, respawn::c_player* me)
{
	if (!cfg::other_heirloom_changer_active || !respawn::is_playing())
		return;

	if (me == nullptr)
		return;

	switch (stage)
	{
	case respawn::frame_render_start:
	{
		if (!respawn::is_alive(me) || respawn::is_knocked(me))
			return;

		auto my_wpn = me->get_active_weapon();
		auto my_vm = me->get_view_model();

		if (my_wpn == nullptr || my_vm == nullptr)
			return;

		auto name_idx = my_wpn->name_index();

		if (name_idx != respawn::weapon_name_indexes::hands)
			return;

		if (!is_hands(my_vm))
			return;

		auto heirloom_idx = ix::model_info->get_model_index(respawn::_heirloom_table.get(cfg::other_heirloom_index));

		if (!heirloom_idx || heirloom_idx == -1)
			return;

		const auto cur_sequence = my_vm->anim_sequence();
		const auto cur_activity = my_vm->get_sequence_activity(cur_sequence);

		force_heirloom(my_vm, heirloom_idx);

		auto new_sequence = my_vm->get_activity_sequence(cur_activity);

		if (new_sequence != UINT32_MAX)
		{
			if (!is_valid_sequence(cfg::other_heirloom_index, new_sequence))
				reset_sequence(cfg::other_heirloom_index, &new_sequence);

			my_vm->anim_sequence() = new_sequence;
		}

		break;
	}
	default:
		break;
	}
}
