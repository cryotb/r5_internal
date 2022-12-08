#include <include.h>

#define DA_CODE(NAME, PAT, OFFS) dyn_addr::add_dynamic_address_code(_XS(NAME), _XSW(PAT), OFFS)
#define DA_PTR(NAME, PAT, OFFS, INSTR_OFFS, INSTR_LEN) dyn_addr::add_dynamic_address_ptr(_XS(NAME), _XSW(PAT), OFFS, INSTR_OFFS, INSTR_LEN)

bool r5::setup_context()
{
	const auto get_process_info = []() -> bool
	{
		auto& process_info = g::p::context->m_process_info;
		auto process_handle = win_api::get_module_handle(nullptr);

		if (process_handle == nullptr)
			return false;

		process_info.base = mem::addr(process_handle).Base();

		auto* process_dh = PIMAGE_DOS_HEADER{};
		auto* process_nth = PIMAGE_NT_HEADERS{};

		process_dh = mem::addr(process_info.base).As<decltype(process_dh)>();

		if (process_dh->e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		process_nth = mem::addr(process_info.base).Add(process_dh->e_lfanew).As<decltype(process_nth)>();

		if (process_nth->Signature != IMAGE_NT_SIGNATURE)
			return false;

		process_info.length = process_nth->OptionalHeader.SizeOfCode;

		return true;
	};

	const auto get_self_info = []()
	{
		auto decrypt = [](void* data, size_t length)
		{
			constexpr uint64_t header_encryption_key = 0xDE9644521;

			for (size_t i = 0; i < length; i++)
				reinterpret_cast<uint8_t*>(data)[i] ^= header_encryption_key;
		};

		g::p::context->my_base = mem::addr(&__ImageBase).Base();

		auto decrypted_dh = IMAGE_DOS_HEADER{ };
		auto decrypted_nth = IMAGE_NT_HEADERS{ };
		
		memset(&decrypted_dh, 0, sizeof(decrypted_dh));
		memset(&decrypted_nth, 0, sizeof(decrypted_nth));

		if (!util::read_memory(&decrypted_dh, g::p::context->my_base, sizeof(decrypted_dh)))
			return false;

		decrypt(&decrypted_dh, sizeof(decrypted_dh));

		if (decrypted_dh.e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		auto nth_address = g::p::context->my_base + decrypted_dh.e_lfanew;

		if(!util::read_memory(&decrypted_nth, nth_address, sizeof(decrypted_nth)))
			return false;

		decrypt(&decrypted_nth, sizeof(decrypted_nth));

		if (decrypted_nth.Signature != IMAGE_NT_SIGNATURE)
			return false;

		g::p::context->my_length = decrypted_nth.OptionalHeader.SizeOfImage;

		return true;
	};

	g::p::context = stl::mem_alloc<context_t*>( sizeof(context_t) );

	if (g::p::context == nullptr)
		return false;

	memset(g::p::context, 0, sizeof(context_t));

	if (!get_process_info())
		return false;

	if (!get_self_info())
		return false;

	g::build_hash = crc_calculate32(reinterpret_cast<void*>(g::p::context->my_base), g::p::context->my_length);

	return true;
}

bool r5::setup_dyn_addrs()
{
	DA_PTR("ptr_local_player", L"48 8B 05 ? ? ? ? 48 0F 44 C7", 0, 0x3, 0x7);
	DA_PTR("ptr_input", L"48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 4C 8B 80 B8 00 00 00", 0, 0x3, 0x7);
	DA_PTR("ptr_input_system", L"48 8B 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 08 4C 8B 81", 0, 0x3, 0x7);
	DA_PTR("ptr_global_vars", L"4C 8B 05 ? ? ? ? 0F 57 C9", 0, 0x3, 0x7);
	DA_PTR("ptr_engine_vgui", L"48 8B 0D ? ? ? ? 48 83 C4 40", 0, 0x3, 0x7);
	DA_PTR("ptr_game_movement", L"48 89 05 ? ? ? ? 48 85 C9 74 3C", 0, 0x3, 0x7);
	DA_PTR("ptr_engine_client", L"48 8B 05 ? ? ? ? 4C 8D 25 ? ? ? ? 48 8D 15", 0, 0x3, 0x7);
	DA_PTR("ptr_convar_mgr", L"48 8B 3D ? ? ? ? 48 8B D9 48 8D 0D ? ? ? ? 48 8B 07 4C 8B 80 ? ? ? ? 48 3B C1", 0, 0x3, 0x7);
	DA_PTR("ptr_engine_time", L"F3 0F 11 05 ? ? ? ? 83 F8 FF 74 29", 0, 0x4, 0x8);
	DA_PTR("ptr_mat_system_surface", L"4C 8B 1D ? ? ? ? 49 8B 03 49 3B C7", 0, 0x3, 0x7);
	DA_PTR("ptr_net_chan", L"48 8B 0D ? ? ? ? 0F 57 F6 0F 2F B1 ? ? ? ? 77 21", 0, 0x3, 0x7);
	DA_PTR("ptr_view_render", L"48 8B 3D ? ? ? ? 48 8D 0D ? ? ? ? 4C 89 3D", 0, 0x3, 0x7);
	DA_PTR("ptr_last_out_cmd", L"44 8B 2D ? ? ? ? 44 8B 25 ? ? ? ? 48 85 C0 74 05 8B 50 40 EB 03", 0, 0x3, 0x7);
	DA_PTR("ptr_engine_trace", L"48 8B 0D ? ? ? ? 48 8D 55 10 48 8B 01 4D 85 F6 74 18", 0, 0x3, 0x7);
	DA_PTR("ptr_game_instance_data", L"48 8B 0D ? ? ? ? 48 C1 E2 08 48 0B D0 48 03 15 ? ? ? ? 48 8B 01 48 8B 5C 24 ? 48 83 C4 28 48 FF 60 70 48 8B 0D", 0, 0x3, 0x7);
	DA_PTR("ptr_model_info", L"48 8B 0D ? ? ? ? 44 8B C3 44 8B C8 48 89 74 24 ? 49 8B D6 E8", 0, 0x3, 0x7);
	DA_PTR("ptr_client_mode", L"48 8B 05 ? ? ? ? 48 85 C0 74 0A F3 0F 11 48 ? 48 83 C4 28 C3", 0, 0x3, 0x7);
	DA_PTR("ptr_file_system", L"48 8B 1D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05", 0, 0x3, 0x7);

	DA_PTR("cursor_enable", L"0F B6 05 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8B 01 48 3B C3", 0, 0x3, 0x7);
	DA_PTR("crosshair_state", L"83 3D ? ? ? ? ? 0F 85 ? ? ? ? 49 8B CD E8 ? ? ? ? A9 ? ? ? ? 0F 85 ? ? ? ? 83 F8 01 0F 84 ? ? ? ? 41 80 BD", 0, 0x2, 0x7);
	
	DA_PTR("fn_cl_move_send", L"E8 ? ? ? ? EB 21 0F 57 C0 F3 0F 5A C6", 0, 0x1, 0x5);
	DA_PTR("fn_create_interface", L"E8 ? ? ? ? 48 89 05 ? ? ? ? 48 83 3D ? ? ? ? ? 0F 84 ? ? ? ? 33 D2", 0, 0x1, 0x5);
	DA_PTR("fn_write_user_cmd", L"E8 ? ? ? ? 80 7D B4 00 0F 94 C0", 0, 0x1, 0x5);
	DA_PTR("fn_paint_end", L"E8 ? ? ? ? 49 8B 8E ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 40 F6 C5 01", 0, 0x1, 0x5);
	DA_PTR("fn_set_model_index", L"E8 ? ? ? ? 48 8B 8B ? ? ? ? 8B 91 ? ? ? ? 33 ED 83 FA FF", 0, 0x1, 0x5);
	DA_PTR("fn_set_abs_angles", L"E8 ? ? ? ? 48 8B 83 ? ? ? ? 48 8D 8B ? ? ? ? F3 0F 10 A8", 0, 0x1, 0x5);

	DA_CODE("fn_seq_desc", L"4C 8B 41 ? 4D 85 C0 75 ? 4C 8B 41 ? 33 C0 41 3B 90 ? ? ? ? 0F 42 C2 48 98", 0);
	DA_CODE("fn_set_legendary_view_model", L"40 53 48 83 EC 20 48 8B D9 83 FA 10 7C 17 41 B8", 0);
	DA_CODE("fn_get_model", L"40 53 48 83 EC ? 48 83 B9 ? ? ? ? ? 48 8B D9 75 ? 48 8B 41 ?", 0);
	DA_CODE("fn_set_model", L"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B EA 48 8B F1 48 85 C9 75 0E 48 8D 0D", 0);

	DA_CODE("paint_start_drawing", L"48 8B C4 53 56 57 48 81 EC ? ? ? ? 0F 29 70 D8 48 8D", 0);
	DA_CODE("paint_finish_drawing", L"40 53 48 83 EC 20 48 8B 0D ? ? ? ? C6 05 ? ? ? ? ? 48 8B", 0);
	DA_CODE("view_render_start", L"48 8B C4 48 89 48 08 53 55 56 57 41 55 48 81 EC ? ? ? ? 0F 29 70 B8 4C 8B E9", 0);
	DA_CODE("cl_move", L"48 81 EC ? ? ? ? 83 3D ? ? ? ? ? 44 0F 29 54 24", 0);

	DA_CODE("silent1", L"F3 0F 11 87 ? ? ? ? 89 44 24 38 F3 0F 10 44 24 ? F3 0F 11 87", 0);

	if (!dyn_addr::grab_all())
		return false;

	if (!dyn_offs::initialize())
		return false;

	return true;
}

bool r5::setup_interfaces_early()
{
	ix::file_system = *reinterpret_cast<respawn::c_file_system**>(DYN_ADDR("ptr_file_system"));

	log_info("file_system_ptr => %p", ix::file_system);

	return true;
}

bool r5::setup_interfaces()
{
	const auto grab_interface = [](const char* name, void** result)
	{
		*result = respawn::create_interface(name);

		if ((*result))
		{
			log_info("grabbed interface '%s' at address '%p'.", name, *result);
		}
		else
		{
			log_info("failed to grab interface '%s'.", name);
		}

		return !( (*result) == nullptr );
	};

	if (!grab_interface(_XS("VClient018"), (void**)&ix::base_client))
		return false;

	if (!grab_interface(_XS("VClientEntityList003"), (void**)&ix::entity_list))
		return false;

	ix::input = reinterpret_cast<respawn::c_input*>(DYN_ADDR("ptr_input"));
	ix::input_system = *reinterpret_cast<respawn::c_input_system**>(DYN_ADDR("ptr_input_system"));
	ix::global_vars = *reinterpret_cast<respawn::c_global_vars**>(DYN_ADDR("ptr_global_vars"));
	ix::local_player_ptr = reinterpret_cast<respawn::c_player**>(DYN_ADDR("ptr_local_player"));
	ix::engine_vgui = *reinterpret_cast<respawn::c_engine_vgui**>(DYN_ADDR("ptr_engine_vgui"));
	ix::game_movement = reinterpret_cast<respawn::c_game_movement*>(DYN_ADDR("ptr_game_movement"));
	ix::shared_client = *reinterpret_cast<respawn::c_shared_client**>(DYN_ADDR("ptr_client_mode"));
	ix::engine_client = reinterpret_cast<respawn::c_engine_client*>(DYN_ADDR("ptr_engine_client"));
	ix::engine_trace = *reinterpret_cast<respawn::c_engine_trace**>(DYN_ADDR("ptr_engine_trace"));
	ix::mat_system_surface = *reinterpret_cast<respawn::c_mat_system_surface**>(DYN_ADDR("ptr_mat_system_surface"));
	ix::view_render = *reinterpret_cast<respawn::c_view_render**>(DYN_ADDR("ptr_view_render"));
	ix::model_info = *reinterpret_cast<respawn::c_model_info**>(DYN_ADDR("ptr_model_info"));
	ix::net_chan = reinterpret_cast<respawn::c_net_chan**>(DYN_ADDR("ptr_net_chan"));
	ix::cvar_mgr = *reinterpret_cast<respawn::c_convar_mgr**>(DYN_ADDR("ptr_convar_mgr"));

	gv::engine_time = reinterpret_cast<float*>(DYN_ADDR("ptr_engine_time"));
	gv::input_disable = reinterpret_cast<bool*>(DYN_ADDR("cursor_enable"));
	gv::crosshair_state = reinterpret_cast<int*>(DYN_ADDR("crosshair_state"));

	return true;
}

bool r5::setup_convars()
{
	if ((cvar::time_scale = ix::cvar_mgr->find(_XS("host_timescale"))) == nullptr) return false;
	if ((cvar::third_person = ix::cvar_mgr->find(_XS("thirdperson_override"))) == nullptr) return false;
	if ((cvar::draw_view_model = ix::cvar_mgr->find(_XS("r_drawviewmodel"))) == nullptr) return false;
	if ((cvar::move_use_dt = ix::cvar_mgr->find(_XS("cl_move_use_dt"))) == nullptr) return false;

	return true;
}

void r5::initialize_full()
{
	if (!setup_interfaces())
		return report_failure();

	if (!g::p::netvar_mgr.initialize())
	{
		log_info_("failed to initialize networked variables.\n");
		return report_failure();
	}

	g::p::ent_cache.initialize();

	g::p::window_mgr.add<window::c_menu>({ 5, 250 }, { 450, 500 });
	g::p::window_mgr.add<window::c_info_panel>({ 250, 15 }, { 110, 5 });
	g::p::window_mgr.add<window::c_radar>({ 275, 500 }, { 250, 0 });
	g::p::window_mgr.add<window::c_spectator_list>({5, 750}, { 210, 5 });

	if (!setup_convars())
		return report_failure();

	if (!respawn::initialize())
		return report_failure();

	if (!hooks::initialize())
		return report_failure();

	log_info_("r5trainer has been initialized successfully.");

	g::p::context->m_initialized = true;
}

void r5::initialize()
{
	log_call();

	if (!setup_context())
		return report_failure();

	log_info("context is located at %p.", g::p::context);
	log_info("process_base= %p, process_len= %p.",
		mem::addr(g::p::context->m_process_info.base).Ptr(),
		mem::addr(g::p::context->m_process_info.length).Ptr()
	);

	g::return_address_gadget = shared::sig::FindInImageEx(
		g::p::context->m_process_info.base,
		_XS(".text"),
		_XSW(L"FF 26")
	);

	if (!g::return_address_gadget)
	{
		log_info_("failed to find ret_addr gadget inside target process.");
		return report_failure();
	}

	log_info_("the ret_addr gadget is located at %p.", 
		(void*)g::return_address_gadget);

	if (!setup_dyn_addrs() || !setup_interfaces_early())
		return report_failure();

	if (!hvi::query_info(hvi::HVI_AC_CONTEXT_EAC, (void**)&g::p::acc::eac))
	{
		log_info_("failed to query AC_CONTEXT for EAC.\n");
		return report_failure();
	}

	if (g::p::acc::eac->launched && g::p::acc::eac->internal_module.found)
	{
		log_info("internal EAC module base= %p, length=%p.\n",
			g::p::acc::eac->internal_module.base, g::p::acc::eac->internal_module.length);
	}
	else
	{
		log_info_("failed to find EAC internal module.");
		return report_failure();
	}

	if (!hooks::initialize_early())
	{
		log_info_("failed to initialize hooks (early).");
		return report_failure();
	}

	//
	// Wait for the game to initialize fully.
	// TODO: Find a better way to determine this desired state has been reached!
	//

	win_api::rebuilt::sleep( SECONDS_IN_MS(12) );

	initialize_full();
}

void r5::shutdown()
{
	log_call();

	//
	// ...
	// 
}

bool r5::is_within_eac(void* address)
{
	const auto base = mem::addr(address).Base();
	const auto ctx = g::p::acc::eac;

	if (!base || !ctx->launched || !ctx->internal_module.found)
		return false;

	const auto im_begin = ctx->internal_module.base;
	const auto im_end = mem::addr(im_begin).Add(ctx->internal_module.length).Base();

	return (base >= im_begin && base <= im_end);
}

void r5::think()
{
	const auto maintain_crosshair_visibility = []()
	{
		using ch_state = respawn::crosshair_states;
		*gv::crosshair_state = cfg::other_disable_default_crosshair ? ch_state::CROSSHAIR_STATE_HIDE_ALL : ch_state::CROSSHAIR_STATE_SHOW_ALL;
	};

	const auto maintain_view_model_visibility = []()
	{
		static bool previous{ };

		if (previous != cfg::other_disable_view_model)
		{
			cvar::draw_view_model->m_value_numeric = !cfg::other_disable_view_model;

			previous = cfg::other_disable_view_model;
		}
	};

	maintain_crosshair_visibility();
	maintain_view_model_visibility();
}

void r5::maintain_screen_size()
{
	int temp_w{ }, temp_h{ };
	ix::mat_system_surface->get_screen_size(temp_w, temp_h);

	if (temp_w != g::p::screen.w || temp_h != g::p::screen.h)
	{
		g::p::screen.w = temp_w;
		g::p::screen.h = temp_h;

		LOGGER_MSG_WARN(10.f, "game resolution has changed.");
	}
}

void r5::on_game_state_change(bool current, bool previous)
{
	LOGGER_MSG_INFO(5.f, "game state has changed from %i to %i.", previous, current);

	maintain_screen_size();

	if (current)
	{
		g::p::game_mode = respawn::get_game_mode();
	}

	memset(&g::p::cache, 0, sizeof(g::p::cache));
	g::p::ent_cache.reset();
}

bool build_in_game_context(respawn::user_cmd_t* cmd, respawn::c_net_chan** out_nc, respawn::c_player** out_lp, respawn::c_weapon** out_lw) noexcept
{
	if (cmd == nullptr || !cmd->command_number)
		return false;

	if (!ix::engine_client->is_in_game())
		return false;

	auto net_chan = respawn::net_chan();

	if (net_chan == nullptr)
		return false;

	if(out_nc) *out_nc = net_chan;

	auto local_player = respawn::get_local_player();

	if (local_player == nullptr || !respawn::is_player(local_player))
		return false;

	if (out_lp) *out_lp = local_player;

	if (respawn::is_alive(local_player) && !respawn::is_knocked(local_player))
	{
		auto local_weapon = local_player->get_active_weapon();

		if (local_weapon)
		{
			if (out_lw) *out_lw = local_weapon;

			g::is_playing = true;
		}
	}
	else
		g::is_playing = false;

	return true;
}

void r5::on_createmove(respawn::user_cmd_t* user_cmd)
{
	g::p::input_mgr.sync(&si::create_move);

	if (g::is_in_game_last != respawn::is_playing())
	{
		on_game_state_change(respawn::is_playing(), g::is_in_game_last);

		g::is_in_game_last = respawn::is_playing();
	}

	respawn::c_net_chan*	net_chan{ };
	respawn::c_player*		me{ };
	respawn::c_weapon*		my_wpn{ };

	g::is_in_game = build_in_game_context(user_cmd, &net_chan, &me, &my_wpn);

	auto is_choked = net_chan->stop_sending();

	net_chan->stop_sending() = false;

	math::Angle3 old_angles = user_cmd->viewangles;
	float old_move_forward = user_cmd->forwardmove, old_move_side = user_cmd->sidemove;

	respawn::on_update_cache(me, my_wpn, net_chan);
	
	f::aim.create_move(me, user_cmd);
	f::movement.create_move(me, user_cmd);
	f::glow.create_move(me, user_cmd);
	f::skin_changer.create_move(me, user_cmd);

	if (respawn::is_playing() && my_wpn)
	{
		if (cfg::aim_silent == 2 && f::aim.is_weapon_compatible(my_wpn))
		{
			bool wants_attack{ };

			if (!is_choked)
			{
				user_cmd->viewangles = old_angles;
				user_cmd->forwardmove = old_move_forward;
				user_cmd->sidemove = old_move_side;

				if (user_cmd->buttons & IN_ATTACK)
				{
					wants_attack = true;
					user_cmd->buttons &= ~IN_ATTACK;
				}
			}

			if ((user_cmd->buttons & IN_ATTACK || wants_attack) && my_wpn->can_fire(me))
				net_chan->stop_sending() = true;
		}

		if (cfg::other_bullet_time && (user_cmd->buttons & IN_ATTACK && !my_wpn->can_fire(me)))
			user_cmd->buttons &= ~IN_ATTACK;
	}
}

void r5::on_clmove()
{
	if (!respawn::is_playing())
		return;

	//
	// ...
	//

	think();
}

void r5::on_paint()
{
	g::p::input_mgr.update();

	respawn::start_drawing(ix::mat_system_surface);

	on_render();

	respawn::finish_drawing();
}

void r5::on_render()
{
	g::curtime = ix::global_vars->Curtime;

	g::p::input_mgr.sync(&si::paint);

	if (!g::p::screen.w && !g::p::screen.h)
		ix::mat_system_surface->get_screen_size(g::p::screen.w, g::p::screen.h);
	
	f::logger.paint();
	f::esp.paint();
	f::aim.paint();
	g::p::window_mgr.paint();
}

void r5::on_fsn(int stage)
{
	if (!respawn::is_playing())
		return;

	auto* const me = respawn::get_local_player();

	if (me == nullptr)
		return;

	f::skin_changer.frame_stage_notify(stage, me);
}

void r5::on_window_proc(UINT msg, WPARAM w_param, LPARAM l_param)
{
	g::p::input_mgr.window_proc(msg, w_param, l_param);
}
