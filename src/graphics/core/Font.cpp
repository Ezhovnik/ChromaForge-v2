#include "Font.h"

#include <limits.h>

#include "Texture.h"
#include "Batch2D.h"

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

Font::~Font(){
}

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
	return std::min(text.length() - offset, length) * 8;
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

static inline void drawGlyph(Batch2D* batch, int x, int y, uint c, FontStyle style) {
    switch (style) {
		case FontStyle::None:
			break;
        case FontStyle::Shadow:
			// Рисуем тень со смещением (1,1) чёрным цветом
            batch->sprite(
				x + 1, y + 1, 
				FontsConsts::GLYPH_SIZE,
				FontsConsts::GLYPH_SIZE,
				16,
				c,
				FontsConsts::SHADOW_TINT
			);
            break;
        case FontStyle::Outline:
			// Рисуем 8 соседних пикселей вокруг основного глифа для создания контура
            for (int oy = -1; oy <= 1; ++oy){
                for (int ox = -1; ox <= 1; ++ox){
                    if (ox || oy) {
                        batch->sprite(
							x + ox, y + oy, 
							FontsConsts::GLYPH_SIZE,
							FontsConsts::GLYPH_SIZE,
							16,
							c,
							FontsConsts::SHADOW_TINT
						);
                    }
                }
            }
            break;
    }
	// Рисуем основной глиф текущим цветом пакетного рендерера
    batch->sprite(x, y, FontsConsts::GLYPH_SIZE, FontsConsts::GLYPH_SIZE, 16, c, batch->getColor());
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y) {
	draw(batch, text, x, y, FontStyle::None);
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y, FontStyle style) {
    draw(batch, std::wstring_view(text.c_str(), text.length()), x, y, style);
}

void Font::draw(Batch2D* batch, std::wstring_view text, int x, int y, FontStyle style) {
    uint page = 0; // текущая обрабатываемая страница
    uint next = FontsConsts::MAX_CODEPAGES; // следующая страница, которую нужно обработать
    int init_x = x; // запоминаем начальную позицию X для каждой страницы
    do {
		// Проходим по всем символам строки
        for (uint c : text) {
			// Пропускаем пробельные символы
            if (!isPrintableChar(c)) {
                x += 8;
                continue;
            }
            uint charpage = c >> 8;
            if (charpage == page) {
				// Символ принадлежит текущей странице
                Texture* texture = pages[charpage].get();
                if (texture == nullptr) texture = pages[0].get();
                batch->texture(texture);
                drawGlyph(batch, x, y, c, style);
            } else if (charpage > page && charpage < next) {
                next = charpage;
            }
            x += 8; // смещаемся к следующему символу
        }

		// Переходим к следующей странице
        page = next;
        next = FontsConsts::MAX_CODEPAGES;
        x = init_x;
    } while (page < FontsConsts::MAX_CODEPAGES);
}
