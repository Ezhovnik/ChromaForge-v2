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
        ptrdiff_t fontId = 0;
        FontMetrics metrics;

        std::vector<LineScheme> lines;
        bool resetFlag = true;
        size_t wrapWidth = -1;
        int multilineWidth = 0;

        void prepare(std::ptrdiff_t fontId, FontMetrics metrics, size_t wrapWidth);
        void update(std::wstring_view text, bool multiline, bool wrap);

        size_t getTextLineOffset(size_t line) const;
        uint getLineByTextIndex(size_t index) const;
    };

    class Label : public UINode {
    private:
        LabelCache cache;

        glm::vec2 calcSize();
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
        float lineInterval = 1.5f;
        Align valign = Align::center;

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

        virtual ~Label();

        virtual void setText(std::wstring text);
        const std::wstring& getText() const;

        virtual void setFontName(std::string name);
        virtual const std::string& getFontName() const;

        virtual void setVerticalAlign(Align align);
        virtual Align getVerticalAlign() const;

        virtual float getLineInterval() const;

        virtual void setLineInterval(float interval);

        virtual int getTextYOffset() const;

        virtual int getLineYOffset(uint line) const;

        virtual size_t getTextLineOffset(size_t line) const;

        virtual uint getLineByYOffset(int offset) const;
        virtual uint getLineByTextIndex(size_t index) const;
        virtual uint getLinesNumber() const;

        virtual bool isFakeLine(size_t line) const;

        virtual void draw(const DrawContext& pctx, const Assets& assets) override;

        virtual void textSupplier(wstringsupplier supplier);

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;

        virtual void setTextWrapping(bool flag);
        virtual bool isTextWrapping() const;

        virtual void setMarkup(std::string_view lang);
        virtual const std::string& getMarkup() const;

        virtual void setStyles(std::unique_ptr<FontStylesScheme> styles);
    };
}
