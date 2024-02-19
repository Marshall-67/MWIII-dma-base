#pragma once
#include "../driver/dma.h"

namespace sdk {
    uintptr_t module_base = 0;
    uintptr_t peb = 0;
}

struct NameEntry
{
    uint32_t index;
    char name[36];
    uint8_t pad[92];
    int32_t health;
    uint8_t pad2[70];
};

enum bone : int
{
    bone_pos_helmet = 8,

    bone_pos_head = 7,
    bone_pos_neck = 6,
    bone_pos_chest = 5,
    bone_pos_mid = 4,
    bone_pos_tummy = 3,
    bone_pos_pelvis = 2,

    bone_pos_right_foot_1 = 21,
    bone_pos_right_foot_2 = 22,
    bone_pos_right_foot_3 = 23,
    bone_pos_right_foot_4 = 24,

    bone_pos_left_foot_1 = 17,
    bone_pos_left_foot_2 = 18,
    bone_pos_left_foot_3 = 19,
    bone_pos_left_foot_4 = 20,

    bone_pos_left_hand_1 = 13,
    bone_pos_left_hand_2 = 14,
    bone_pos_left_hand_3 = 15,
    bone_pos_left_hand_4 = 16,

    bone_pos_right_hand_1 = 9,
    bone_pos_right_hand_2 = 10,
    bone_pos_right_hand_3 = 11,
    bone_pos_right_hand_4 = 12
};

enum offset {
    refdef = 0x1069A1C0,
    game_mode = 0xD1EDF60,
    local_index = 0x522F8,
    local_index_pos = 0x02F0,
    player_size = 0x13DA8,
    name_arrary = 0x1069A188,
    name_arrary_pos = 0x2C80,
    name_size = 0xC0,
    player_valid = 0x18ED,
    player_team = 0x1298,
    player_pos = 0x1B10,
    index_struct_size = 0x188,
    camera_base = 0x10F0F020,
    camera_position = 0x204,
    bone_base_position = 0x90F48,
};

class Pointers {
public:
    uintptr_t
        client_info,
        client_info_base,
        bone_base,
        bone_index,
        currentvisoffset,
        last_visible_offset,
        player_controller,
        root_component,
        skeletal_mesh,
        persistent_level,
        player_array,
        current_weapon,
        current_vehicle;
    int
        player_array_size,
        team_index;
    fvector
        relative_location;
}; static Pointers* pointer = new Pointers();

struct camera {
    fvector location;
    fvector rotation;
    float field_of_view;
    char Useless[0x18];
};
class player {
public:
    player(uintptr_t address) {
        this->address = address;
    }

    uintptr_t address{};

    int id{};

    bool is_player_valid();

    bool is_dead();


    int team_id();

    fvector get_position();
    uint32_t get_index();
    NameEntry get_name_entry(uint32_t index);
    uintptr_t bone_pointer(uint64_t bone_base, uint64_t bone_index);
};
struct ref_def_view {
    fvector2d tan_half_fov;
    char pad[0xC];
    fvector axis[3];
};

struct ref_def_t {
    int x;
    int y;
    int width;
    int height;
    ref_def_view view;
};
class refdef_class {

public:
    struct key {
        int ref0;
        int ref1;
        int ref2;
    };

    auto retrieve_ref_def() -> uintptr_t {
        key encrypted = DMA::Read<key>(sdk::module_base + offset::refdef, sizeof(key));

        DWORD lowerref = encrypted.ref0 ^ (encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef)) * ((encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef)) + 2);
        DWORD upperref = encrypted.ref1 ^ (encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef + 0x4)) * ((encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef + 0x4)) + 2);

        return (uint64_t)upperref << 32 | lowerref;
    }
    ref_def_t ref_def_nn;
};
static refdef_class* decrypt_refdef = new refdef_class();
namespace utilities {


    class c_utilities {
    public:


        auto InScreen(fvector2d screen_position) -> bool {

            if (screen_position.x > 0 && screen_position.x < globals->width && screen_position.y > 0 && screen_position.y < globals->height)
                return true;
            else
                return false;

        }

        auto platform_check() -> int
        {
            globals->platform_battlenet = true;
            return 0;
        }

        auto IsShootable(fvector lur, fvector wl) -> bool {

            if (lur.x >= wl.x - 20 && lur.x <= wl.x + 20 && lur.y >= wl.y - 20 && lur.y <= wl.y + 20 && lur.z >= wl.z - 30 && lur.z <= wl.z + 30)
                return true;
            else
                return false;

        }

        auto is_user_in_game() -> bool {
            auto ingame = DMA::Read<int>(sdk::module_base + offset::game_mode, sizeof(int)) > 1;
            return ingame;
        }
        auto retrieve_name_list() -> uint64_t {
            auto ptr = DMA::Read<uint64_t>(sdk::module_base + offset::name_arrary, sizeof(uint64_t));
            return ptr + offset::name_arrary_pos;
        }
        auto player_count() -> int {
            auto playercount = DMA::Read<int>(sdk::module_base + offset::game_mode, sizeof(int));
            return playercount;
        }
        auto local_player_index() -> int {
            auto index = DMA::Read<uintptr_t>(pointer->client_info + offset::local_index, sizeof(uintptr_t));
            auto read_return = DMA::Read<int>(index + offset::local_index_pos, sizeof(int));
            return read_return;
        }

        auto retrieve_camera_posistion() -> fvector {
            auto player_camera = DMA::Read<uintptr_t>(sdk::module_base + offset::camera_base, sizeof(uintptr_t));
            if (!player_camera)
                return {}; // Return an empty fvector if player_camera is null

            auto received = DMA::Read<fvector>(player_camera + offset::camera_position, sizeof(fvector));
            return received;
        }
        auto get_angles() -> fvector2d {
            auto camera_angle = DMA::Read<uintptr_t>(sdk::module_base + offset::camera_base, sizeof(uintptr_t));
            if (!camera_angle)
                return {}; // Return an empty fvector2d if camera_angle is null

            auto received = DMA::Read<fvector2d>(camera_angle + offset::camera_position + 0xC, sizeof(fvector2d));
            return received;
        }

        auto retrieve_bone_position(const uintptr_t pointer, const fvector& bone_position, const int Bone) -> fvector
        {
            fvector position = DMA::Read<fvector>(pointer + ((uint64_t)Bone * 0x20) + 0x10, sizeof(fvector));
            position.x += bone_position.x;
            position.y += bone_position.y;
            position.z += bone_position.z;
            return position;
        }

        auto retrieve_bone_position_vec(const uintptr_t Client_Information) -> fvector {
            auto information = DMA::Read<fvector>(Client_Information + offset::bone_base_position, sizeof(fvector));
            return information;
        }
        auto world_to_screen(fvector world_location, fvector2d& out, fvector camera_pos, int screen_width, int screen_height, fvector2d fov, fvector matricies[3]) -> bool
        {
            auto local = world_location - camera_pos;
            auto trans = fvector{
                local.Dot(matricies[1]),
                local.Dot(matricies[2]),
                local.Dot(matricies[0])
            };

            if (trans.z < 0.01f) {
                return false;
            }

            out.x = ((float)screen_width / 2.0) * (1.0 - (trans.x / fov.x / trans.z));
            out.y = ((float)screen_height / 2.0) * (1.0 - (trans.y / fov.y / trans.z));

            if (out.x < 1 || out.y < 1 || (out.x > decrypt_refdef->ref_def_nn.width) || (out.y > decrypt_refdef->ref_def_nn.height)) {
                return false;
            }

            return true;
        }

        auto w2s(fvector world_position, fvector2d& screen_position) -> bool
        {
            return world_to_screen(world_position, screen_position, retrieve_camera_posistion(), decrypt_refdef->ref_def_nn.width, decrypt_refdef->ref_def_nn.height, decrypt_refdef->ref_def_nn.view.tan_half_fov, decrypt_refdef->ref_def_nn.view.axis);
        }

        auto units_to_m(float units) -> float {
            return units * 0.0254;
        }

        auto ConvertDistanceToString(float dist) -> std::string
        {
            std::stringstream strs;
            strs << dist;
            std::string temp_str = strs.str();
            const char* text = (const char*)temp_str.c_str();
            return (std::string)text + "m";
        }

