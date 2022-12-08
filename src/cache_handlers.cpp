#include <include.h>

void cache::process_player(int index, respawn::c_player* player, cache::entry_t& entry, cache::player_t& data)
{
	if (ix::entity_list->get_client_entity<respawn::c_player>(index) != player)
		return;

	data.m_valid = true;

	data.m_life_state = player->life_state();

	if (data.m_life_state == LIFE_ALIVE)
	{
		data.m_team_number = player->team_number();
		data.m_color = respawn::get_player_color(player);
		data.m_health = player->health();
		data.m_health_max = player->health_max();
		data.m_armor = player->armor();
		data.m_armor_max = player->armor_max();
		data.m_visible = player->is_visible();
		data.m_teammate = respawn::is_teammate(player);

		//
		// this data is only valid for actual players and not for dummies.
		// make sure to NOT collect them, otherwise you might get bogus data and crashes.
		//
		if (entry.type == cache::types::player)
		{
			data.m_bleedout_state = player->bleed_out_state();
			data.m_velocity = player->velocity();
		}
		else
		{
			data.m_bleedout_state = 0;
			data.m_velocity = { 0.f, 0.f, 0.f };
		}

		data.m_origin = player->origin();

		respawn::get_entity_center_pos(player, data.m_center);

		if (data.m_name[0] == NULL)
		{
			if (respawn::is_player(player, true, false))
			{
				respawn::player_info_t pl_info{};

				if (ix::engine_client->get_player_info(index, &pl_info))
					memcpy(data.m_name, pl_info.pad, sizeof(data.m_name));
			}
			else
			{
				stl::strcpy(data.m_name, _XS("dummie"));
			}
		}

		for (auto bone_id = 0; bone_id < respawn::bone_count; bone_id++)
		{
			if (!player->get_bone_position(bone_id, data.m_bones[bone_id]))
				break;
		}
	}
}

void cache::process_npc(int index, respawn::c_npc* npc, cache::entry_t& entry, cache::npc_t& data)
{
	if (ix::entity_list->get_client_entity<respawn::c_npc>(index) != npc)
		return;

	data.m_valid = true;

	data.m_life_state = npc->life_state();

	if (data.m_life_state == LIFE_ALIVE)
	{
		data.m_origin = npc->origin();

		data.m_health = npc->health();
		data.m_health_max = npc->health_max();

		respawn::get_entity_center_pos(npc, data.m_center);
	}
}

void cache::process_loot(int index, respawn::c_prop_survival* entity, cache::loot_t& data)
{
	if (ix::entity_list->get_client_entity<respawn::c_prop_survival>(index) != entity)
		return;

	data.m_valid = true;
	data.m_origin = entity->origin();
	data.m_script_id = entity->custom_script_int();

	respawn::get_entity_center_pos(entity, data.m_center);
}

void cache::process_dynamic_prop(int index, respawn::c_entity* entity, cache::entry_t& entry, cache::prop_dynamic_t& data)
{
	if (ix::entity_list->get_client_entity<respawn::c_entity>(index) != entity)
		return;

	data.m_valid = true;
	data.m_origin = entity->origin();

	respawn::get_entity_center_pos(entity, data.m_center);

	//
	// Need to identify dynamic props by their model name.
	//

	using pd_types = cache::prop_dynamic_types;

	switch (util::string_hash(entry.m_model_name, stl::strlen(entry.m_model_name)))
	{
	case 0x853c46d9:
		data.m_type = pd_types::shooting_range_target;
		break;
	case 0x84a17394:
		data.m_type = pd_types::trap_gas;
		break;
	}
}

void cache::process_base(int index, respawn::c_entity* entity, cache::entry_t& entry)
{
	using type = respawn::entity_types;

	switch (respawn::get_entity_type(entity))
	{
	case type::PLAYER:
	{
		entry.type = cache::types::player;
		process_player(index, mem::addr(entity).As<respawn::c_player*>(), entry, entry.type_specific.plr);

		break;
	}
	case type::DUMMIE:
	{
		entry.type = cache::types::dummie;
		process_player(index, mem::addr(entity).As<respawn::c_player*>(), entry, entry.type_specific.plr);

		break;
	}
	case type::PROWLER:
	{
		entry.type = cache::types::npc;
		process_npc(index, mem::addr(entity).As<respawn::c_npc*>(), entry, entry.type_specific.npc);

		break;
	}
	case type::PROP_SURVIVAL:
	{
		entry.type = cache::types::loot;
		process_loot(index, mem::addr(entity).As<respawn::c_prop_survival*>(), entry.type_specific.loot);

		break;
	}
	case type::PROP_DYNAMIC:
	{
		entry.type = cache::types::dynamic_prop;
		process_dynamic_prop(index, entity, entry, entry.type_specific.prop_dynamic);

		break;
	}
	case type::PROP_SCRIPT:
	{
		entry.type = cache::types::script_prop;
		process_dynamic_prop(index, entity, entry, entry.type_specific.prop_dynamic);

		break;
	}
	default:
		entry.type = cache::types::none;
		break;
	}
}
