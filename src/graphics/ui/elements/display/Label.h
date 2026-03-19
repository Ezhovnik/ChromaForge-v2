#ifndef GRAPHICS_UI_ELEMENTS_LABEL_H_
#define GRAPHICS_UI_ELEMENTS_LABEL_H_

#include "../UINode.h"

namespace gui {
    class Label : public UINode {
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
        uint lines = 1;
        float lineInterval = 1.5f;
        Align valign = Align::center;

        bool multiline = false;

        int textYOffset = 0;

        int totalLineHeight = 1;
    public:
        Label(std::string text, std::string fontName="normal");
        Label(std::wstring text, std::string fontName="normal");

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

        virtual size_t getTextLineOffset(uint line) const;

        virtual uint getLineByYOffset(int offset) const;
        virtual uint getLineByTextIndex(size_t index) const;
        virtual uint getLinesNumber() const;

        virtual void draw(const GfxContext* pctx, Assets* assets) override;

        virtual void textSupplier(wstringsupplier supplier);

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_LABEL_H_
