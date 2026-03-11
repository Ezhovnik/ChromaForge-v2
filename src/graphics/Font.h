#ifndef GRAPHICS_FONT_H_
#define GRAPHICS_FONT_H_

#include <string>
#include <vector>
#include <memory>

#include "../typedefs.h"

class Texture;
class Batch2D;

/**
 * @brief Стили отрисовки текста (тени, контур).
 */
enum class FontStyle {
    None, Shadow, Outline
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
public:
	std::vector<std::unique_ptr<Texture>> pages; ///< Страницы текстуры шрифта

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

	/**
     * @brief Вычисляет ширину текста в пикселях.
     * @param text Текст (широкая строка).
     * @param length Максимальное количество символов для учёта (по умолчанию -1 — все).
     * @return Ширина в пикселях.
     */
	int calcWidth(std::wstring text, size_t length=-1);

	/**
     * @brief Проверяет, является ли символ печатным (не пробельным).
     * @param codepoint Код символа.
     * @return true, если символ должен отображаться.
     */
	bool isPrintableChar(uint codepoint) const;

	/**
     * @brief Рисует текст без дополнительного стиля.
     * @param batch Указатель на Batch2D.
     * @param text Текст.
     * @param x,y Позиция левого верхнего угла.
     */
	void draw(Batch2D* batch, std::wstring text, int x, int y);

	/**
     * @brief Рисует текст с указанным стилем.
     * @param batch Batch2D.
     * @param text Текст.
     * @param x,y Позиция.
     * @param style Стиль.
     */
    void draw(Batch2D* batch, std::wstring text, int x, int y, FontStyle style);

	/**
     * @brief Рисует текст с указанным стилем (перегрузка для std::wstring_view).
     * @param batch Batch2D.
     * @param text Текст.
     * @param x,y Позиция.
     * @param style Стиль.
     */
    void draw(Batch2D* batch, std::wstring_view text, int x, int y, FontStyle style);
};

#endif // GRAPHICS_FONT_H_
