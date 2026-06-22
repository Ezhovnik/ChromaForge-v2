#pragma once

#include <stack>

#include <graphics/ui/elements/layout/Container.h>

namespace gui {
    struct Page {
        std::string name;
        std::shared_ptr<UINode> panel;
        bool temporal = false;
    };

    using PageLoaderFunc = std::function<std::shared_ptr<UINode>(const std::string&)>;

    class Menu : public Container {
    protected:
        std::unordered_map<std::string, Page> pages;
        std::stack<Page> pageStack;
        Page current;
        std::unordered_map<std::string, supplier<std::shared_ptr<UINode>>> pageSuppliers;
        PageLoaderFunc pagesLoader = nullptr;
    public:
        Menu();

        bool has(const std::string& name);

        void setPage(const std::string& name, bool history=true);
        void setPage(Page page, bool history=true);
        void addPage(
            const std::string& name,
            const std::shared_ptr<UINode>& panel,
            bool temporal = false
        );
        void removePage(const std::string& name);
        Page fetchPage(const std::string& name);

        void addSupplier(
            const std::string& name,
            const supplier<std::shared_ptr<UINode>>& pageSupplier
        );

        void setPageLoader(PageLoaderFunc loader);
        PageLoaderFunc getPageLoader();

        bool back();

        void clearHistory();

        void reset();

        Page& getCurrent();

        bool hasOpenPage() const;
    };
}
