#include <string>
#include <memory>
#include <sstream>
#include <bitset>
#include <utility>

#include <graphics/ui/elements/CheckBox.h>
#include <graphics/ui/elements/TextBox.h>
#include <graphics/ui/elements/TrackBar.h>
#include <graphics/ui/elements/InputBindBox.h>
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
#include <engine/Engine.h>
#include <graphics/render/WorldRenderer.h>
#include <audio/audio.h>
#include <settings.h>
#include <logic/scripting/scripting.h>
#include <objects/Entities.h>
#include <content/Content.h>
#include <objects/Entity.h>
#include <frontend/hud.h>
#include <graphics/render/ParticlesRenderer.h>
#include <graphics/render/ChunksRenderer.h>
#include <voxels/GlobalChunks.h>
#include <objects/Players.h>
#include <network/Network.h>

static std::shared_ptr<gui::Label> create_label(gui::GUI& gui, wstringsupplier supplier) {
    auto label = std::make_shared<gui::Label>(gui, L"-");
    label->textSupplier(std::move(supplier));
    return label;
}

std::shared_ptr<gui::UINode> create_debug_panel(
    Engine& engine,
    Level& level,
    Player& player,
    bool allowDebugCheats
) {
    auto& gui = engine.getGUI();
	auto panel = std::make_shared<gui::Panel>(
        gui, glm::vec2(350, 200), glm::vec4(5.0f), 2.0f
    );
    panel->setId("hud.debug-panel");
    panel->setPos(glm::vec2(10, 10));

    static int fps = 0;
    static int fpsMin = fps;
    static int fpsMax = fps;
    static std::wstring fpsString = L"FPS: - / -";

    static size_t lastTotalDownload = 0;
    static size_t lastTotalUpload = 0;
    static std::wstring netSpeedString = L"Download: - B/s upload: - B/s";

    panel->listenInterval(0.016f, [&engine]() {
        fps = 1.0f / engine.getTime().getDeltaTime();
        fpsMin = std::min(fps, fpsMin);
        fpsMax = std::max(fps, fpsMax);
    });

    panel->listenInterval(0.5f, []() {
        fpsString = std::to_wstring(fpsMax) + L" / " + std::to_wstring(fpsMin);
        fpsMin = fps;
        fpsMax = fps;
    });

    panel->listenInterval(1.0f, [&engine]() {
        const auto& network = engine.getNetwork();
        size_t totalDownload = network.getTotalDownload();
        size_t totalUpload = network.getTotalUpload();
        netSpeedString =
            L"Download: " + std::to_wstring(totalDownload - lastTotalDownload) +
            L" B/s upload: " + std::to_wstring(totalUpload - lastTotalUpload) +
            L" B/s";
        lastTotalDownload = totalDownload;
        lastTotalUpload = totalUpload;
    });

	panel->add(std::shared_ptr<gui::Label>(create_label(gui, [](){
		return L"FPS: " + fpsString;
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label(gui, [](){
		return L"Meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
    panel->add(create_label(gui, [](){
        int drawCalls = Mesh::drawCalls;
        Mesh::drawCalls = 0;
        return L"Draw-calls: " + std::to_wstring(drawCalls);
    }));
    panel->add(std::shared_ptr<gui::Label>(create_label(gui, [&](){
		return L"Chunks: " + std::to_wstring(level.chunks->size()) + L" (visible: " + std::to_wstring(ChunksRenderer::visibleChunks) + L")";
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label(gui, [=](){
		return L"Particles: " +
                std::to_wstring(ParticlesRenderer::visibleParticles) +
                L" Emitters: " +
                std::to_wstring(ParticlesRenderer::aliveEmitters);
	})));
    panel->add(create_label(gui, [&]() {
        return L"Entities: " + std::to_wstring(level.entities->size()) +
        L" Next: " + std::to_wstring(level.entities->peekNextID());
    }));
    panel->add(create_label(gui, [&]() {
        return L"Players: " + std::to_wstring(level.players->size()) +
        L" Local: " + std::to_wstring(player.getId());
    }));
    panel->add(create_label(gui, [](){
        return L"Speakers: " + std::to_wstring(audio::count_speakers()) + L" Streams: " + std::to_wstring(audio::count_streams());
    }));
    panel->add(create_label(gui, [](){
        return L"Lua stack: " + std::to_wstring(scripting::get_values_on_stack());
    }));
    panel->add(create_label(gui, []() {return netSpeedString;}));
	panel->add(std::shared_ptr<gui::Label>(create_label(gui, [&]() -> std::wstring{
        const auto& vox = player.selection.vox;
        std::wstringstream stream;
        stream << "R:" << vox.state.rotation << " S:"
            << std::bitset<3>(vox.state.segment) << " U:"
            << std::bitset<8>(vox.state.userbits);
        if (vox.id == BLOCK_VOID) {
            return L"Block: -";
        } else {
            return L"Block: " + std::to_wstring(vox.id) + L" " + stream.str();
        }
	})));
    panel->add(create_label(gui, [&]() -> std::wstring {
        const auto& selection = player.selection;
        const auto& vox = selection.vox;
        if (vox.id == BLOCK_VOID) {
            return L"x: - y: - z: -";
        }
        return L"x: " + std::to_wstring(selection.actualPosition.x) +
            L" y: " + std::to_wstring(selection.actualPosition.y) +
            L" z: " + std::to_wstring(selection.actualPosition.z);
    }));
    panel->add(create_label(gui, [&](){
        auto indices = level.content.getIndices();
        if (auto def = indices->blocks.get(player.selection.vox.id)) {
            return L"Name: " + util::str2wstr_utf8(def->name);
        } else {
            return std::wstring {L"Name: void"};
        }
    }));
    panel->add(create_label(gui, [&]() {
        auto eid = player.getSelectedEntity();
        if (eid == ENTITY_NONE) {
            return std::wstring {L"Entity: -"};
        } else if (auto entity = level.entities->get(eid)) {
            return L"Entity: " + util::str2wstr_utf8(entity->getDef().name) + L" (UID: "+std::to_wstring(entity->getUID()) + L")";
        } else {
            return std::wstring {L"Entity: error (invalid UID)"};
        }
    }));
	panel->add(std::shared_ptr<gui::Label>(create_label(gui, [&](){
		return L"Seed: " + std::to_wstring(level.getWorld()->getSeed());
	})));
	for (int ax = 0; ax < 3; ++ax){
		auto sub = std::make_shared<gui::Container>(
            gui, glm::vec2(350, 27)
        );

        std::wstring str = L"x: ";
        str[0] += ax;
        auto label = std::make_shared<gui::Label>(
            gui, str
        );
        label->setMargin(glm::vec4(2, 3, 2, 3));
        label->setSize(glm::vec2(20, 27));
        sub->add(label);
        sub->setColor(glm::vec4(0.0f));

        auto box = std::make_shared<gui::TextBox>(gui, L"");
        auto boxRef = box.get();
        box->setTextSupplier([&player, ax]() {
            return std::to_wstring(static_cast<int>(player.getPosition()[ax]));
        });
        if (allowDebugCheats) {
            box->setTextConsumer([&player, ax](const std::wstring& text) {
                try {
                    glm::vec3 position = player.getPosition();
                    position[ax] = std::stoi(text);
                    player.teleport(position);
                } catch (std::exception& _){
                }
            });
        }
        box->setOnEditStart([&player, boxRef, ax]() {
            boxRef->setText(
                std::to_wstring(static_cast<int>(player.getPosition()[ax]))
            );
        });
        box->setSize(glm::vec2(230, 27));

        sub->add(box, glm::vec2(20, 0));
        panel->add(sub);
	}

    auto& worldInfo = level.getWorld()->getInfo();

	panel->add(std::shared_ptr<gui::Label>(create_label(gui, [&](){
		std::wstringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << worldInfo.skyClearness;
		return L"Sky clearness: " + ss.str();
	})));

	if (allowDebugCheats) {
		auto bar = std::make_shared<gui::TrackBar>(
            gui, 0.0f, 1.0f, 0.0f, 0.005f, 8
        );
		bar->setSupplier([&]() {
			return worldInfo.skyClearness;
		});
		bar->setConsumer([&](double val) {
			worldInfo.skyClearness = val;
		});
		panel->add(bar);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label(gui, [&](){
		int hour, minute, second;
		timeutil::from_value(worldInfo.daytime, hour, minute, second);

		std::wstring timeString = 
					util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
					util::lfill(std::to_wstring(minute), 2, L'0');
		return L"Time: " + timeString;
	})));

	if (allowDebugCheats) {
		auto bar = std::make_shared<gui::TrackBar>(
            gui, 0.0f, 1.0f, 1.0f, 0.005f, 8
        );
		bar->setSupplier([&]() {
			return worldInfo.daytime;
		});
		bar->setConsumer([&](double val) {
			worldInfo.daytime = val;
		});
		panel->add(bar);
	}

	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(
            gui, L"Frustum-Culling", glm::vec2(400, 24)
        );
        checkbox->setSupplier([&engine]() {
            return engine.getSettings().graphics.frustumCulling.get();
        });
        checkbox->setConsumer([&engine](bool checked) {
            engine.getSettings().graphics.frustumCulling.set(checked);
        });
        panel->add(checkbox);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(
            gui, L"Show Chunk Borders", glm::vec2(400, 24)
        );
        checkbox->setSupplier([=]() {
            return WorldRenderer::drawChunkBorders;
        });
        checkbox->setConsumer([=](bool checked) {
            WorldRenderer::drawChunkBorders = checked;
        });
		panel->add(checkbox);
	}
    {
        auto checkbox = std::make_shared<gui::FullCheckBox>(
            gui, L"Show Hitboxes", glm::vec2(400, 24)
        );
        checkbox->setSupplier([=]() {
            return WorldRenderer::drawEntityHitboxes;
        });
        checkbox->setConsumer([=](bool checked) {
            WorldRenderer::drawEntityHitboxes = checked;
        });
		panel->add(checkbox);
	}
    {
        auto checkbox = std::make_shared<gui::FullCheckBox>(
            gui, L"Show Generator Minimap", glm::vec2(400, 24)
        );
        checkbox->setSupplier([=]() {
            return Hud::showGeneratorMinimap;
        });
        checkbox->setConsumer([=](bool checked) {
            Hud::showGeneratorMinimap = checked;
        });
        panel->add(checkbox);
    }

	panel->refresh();
	return panel;
}
