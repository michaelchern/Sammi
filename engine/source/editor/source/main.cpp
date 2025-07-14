#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "runtime/engine.h"
#include "editor/include/editor.h"

#define SAMMI_XSTR(s) SAMMI_STR(s)
#define SAMMI_STR(s) #s

int main(int argc, char** argv)
{
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "PiccoloEditor.ini";

    Sammi::SammiEngine* engine = new Sammi::SammiEngine();

    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    Sammi::SammiEditor* editor = new Sammi::SammiEditor();
    editor->initialize(engine);

    editor->run();

    editor->clear();

    engine->clear();
    engine->shutdownEngine();

    return 0;
}