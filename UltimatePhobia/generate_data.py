from sys import argv
from sys import exit
import json

if len(argv) == 1:
    print(f"Usage: {argv[0]} <script.json file>")
    exit(0)

output = open("generated/game_data.inc", "w")
output.write("""
#include <string_view>
#include <vector>


static std::vector<GameData::Method> methods = {
""")

script = json.load(open(argv[1], "r"))
for method in script["ScriptMethod"]:
    output.write(f"    {{(void*){method['Address']}, {json.dumps(method['Name'])}, {json.dumps(method['Signature'])}, {json.dumps(method['TypeSignature'])}}},\n");

output.write("};")