        auto cursor_to(float x, float y) -> void {
            fvector center(globals->width / 2, globals->height / 2, 0);
            fvector target(0, 0, 0);

            if (x != 0)
            {
                if (x > center.x)
                {
                    target.x = -(center.x - x);
                    target.x /= (globals->smooth + 3);
                    if (target.x + center.x > center.x * 2)
                        target.x = 0;
                }

                if (x < center.x)
                {
                    target.x = x - center.x;
                    target.x /= (globals->smooth + 3);
                    if (target.x + center.x < 0)
                        target.x = 0;
                }
            }
            if (y != 0)
            {
                if (y > center.y)
                {
                    target.y = -(center.y - y);
                    target.y /= (globals->smooth + 3);
                    if (target.y + center.y > center.y * 2)
                        target.y = 0;
                }

                if (y < center.y)
                {
                    target.y = y - center.y;
                    target.y /= (globals->smooth + 3);
                    if (target.y + center.y < 0)
                        target.y = 0;
                }
            }

            auto offset_x = static_cast<float>(rand() % 5) / 10.0f - 1.5f;
            auto offset_y = static_cast<float>(rand() % 5) / 10.0f - 1.5f;

            Inject->set_cursor_position(target.x + offset_x, target.y + offset_y);
        }

        static float powf_(float _X, float _Y) {
            return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
        }
        static float sqrtf_(float _X) {
            return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
        }
        static double GetCrossDistance(double x1, double y1, double x2, double y2) {
            return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
        }

    };
} static utilities::c_utilities* Utilities = new utilities::c_utilities();


auto player::get_index() -> uint32_t
{
    auto index = (this->address - sdk::module_base) / offset::player_size;
    return index;
}
auto player::get_name_entry(uint32_t p_index) -> NameEntry {
    auto name = DMA::Read<NameEntry>(Utilities->retrieve_name_list() + (p_index * offset::name_size), sizeof(NameEntry));
    return name;
}
auto player::is_player_valid() -> bool {
    auto response = DMA::Read<bool>(this->address + offset::player_valid, sizeof(bool));
    return response;
}
auto player::team_id() -> int {
    auto team = DMA::Read<int>(this->address + offset::player_team, sizeof(int));
    return team;
}
auto player::get_position() -> fvector {
    auto local_position_ptr = DMA::Read<uintptr_t>(this->address + offset::player_pos, sizeof(uintptr_t));
    if (!local_position_ptr)
        return {}; 

    auto final_pos = DMA::Read<fvector>(local_position_ptr + 0x48, sizeof(fvector));
    return final_pos;
}
auto player::bone_pointer(uint64_t base, uint64_t index) -> uintptr_t {
    auto bone_address = DMA::Read<uintptr_t>(base + (index * offset::index_struct_size) + 0xD8, sizeof(uintptr_t));
    return bone_address;
}
class decryption
{
public:

