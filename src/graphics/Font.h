#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>
#include <vector>
#include <memory>

class Texture;
class Batch2D;

enum class FontStyle {
    None, Shadow, Outline
};

class Font {
private:
    int lineHeight;
	int yoffset;
public:
	std::vector<std::unique_ptr<Texture>> pages;

	Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset);
	~Font();

    int getLineHeight() const;
    int getYOffset() const;
	int calcWidth(std::wstring text, size_t length=-1);

	bool isPrintableChar(int c);
	void draw(Batch2D* batch, std::wstring text, int x, int y);
	void draw(Batch2D* batch, std::wstring text, int x, int y, FontStyle style);
};

#endif // GRAPHICS_FONT_H_
