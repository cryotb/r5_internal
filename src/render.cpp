#include <include.h>

size_t c_render::get_text_width(const char* text, size_t font_size)
{
	const auto count = stl::strlen(text);

	return (count * font_size) / 2;
}

void c_render::draw_rect(respawn::color_t color, int x1, int y1, int x2, int y2, int opacity)
{
	ix::mat_system_surface->draw_set_color(color.r, color.g, color.b, opacity);
	ix::mat_system_surface->draw_outlined_rect(x1, y1, x2, y2);
}

void c_render::draw_rect_filled(respawn::color_t color, int x1, int y1, int x2, int y2, int opacity)
{
	ix::mat_system_surface->draw_set_color(color.r, color.g, color.b, opacity);
	ix::mat_system_surface->draw_filled_rect(x1, y1, x2, y2);
}

void c_render::draw_line(respawn::color_t color, int x1, int y1, int x2, int y2, int opacity)
{
	ix::mat_system_surface->draw_set_color(color.r, color.g, color.b, opacity);
	ix::mat_system_surface->draw_line(x1, y1, x2, y2);
}

void c_render::draw_rect_bordered(respawn::color_t sColor, float flX0, float flY0, float flX1, float flY1, int opacity)
{
	draw_rect({ 5, 5, 5 }, flX0 - 1.0f, flY0 - 1.0f, flX1 + 1.0f, flY1 + 1.0f, opacity);
	draw_rect(sColor, flX0, flY0, flX1, flY1, opacity);
	draw_rect({ 5, 5, 5 }, flX0 + 1.0f, flY0 + 1.0f, flX1 - 1.0f, flY1 - 1.0f, opacity);
}

void c_render::draw_circle(respawn::color_t color, int x, int y, int radius, int segments, int opacity)
{
	ix::mat_system_surface->draw_set_color(color.r, color.g, color.b, opacity);
	ix::mat_system_surface->draw_outlined_circle(x, y, radius, segments);
}

void c_render::draw_text(respawn::color_t color, ulong font, int size, int x, int y, const char* text)
{
	ix::mat_system_surface->draw_text(font, size, x, y, color.r, color.g, color.b, 255, 0, text);
}

void c_render::draw_text_ex(respawn::color_t color, ulong font, int size, int x, int y, const char* text, ...)
{
	va_list list;
	char buffer[512];

	va_start(list, text);
	LI_FN(wvsprintfA).cached()(buffer, text, list);
	va_end(list);

	draw_text(color, font, size, x, y, buffer);
}

void c_render::draw_text_centered(respawn::color_t color, ulong font, int size, int x, int y, const char* text, ...)
{
	const auto width = get_text_width(text, size);

	va_list list;
	char buffer[512];

	va_start(list, text);
	LI_FN(wvsprintfA).cached()(buffer, text, list);
	va_end(list);

	return draw_text_ex(color, font, size, x - (width / 2.f), y, buffer);
}
