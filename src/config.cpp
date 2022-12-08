#include <include.h>

void c_config::initialize()
{
	m_parser = new (stl::mem_alloc<void*>(sizeof(c_json))) c_json();

	if (m_parser == nullptr)
		return;

	if (!get_path(m_path, sizeof(m_path)))
		return;

	m_ready = true;
}

void c_config::shutdown()
{
	if (!m_ready)
		return;

	if (m_parser)
		stl::mem_free(m_parser);
}

bool c_config::validate(void* file_ptr)
{
	if (m_parser->parse(reinterpret_cast<LPCSTR>(file_ptr)))
	{
		m_valid = true;
		return true;
	}

	log_info_("config file is invalid.");

	m_valid = false;
	return false;
}

void c_config::on_start()
{
	if (util::file_exists_wide(m_path))
	{
		//
		// We found a configuration file, process it.
		//

		void* file_ptr = util::get_file_ptr(m_path);

		if (file_ptr)
		{
			validate(file_ptr);

			stl::mem_free(file_ptr);
		}
	}
	else
	{
		log_info_("config file could not be found.");

		//
		// No configuration file was found.
		//

		//
		// TODO: Implement.
		//

		__noop();
	}
}

bool c_config::get_path(wchar_t* buffer, size_t length)
{
	if (buffer && length)
	{
		char path_desktop[MAX_PATH];
		char path_result[MAX_PATH];

		ZERO_MEM(path_desktop, sizeof(path_desktop));

		if (util::get_desktop_directory(path_desktop, sizeof(path_desktop)))
		{
			util::format_string(
				path_result, length, 
				_XS("\\??\\%s\\%s"), 
				path_desktop, 
				_XS("hs.cfg")
			);

			stl::mbstowcs(buffer, path_result, 0, length);

			return true;
		}
	}

	return false;
}
