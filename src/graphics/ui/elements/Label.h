#pragma once

#include <graphics/ui/elements/UINode.h>
#include <graphics/core/FontMetrics.h>

class Font;
struct FontStylesScheme;

namespace gui {
    struct LineScheme {
        size_t offset;
        bool fake;
    };

    struct LabelCache {
        FontMetrics metrics;

        std::vector<LineScheme> lines;
        bool resetFlag = true;
        size_t wrapWidth = -1;
        int multilineWidth = 0;

        void prepare(
            const std::shared_ptr<Font>& font,
            FontMetrics metrics,
            size_t wrapWidth
        );
        void update(std::wstring_view text, bool multiline, bool wrap);

        size_t getTextLineOffset(size_t line) const;
        uint getLineByTextIndex(size_t index) const;
    };

    class Label final : public UINode {
    private:
        LabelCache cache;

        glm::vec2 calcSize();
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
        float lineInterval = 1.5f;
        Align valign = Align::Center;

        bool multiline = false;
        bool textWrap = true;
        bool autoresize = false;

        int textYOffset = 0;
        int totalLineHeight = 1;

        std::string markup;

        std::unique_ptr<FontStylesScheme> styles;
    public:
        Label(GUI& gui, const std::string& text, std::string fontName="normal");
        Label(GUI& gui, const std::wstring& text, std::string fontName="normal");

        ~Label();

        void setText(std::wstring text);
        const std::wstring& getText() const;

        void setFontName(std::string name);
        const std::string& getFontName() const;

        void setVerticalAlign(Align align);
        Align getVerticalAlign() const;

        float getLineInterval() const;

        void setLineInterval(float interval);

        int getTextYOffset() const;

        int getLineYOffset(uint line) const;

        size_t getTextLineOffset(size_t line) const;

        uint getLineByYOffset(int offset) const;
        uint getLineByTextIndex(size_t index) const;
        uint getLinesNumber() const;

        bool isFakeLine(size_t line) const;

        void draw(const DrawContext& pctx, const Assets& assets) override;

        void textSupplier(wstringsupplier supplier);

        void setMultiline(bool multiline);
        bool isMultiline() const;

        void setAutoResize(bool flag);
        bool isAutoResize() const;

        void setTextWrapping(bool flag);
        bool isTextWrapping() const;

        void setMarkup(std::string_view lang);
        const std::string& getMarkup() const;

        void setStyles(std::unique_ptr<FontStylesScheme> styles);
    };
}
