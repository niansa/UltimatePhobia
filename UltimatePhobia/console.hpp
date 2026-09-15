#pragma once

#include <string>
#include <string_view>

namespace console {

void print(std::string_view string);
void println(std::string_view string);

void onInput(std::string_view input);
bool onTab(std::string& input);

} // namespace console
