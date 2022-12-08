#include <include.h>

void dyn_addr::add_dynamic_address_code(const char* name, const wchar_t* pattern, uint32_t offset)
{
	auto entry = entry_t{};

	memset(&entry, 0, sizeof(entry));

	entry.type = entry_t::DYN_ADDR_TYPE_CODE;
	entry.offset = offset;

	memcpy(entry.name, name, stl::strlen(name) * sizeof(char));
	memcpy(entry.pattern, pattern, stl::wcslen(pattern) * sizeof(wchar_t));

	_list.insert(entry);
}

void dyn_addr::add_dynamic_address_ptr(const char* name, const wchar_t* pattern,
	uint32_t offset, uint32_t offset_to_instr, uint32_t length_of_instr)
{
	auto entry = entry_t{};

	memset(&entry, 0, sizeof(entry));

	entry.type = entry_t::DYN_ADDR_TYPE_PTR;
	entry.offset = offset;
	entry.ptr = { offset_to_instr, length_of_instr };

	memcpy(entry.name, name, stl::strlen(name) * sizeof(char));
	memcpy(entry.pattern, pattern, stl::wcslen(pattern) * sizeof(wchar_t));

	_list.insert(entry);
}

bool dyn_addr::grab_all()
{
	auto handled = true;

	for (auto* entry_info = _list.first_entry();
		entry_info != nullptr;
		entry_info = entry_info->m_next)
	{
		if (entry_info->m_is_head)
			continue;

		auto& entry = entry_info->m_data;

		switch (entry.type)
		{
		case entry_t::DYN_ADDR_TYPE_CODE:
		{
			entry.result = FIND_SIG(
				g::p::context->m_process_info.base,
				g::p::context->m_process_info.length,
				entry.pattern, entry.offset);

			break;
		}
		case entry_t::DYN_ADDR_TYPE_PTR:
		{
			entry.result = FIND_PTR(
				g::p::context->m_process_info.base,
				g::p::context->m_process_info.length,
				entry.pattern,
				entry.offset,
				entry.ptr.offset_to_instr,
				entry.ptr.length_of_instr
			);

			break;
		}
		default:
			handled = false;
			break;
		}

		if (handled)
		{
			log_info("found DA '%s' -> %p",
				entry.name, mem::addr(entry.result).Ptr());
		}
		else
		{
			return false;
		}
	}

	return true;
}

uintptr_t dyn_addr::get_dynamic_address(const char* name)
{
	uintptr_t result = 0;

	for (auto* entry_info = _list.first_entry();
		entry_info != nullptr;
		entry_info = entry_info->m_next)
	{
		if (entry_info->m_is_head)
			continue;

		auto& entry = entry_info->m_data;

		if (!stl::strcmp(entry.name, name))
		{
			result = entry.result;
			break;
		}
	}

	return result;
}
