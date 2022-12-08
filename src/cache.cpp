#include <include.h>

void c_entity_cache::initialize()
{
	_entities = stl::mem_alloc<decltype(_entities)>( sizeof(cache::player_t) * cache::capacity );

	if (_entities == nullptr)
	{
		log_info_("failed to allocate entity-cache storage space.");
		return report_failure();
	}
}

void c_entity_cache::on_createmove()
{
	on_update();
}

void c_entity_cache::collect_entities()
{
	size_t valid_count = 0;
	size_t player_count = ix::global_vars->MaxClients;

	const auto my_index = ix::engine_client->get_local_player();

	for (auto i = 0; i < ix::entity_list->get_highest_entity_index(); i++)
	{
		auto entity = ix::entity_list->get_client_entity<respawn::c_entity>(i);
		auto& entry = _entities[i];

		if (entity == nullptr)
		{
			entry.valid = false;
			continue;
		}

		entry.valid = true;
		entry.index = i;
		entry.entity_type = respawn::get_entity_type(entity);

		if (!entry.m_pointer || entry.m_pointer != entity)
		{
			//
			// Invalidate our cached information.
			//

			auto model_name = entity->model_name();

			if (model_name && stl::strlen(model_name) < sizeof(entry.m_model_name))
				stl::strcpy(entry.m_model_name, model_name);

			entry.m_pointer = entity;
		}

		process_base(i, entity, entry);

		++valid_count;
	}

	_entity_count = ix::entity_list->get_highest_entity_index();
}

void c_entity_cache::on_update()
{
	if (!g::is_in_game)
		return;

	collect_entities();
}

void c_entity_cache::reset()
{
	memset(_entities, 0, sizeof(cache::player_t) * cache::capacity);
}

void* c_entity_cache::get_pointer_internal(size_t index)
{
	if (index >= _entity_count || index >= cache::capacity)
		return {};

	auto info = _entities[index];

	if (!info.valid)
		return {};

	//
	// check if the cached data is still valid for the given index.
	//
	if (info.m_pointer != ix::entity_list->get_client_entity<void*>( static_cast<int>(index) ))
		return {};

	return info.m_pointer;
}
