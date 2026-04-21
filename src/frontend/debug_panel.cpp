#include <string>
#include <memory>
#include <sstream>
#include <bitset>
#include <utility>

#include <graphics/ui/elements/control/CheckBox.h>
#include <graphics/ui/elements/control/TextBox.h>
#include <graphics/ui/elements/control/TrackBar.h>
#include <graphics/ui/elements/control/InputBindBox.h>
#include <graphics/core/Mesh.h>
#include <objects/Player.h>
#include <physics/Hitbox.h>
#include <world/Level.h>
#include <world/World.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/Block.h>
#include <util/stringutil.h>
#include <delegates.h>
#include <engine.h>
#include <graphics/render/WorldRenderer.h>
#include <audio/audio.h>
#include <settings.h>
#include <logic/scripting/scripting.h>
#include <objects/Entities.h>
#include <content/Content.h>
#include <objects/Entity.h>

static std::shared_ptr<gui::Label> create_label(wstringsupplier supplier) {
    auto label = std::make_shared<gui::Label>(L"-");
    label->textSupplier(std::move(supplier));
    return label;
}

std::shared_ptr<gui::UINode> create_debug_panel(Engine* engine, Level* level, Player* player) {
	auto panel = std::make_shared<gui::Panel>(glm::vec2(350, 200), glm::vec4(5.0f), 2.0f);
    panel->setId("hud.debug-panel");
    panel->setPos(glm::vec2(10, 10));

    static int fps = 0;
    static int fpsMin = fps;
    static int fpsMax = fps;
    static std::wstring fpsString = L"";

    panel->listenInterval(0.016f, [engine]() {
        fps = 1.0f / engine->getDeltaTime();
        fpsMin = std::min(fps, fpsMin);
        fpsMax = std::max(fps, fpsMax);
    });

    panel->listenInterval(0.5f, []() {
        fpsString = std::to_wstring(fpsMax) + L" / " + std::to_wstring(fpsMin);
        fpsMin = fps;
        fpsMax = fps;
    });
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"FPS: " + fpsString;
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
    panel->add(create_label([](){
        int drawCalls = Mesh::drawCalls;
        Mesh::drawCalls = 0;
        return L"Draw-calls: " + std::to_wstring(drawCalls);
    }));
    panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Chunks: " + std::to_wstring(level->chunks->chunksCount) + L" (visible: " + std::to_wstring(level->chunks->visibleCount) + L")";
	})));
    panel->add(create_label([=]() {
        return L"Entities: " + std::to_wstring(level->entities->size()) +
        L" Next: " + std::to_wstring(level->entities->peekNextID());
    }));
    panel->add(create_label([](){
        return L"Speakers: " + std::to_wstring(audio::count_speakers()) + L" Streams: " + std::to_wstring(audio::count_streams());
    }));
    panel->add(create_label([](){
        return L"Lua stack: " + std::to_wstring(scripting::get_values_on_stack());
    }));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
        const auto& vox = player->selection.vox;
        std::wstringstream stream;
        stream << "R:" << vox.state.rotation << " S:"
            << std::bitset<3>(vox.state.segment) << " U:"
            << std::bitset<8>(vox.state.userbits);
        if (vox.id == BLOCK_VOID) {
            return std::wstring {L"Block: -"};
        } else {
            return L"Block: " + std::to_wstring(vox.id) + L" " + stream.str();
        }
	})));
    panel->add(create_label([=](){
        auto* indices = level->content->getIndices();
        if (auto def = indices->blocks.get(player->selection.vox.id)) {
            return L"Name: " + util::str2wstr_utf8(def->name);
        } else {
            return std::wstring {L"Name: void"};
        }
    }));
    panel->add(create_label([=]() {
        auto eid = player->getSelectedEntity();
        if (eid == ENTITY_NONE) {
            return std::wstring {L"Entity: -"};
        } else if (auto entity = level->entities->get(eid)) {
            return L"Entity: " + util::str2wstr_utf8(entity->getDef().name) + L" (UID: "+std::to_wstring(entity->getUID()) + L")";
        } else {
            return std::wstring {L"Entity: error (invalid UID)"};
        }
    }));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Seed: " + std::to_wstring(level->getWorld()->getSeed());
	})));
	for (int ax = 0; ax < 3; ++ax){
		auto sub = std::make_shared<gui::Container>(glm::vec2(350, 27));

        std::wstring str = L"x: ";
        str[0] += ax;
        auto label = std::make_shared<gui::Label>(str);
        label->setMargin(glm::vec4(2, 3, 2, 3));
        label->setSize(glm::vec2(20, 27));
        sub->add(label);
        sub->setColor(glm::vec4(0.0f));

        auto box = std::make_shared<gui::TextBox>(L"");
        auto boxRef = box.get();
        box->setTextSupplier([=]() {
            return std::to_wstring(static_cast<int>(player->getPosition()[ax]));
        });
        box->setTextConsumer([=](const std::wstring& text) {
            try {
                glm::vec3 position = player->getPosition();
                position[ax] = std::stoi(text);
                player->teleport(position);
            } catch (std::exception& _) {
            }
        });
        box->setOnEditStart([=](){
            boxRef->setText(std::to_wstring(static_cast<int>(player->getPosition()[ax])));
        });
        box->setSize(glm::vec2(230, 27));

        sub->add(box, glm::vec2(20, 0));
        panel->add(sub);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		std::wstringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << level->getWorld()->skyClearness;
		return L"Sky clearness: " + ss.str();
	})));

	{
		auto bar = std::make_shared<gui::TrackBar>(0.0f, 1.0f, 0.0f, 0.005f, 8);
		bar->setSupplier([=]() {
			return level->getWorld()->skyClearness;
		});
		bar->setConsumer([=](double val) {
			level->getWorld()->skyClearness = val;
		});
		panel->add(bar);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		int hour, minute, second;
		timeutil::from_value(level->getWorld()->daytime, hour, minute, second);

		std::wstring timeString = 
					util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
					util::lfill(std::to_wstring(minute), 2, L'0');
		return L"Time: " + timeString;
	})));

	{
		auto bar = std::make_shared<gui::TrackBar>(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->setSupplier([=]() {
			return level->getWorld()->daytime;
		});
		bar->setConsumer([=](double val) {
			level->getWorld()->daytime = val;
		});
		panel->add(bar);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Frustum-Culling", glm::vec2(400, 24));
        checkbox->setSupplier([=]() {
            return engine->getSettings().graphics.frustumCulling.get();
        });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().graphics.frustumCulling.set(checked);
        });
        panel->add(checkbox);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Show Chunk Borders", glm::vec2(400, 24));
        checkbox->setSupplier([=]() {
            return WorldRenderer::drawChunkBorders;
        });
        checkbox->setConsumer([=](bool checked) {
            WorldRenderer::drawChunkBorders = checked;
        });
		panel->add(checkbox);
	}
    {
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Show Hitboxes", glm::vec2(400, 24));
        checkbox->setSupplier([=]() {
            return WorldRenderer::drawEntityHitboxes;
        });
        checkbox->setConsumer([=](bool checked) {
            WorldRenderer::drawEntityHitboxes = checked;
        });
		panel->add(checkbox);
	}

	panel->refresh();
	return panel;
}
