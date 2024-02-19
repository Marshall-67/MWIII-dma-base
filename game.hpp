#pragma once
#include "../render/Vectors.h"
#include <array>


namespace drawing
{
	void DrawFilledRect(int x, int y, int w, int h, ImColor color)
	{
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImColor{ 255, 255, 255, 255 }, 0, 0);
	}

	class draw
	{


	public:
		static auto DrawNormalBox(int x, int y, int w, int h, int borderPx, ImColor color) -> void
		{
			DrawFilledRect(x + borderPx, y, w, borderPx, color); //top 
			DrawFilledRect(x + w - w + borderPx, y, w, borderPx, color); //top 
			DrawFilledRect(x, y, borderPx, h, color); //left 
			DrawFilledRect(x, y + h - h + borderPx * 2, borderPx, h, color); //left 
			DrawFilledRect(x + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
			DrawFilledRect(x + w - w + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
			DrawFilledRect(x + w + borderPx, y, borderPx, h, color);//right 
			DrawFilledRect(x + w + borderPx, y + h - h + borderPx * 2, borderPx, h, color);//right 
		}
		void draw_line(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness)
		{
			auto window = ImGui::GetBackgroundDrawList();;
			/*float a = (color >> 24) & 0xff;
			float r = (color >> 16) & 0xff;
			float g = (color >> 8) & 0xff;
			float b = (color) & 0xff;*/
			window->AddLine(from, to, color, thickness);
		}
		void draw_box(const float x, const float y, const float width, const float height, const uint32_t color, float thickness)
		{
			draw_line(ImVec2(x, y), ImVec2(x + width, y), color, thickness);
			draw_line(ImVec2(x, y), ImVec2(x, y + height), color, thickness);
			draw_line(ImVec2(x, y + height), ImVec2(x + width, y + height), color, thickness);
			draw_line(ImVec2(x + width, y), ImVec2(x + width, y + height), color, thickness);
		}

	};
}static drawing::draw* draw = new drawing::draw();

class entity {
public:
	uintptr_t
		actor,
		skeletal_mesh,
		root_component,
		player_state;
	int
		team_index;
};

std::vector<entity> entity_list;
std::vector<entity> temporary_entity_list;

enum GAME_INFORMATION : int {
	GAME_THREAD_FAILED = 0,
	GAME_SETUP_SUCCESSFUL = 1
};
struct bounds_t
{
	float left, right, top, bottom;
};

ImColor visible_color = ImColor(255, 0, 0);
bool in_lobby = false;

namespace game {
	class c_game {
	public:

		auto Setup() -> GAME_INFORMATION {

			//std::thread(game::c_game::CacheData).detach();
			std::cout << "Process Enivroment Block :";
			return GAME_INFORMATION::GAME_SETUP_SUCCESSFUL;
		}

		static auto CacheData() -> void {

			for (;;)
			{
				// incase you want to use caching
			}

		}
		auto ActorLoop() -> void
		{
			globals->is_in_game = Utilities->is_user_in_game();
			if (!globals->is_in_game) return;

			globals->player_count = Utilities->player_count();
			pointer->client_info = decrypt->Client_Information();
			pointer->client_info_base = decrypt->Client_Base(pointer->client_info);
			auto ref_def_pointer = decrypt_refdef->retrieve_ref_def();
			decrypt_refdef->ref_def_nn = DMA::Read<ref_def_t>(ref_def_pointer, sizeof(ref_def_t));
			player Local_Player(pointer->client_info_base + (Utilities->local_player_index() * offset::player_size));
			auto Local_Player_Position = Local_Player.get_position();
			auto Local_Player_Team = Local_Player.team_id();
			auto Entity_Bone_Base = decrypt->bone_base();
			auto Entity_Bone_Position = Utilities->retrieve_bone_position_vec(pointer->client_info);

			for (int i = 0; i < globals->player_count; i++)
			{
				player entity(pointer->client_info_base + (i * offset::player_size));

				fvector2d player_screen;
				fvector2d entity_head;
				fvector position = entity.get_position();
				auto Bone_Pointer_Index = decrypt->bone_index(i);
				auto bone_pointer = entity.bone_pointer(Entity_Bone_Base, Bone_Pointer_Index);

				auto corner_height = abs(entity_head.y - player_screen.y);
				auto corner_width = corner_height * 0.65;

				fvector2d screen_middle = { (float)decrypt_refdef->ref_def_nn.width / 2, (float)decrypt_refdef->ref_def_nn.height / 2 };

				if (!entity.is_player_valid() || entity.get_name_entry(i).health < 0)
					continue;
				if (entity.team_id() == Local_Player_Team)
					continue;
				if (!bone_pointer)
					continue;

				if (Utilities->w2s(position, player_screen)) // Convert base position to screen coordinates
				{
					NameEntry player_name_entry = entity.get_name_entry(i);

					if (player_name_entry.name == NULL)
						continue;

					auto entity_distance = Utilities->units_to_m(Local_Player_Position.distance_to(position));

					if (entity_distance < globals->maximum_distance)
					{

						if (globals->box) {
							fvector2d head_screen;
							auto head_position = Utilities->retrieve_bone_position(bone_pointer, Entity_Bone_Position, 7); 
							if (Utilities->w2s(head_position, head_screen)) {
								auto player_height = abs(head_screen.y - player_screen.y);
								auto player_width = player_height * 0.65;
								auto box_top_left_x = player_screen.x - (player_width / 2);
								auto box_top_left_y = player_screen.y - player_height; 
								draw->draw_box(box_top_left_x, box_top_left_y, player_width, player_height, ImColor(255, 255, 255, 255), 1.0f);
								auto box_bottom_edge_y = box_top_left_y + player_height;

								if (globals->username) {
									ImGui::GetBackgroundDrawList()->AddText(
										ImVec2(player_screen.x - (ImGui::CalcTextSize(player_name_entry.name).x / 2), box_top_left_y - 20), // 20 pixels above the box for padding
										ImColor(255, 255, 0, 255),
										player_name_entry.name);
								}
								if (globals->distance) {
									ImGui::GetBackgroundDrawList()->AddText(
										ImVec2(player_screen.x - (ImGui::CalcTextSize(Utilities->ConvertDistanceToString((int)entity_distance).c_str()).x / 2), box_bottom_edge_y + 5), // 5 pixels below the box for padding
										ImColor(255, 255, 255, 255),
										Utilities->ConvertDistanceToString((int)entity_distance).c_str());
								}
							}
						}


						ImColor ESPColor = ImColor(255, 0, 0, 255);
						if (globals->line_to_center) {
							ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screen_middle.x, screen_middle.y), ImVec2(player_screen.x, player_screen.y), ESPColor, 2.0f);
						}
						if (globals->line_to_bottom) {
							ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screen_middle.x, decrypt_refdef->ref_def_nn.height), ImVec2(player_screen.x, player_screen.y), ESPColor, 2.0f);
						}

						if (globals->aimbot) {

							fvector2d select_bone;
							if (entity.team_id() != Local_Player_Team)
								continue;
							if (Utilities->w2s(Utilities->retrieve_bone_position(bone_pointer, Entity_Bone_Position, 7), select_bone)) {
								if (globals->aimbot_key) {

									if (select_bone.x > 1 && select_bone.y > 1)
									{
										Utilities->cursor_to(select_bone.x, select_bone.y);
									}
								}
							}
						}
					}
				}
				ImGui::GetForegroundDrawList()->AddText(ImVec2(100, 100), IM_COL32(255, 255, 0, 255), "ActorLoop completed");

			}
		}
	
	};
} static game::c_game* Game = new game::c_game();