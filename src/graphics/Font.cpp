#include "Font.h"

#include <limits.h>

#include "Texture.h"
#include "Batch2D.h"

namespace Fonts_Consts {
    constexpr int GLYPH_SIZE = 16;
}

Font::Font(std::vector<Texture*> pages, int lineHeight) : lineHeight_(lineHeight), pages(pages) {
}

Font::~Font(){
	for (Texture* texture : pages) {
		delete texture;
    }
}

// int Font::getGlyphWidth(char c) {
// 	switch (c){
// 		case 'l':
// 		case 'i':
// 		case 'j':
// 		case '|':
// 		case '.':
// 		case ',':
// 		case ':':
// 		case ';': return 7;
// 		case 't': return 8;
// 		case ' ': return 7;
// 	}
// 	return 7;
// }

int Font::lineHeight() const {
    return lineHeight_;
}

bool Font::isPrintableChar(int c) {
	switch (c){
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

int Font::calcWidth(std::wstring text) {
    return text.length() * 8;
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y) {
	draw(batch, text, x, y, FONT_STYLES::NONE);
}

void Font::draw(Batch2D* batch, std::wstring text, int x, int y, int style) {
	int page = 0;
	int next = INT_MAX;
	int init_x = x;
	do {
		for (unsigned c : text){
			if (isPrintableChar(c)){
				int charpage = c >> 8;
				if (charpage == page){
                    Texture* texture = pages[charpage];
                    if (texture == nullptr) texture = pages[0];
					batch->texture(pages[charpage]);

					switch (style){
						case FONT_STYLES::SHADOW:
							batch->sprite(x+1, y+1, Fonts_Consts::GLYPH_SIZE, Fonts_Consts::GLYPH_SIZE, 16, c, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
							break;
						case FONT_STYLES::OUTLINE:
							for (int oy = -1; oy <= 1; oy++){
								for (int ox = -1; ox <= 1; ox++){
									if (ox || oy) batch->sprite(x+ox, y+oy, Fonts_Consts::GLYPH_SIZE, Fonts_Consts::GLYPH_SIZE, 16, c, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
								}
							}
							break;
					}

					batch->sprite(x, y, Fonts_Consts::GLYPH_SIZE, Fonts_Consts::GLYPH_SIZE, 16, c, batch->color);
				} else if (charpage > page && charpage < next){
					next = charpage;
				}
			}
			// x += getGlyphWidth(c);
            x += 8;
		}
		page = next;
		next = INT_MAX;
		x = init_x;
	} while (page < INT_MAX);
}
