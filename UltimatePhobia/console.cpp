#include "console.hpp"
#include "application.hpp"
#include "command_handler.hpp"
#include "mods/game_console.hpp"
#include "CLI11.hpp"

#include <format>
#include <memory>

namespace console {

namespace {
class ExitCommandHandler final : public CommandHandler {
private:
    int exitCode_ = 0;

public:
    void updateApp(CLI::App& app) override {
        CLI::App *cmdExit = app.add_subcommand("exit", "Exits the application");
        cmdExit->add_option("code", exitCode_, "Exit code to return to the OS")->default_val(0);
        cmdExit->callback([this]() { Application::exit(exitCode_); });
    }
};

std::unique_ptr<CLI::App> buildApp() {
    auto app = std::make_unique<CLI::App>();

    static ExitCommandHandler exitCommand;
    exitCommand.updateApp(*app);

    for (const auto& modInfo : currentApplication->getMods()) {
        if (auto& mod = modInfo->instance)
            if (auto commandHandler = mod->getCommandHandler())
                commandHandler->updateApp(*app);
    }

    app->allow_extras(false);

    return app;
}
} // namespace

void print(std::string_view string) {
    std::string formatted;
    formatted.reserve(string.size());

    char previous = '\0';
    for (char c : string) {
        if (c == '\n' && previous != '\r')
            formatted.push_back('\r');
        formatted.push_back(c);
        previous = c;
    }

    if (auto console = gameConsoleInfo.get<GameConsole>())
        console->print(formatted);
}

void println(std::string_view string) { print(std::format("{}\n", string)); }

void onInput(std::string_view inputView) {
    std::string input(inputView);
    if (input == "help")
        input = "--help";
    else if (input.starts_with("help "))
        input = input.substr(5) + " --help";

    auto app = buildApp();

    try {
        app->parse(std::string(input));
    } catch (const CLI::ParseError& e) {
        std::stringstream out;
        app->exit(e, out, out);
        println(out.str());
    }
}

bool onTab(std::string& input) {
    auto app = buildApp();

    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token)
        tokens.push_back(token);

    const bool endsWithSpace = input.empty() || input.back() == ' ';
    const std::string prefix = endsWithSpace ? "" : (tokens.empty() ? "" : tokens.back());

    // Exclude word currently being typed from context traversal
    if (!endsWithSpace && !tokens.empty())
        tokens.pop_back();

    CLI::App *currentContext = app.get();
    int positionalArgsProvided = 0;
    bool skipNextPositional = false;

    // Traverse command tree to resolve context and count fulfilled arguments
    for (const auto& t : tokens) {
        bool isSubcommand = false;

        for (auto *sub : currentContext->get_subcommands([](const CLI::App *) { return true; }))
            if (sub->get_name() == t) {
                currentContext = sub;
                isSubcommand = true;
                positionalArgsProvided = 0;
                skipNextPositional = false;
                break;
            }

        if (isSubcommand)
            continue;

        if (t.starts_with("-")) {
            // Flag arguments may consume next token as a value
            for (auto *opt : currentContext->get_options())
                if (!opt->get_positional() && opt->check_name(t) && opt->get_expected_min() > 0) {
                    skipNextPositional = true;
                    break;
                }
        } else if (skipNextPositional)
            skipNextPositional = false;
        else
            positionalArgsProvided++;
    }

    std::vector<std::string> matches;
    const bool typingFlag = prefix.starts_with("-");

    // Suggest matching subcommands
    if (!typingFlag)
        for (auto *sub : currentContext->get_subcommands([](const CLI::App *) { return true; }))
            if (sub->get_name().starts_with(prefix))
                matches.push_back(sub->get_name());

    std::vector<CLI::Option *> positionals;
    for (auto *opt : currentContext->get_options())
        if (opt->get_positional())
            positionals.push_back(opt);

    // Suggest matching flags
    for (auto *opt : currentContext->get_options())
        if (!opt->get_positional() && typingFlag) {
            for (const auto& lname : opt->get_lnames())
                if (std::string flag = "--" + lname; flag.starts_with(prefix))
                    matches.push_back(flag);

            for (const auto& sname : opt->get_snames())
                if (std::string flag = "-" + sname; flag.starts_with(prefix))
                    matches.push_back(flag);
        }

    // Suggest default value for active positional argument
    if (!typingFlag && positionalArgsProvided < positionals.size())
        if (std::string defVal = positionals[positionalArgsProvided]->get_default_str(); !defVal.empty() && defVal.starts_with(prefix))
            matches.push_back(defVal);

    if (matches.empty())
        return false;

    std::sort(matches.begin(), matches.end());
    matches.erase(std::unique(matches.begin(), matches.end()), matches.end());

    // Single match: complete immediately
    if (matches.size() == 1) {
        input = input.substr(0, input.length() - prefix.length()) + matches[0] + " ";
        return true;
    }

    // Multiple matches: print suggestions
    std::string suggestions;
    for (const auto& m : matches)
        suggestions += m + " ";
    println(std::format("\x1b[33m{}\x1b[0m", suggestions));

    // Fill longest common prefix (LCP)
    std::string lcp = matches[0];
    for (std::size_t i = 1; i < matches.size(); ++i) {
        std::size_t j = 0;

        while (j < lcp.size() && j < matches[i].size() && lcp[j] == matches[i][j])
            j++;

        lcp = lcp.substr(0, j);
    }

    if (lcp.length() > prefix.length()) {
        input = input.substr(0, input.length() - prefix.length()) + lcp;
        return true;
    }

    return false;
}

} // namespace console
