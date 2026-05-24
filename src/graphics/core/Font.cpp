#include <graphics/core/Font.h>

#include <limits.h>
#include <utility>

#include <graphics/core/Texture.h>
#include <graphics/core/Batch2D.h>

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

int Font::calcWidth(const std::wstring& text, size_t length) {
	return calcWidth(text, 0, length);
}

int Font::calcWidth(const std::wstring& text, size_t offset, size_t length) {
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

static inline void drawGlyph(
    Batch2D* batch, int x, int y, uint c, int glyphSize
) {
    batch->sprite(x, y, glyphSize, glyphSize, 16, c, batch->getColor());
}

void Font::draw(Batch2D* batch, std::wstring_view text, int x, int y) {
    uint page = 0; // текущая обрабатываемая страница
    uint next = FontsConsts::MAX_CODEPAGES; // следующая страница, которую нужно обработать
    int init_x = x; // запоминаем начальную позицию X для каждой страницы
    do {
		// Проходим по всем символам строки
        for (uint c : text) {
			// Пропускаем пробельные символы
            if (!isPrintableChar(c)) {
                x += glyphInterval;
                continue;
            }
            uint charpage = c >> 8;
            if (charpage == page) {
				// Символ принадлежит текущей странице
                Texture* texture = nullptr;
                if (charpage < pages.size()) {
                    texture = pages[charpage].get();
                }
                if (texture == nullptr) texture = pages[0].get();
                batch->texture(texture);
                drawGlyph(batch, x, y, c, lineHeight);
            } else if (charpage > page && charpage < next) {
                next = charpage;
            }
            x += glyphInterval;
        }

		// Переходим к следующей странице
        page = next;
        next = FontsConsts::MAX_CODEPAGES;
        x = init_x;
    } while (page < FontsConsts::MAX_CODEPAGES);
}
