#include <graphics/core/Font.h>

#include <limits.h>
#include <utility>

#include <graphics/core/Texture.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/Batch3D.h>
#include <window/Camera.h>

// Внутренние константы для работы со шрифтом
namespace FontsConsts {
    inline constexpr int GLYPH_SIZE = 16; ///< Размер одного глифа в пикселях (16x16)
	inline constexpr int MAX_CODEPAGES = INT_MAX; ///< Максимальный номер страницы (используется как ограничитель)
	inline constexpr glm::vec4 SHADOW_TINT(0.0f, 0.0f, 0.0f, 1.0f); ///< Цвет тени (чёрный, непрозрачный)
}

Font::Font(
	std::vector<std::unique_ptr<Texture>> pages, 
	int lineHeight, 
	int yoffset
) : pages(std::move(pages)), 
	lineHeight(lineHeight), 
	yoffset(yoffset) {}

Font::~Font() = default;

int Font::getYOffset() const {
    return yoffset;
}

int Font::getLineHeight() const {
	return lineHeight;
}

int Font::calcWidth(const std::wstring& text, size_t length) const {
	return calcWidth(text, 0, length);
}

int Font::calcWidth(const std::wstring& text, size_t offset, size_t length) const {
	return std::min(text.length() - offset, length) * glyphInterval;
}

bool Font::isPrintableChar(uint codepoint) const {
	switch (codepoint) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case '\r':
			return false;
		default:
			return true;
	}
}

static inline void draw_glyph(
    Batch2D& batch,
    const glm::vec3& pos,
    const glm::vec2& offset,
    uint c,
    const glm::vec3& right,
    const glm::vec3& up,
    float glyphInterval
) {
    batch.sprite(
        pos.x + offset.x * right.x,
        pos.y + offset.y * right.y,
        right.x / glyphInterval,
        up.y,
        16,
        c,
        batch.getColor()
    );
}

static inline void draw_glyph(
    Batch3D& batch,
    const glm::vec3& pos,
    const glm::vec2& offset,
    uint c,
    const glm::vec3& right,
    const glm::vec3& up,
    float glyphInterval
) {
    batch.sprite(
        pos + right * offset.x + up * offset.y,
        up, right / glyphInterval,
        0.5f,
        0.5f,
        16,
        c,
        glm::vec4(1.0f)
    );
}

template <class Batch>
static inline void draw_text(
    const Font& font,
    Batch& batch,
    std::wstring_view text,
    const glm::vec3& pos,
    const glm::vec3& right,
    const glm::vec3& up,
    float glyphInterval
) {
    uint page = 0; // текущая обрабатываемая страница
    uint next = FontsConsts::MAX_CODEPAGES; // следующая страница, которую нужно обработать
    int x = 0;
    int y = 0;
    do {
		// Проходим по всем символам строки
        for (uint c : text) {
			// Пропускаем пробельные символы
            if (!font.isPrintableChar(c)) {
                x++;
                continue;
            }
            uint charpage = c >> 8;
            if (charpage == page) {
				batch.texture(font.getPage(charpage));
                draw_glyph(
                    batch, pos, glm::vec2(x, y), c, right, up, glyphInterval
                );
            } else if (charpage > page && charpage < next) {
                next = charpage;
            }
            x++;
        }

		// Переходим к следующей странице
        page = next;
        next = FontsConsts::MAX_CODEPAGES;
        x = 0;
    } while (page < FontsConsts::MAX_CODEPAGES);
}

const Texture* Font::getPage(int charpage) const {
    Texture* texture = nullptr;
    if (charpage < pages.size()) {
        texture = pages[charpage].get();
    }
    if (texture == nullptr){
        texture = pages[0].get();
    }
    return texture;
}

void Font::draw(
    Batch2D& batch, std::wstring_view text, int x, int y, float scale
) const {
    draw_text(
        *this,
        batch,
        text,
        glm::vec3(x, y, 0),
        glm::vec3(glyphInterval * scale, 0, 0),
        glm::vec3(0, lineHeight * scale, 0),
        glyphInterval / static_cast<float>(lineHeight)
    );
}

void Font::draw(
    Batch3D& batch,
    std::wstring_view text,
    const glm::vec3& pos,
    const glm::vec3& right,
    const glm::vec3& up
) const {
    draw_text(
        *this,
        batch,
        text,
        pos,
        right * static_cast<float>(glyphInterval),
        up * static_cast<float>(lineHeight),
        glyphInterval / static_cast<float>(lineHeight)
    );
}
