#include "misc_commands.hpp"
#include "command_handler.hpp"
#include "console.hpp"
#include "global_instance_manager.hpp"
#include "il2cpp_api_cpp.hpp"
#include "bindings/unityengine.hpp"

#include <format>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

class MiscCommandsHandler final : public CommandHandler {
    static inline Il2Cpp::API::Object getPlayer(const std::string& target) {
        if (target != "me")
            throw std::runtime_error(std::format("Unknown player target '{}'. Only 'me' is currently supported", target));

        auto gim = globalInstanceManagerInfo.get<GlobalInstanceManager>();
        if (!gim)
            throw std::runtime_error("Global instance manager is unavailable");

        auto playerPtr = reinterpret_cast<Il2CppObject *>(gim->getPlayer());
        if (!playerPtr)
            throw std::runtime_error("Local player instance not found");

        return Il2Cpp::API::Object{playerPtr};
    }

    static inline Il2Cpp::API::Object getGhost() {
        auto gim = globalInstanceManagerInfo.get<GlobalInstanceManager>();
        if (!gim)
            throw std::runtime_error("Global instance manager is unavailable");

        auto ghostPtr = reinterpret_cast<Il2CppObject *>(gim->getGhostAI());
        if (!ghostPtr)
            throw std::runtime_error("GhostAI instance not found");

        return Il2Cpp::API::Object{ghostPtr};
    }

    static inline Il2Cpp::API::Object getGameController() {
        auto gim = globalInstanceManagerInfo.get<GlobalInstanceManager>();
        if (!gim)
            throw std::runtime_error("Global instance manager is unavailable");

        auto gcPtr = reinterpret_cast<Il2CppObject *>(gim->getGameController());
        if (!gcPtr)
            throw std::runtime_error("GameController instance not found");

        return Il2Cpp::API::Object{gcPtr};
    }

public:
    explicit MiscCommandsHandler() = default;

