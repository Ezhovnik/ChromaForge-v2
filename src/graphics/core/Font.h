#pragma once

#include <string>
#include <vector>
#include <memory>

#include <typedefs.h>

class Texture;
class Batch2D;

/**
 * @brief Стили отрисовки текста (тени, контур).
 */
enum class FontStyle {
     None, 
     Shadow, 
     Outline
};

/**
 * @brief Класс для работы с растровыми шрифтами.
 *
 * Шрифт состоит из нескольких страниц (текстур), каждая из которых содержит
 * 16x16 глифов (256 глифов на страницу). Код символа определяет страницу
 * (старший байт) и индекс глифа (младший байт).
 */
class Font {
private:
     int lineHeight; ///< Высота строки в пикселях
	int yoffset; ///< Смещение по Y (для коррекции позиции)
     int glyphInterval = 8;
	std::vector<std::unique_ptr<Texture>> pages; ///< Страницы текстуры шрифта
public:
	/**
     * @brief Конструктор.
     * @param pages Вектор страниц (текстур). Вектор будет перемещён.
     * @param lineHeight Высота строки.
     * @param yoffset Смещение по вертикали.
     */
	Font(std::vector<std::unique_ptr<Texture>> pages, int lineHeight, int yoffset);

	~Font();

	/**
     * @brief Возвращает высоту строки.
     */
     int getLineHeight() const;

	/**
     * @brief Возвращает вертикальное смещение.
     */
     int getYOffset() const;

	int calcWidth(const std::wstring& text, size_t length=-1);
     int calcWidth(const std::wstring& text, size_t offset, size_t length);

	/**
     * @brief Проверяет, является ли символ печатным (не пробельным).
     * @param codepoint Код символа.
     * @return true, если символ должен отображаться.
     */
	bool isPrintableChar(uint codepoint) const;

	void draw(Batch2D* batch, std::wstring_view text, int x, int y);
};
