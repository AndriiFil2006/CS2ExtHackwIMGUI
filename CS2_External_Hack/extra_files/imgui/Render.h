#pragma once
#include "imgui.h"

typedef struct
{
	ImU32 R;
	ImU32 G;
	ImU32 B;
}RGB;

ImU32 Color(RGB color, float alpha)
{
	return IM_COL32(color.R, color.G, color.B, alpha);
}

namespace Render
{
	void DrawRect(int x, int y, int w, int h, RGB color, float thickness, bool bFilled, float alpha)
	{
		if (!bFilled)
		{
			ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), Color(color, alpha), 0, 0, thickness);

		}
		else
		{
			ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), Color(color, alpha), thickness);
		}
	}

	void DrawHealth(int x, int y, int h, RGB color, float thickness, int health, float alpha)
	{
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x - thickness * 3, y + h), ImVec2(x - thickness * 2, y - h * health / 100 + h), Color(color, alpha), 0, 0);
	}

	void DrawLine(int x1, int y1, int x2, int y2, RGB color, float thickness, float alpha)
	{
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), Color(color, alpha), thickness);
	}

	void DrawCircle(float x, float y, float radius, RGB color, float thickness, bool bFilled, float alpha)
	{
		if (!bFilled)
		{
			ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), radius, Color(color, alpha), 0, thickness);

		}
		else
		{
			ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, Color(color, alpha), thickness);
		}
	}
}