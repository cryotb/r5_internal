#include <include.h>

using namespace features;

void c_logger::paint()
{
	draw_messages();
}

void c_logger::draw_messages()
{
	float push_y = 0.0f;
	float start_x = 2.0f, start_y = 2.0f;

	for (auto i = 0ul; i < m_fields.count(); i++)
	{
		field_t* entry = m_fields[i];

		if (entry == nullptr)
			continue;

		bool requires_removal = (entry->m_time_registered && math::fabsf(entry->m_time_registered - *gv::engine_time) > entry->m_duration);

		size_t text_height = 14;
		size_t text_width = g::p::render.get_text_width(entry->m_text, text_height);

		g::p::render.draw_rect_filled(
			respawn::color_t{25, 25, 25}, start_x, start_y + push_y, 
			start_x + 1 + text_width, 
			start_y + push_y + text_height + 1.0f
		);

		g::p::render.draw_text(entry->m_color, 2, static_cast<int>(text_height), start_x + 1, start_y + push_y, entry->m_text);

		if (requires_removal)
		{
			m_fields.remove(*entry);
			break;
		}

		push_y += text_height;
	}
}

void c_logger::add_message(float duration, respawn::color_t color, const char* fmt, ...)
{
	stl::my_lock_guard _(&m_lock);

	if (!duration || !fmt)
		return;

	va_list list{ };
	field_t entry{ };

	memset(&entry, 0, sizeof(entry));

	if( stl::strlen(fmt) >= (sizeof(entry.m_text) - 1) )
		return;

	entry.m_duration = duration;
	entry.m_color = color;
	entry.m_time_registered = *gv::engine_time;

	va_start(list, fmt);
	LI_FN(wvsprintfA).cached()(entry.m_text, fmt, list);
	va_end(list);

	m_fields.insert(entry);
}
