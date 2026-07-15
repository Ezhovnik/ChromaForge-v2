#pragma once

#include <string>
#include <functional>
#include <memory>

#include <typedefs.h>
#include <io/fwd.h>

class Hud;
class WorldRenderer;
class PostProcessing;

namespace gui {
    class UINode;
    using PageLoaderFunc = std::function<std::shared_ptr<UINode>(const std::string&)>;
}

namespace scripting {
    extern Hud* hud;
    extern WorldRenderer* renderer;
    extern PostProcessing* post_processing;

    void on_frontend_init(
        Hud* hud, WorldRenderer* renderer, PostProcessing* postProcessing
    );
    void on_frontend_close();
    void on_frontend_render();

    void load_hud_script(
        const scriptenv& env,
        const std::string& packid,
        const io::path& file,
        const std::string& fileName
    );

    gui::PageLoaderFunc create_page_loader();
}
