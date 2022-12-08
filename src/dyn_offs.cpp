#include <include.h>

namespace dyn_offs
{
	auto retrieve(LPCWSTR sig, std::initializer_list<uint8_t> pattern, u_long offset, size_t opcode_len) noexcept
	{
		uintptr_t result{ };

		uintptr_t game_base = g::p::context->m_process_info.base;
		uintptr_t game_length = g::p::context->m_process_info.length;

		if (game_base && game_length)
		{
			uintptr_t address = FIND_SIG(game_base, game_length, sig, offset);

			if (address)
				result = shared::sig::find_offset(address, pattern.begin(), pattern.size(), offset, opcode_len);
		}

		return result;
	}
}

bool dyn_offs::initialize()
{
	_::player::last_visible_time = retrieve(_XSW(L"41 B1 09 48 8D 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 85 C0 74 08"), { 0x8B, 0x8B }, 0, 2);
	_::player::view_offset = retrieve(_XSW(L"48 83 EC 38 8B 05 ? ? ? ? 0F 29 74 24 ? 83 F8 FF 74"), { 0xF3, 0x41, 0x0F, 0x10, 0x80 }, 0, 5);
	_::player::eye_angles = retrieve(_XSW(L"75 08 4C 8B C0 0F 28 CC EB 33"), { 0xF3, 0x0F, 0x11, 0x81 }, 0, 4);

	return true;
}
