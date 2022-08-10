#pragma once

class WDraw {
	Color_t CurrentColor = { 255, 255, 255, 255 };
	int CurrentFont = FONT_MENU;

public:
	// TODO: try default args using 'int align = 2'
	void Text(int x, int y, const char* text)
	{
		g_Draw.String(CurrentFont, x, y, CurrentColor, ALIGN_DEFAULT, "%s", text);
	}

	void Line(int x, int y, int x1, int y1)
	{
		g_Draw.Line(x, y, x1, y1, CurrentColor);
	}

	void Rect(int x, int y, int w, int h)
	{
		g_Draw.Rect(x, y, w, h, CurrentColor);
	}

	void OutlinedRect(int x, int y, int w, int h)
	{
		g_Draw.OutlinedRect(x, y, w, h, CurrentColor);
	}

	void FilledCircle(int x, int y, int radius, int segments)
	{
		g_Draw.FilledCircle(x, y, radius, segments, CurrentColor);
	}

	void SetColor(byte r, byte g, byte b, byte a)
	{
		CurrentColor.r = r;
		CurrentColor.g = g;
		CurrentColor.b = b;
		CurrentColor.a = a;
	}

	void SetFont(int font)
	{
		CurrentFont = font;
	}
};