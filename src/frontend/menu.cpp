#include <frontend/menu.h>

#include <filesystem>
#include <random>
#include <chrono>

#include <glm/glm.hpp>

#include <delegates.h>
#include <engine.h>
#include <files/engine_paths.h>
#include <graphics/ui/elements/display/Label.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/ui/gui_util.h>
#include <graphics/ui/GUI.h>
#include <logic/scripting/scripting.h>
#include <settings.h>
#include <util/stringutil.h>
#include <window/Window.h>
#include <frontend/UIDocument.h>
#include <core_content_defs.h>
#include <frontend/locale/langs.h>
#include <interfaces/Task.h>
#include <graphics/ui/elements/layout/Panel.h>
#include <coders/commons.h>
#include <frontend/screens/MenuScreen.h>

gui::page_loader_func menus::create_page_loader(Engine* engine) {
    return [=](const std::string& query) {
        std::vector<dv::value> args;

        std::string name;
        size_t index = query.find('?');
        if (index != std::string::npos) {
            auto argstr = query.substr(index+1);
            name = query.substr(0, index);

            auto map = dv::object();
            auto filename = "Query for " + name;
            BasicParser parser(filename, argstr);
            while (parser.hasNext()) {
                auto key = std::string(parser.readUntil('='));
                parser.nextChar();
                auto value = std::string(parser.readUntil('&'));
                map[key] = value;
            }
            args.emplace_back(map);
        } else {
            name = query;
        }
        auto file = engine->getResPaths()->find("layouts/pages/" + name + ".xml");
        auto fullname = BUILTIN_CONTENT_NAMESPACE + ":pages/" + name;
        auto document_ptr = UIDocument::read(
            scripting::get_root_environment(), fullname, file
        );
        auto document = document_ptr.get();
        engine->getAssets()->store(std::move(document_ptr), fullname);
        scripting::on_ui_open(document, std::move(args));
        return document->getRoot();
    };
}

void menus::create_version_label(Engine* engine) {
    auto gui = engine->getGUI();
    auto text = "v" + ENGINE_VERSION_STRING + " development build ";
    gui->add(guiutil::create(
        "<label z-index='1000' color='#FFFFFF80' gravity='top-right' margin='4'>"
        + text +
        "</label>"
    ));
}

UIDocument* menus::show(Engine* engine, const std::string& name, std::vector<dv::value> args) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/" + name + ".xml");
    auto fullname = BUILTIN_CONTENT_NAMESPACE + ":layouts/" + name;

    auto document_ptr = UIDocument::read(
        scripting::get_root_environment(), fullname, file
    );
    auto document = document_ptr.get();
    engine->getAssets()->store(std::move(document_ptr), fullname);
    scripting::on_ui_open(document, std::move(args));
    menu->addPage(name, document->getRoot());
    menu->setPage(name);

    return document;
}

void menus::show_process_panel(
    Engine* engine,
    const std::shared_ptr<Task>& task,
    const std::wstring& text
) {
    uint initialWork = task->getWorkTotal();

    auto menu = engine->getGUI()->getMenu();
    menu->reset();
    auto doc = menus::show(engine, "process", {
        util::wstr2str_utf8(langs::get(text))
    });
    std::dynamic_pointer_cast<gui::Container>(doc->getRoot())->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        scripting::on_ui_progress(doc, tasksDone, initialWork);
    });
}

bool menus::call(Engine* engine, runnable func) {
    try {
        func();
        return true;
    } catch (const contentpack_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            engine->getGUI(), langs::get(L"error.pack-not-found") + L": " +
            util::str2wstr_utf8(error.getPackId())
        );
        return false;
    } catch (const asset_loader::error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            engine->getGUI(), langs::get(L"Assets Load Error", L"menu") + L":\n" +
            util::str2wstr_utf8(error.what())
        );
        return false;
    } catch (const parsing_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(engine->getGUI(), util::str2wstr_utf8(error.errorLog()));
        return false;
    } catch (const std::runtime_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            engine->getGUI(), langs::get(L"Content Error", L"menu") + L":\n" +
            util::str2wstr_utf8(error.what())
        );
        return false;
    }
}
