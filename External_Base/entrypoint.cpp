#ifndef ENTRYPOINT_CPP
#define ENTRYPOINT_CPP

#include "impl/include.hpp"
#include "impl/driver/dma.h"

auto main() -> void
{
    if (!DMA::Connect()) {
        std::cerr << "\n [log] -> failed to connect to DMA.\n";
        std::cin.get();
        return;
    }
    std::cout << "\n [log] -> DMA connected successfully.\n";

    globals->width = GetSystemMetrics(SM_CXSCREEN);
    globals->height = GetSystemMetrics(SM_CYSCREEN);

    MessageBoxA(0, skCrypt("Press [OK] In Lobby."), skCrypt("Alert"), MB_OK);

    auto result = DMA::AttachToProcessId();
    if (!result) {
        std::cerr << "\n [log] -> failed to attach to process.\n";
        std::cin.get();
        return; // Return added to ensure we exit if attachment fails
    }
    std::cout << "\n [log] -> attached to process successfully.\n";

    if (!DMA::GetPEBAddress(DMA::AttachedProcessId)) {
        std::cerr << "\n [log] -> failed to retrieve PEB address.\n";
        std::cin.get();
        return; // Return added to ensure we exit if PEB retrieval fails
    }
    std::cout << "\n [log] -> retrieved PEB address successfully.\n";

    // Updating sdk namespace variables with DMA addresses
    sdk::module_base = DMA::BaseAddress; // Populate sdk::module_base with DMA Base Address
    sdk::peb = DMA::PebAddress; // Populate sdk::peb with DMA PEB Address

    // Log the addresses
    std::cout << " [log] -> Module Base Address: " << std::hex << sdk::module_base << std::dec << "\n";
    std::cout << " [log] -> PEB Address: " << std::hex << sdk::peb << std::dec << "\n";

    if (Hook->Setup() != HOOK_INFORMATION::HOOKS_SUCCESSFUL) {
        std::cerr << " [hook] -> failed to setup hook.\n";
        return; // Return added to ensure we exit if hook setup fails
    }
    else {
        std::cout << " [hook] -> hook setup successfully.\n";
    }

    if (Renderer->Setup() != RENDER_INFORMATION::RENDER_SETUP_SUCCESSFUL) {
        std::cerr << " [render] -> failed to setup render.\n";
        return; // Return added to ensure we exit if render setup fails
    }
    else {
        std::cout << " [render] -> render setup successfully.\n";
    }

    if (Game->Setup() != GAME_INFORMATION::GAME_SETUP_SUCCESSFUL) {
        std::cerr << " [game] -> failed to setup game.\n";
        return; // Return added to ensure we exit if game setup fails
    }
    else {
        std::cout << " [game] -> game setup successfully.\n";
    }

    Renderer->Render();
    std::cin.get();
}

#endif // !ENTRYPOINT_CPP