    void updateApp(CLI::App& app) override {
        // Player subcommands
        CLI::App *playerCmd = app.add_subcommand("player", "Player management commands");

        // DropAllVRObjects
        CLI::App *dropVrCmd = playerCmd->add_subcommand("drop_all_vr_objects", "Force player to drop all VR objects");
        auto dropVrTarget = std::make_shared<std::string>("me");
        dropVrCmd->add_option("target", *dropVrTarget, "Target player");
        dropVrCmd->callback([dropVrTarget]() {
            auto player = getPlayer(*dropVrTarget);
            Il2Cpp::API::call(player, "DropAllVRObjects");
            console::println(std::format("Dropped all VR objects for '{}'", *dropVrTarget));
        });

        // ForceDropPropsNetworked
        CLI::App *dropNetCmd = playerCmd->add_subcommand("drop_props", "Force player to drop all props");
        auto dropNetTarget = std::make_shared<std::string>("me");
        dropNetCmd->add_option("target", *dropNetTarget, "Target player");
        dropNetCmd->callback([dropNetTarget]() {
            auto player = getPlayer(*dropNetTarget);
            Il2Cpp::API::call(player, "ForceDropPropsNetworked");
            console::println(std::format("Dropped all VR props for '{}'", *dropNetTarget));
        });

        // RevivePlayer
        CLI::App *reviveCmd = playerCmd->add_subcommand("revive", "Revive the player");
        auto reviveTarget = std::make_shared<std::string>("me");
        reviveCmd->add_option("target", *reviveTarget, "Target player");
        reviveCmd->callback([reviveTarget]() {
            console::println(std::format("Reviving player '{}'", *reviveTarget));
            auto player = getPlayer(*reviveTarget);
            Il2Cpp::API::call(player, "RevivePlayer");
        });

        // Teleport
        CLI::App *teleportCmd = playerCmd->add_subcommand("teleport", "Teleport player to world position (x y z)");
        struct TeleportArgs {
            std::string target{"me"};
            float x{0.0f};
            float y{0.0f};
            float z{0.0f};
        };
        auto tpArgs = std::make_shared<TeleportArgs>();
        teleportCmd->add_option("target", tpArgs->target, "Target player");
        teleportCmd->add_option("x", tpArgs->x, "Target X coordinate")->required();
        teleportCmd->add_option("y", tpArgs->y, "Target Y coordinate")->required();
        teleportCmd->add_option("z", tpArgs->z, "Target Z coordinate")->required();
        teleportCmd->callback([tpArgs]() {
            auto player = getPlayer(tpArgs->target);
            UnityEngine_Vector3_o position{tpArgs->x, tpArgs->y, tpArgs->z};
            auto method = player.klass().get_method("Teleport", 1);
            void *args[] = {&position};
            method.invoke(player, args);
            console::println(std::format("Teleported '{}' to ({:.2f}, {:.2f}, {:.2f})", tpArgs->target, tpArgs->x, tpArgs->y, tpArgs->z));
        });

        // StartKillingPlayer/KillPlayer
        CLI::App *startKillCmd = playerCmd->add_subcommand("kill", "Kill the player");
        struct KillArgs {
            std::string target{"me"};
            bool no_animation{false};
        };
        auto killArgs = std::make_shared<KillArgs>();
        startKillCmd->add_option("target", killArgs->target, "Target player");
        startKillCmd->add_option("-n,--no-animate", killArgs->no_animation, "Skip death animation");
        startKillCmd->callback([killArgs]() {
            console::println(std::format("Killing player '{}'", killArgs->target));
            auto player = getPlayer(killArgs->target);
            Il2Cpp::API::call(player, killArgs->no_animation ? "KillPlayer" : "StartKillingPlayer");
        });

        // Ghost AI subcommands
        CLI::App *ghostCmd = app.add_subcommand("ghost", "Ghost AI management commands");

        // UnAppear
        CLI::App *unappearCmd = ghostCmd->add_subcommand("unappear", "Make the ghost un-appear");
        unappearCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "UnAppear");
            console::println("Ghost un-appeared");
        });

        // SetCanFlash
        CLI::App *canFlashCmd = ghostCmd->add_subcommand("can_flash", "Set whether the ghost can flash");
        auto canFlashState = std::make_shared<bool>(true);
        canFlashCmd->add_option("state", *canFlashState, "Enable or disable ghost flashing");
        canFlashCmd->callback([canFlashState]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "SetCanFlash", *canFlashState);
            console::println(std::format("Ghost SetCanFlash set to {}", *canFlashState));
        });

        // StartHuntingTimer
        CLI::App *startHuntTimerCmd = ghostCmd->add_subcommand("start_hunting_timer", "Start the ghost hunting timer");
        startHuntTimerCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "StartHuntingTimer");
            console::println("Ghost hunting timer started");
        });

        // FlashAppear
        CLI::App *flashAppearCmd = ghostCmd->add_subcommand("flash_appear", "Make the ghost flash appear");
        flashAppearCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "FlashAppear");
            console::println("Ghost flash appeared");
        });

        // LookAtPlayer
        CLI::App *lookAtPlayerCmd = ghostCmd->add_subcommand("look_at_player", "Force ghost to look at a player");
        auto lookTarget = std::make_shared<std::string>("me");
        lookAtPlayerCmd->add_option("target", *lookTarget, "Target player");
        lookAtPlayerCmd->callback([lookTarget]() {
            auto ghost = getGhost();
            auto player = getPlayer(*lookTarget);
            Il2Cpp::API::call(ghost, "LookAtPlayer", player);
            console::println(std::format("Ghost looking at player '{}'", *lookTarget));
        });

        // SetNewBansheeTarget
        CLI::App *bansheeTargetCmd = ghostCmd->add_subcommand("set_banshee_target", "Set a random new Banshee target");
        bansheeTargetCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "SetNewBansheeTarget");
            console::println("Banshee target updated");
        });

        // StopGhostFromHunting
        CLI::App *stopHuntCmd = ghostCmd->add_subcommand("stop_hunting", "Stop ghost from hunting");
        stopHuntCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "StopGhostFromHunting");
            console::println("Ghost hunting stopped");
        });

        // TemporarilyStopWander
        CLI::App *stopWanderCmd = ghostCmd->add_subcommand("stop_wander", "Temporarily stop ghost wander");
        stopWanderCmd->callback([]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "TemporarilyStopWander");
            console::println("Ghost wandering temporarily stopped");
        });

        // DelayTeleportToFavouriteRoom
        CLI::App *teleportFavCmd = ghostCmd->add_subcommand("teleport_fav_room", "Teleport ghost to favourite room with optional delay");
        auto teleportDelay = std::make_shared<float>(0.0f);
        teleportFavCmd->add_option("delay", *teleportDelay, "Delay in seconds");
        teleportFavCmd->callback([teleportDelay]() {
            auto ghost = getGhost();
            Il2Cpp::API::call(ghost, "DelayTeleportToFavouriteRoom", *teleportDelay);
            console::println(std::format("Teleporting ghost to favourite room (delay: {:.2f}s)", *teleportDelay));
        });

        // LookAtNearestPlayer
        CLI::App *lookNearestCmd = ghostCmd->add_subcommand("look_at_nearest_player", "Make ghost look at the nearest player");
        lookNearestCmd->callback([]() {
            auto ghost = getGhost();
            auto res = Il2Cpp::API::call(ghost, "LookAtNearestPlayer");
            bool success = res.ptr ? *Il2Cpp::API::object_unbox<bool>(res) : false;
            console::println(std::format("Ghost looking at nearest player (Success: {})", success));
        });

        // GameController subcommands
        CLI::App *gameCmd = app.add_subcommand("game", "Game controller commands");

        // GetAveragePlayerInsanity
        CLI::App *insanityCmd = gameCmd->add_subcommand("get_sanity", "Print average player sanity");
        insanityCmd->callback([]() {
            auto gc = getGameController();
            auto res = Il2Cpp::API::call(gc, "GetAveragePlayerInsanity");
            float insanity = res.ptr ? *Il2Cpp::API::object_unbox<float>(res) : 0.0f;
            console::println(std::format("Average player insanity: {:.2f}%", 100.0f - insanity));
        });

        // ResetHuntEffects
        CLI::App *resetHuntCmd = gameCmd->add_subcommand("reset_hunt_effects", "Reset hunt effects");
        resetHuntCmd->callback([]() {
            auto gc = getGameController();
            Il2Cpp::API::call(gc, "ResetHuntEffects");
            console::println("Hunt effects reset");
        });

        // LoadBackToMenu
        CLI::App *loadMenuCmd = gameCmd->add_subcommand("load_menu", "Return to main menu");
        loadMenuCmd->callback([]() {
            auto gc = getGameController();
            Il2Cpp::API::call(gc, "LoadBackToMenu");
            console::println("Loading back to main menu..");
        });
    }
};

} // namespace

std::unique_ptr<CommandHandler> MiscCommands::getCommandHandler() { return std::make_unique<MiscCommandsHandler>(); }

ModInfo miscCommandsInfo{"Misc Commands", true, []() { return std::make_unique<MiscCommands>(); }, []() { miscCommandsInfo.load(); }};
