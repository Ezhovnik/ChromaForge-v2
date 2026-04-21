#ifndef GRAPHICS_UI_ELEMENTS_LAYOUT_MENU_H_
#define GRAPHICS_UI_ELEMENTS_LAYOUT_MENU_H_

#include <stack>

#include <graphics/ui/elements/layout/Container.h>

namespace gui {
    struct Page {
        std::string name;
        std::shared_ptr<UINode> panel = nullptr;
    };

    using page_loader_func = std::function<std::shared_ptr<UINode>(const std::string& name)>;

    class Menu : public Container {
    protected:
        std::unordered_map<std::string, Page> pages;
        std::stack<Page> pageStack;
        Page current;
        std::unordered_map<std::string, supplier<std::shared_ptr<UINode>>> pageSuppliers;
        page_loader_func pagesLoader = nullptr;
    public:
        Menu();

        bool has(const std::string& name);

        void setPage(const std::string& name, bool history=true);
        void setPage(Page page, bool history=true);
        void addPage(
            const std::string& name,
            const std::shared_ptr<UINode>& panel
        );
        std::shared_ptr<UINode> fetchPage(const std::string& name);

        void addSupplier(
            const std::string& name,
            const supplier<std::shared_ptr<UINode>>& pageSupplier
        );

        void setPageLoader(page_loader_func loader);

        void back();

        void clearHistory();

        void reset();

        Page& getCurrent();
    };
}

#endif // GRAPHICS_UI_ELEMENTS_LAYOUT_MENU_H_