    auto Client_Information() -> uint64_t {
        if (globals->platform_steam) {
            uint64_t mb = sdk::module_base; 
            uint64_t peb = sdk::peb; 
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

            rbx = request->read<uintptr_t>(sdk::module_base + 0x11A71C48);
            if (!rbx)
                return rbx;

            rcx = sdk::peb;              //mov rcx, gs:[rax]
            rbx -= rcx;             //sub rbx, rcx
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rdx = 0x693186CC4D1F9DB;                //mov rdx, 0x693186CC4D1F9DB
            rbx *= rdx;             //imul rbx, rdx
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rdx = 0x57548B4D82F080EE;               //mov rdx, 0x57548B4D82F080EE
            rax ^= request->read<uintptr_t>(DMA::BaseAddress + 0x91230E6);             //xor rax, [0x000000000113FF61]
            rbx += rdx;             //add rbx, rdx
            rax = _byteswap_uint64(rax);            //bswap rax
            rbx *= request->read<uintptr_t>(rax + 0x13);             //imul rbx, [rax+0x13]
            rax = rbx;              //mov rax, rbx
            rbx >>= 0x20;           //shr rbx, 0x20
            rbx ^= rax;             //xor rbx, rax
            rbx += rcx;             //add rbx, rcx
            return rbx;
        }
        else if (globals->platform_battlenet)
        {
            const uint64_t mb = sdk::module_base;
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rbx = DMA::Read<uintptr_t>(sdk::module_base + 0x104C2408, sizeof(uintptr_t));
            if (!rbx)
                return rbx;
            rcx = ~sdk::peb;              //mov rcx, gs:[rax]
            rdx = sdk::module_base;              //lea rdx, [0xFFFFFFFFFE31F16B]
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rbx -= rdx;             //sub rbx, rdx
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F00C3, sizeof(uintptr_t));              //xor rax, [0x000000000540F21A]
            rax = ~rax;             //not rax
            rbx *= DMA::Read<uintptr_t>(rax + 0x13, sizeof(uintptr_t));            //imul rbx, [rax+0x13]
            rax = 0xBD20874E271585EB;               //mov rax, 0xBD20874E271585EB
            rbx *= rax;             //imul rbx, rax
            rax = 0x35860EA22C1F2550;               //mov rax, 0x35860EA22C1F2550
            rbx -= rax;             //sub rbx, rax
            rax = sdk::module_base + 0x9F80;             //lea rax, [0xFFFFFFFFFE3290AD]
            rax += rcx;             //add rax, rcx
            rbx ^= rax;             //xor rbx, rax
            rax = rbx;              //mov rax, rbx
            rax >>= 0x28;           //shr rax, 0x28
            rbx ^= rax;             //xor rbx, rax
            return rbx;
        }
    }
    auto Client_Base(uintptr_t client_info) -> uintptr_t {
        if (globals->platform_steam) {
            const uint64_t mb = sdk::module_base; // Directly use DMA::BaseAddress
            const uint64_t peb = sdk::peb; // Directly use DMA::PebAddress
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rax = request->read<uintptr_t>(client_info + 0x171a10);
            if (!rax)
                return rax;
            r11 = ~sdk::peb;              //mov r11, gs:[rcx]
            rcx = r11;              //mov rcx, r11
            //failed to translate: mov [rsp+0x3D0], r13
            rcx = _rotl64(rcx, 0x21);               //rol rcx, 0x21
            rcx &= 0xF;
            switch (rcx) {
            case 0:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);               //mov r9, [0x00000000063A9041]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD285F20]
                rcx = 0x9141C45BFD5B39F7;               //mov rcx, 0x9141C45BFD5B39F7
                rax ^= rcx;             //xor rax, rcx
                rcx = rax + rbx * 1;            //lea rcx, [rax+rbx*1]
                rax = 0xF605A67470E7C53D;               //mov rax, 0xF605A67470E7C53D
                rcx *= rax;             //imul rcx, rax
                rax = r11;              //mov rax, r11
                uintptr_t RSP_0x50;
                RSP_0x50 = DMA::BaseAddress + 0x6E33AF72;                 //lea rcx, [0x000000006B5C0E9E] : RSP+0x50
                rax ^= RSP_0x50;                //xor rax, [rsp+0x50]
                rcx -= rax;             //sub rcx, rax
                rax = r11 + 0xffffffffd10685d8;                 //lea rax, [r11-0x2EF97A28]
                rcx -= rbx;             //sub rcx, rbx
                rax += rcx;             //add rax, rcx
                rcx = DMA::BaseAddress + 0x526CB4F4;              //lea rcx, [0x000000004F951347]
                rcx = ~rcx;             //not rcx
                rcx -= r11;             //sub rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1A;           //shr rcx, 0x1A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x34;           //shr rcx, 0x34
                rax ^= rcx;             //xor rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r9;              //xor rcx, r9
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                return rax;
            }
            case 1:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A8BA8]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD285A93]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x24;           //shr rcx, 0x24
                rax ^= rcx;             //xor rax, rcx
                rcx = 0xE570A6F93EC9464F;               //mov rcx, 0xE570A6F93EC9464F
                rax *= rcx;             //imul rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x24;           //shr rcx, 0x24
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x11;           //shr rcx, 0x11
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x22;           //shr rcx, 0x22
                rax ^= rcx;             //xor rax, rcx
                rax -= rbx;             //sub rax, rbx
                rax += r11;             //add rax, r11
                rcx = 0x14F095F380F9EB43;               //mov rcx, 0x14F095F380F9EB43
                rax += rcx;             //add rax, rcx
                return rax;
            }
            case 2:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A86F7]
                rcx = 0x8ADB88DACDCF2087;               //mov rcx, 0x8ADB88DACDCF2087
                rax *= rcx;             //imul rax, rcx
                rcx = 0x7962CBE13BD24CEA;               //mov rcx, 0x7962CBE13BD24CEA
                rax += rcx;             //add rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x10;           //shr rcx, 0x10
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x20;           //shr rcx, 0x20
                rax ^= rcx;             //xor rax, rcx
                rax += r11;             //add rax, r11
                rax += r11;             //add rax, r11
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rax -= r11;             //sub rax, r11
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1D;           //shr rcx, 0x1D
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x3A;           //shr rcx, 0x3A
                rax ^= rcx;             //xor rax, rcx
                return rax;
            }
            case 3:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A8317]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD285202]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x25;           //shr rcx, 0x25
                rcx ^= rax;             //xor rcx, rax
                rax = 0x99B04B837FD2242B;               //mov rax, 0x99B04B837FD2242B
                rax += rcx;             //add rax, rcx
                rax += rbx;             //add rax, rbx
                rax ^= r11;             //xor rax, r11
                rax -= rbx;             //sub rax, rbx
                rcx = DMA::BaseAddress + 0x11751E8B;              //lea rcx, [0x000000000E9D6CF9]
                rcx = ~rcx;             //not rcx
                rcx -= r11;             //sub rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = 0xDA47FAB853EFDBF7;               //mov rcx, 0xDA47FAB853EFDBF7
                rax *= rcx;             //imul rax, rcx
                return rax;
            }
            case 4:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A7E5C]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD284D3B]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x2;            //shr rcx, 0x02
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x4;            //shr rcx, 0x04
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x8;            //shr rcx, 0x08
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x10;           //shr rcx, 0x10
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x20;           //shr rcx, 0x20
                rax ^= rcx;             //xor rax, rcx
                rcx = 0x718CFE6D52D76081;               //mov rcx, 0x718CFE6D52D76081
                rax *= rcx;             //imul rax, rcx
                rcx = r11;              //mov rcx, r11
                rcx ^= rax;             //xor rcx, rax
                rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
                rax = DMA::BaseAddress + 0x746D9936;              //lea rax, [0x000000007195E3F7]
                rcx ^= rax;             //xor rcx, rax
                rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
                rdx ^= r10;             //xor rdx, r10
                rax = 0x6A0BA8494B6820F5;               //mov rax, 0x6A0BA8494B6820F5
                rdx = ~rdx;             //not rdx
                rcx *= request->read<uintptr_t>(rdx + 0x19);             //imul rcx, [rdx+0x19]
                rax += rcx;             //add rax, rcx
                rax ^= rbx;             //xor rax, rbx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x15;           //shr rcx, 0x15
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x2A;           //shr rcx, 0x2A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1C;           //shr rcx, 0x1C
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x38;           //shr rcx, 0x38
                rax ^= rcx;             //xor rax, rcx
                return rax;
            }
            case 5:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);               //mov r9, [0x00000000063A78E2]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x15;           //shr rcx, 0x15
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x2A;           //shr rcx, 0x2A
                rax ^= rcx;             //xor rax, rcx
                rcx = 0x6B4D10E3FCFC0235;               //mov rcx, 0x6B4D10E3FCFC0235
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x3;            //shr rcx, 0x03
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x6;            //shr rcx, 0x06
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xC;            //shr rcx, 0x0C
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x18;           //shr rcx, 0x18
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x30;           //shr rcx, 0x30
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1A;           //shr rcx, 0x1A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x34;           //shr rcx, 0x34
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1C;           //shr rcx, 0x1C
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x38;           //shr rcx, 0x38
                rax ^= rcx;             //xor rax, rcx
                uintptr_t RSP_0x70;
                RSP_0x70 = 0x9FE7D7D2C91086EF;          //mov rcx, 0x9FE7D7D2C91086EF : RSP+0x70
                rax *= RSP_0x70;                //imul rax, [rsp+0x70]
                uintptr_t RSP_0x48;
                RSP_0x48 = 0x3B044E06AA0DC65D;          //mov rcx, 0x3B044E06AA0DC65D : RSP+0x48
                rax += RSP_0x48;                //add rax, [rsp+0x48]
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r9;              //xor rcx, r9
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                return rax;
            }
            case 6:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);               //mov r9, [0x00000000063A737D]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD284268]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xF;            //shr rcx, 0x0F
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1E;           //shr rcx, 0x1E
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x3C;           //shr rcx, 0x3C
                rax ^= rcx;             //xor rax, rcx
                rax -= r11;             //sub rax, r11
                rax ^= r11;             //xor rax, r11
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r9;              //xor rcx, r9
                rcx = ~rcx;             //not rcx
                rcx = request->read<uintptr_t>(rcx + 0x19);              //mov rcx, [rcx+0x19]
                uintptr_t RSP_0x40;
                RSP_0x40 = 0xB64C05FA8BB41ED5;          //mov rcx, 0xB64C05FA8BB41ED5 : RSP+0x40
                rcx *= RSP_0x40;                //imul rcx, [rsp+0x40]
                rax *= rcx;             //imul rax, rcx
                rax -= rbx;             //sub rax, rbx
                rcx = 0xA1839DE961442277;               //mov rcx, 0xA1839DE961442277
                rax *= rcx;             //imul rax, rcx
                return rax;
            }
            case 7:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A6EF5]
                rcx = DMA::BaseAddress + 0x7E9B;          //lea rcx, [0xFFFFFFFFFD28B877]
                rdx = r11;              //mov rdx, r11
                rax += rcx;             //add rax, rcx
                rdx = ~rdx;             //not rdx
                rax += rdx;             //add rax, rdx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xD;            //shr rcx, 0x0D
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1A;           //shr rcx, 0x1A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x34;           //shr rcx, 0x34
                rax ^= rcx;             //xor rax, rcx
                rcx = r11;              //mov rcx, r11
                uintptr_t RSP_0x50;
                RSP_0x50 = DMA::BaseAddress + 0x2B12;             //lea rcx, [0xFFFFFFFFFD2868F2] : RSP+0x50
                rcx ^= RSP_0x50;                //xor rcx, [rsp+0x50]
                rax -= rcx;             //sub rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x27;           //shr rcx, 0x27
                rax ^= rcx;             //xor rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = 0x6B0468CD6B4B36F5;               //mov rcx, 0x6B0468CD6B4B36F5
                rax *= rcx;             //imul rax, rcx
                rcx = 0x67D32343BA582459;               //mov rcx, 0x67D32343BA582459
                rax -= rcx;             //sub rax, rcx
                rcx = 0x2AB381DC49040AEF;               //mov rcx, 0x2AB381DC49040AEF
                rax ^= rcx;             //xor rax, rcx
                return rax;
            }
            case 8:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A69F7]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD2838E2]
                rcx = 0xF27764D7BCC134E1;               //mov rcx, 0xF27764D7BCC134E1
                rax *= rcx;             //imul rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x7;            //shr rcx, 0x07
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xE;            //shr rcx, 0x0E
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1C;           //shr rcx, 0x1C
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x38;           //shr rcx, 0x38
                rax ^= rcx;             //xor rax, rcx
                rax ^= rbx;             //xor rax, rbx
                rcx = 0xC3107C6F6CB6AAB7;               //mov rcx, 0xC3107C6F6CB6AAB7
                rax *= rcx;             //imul rax, rcx
                rcx = 0xD1B5E7C8461A7E03;               //mov rcx, 0xD1B5E7C8461A7E03
                rax ^= rcx;             //xor rax, rcx
                rcx = DMA::BaseAddress + 0x4977696C;              //lea rcx, [0x00000000469FA081]
                rcx = ~rcx;             //not rcx
                rcx += r11;             //add rcx, r11
                rax += rcx;             //add rax, rcx
                rax += 0xFFFFFFFFFFFF4E09;              //add rax, 0xFFFFFFFFFFFF4E09
                rcx = r11;              //mov rcx, r11
                rcx -= rbx;             //sub rcx, rbx
                rax += rcx;             //add rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                return rax;
            }
            case 9:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A6553]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD28343E]
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x23;           //shr rcx, 0x23
                rax ^= rcx;             //xor rax, rcx
                rcx = 0xA7B0F0AA378850A7;               //mov rcx, 0xA7B0F0AA378850A7
                rax *= rcx;             //imul rax, rcx
                rax ^= rbx;             //xor rax, rbx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1B;           //shr rcx, 0x1B
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x36;           //shr rcx, 0x36
                rax ^= rcx;             //xor rax, rcx
                rcx = rbx + 0xd85;              //lea rcx, [rbx+0xD85]
                rcx += r11;             //add rcx, r11
                rax += rcx;             //add rax, rcx
                rax -= rbx;             //sub rax, rbx
                uintptr_t RSP_0x60;
                RSP_0x60 = 0x4E5E1AE762C3863A;          //mov rcx, 0x4E5E1AE762C3863A : RSP+0x60
                rax ^= RSP_0x60;                //xor rax, [rsp+0x60]
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                return rax;
            }
            case 10:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);               //mov r9, [0x00000000063A6090]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD282F7B]
                r13 = DMA::BaseAddress + 0xF084;          //lea r13, [0xFFFFFFFFFD291FE7]
                rcx = 0x98CD10A39FEEABC3;               //mov rcx, 0x98CD10A39FEEABC3
                rax *= rcx;             //imul rax, rcx
                rcx = rbx + 0x8af0;             //lea rcx, [rbx+0x8AF0]
                rcx += r11;             //add rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x10;           //shr rcx, 0x10
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x20;           //shr rcx, 0x20
                rax ^= rcx;             //xor rax, rcx
                rcx = r11;              //mov rcx, r11
                rcx = ~rcx;             //not rcx
                rcx ^= r13;             //xor rcx, r13
                rax += rcx;             //add rax, rcx
                rcx = 0xDF8A1660CBF5F30F;               //mov rcx, 0xDF8A1660CBF5F30F
                rax *= rcx;             //imul rax, rcx
                rcx = 0xA829D63D19635A8D;               //mov rcx, 0xA829D63D19635A8D
                rax ^= rcx;             //xor rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r9;              //xor rcx, r9
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rax ^= r11;             //xor rax, r11
                return rax;
            }
            case 11:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A5C0D]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD282AF8]
                rdx = rax;              //mov rdx, rax
                rdx >>= 0x23;           //shr rdx, 0x23
                rcx = DMA::BaseAddress + 0x17E7D121;              //lea rcx, [0x00000000150FF8D5]
                rdx ^= rcx;             //xor rdx, rcx
                rcx = r11 + 0x1;                //lea rcx, [r11+0x01]
                rdx ^= r11;             //xor rdx, r11
                rdx ^= rax;             //xor rdx, rax
                rax = DMA::BaseAddress + 0x7D39B186;              //lea rax, [0x000000007A61D926]
                rax *= rcx;             //imul rax, rcx
                rax += rdx;             //add rax, rdx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = rbx + 0x8cb1;             //lea rcx, [rbx+0x8CB1]
                rcx += r11;             //add rcx, r11
                rax ^= rcx;             //xor rax, rcx
                rax ^= r11;             //xor rax, r11
                rcx = 0x71B5B118240CFD7D;               //mov rcx, 0x71B5B118240CFD7D
                rax *= rcx;             //imul rax, rcx
                rcx = rbx + 0x2611654c;                 //lea rcx, [rbx+0x2611654C]
                rcx += r11;             //add rcx, r11
                rax += rcx;             //add rax, rcx
                return rax;
            }
            case 12:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A5826]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD282711]
                rdx = DMA::BaseAddress + 0x4291859E;              //lea rdx, [0x000000003FB9AC55]
                rax -= rbx;             //sub rax, rbx
                rcx = rdx;              //mov rcx, rdx
                rcx = ~rcx;             //not rcx
                rcx ^= r11;             //xor rcx, r11
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x10;           //shr rcx, 0x10
                rax ^= rcx;             //xor rax, rcx
                rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
                rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
                rdx ^= r10;             //xor rdx, r10
                rcx = rax;              //mov rcx, rax
                rdx = ~rdx;             //not rdx
                rcx >>= 0x20;           //shr rcx, 0x20
                rax ^= rcx;             //xor rax, rcx
                rcx = 0x6A01EB295C695943;               //mov rcx, 0x6A01EB295C695943
                rax *= rcx;             //imul rax, rcx
                rcx = 0x9BBE6575DCB15C28;               //mov rcx, 0x9BBE6575DCB15C28
                rax ^= rcx;             //xor rax, rcx
                rax *= request->read<uintptr_t>(rdx + 0x19);             //imul rax, [rdx+0x19]
                rcx = 0x9A76A1C3B04C8361;               //mov rcx, 0x9A76A1C3B04C8361
                rax *= rcx;             //imul rax, rcx
                rax -= rbx;             //sub rax, rbx
                return rax;
            }
            case 13:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A5385]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD282270]
                rax ^= rbx;             //xor rax, rbx
                rcx = 0x424E7D3CE7A4BDA3;               //mov rcx, 0x424E7D3CE7A4BDA3
                rax *= rcx;             //imul rax, rcx
                rcx = DMA::BaseAddress + 0x1A767856;              //lea rcx, [0x00000000179E9692]
                rcx = ~rcx;             //not rcx
                rcx ^= r11;             //xor rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xF;            //shr rcx, 0x0F
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1E;           //shr rcx, 0x1E
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x3C;           //shr rcx, 0x3C
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1;            //shr rcx, 0x01
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x2;            //shr rcx, 0x02
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x4;            //shr rcx, 0x04
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x8;            //shr rcx, 0x08
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x10;           //shr rcx, 0x10
                rax ^= rcx;             //xor rax, rcx
                rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
                rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
                rcx = rax;              //mov rcx, rax
                rdx ^= r10;             //xor rdx, r10
                rcx >>= 0x20;           //shr rcx, 0x20
                rdx = ~rdx;             //not rdx
                rax ^= rcx;             //xor rax, rcx
                rax *= request->read<uintptr_t>(rdx + 0x19);             //imul rax, [rdx+0x19]
                rdx = DMA::BaseAddress + 0x31EB9108;              //lea rdx, [0x000000002F13B117]
                rdx += r11;             //add rdx, r11
                rcx = rax;              //mov rcx, rax
                rax = 0x8DB30096C278A251;               //mov rax, 0x8DB30096C278A251
                rcx *= rax;             //imul rcx, rax
                rax = rdx;              //mov rax, rdx
                rax ^= rcx;             //xor rax, rcx
                return rax;
            }
            case 14:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A4E71]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD281D51]
                rdx = DMA::BaseAddress + 0xA832;          //lea rdx, [0xFFFFFFFFFD28C1AE]
                rdx += r11;             //add rdx, r11
                rdx ^= rax;             //xor rdx, rax
                rax = r11;              //mov rax, r11
                rax = ~rax;             //not rax
                rax += rdx;             //add rax, rdx
                rax -= rbx;             //sub rax, rbx
                rax -= 0x2FECE2F9;              //sub rax, 0x2FECE2F9
                rcx = DMA::BaseAddress + 0x4948;          //lea rcx, [0xFFFFFFFFFD2862ED]
                rcx += r11;             //add rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = 0xE986304E17E64F7D;               //mov rcx, 0xE986304E17E64F7D
                rax *= rcx;             //imul rax, rcx
                rcx = DMA::BaseAddress + 0x5BB7;          //lea rcx, [0xFFFFFFFFFD287777]
                rcx = ~rcx;             //not rcx
                rcx -= r11;             //sub rcx, r11
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x1A;           //shr rcx, 0x1A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x34;           //shr rcx, 0x34
                rax ^= rcx;             //xor rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = r11;              //mov rcx, r11
                rcx = ~rcx;             //not rcx
                uintptr_t RSP_0xFFFFFFFFFFFFFF88;
                RSP_0xFFFFFFFFFFFFFF88 = DMA::BaseAddress + 0x30A5920C;           //lea rcx, [0x000000002DCDAF68] : RBP+0xFFFFFFFFFFFFFF88
                rcx ^= RSP_0xFFFFFFFFFFFFFF88;          //xor rcx, [rbp-0x78]
                rax += rcx;             //add rax, rcx
                return rax;
            }
            case 15:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x912310E);              //mov r10, [0x00000000063A4922]
                rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD28180D]
                rdx = DMA::BaseAddress + 0x56C36699;              //lea rdx, [0x0000000053EB7DFD]
                rcx = 0x14288A7031FA1D2A;               //mov rcx, 0x14288A7031FA1D2A
                rax += rcx;             //add rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0xA;            //shr rcx, 0x0A
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x14;           //shr rcx, 0x14
                rax ^= rcx;             //xor rax, rcx
                rcx = rax;              //mov rcx, rax
                rcx >>= 0x28;           //shr rcx, 0x28
                rax ^= rcx;             //xor rax, rcx
                rcx = 0xF471F0FE111CB275;               //mov rcx, 0xF471F0FE111CB275
                rax *= rcx;             //imul rax, rcx
                rcx = rdx;              //mov rcx, rdx
                rcx = ~rcx;             //not rcx
                rcx *= r11;             //imul rcx, r11
                rax ^= rcx;             //xor rax, rcx
                rax -= r11;             //sub rax, r11
                rax -= rbx;             //sub rax, rbx
                rax -= 0x57EC1422;              //sub rax, 0x57EC1422
                rcx = DMA::BaseAddress + 0xF4C;           //lea rcx, [0xFFFFFFFFFD2824A2]
                rcx *= r11;             //imul rcx, r11
                rax += rcx;             //add rax, rcx
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = ~rcx;             //not rcx
                rax *= request->read<uintptr_t>(rcx + 0x19);             //imul rax, [rcx+0x19]
                rcx = DMA::BaseAddress + 0x1793;          //lea rcx, [0xFFFFFFFFFD282C57]
                rcx -= r11;             //sub rcx, r11
                rax ^= rcx;             //xor rax, rcx
                return rax;
            }
            }
        }
        else if (globals->platform_battlenet)
        {
            const uint64_t mb = sdk::module_base;
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rdx = DMA::Read<uintptr_t>(client_info + 0x190418, sizeof(uintptr_t));
            if (!rdx)
                return rdx;

            r8 = ~sdk::peb;               //mov r8, gs:[rax]
            rax = rdx;              //mov rax, rdx
            rax >>= 0x12;           //shr rax, 0x12
            rdx ^= rax;             //xor rdx, rax
            rax = rdx;              //mov rax, rdx
            rax >>= 0x24;           //shr rax, 0x24
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rdx ^= rax;             //xor rdx, rax
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F00FE, sizeof(uintptr_t));             //xor rcx, [0x000000000540F554]
            rax = 0x7F668ABCF33A470D;               //mov rax, 0x7F668ABCF33A470D
            rcx = _byteswap_uint64(rcx);            //bswap rcx
            rdx *= DMA::Read<uintptr_t>(rcx + 0x13, sizeof(uintptr_t));            //imul rdx, [rcx+0x13]
            rdx *= rax;             //imul rdx, rax
            rax = sdk::module_base + 0x3D23;             //lea rax, [0xFFFFFFFFFE32315C]
            rdx ^= r8;              //xor rdx, r8
            rdx ^= rax;             //xor rdx, rax
            rax = rdx;              //mov rax, rdx
            rax >>= 0x21;           //shr rax, 0x21
            rdx ^= rax;             //xor rdx, rax
            rax = rdx;              //mov rax, rdx
            rax >>= 0x16;           //shr rax, 0x16
            rdx ^= rax;             //xor rdx, rax
            rax = rdx;              //mov rax, rdx
            rax >>= 0x2C;           //shr rax, 0x2C
            rdx ^= rax;             //xor rdx, rax
            return rdx;
    
        }
    }
    auto bone_base() -> uintptr_t {
        if (globals->platform_steam) {
            const uint64_t mb = sdk::module_base; // Use DMA::BaseAddress directly
            const uint64_t peb = sdk::peb; // Use DMA::PebAddress directly
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rdx = request->read<uintptr_t>(DMA::BaseAddress + 0xCC0B3D8);
            if (!rdx)
                return rdx;
            r11 = sdk::peb;              //mov r11, gs:[rax]
            rax = r11;              //mov rax, r11
            rax = _rotr64(rax, 0x15);               //ror rax, 0x15
            rax &= 0xF;
            switch (rax) {
            case 0:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D20D38]
                r15 = DMA::BaseAddress + 0x629DAB46;              //lea r15, [0x00000000605D867B]
                r13 = DMA::BaseAddress + 0x9895;          //lea r13, [0xFFFFFFFFFDC073BE]
                rax = 0xAC145E023332D189;               //mov rax, 0xAC145E023332D189
                rdx ^= rax;             //xor rdx, rax
                rax = r15;              //mov rax, r15
                rax = ~rax;             //not rax
                rax *= r11;             //imul rax, r11
                rdx += rax;             //add rdx, rax
                rax = 0xFDEBD2F07B05670D;               //mov rax, 0xFDEBD2F07B05670D
                rdx *= rax;             //imul rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x3;            //shr rax, 0x03
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x6;            //shr rax, 0x06
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0xC;            //shr rax, 0x0C
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x18;           //shr rax, 0x18
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x30;           //shr rax, 0x30
                rdx ^= rax;             //xor rdx, rax
                rax = 0xF0805972B46E082;                //mov rax, 0xF0805972B46E082
                rdx -= rax;             //sub rdx, rax
                rax = r11;              //mov rax, r11
                rax ^= r13;             //xor rax, r13
                rdx += rax;             //add rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = rdx;              //mov rax, rdx
                rax >>= 0x4;            //shr rax, 0x04
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x8;            //shr rax, 0x08
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x10;           //shr rax, 0x10
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x20;           //shr rax, 0x20
                rdx ^= rax;             //xor rdx, rax
                return rdx;
            }
            case 1:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D206BD]
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = 0x3ECBF33498144A56;               //mov rax, 0x3ECBF33498144A56
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0xA;            //shr rax, 0x0A
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x14;           //shr rax, 0x14
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x28;           //shr rax, 0x28
                rdx ^= rax;             //xor rdx, rax
                rax = 0x87F19886B363B05B;               //mov rax, 0x87F19886B363B05B
                rdx *= rax;             //imul rdx, rax
                rdx -= r11;             //sub rdx, r11
                rax = 0x6303659E1F345AFF;               //mov rax, 0x6303659E1F345AFF
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x15;           //shr rax, 0x15
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x2A;           //shr rax, 0x2A
                rdx ^= rax;             //xor rdx, rax
                rdx += r11;             //add rdx, r11
                return rdx;
            }
            case 2:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D2023D]
                r13 = DMA::BaseAddress + 0x6F7AC17A;              //lea r13, [0x000000006D3A91A8]
                rdx += r11;             //add rdx, r11
                rax = rdx;              //mov rax, rdx
                rax >>= 0x13;           //shr rax, 0x13
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x26;           //shr rax, 0x26
                rdx ^= rax;             //xor rdx, rax
                rax = DMA::BaseAddress + 0x62BA;          //lea rax, [0xFFFFFFFFFDC02FF1]
                rax -= r11;             //sub rax, r11
                rdx += rax;             //add rdx, rax
                rax = 0x6367F6E201B667AF;               //mov rax, 0x6367F6E201B667AF
                rdx *= rax;             //imul rdx, rax
                rax = 0x7EA109C91958478C;               //mov rax, 0x7EA109C91958478C
                rdx -= rax;             //sub rdx, rax
                rdx ^= r11;             //xor rdx, r11
                rdx ^= r13;             //xor rdx, r13
                rax = 0x79658B29969CD86A;               //mov rax, 0x79658B29969CD86A
                rdx -= rax;             //sub rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                return rdx;
            }
            case 3:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D1FD8D]
                r14 = DMA::BaseAddress + 0xF8CE;          //lea r14, [0xFFFFFFFFFDC0C44C]
                rax = rdx;              //mov rax, rdx
                rax >>= 0x20;           //shr rax, 0x20
                rdx ^= rax;             //xor rdx, rax
                rax = 0xEA0A19EF431520D;                //mov rax, 0xEA0A19EF431520D
                rdx ^= rax;             //xor rdx, rax
                rax = 0xFFFFFFFF93B5ED93;               //mov rax, 0xFFFFFFFF93B5ED93
                rax -= r11;             //sub rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rdx += rax;             //add rdx, rax
                rax = r11;              //mov rax, r11
                rax *= r14;             //imul rax, r14
                rdx -= rax;             //sub rdx, rax
                rax = 0x39F863E9187B3F65;               //mov rax, 0x39F863E9187B3F65
                rdx *= rax;             //imul rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x1F;           //shr rax, 0x1F
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x3E;           //shr rax, 0x3E
                rdx ^= rax;             //xor rdx, rax
                rax = 0x44AFB2020B72DD38;               //mov rax, 0x44AFB2020B72DD38
                rdx += rax;             //add rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                return rdx;
            }
            case 4:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D1F872]
                r15 = DMA::BaseAddress + 0xD76E;          //lea r15, [0xFFFFFFFFFDC09DD1]
                rax = r15;              //mov rax, r15
                rax = ~rax;             //not rax
                rax ^= r11;             //xor rax, r11
                rdx -= rax;             //sub rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x24;           //shr rax, 0x24
                rdx ^= rax;             //xor rdx, rax
                rax = 0x2690031C441C94ED;               //mov rax, 0x2690031C441C94ED
                rdx *= rax;             //imul rdx, rax
                rdx ^= r11;             //xor rdx, r11
                rax = 0xA3A6498F1C56BC17;               //mov rax, 0xA3A6498F1C56BC17
                rdx ^= rax;             //xor rdx, rax
                rdx -= r11;             //sub rdx, r11
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                return rdx;
            }
            case 5:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1F425]
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBFC05C]
                rdx += rax;             //add rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x19;           //shr rax, 0x19
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x32;           //shr rax, 0x32
                rdx ^= rax;             //xor rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rcx = request->read<uintptr_t>(rax + 0x11);              //mov rcx, [rax+0x11]
                rax = r11;              //mov rax, r11
                uintptr_t RSP_0x48;
                RSP_0x48 = DMA::BaseAddress + 0x598F00A5;                 //lea rax, [0x00000000574EC274] : RSP+0x48
                rax *= RSP_0x48;                //imul rax, [rsp+0x48]
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rdx += rax;             //add rdx, rax
                rax = 0xC6D870371839E04D;               //mov rax, 0xC6D870371839E04D
                rdx *= rax;             //imul rdx, rax
                rax = 0x2435BC22D4E2922B;               //mov rax, 0x2435BC22D4E2922B
                rdx -= rax;             //sub rdx, rax
                rdx *= rcx;             //imul rdx, rcx
                rax = 0xBBD9DF3CECEEFE74;               //mov rax, 0xBBD9DF3CECEEFE74
                rdx ^= rax;             //xor rdx, rax
                rax = 0x23B4F504FA125955;               //mov rax, 0x23B4F504FA125955
                rdx *= rax;             //imul rdx, rax
                return rdx;
            }
            case 6:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1EFE9]
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rax = r11;              //mov rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rax += 0xFFFFFFFFA7D1474C;              //add rax, 0xFFFFFFFFA7D1474C
                rdx += rax;             //add rdx, rax
                rcx ^= r10;             //xor rcx, r10
                rcx = _byteswap_uint64(rcx);            //bswap rcx
                rdx *= request->read<uintptr_t>(rcx + 0x11);             //imul rdx, [rcx+0x11]
                rax = 0xFFFFFFFFC0CD4EE3;               //mov rax, 0xFFFFFFFFC0CD4EE3
                rax -= r11;             //sub rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rdx += rax;             //add rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x23;           //shr rax, 0x23
                rax ^= rdx;             //xor rax, rdx
                rdx = 0x5A8397EF69EB3410;               //mov rdx, 0x5A8397EF69EB3410
                rax += r11;             //add rax, r11
                rax += rdx;             //add rax, rdx
                rdx = DMA::BaseAddress;           //lea rdx, [0xFFFFFFFFFDBFBAF6]
                rdx += rax;             //add rdx, rax
                rax = 0x94B908816CF2DBE1;               //mov rax, 0x94B908816CF2DBE1
                rdx *= rax;             //imul rdx, rax
                return rdx;
            }
            case 7:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1EBD8]
                r15 = DMA::BaseAddress + 0x6B60;          //lea r15, [0xFFFFFFFFFDC02529]
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBFB6DF]
                rax += 0xC77B;          //add rax, 0xC77B
                rax += r11;             //add rax, r11
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x9;            //shr rax, 0x09
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x12;           //shr rax, 0x12
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x24;           //shr rax, 0x24
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x13;           //shr rax, 0x13
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x26;           //shr rax, 0x26
                rdx ^= rax;             //xor rdx, rax
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rax = r15;              //mov rax, r15
                rax = ~rax;             //not rax
                rax *= r11;             //imul rax, r11
                rcx = _byteswap_uint64(rcx);            //bswap rcx
                rdx += rax;             //add rdx, rax
                rax = 0x3BAB7EE1C2FB5485;               //mov rax, 0x3BAB7EE1C2FB5485
                rdx *= request->read<uintptr_t>(rcx + 0x11);             //imul rdx, [rcx+0x11]
                rdx += rax;             //add rdx, rax
                rax = 0xD64310FF7669DED5;               //mov rax, 0xD64310FF7669DED5
                rdx *= rax;             //imul rdx, rax
                rax = 0xC9A0080E2B52320A;               //mov rax, 0xC9A0080E2B52320A
                rdx ^= rax;             //xor rdx, rax
                return rdx;
            }
            case 8:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1E605]
                rax = r11;              //mov rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rax += 0xFFFFFFFF954B94E9;              //add rax, 0xFFFFFFFF954B94E9
                rdx += rax;             //add rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = rdx;              //mov rax, rdx
                rax >>= 0xE;            //shr rax, 0x0E
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x1C;           //shr rax, 0x1C
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x38;           //shr rax, 0x38
                rdx ^= rax;             //xor rdx, rax
                rax = 0x28853EAC80AAB90;                //mov rax, 0x28853EAC80AAB90
                rdx -= rax;             //sub rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x21;           //shr rax, 0x21
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0xB;            //shr rax, 0x0B
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x16;           //shr rax, 0x16
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x2C;           //shr rax, 0x2C
                rdx ^= rax;             //xor rdx, rax
                rax = 0x9ED615C5A516F48D;               //mov rax, 0x9ED615C5A516F48D
                rdx *= rax;             //imul rdx, rax
                rax = 0x4A5451CFD1051B0F;               //mov rax, 0x4A5451CFD1051B0F
                rdx *= rax;             //imul rdx, rax
                return rdx;
            }
            case 9:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1E0EC]
                r13 = DMA::BaseAddress + 0x9F7F;          //lea r13, [0xFFFFFFFFFDC04E5C]
                rcx = DMA::BaseAddress + 0x590B7B0F;              //lea rcx, [0x0000000056CB2977]
                rax = rdx;              //mov rax, rdx
                rax >>= 0x25;           //shr rax, 0x25
                rdx ^= rax;             //xor rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = 0xE41AAE0B4978C7A7;               //mov rax, 0xE41AAE0B4978C7A7
                rdx *= rax;             //imul rdx, rax
                rax = 0x4884ED1EDA36D9B2;               //mov rax, 0x4884ED1EDA36D9B2
                rdx -= rax;             //sub rdx, rax
                rax = r11;              //mov rax, r11
                rax ^= r13;             //xor rax, r13
                rdx ^= rax;             //xor rdx, rax
                rax = 0xA5F46429036B04E5;               //mov rax, 0xA5F46429036B04E5
                rdx *= rax;             //imul rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x27;           //shr rax, 0x27
                rdx ^= rax;             //xor rdx, rax
                rdx -= r11;             //sub rdx, r11
                rdx += rcx;             //add rdx, rcx
                return rdx;
            }
            case 10:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D1DC5E]
                rdx -= DMA::BaseAddress;          //sub rdx, [rsp+0xA0] -- didn't find trace -> use base
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = r11;              //mov rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rax += 0xFFFFFFFFFFFF88EC;              //add rax, 0xFFFFFFFFFFFF88EC
                rdx += rax;             //add rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x12;           //shr rax, 0x12
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x24;           //shr rax, 0x24
                rdx ^= rax;             //xor rdx, rax
                rax = 0x4DBC160E13E56349;               //mov rax, 0x4DBC160E13E56349
                rdx *= rax;             //imul rdx, rax
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBFA934]
                rdx ^= rax;             //xor rdx, rax
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBFA806]
                rdx += rax;             //add rdx, rax
                return rdx;
            }
            case 11:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1D7B8]
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = DMA::BaseAddress + 0x73A4FAE9;              //lea rax, [0x0000000071649DDD]
                rax = ~rax;             //not rax
                rdx -= r11;             //sub rdx, r11
                rdx += rax;             //add rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x16;           //shr rax, 0x16
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x2C;           //shr rax, 0x2C
                rdx ^= rax;             //xor rdx, rax
                rax = 0x861DF3431C84C629;               //mov rax, 0x861DF3431C84C629
                rdx *= rax;             //imul rdx, rax
                rax = 0x714B44E8CE73C4F0;               //mov rax, 0x714B44E8CE73C4F0
                rdx -= rax;             //sub rdx, rax
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBFA258]
                rcx = rax * 0xFFFFFFFFFFFFFFFE;                 //imul rcx, rax, 0xFFFFFFFFFFFFFFFE
                rax = 0x6F9175143B9ED737;               //mov rax, 0x6F9175143B9ED737
                rdx += rax;             //add rdx, rax
                rdx += rcx;             //add rdx, rcx
                return rdx;
            }
            case 12:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1D3F9]
                r15 = DMA::BaseAddress + 0xD3DA;          //lea r15, [0xFFFFFFFFFDC075C4]
                rcx = r11;              //mov rcx, r11
                rcx = ~rcx;             //not rcx
                rax = DMA::BaseAddress + 0x1F86111B;              //lea rax, [0x000000001D45AFC8]
                rax = ~rax;             //not rax
                rcx *= rax;             //imul rcx, rax
                rax = r15;              //mov rax, r15
                rax -= r11;             //sub rax, r11
                rax += rdx;             //add rax, rdx
                rdx = rcx;              //mov rdx, rcx
                rdx ^= rax;             //xor rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rcx = DMA::BaseAddress;           //lea rcx, [0xFFFFFFFFFDBFA05D]
                rax = r11;              //mov rax, r11
                rax -= rcx;             //sub rax, rcx
                rcx = rax + 0xffffffffca7be9d9;                 //lea rcx, [rax-0x35841627]
                rcx += rdx;             //add rcx, rdx
                rax = rcx;              //mov rax, rcx
                rax >>= 0x18;           //shr rax, 0x18
                rcx ^= rax;             //xor rcx, rax
                rax = 0xFBA7ABC8BBB4629D;               //mov rax, 0xFBA7ABC8BBB4629D
                rdx = rcx;              //mov rdx, rcx
                rdx >>= 0x30;           //shr rdx, 0x30
                rdx ^= rcx;             //xor rdx, rcx
                rdx *= rax;             //imul rdx, rax
                rax = 0x1FE6307AA1F54B4D;               //mov rax, 0x1FE6307AA1F54B4D
                rdx *= rax;             //imul rdx, rax
                rax = 0x57A7A919AF723E1B;               //mov rax, 0x57A7A919AF723E1B
                rdx -= rax;             //sub rdx, rax
                return rdx;
            }
            case 13:
            {
                r9 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);               //mov r9, [0x0000000006D1CFFF]
                rax = rdx;              //mov rax, rdx
                rax >>= 0x1A;           //shr rax, 0x1A
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x34;           //shr rax, 0x34
                rdx ^= rax;             //xor rdx, rax
                rax = 0x525F068BC2643DF7;               //mov rax, 0x525F068BC2643DF7
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0xD;            //shr rax, 0x0D
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x1A;           //shr rax, 0x1A
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x34;           //shr rax, 0x34
                rdx ^= rax;             //xor rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r9;              //xor rax, r9
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = 0x75DFF140FA1FB5BC;               //mov rax, 0x75DFF140FA1FB5BC
                rdx += rax;             //add rdx, rax
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBF9A36]
                rdx += rax;             //add rdx, rax
                rax = 0xE5945E699002C625;               //mov rax, 0xE5945E699002C625
                rdx *= rax;             //imul rdx, rax
                rax = DMA::BaseAddress;           //lea rax, [0xFFFFFFFFFDBF9A1E]
                rdx ^= rax;             //xor rdx, rax
                return rdx;
            }
            case 14:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1CB9A]
                r15 = DMA::BaseAddress + 0x73A4A654;              //lea r15, [0x0000000071643FDF]
                rax = 0x75736E13202430E1;               //mov rax, 0x75736E13202430E1
                rdx *= rax;             //imul rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x26;           //shr rax, 0x26
                rdx ^= rax;             //xor rdx, rax
                rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
                rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
                rcx ^= r10;             //xor rcx, r10
                rcx = _byteswap_uint64(rcx);            //bswap rcx
                rdx *= request->read<uintptr_t>(rcx + 0x11);             //imul rdx, [rcx+0x11]
                rax = rdx;              //mov rax, rdx
                rax >>= 0x9;            //shr rax, 0x09
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x12;           //shr rax, 0x12
                rdx ^= rax;             //xor rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x24;           //shr rax, 0x24
                rdx ^= rax;             //xor rdx, rax
                rdx += r11;             //add rdx, r11
                rax = r11;              //mov rax, r11
                rax = ~rax;             //not rax
                rax *= r15;             //imul rax, r15
                rdx ^= rax;             //xor rdx, rax
                rax = 0xABD8E138F25E5687;               //mov rax, 0xABD8E138F25E5687
                rdx ^= rax;             //xor rdx, rax
                return rdx;
            }
            case 15:
            {
                r10 = request->read<uintptr_t>(DMA::BaseAddress + 0x91231FC);              //mov r10, [0x0000000006D1C7E9]
                rsi = 0xB656FAE057EB613B;               //mov rsi, 0xB656FAE057EB613B
                rdx *= rsi;             //imul rdx, rsi
                rax = rdx;              //mov rax, rdx
                rax >>= 0x21;           //shr rax, 0x21
                rdx ^= rax;             //xor rdx, rax
                rax = 0;                //and rax, 0xFFFFFFFFC0000000
                rax = _rotl64(rax, 0x10);               //rol rax, 0x10
                rax ^= r10;             //xor rax, r10
                rax = _byteswap_uint64(rax);            //bswap rax
                rdx *= request->read<uintptr_t>(rax + 0x11);             //imul rdx, [rax+0x11]
                rax = 0x5CA0A4447C245D90;               //mov rax, 0x5CA0A4447C245D90
                rdx -= rax;             //sub rdx, rax
                rdx -= r11;             //sub rdx, r11
                rax = 0xF071D0312866EB9D;               //mov rax, 0xF071D0312866EB9D
                rdx *= rax;             //imul rdx, rax
                rax = 0xFFFFFFFFFFFFF34A;               //mov rax, 0xFFFFFFFFFFFFF34A
                rax -= r11;             //sub rax, r11
                rax -= DMA::BaseAddress;          //sub rax, [rsp+0xA0] -- didn't find trace -> use base
                rdx += rax;             //add rdx, rax
                rax = rdx;              //mov rax, rdx
                rax >>= 0x21;           //shr rax, 0x21
                rdx ^= rax;             //xor rdx, rax
                return rdx;
            }
            }

        }
        else if (globals->platform_battlenet)
        {
            const uint64_t mb = sdk::module_base;
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rcx = DMA::Read<uintptr_t>(sdk::module_base + 0xAF36518, sizeof(uintptr_t));
            if (!rcx)
                return rcx;

            rdx = sdk::peb;              //mov rdx, gs:[rax]
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F01EE, sizeof(uintptr_t));              //xor rax, [0x0000000002EE305E]
            rax = ~rax;             //not rax
            rcx *= DMA::Read<uintptr_t>(rax + 0x13, sizeof(uintptr_t));            //imul rcx, [rax+0x13]
            rax = 0xE500FD3290E1F913;               //mov rax, 0xE500FD3290E1F913
            rcx *= rax;             //imul rcx, rax
            rax = 0x94AEADE7D7695FD6;               //mov rax, 0x94AEADE7D7695FD6
            rcx += rdx;             //add rcx, rdx
            rdx = sdk::module_base;              //lea rdx, [0xFFFFFFFFFBDF2E46]
            rcx += rax;             //add rcx, rax
            rcx += rdx;             //add rcx, rdx
            rax = rcx;              //mov rax, rcx
            rax >>= 0x9;            //shr rax, 0x09
            rcx ^= rax;             //xor rcx, rax
            rax = rcx;              //mov rax, rcx
            rax >>= 0x12;           //shr rax, 0x12
            rcx ^= rax;             //xor rcx, rax
            rax = rcx;              //mov rax, rcx
            rax >>= 0x24;           //shr rax, 0x24
            rcx ^= rax;             //xor rcx, rax
            rax = rcx;              //mov rax, rcx
            rax >>= 0xD;            //shr rax, 0x0D
            rcx ^= rax;             //xor rcx, rax
            rax = rcx;              //mov rax, rcx
            rax >>= 0x1A;           //shr rax, 0x1A
            rcx ^= rax;             //xor rcx, rax
            rax = rcx;              //mov rax, rcx
            rax >>= 0x34;           //shr rax, 0x34
            rcx ^= rax;             //xor rcx, rax
            return rcx;
    
        }
    }
    auto bone_index(uint32_t index) -> uint64_t {
        if (globals->platform_steam) {
            const uint64_t mb = sdk::module_base; // Use DMA::BaseAddress directly
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = index, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rdi = index;
            rcx = rdi * 0x13C8;
            rax = 0xD73F3E9D2DBEC8E7;               //mov rax, 0xD73F3E9D2DBEC8E7
            rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
            r11 = 0xCCCCCCCCCCCCCCCD;               //mov r11, 0xCCCCCCCCCCCCCCCD
            rbx = DMA::BaseAddress;           //lea rbx, [0xFFFFFFFFFD7091CB]
            rdx >>= 0xD;            //shr rdx, 0x0D
            r10 = 0xE98285CCFA0AE387;               //mov r10, 0xE98285CCFA0AE387
            rax = rdx * 0x260F;             //imul rax, rdx, 0x260F
            rcx -= rax;             //sub rcx, rax
            rax = 0xC388D5333BAA90CD;               //mov rax, 0xC388D5333BAA90CD
            r8 = rcx * 0x260F;              //imul r8, rcx, 0x260F
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rax = r8;               //mov rax, r8
            rax -= rdx;             //sub rax, rdx
            rax >>= 0x1;            //shr rax, 0x01
            rax += rdx;             //add rax, rdx
            rax >>= 0xE;            //shr rax, 0x0E
            rax = rax * 0x4892;             //imul rax, rax, 0x4892
            r8 -= rax;              //sub r8, rax
            rax = r11;              //mov rax, r11
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rax = 0xBAA551EE51D6FD2D;               //mov rax, 0xBAA551EE51D6FD2D
            rdx >>= 0x3;            //shr rdx, 0x03
            rcx = rdx + rdx * 4;            //lea rcx, [rdx+rdx*4]
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rdx >>= 0xB;            //shr rdx, 0x0B
            rax = rdx + rcx * 2;            //lea rax, [rdx+rcx*2]
            rcx = rax * 0x15F2;             //imul rcx, rax, 0x15F2
            rax = r8 * 0x15F4;              //imul rax, r8, 0x15F4
            rax -= rcx;             //sub rax, rcx
            rax = request->read<uint16_t>(rax + rbx * 1 + 0x91F4D80);                //movzx eax, word ptr [rax+rbx*1+0x91F4D80]
            r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
            rax = r10;              //mov rax, r10
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rax = r10;              //mov rax, r10
            rdx >>= 0xD;            //shr rdx, 0x0D
            rcx = rdx * 0x2315;             //imul rcx, rdx, 0x2315
            r8 -= rcx;              //sub r8, rcx
            r9 = r8 * 0x351B;               //imul r9, r8, 0x351B
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rax = r11;              //mov rax, r11
            rdx >>= 0xD;            //shr rdx, 0x0D
            rcx = rdx * 0x2315;             //imul rcx, rdx, 0x2315
            r9 -= rcx;              //sub r9, rcx
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rax = 0x18AB083902BDAB95;               //mov rax, 0x18AB083902BDAB95
            rdx >>= 0x4;            //shr rdx, 0x04
            rcx = rdx + rdx * 4;            //lea rcx, [rdx+rdx*4]
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rax = r9;               //mov rax, r9
            rax -= rdx;             //sub rax, rdx
            rax >>= 0x1;            //shr rax, 0x01
            rax += rdx;             //add rax, rdx
            rax >>= 0x8;            //shr rax, 0x08
            rax = rax + rcx * 4;            //lea rax, [rax+rcx*4]
            rcx = rax * 0x3A6;              //imul rcx, rax, 0x3A6
            rax = r9 * 0x3A8;               //imul rax, r9, 0x3A8
            rax -= rcx;             //sub rax, rcx
            r12 = request->read<uint16_t>(rax + rbx * 1 + 0x9202900);                //movsx r12d, word ptr [rax+rbx*1+0x9202900]
            return r12;
        }
        else if (globals->platform_battlenet)
        {
            const uint64_t mb = sdk::module_base;
            uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
            rsi = index;
            rcx = rsi * 0x13C8;
            rax = 0xC4663A35CC80B1FD;               //mov rax, 0xC4663A35CC80B1FD
            r11 = DMA::BaseAddress;              //lea r11, [0xFFFFFFFFFE3157D3]
            rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
            r10 = 0xAA07B899C1FB6F0B;               //mov r10, 0xAA07B899C1FB6F0B
            rdx >>= 0xC;            //shr rdx, 0x0C
            rax = rdx * 0x14DB;             //imul rax, rdx, 0x14DB
            rcx -= rax;             //sub rcx, rax
            rax = 0x4F98277C7B96C15;                //mov rax, 0x4F98277C7B96C15
            r8 = rcx * 0x14DB;              //imul r8, rcx, 0x14DB
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rdx >>= 0x7;            //shr rdx, 0x07
            rax = rdx * 0x19BB;             //imul rax, rdx, 0x19BB
            r8 -= rax;              //sub r8, rax
            rax = 0xB244C4069D8D463F;               //mov rax, 0xB244C4069D8D463F
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rcx = r8;               //mov rcx, r8
            r8 &= 0x3;             //and r8d, 0x03
            rdx >>= 0xB;            //shr rdx, 0x0B
            rax = rdx * 0xB7D;              //imul rax, rdx, 0xB7D
            rcx -= rax;             //sub rcx, rax
            rax = r8 + rcx * 4;             //lea rax, [r8+rcx*4]
            rax = DMA::Read<uint16_t>(r11 + rax * 2 + 0x71670B0, sizeof(rax));                //movzx eax, word ptr [r11+rax*2+0x71670B0]
            r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
            rax = r10;              //mov rax, r10
            rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
            rax = r10;              //mov rax, r10
            rdx >>= 0xC;            //shr rdx, 0x0C
            rcx = rdx * 0x1817;             //imul rcx, rdx, 0x1817
            r8 -= rcx;              //sub r8, rcx
            r9 = r8 * 0x261A;               //imul r9, r8, 0x261A
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rdx >>= 0xC;            //shr rdx, 0x0C
            rax = rdx * 0x1817;             //imul rax, rdx, 0x1817
            r9 -= rax;              //sub r9, rax
            rax = 0x8FB823EE08FB823F;               //mov rax, 0x8FB823EE08FB823F
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rax = 0xBD69104707661AA3;               //mov rax, 0xBD69104707661AA3
            rdx >>= 0x5;            //shr rdx, 0x05
            rcx = rdx * 0x39;               //imul rcx, rdx, 0x39
            rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
            rdx >>= 0x7;            //shr rdx, 0x07
            rcx += rdx;             //add rcx, rdx
            rax = rcx * 0x15A;              //imul rax, rcx, 0x15A
            rcx = r9 * 0x15C;               //imul rcx, r9, 0x15C
            rcx -= rax;             //sub rcx, rax
            r8 = DMA::Read<uint16_t>(rcx + r11 * 1 + 0x716CCA0, sizeof(r8));                //movsx r8d, word ptr [rcx+r11*1+0x716CCA0]
            return r8;
        }

    }
};
static decryption* decrypt = new decryption();