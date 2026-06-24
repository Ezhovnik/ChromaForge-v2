#pragma once

#include <vector>
#include <memory>

class Action {
public:
    virtual ~Action() = default;

    virtual void apply() = 0;
    virtual void revert() = 0;
};

class InversedAction : public Action {
public:
    InversedAction(std::unique_ptr<Action> action) : action(std::move(action)) {}

    void apply() override {
        action->revert();
    }

    void revert() override {
        action->apply();
    }
private:
    std::unique_ptr<Action> action;
};

class CombinedAction : public Action {
public:
    CombinedAction(std::vector<std::unique_ptr<Action>> actions) : actions(std::move(actions)) {
    }

    void apply() override {
        for (auto& action : actions) {
            action->apply();
        }
    }

    void revert() override {
        for (int i = actions.size() - 1; i >= 0; --i) {
            actions[i]->revert();
        }
    }
private:
    std::vector<std::unique_ptr<Action>> actions;
};

class ActionsHistory {
public:
    ActionsHistory() {};

    void clearRedo() {
        if (actionPtr < actions.size()) {
            actions.erase(actions.begin() + actionPtr, actions.end());
        }
    }

    void store(std::unique_ptr<Action> action, bool reverse=false) {
        if (lock) return;

        if (reverse) {
            action = std::make_unique<InversedAction>(std::move(action));
        }
        clearRedo();
        actions.emplace_back(std::move(action));
        actionPtr++;
    }

    void apply(std::unique_ptr<Action> action) {
        if (lock) return;

        clearRedo();
        lock = true;
        action->apply();
        lock = false;
        actions.emplace_back(std::move(action));
        actionPtr++;
    }

    bool undo() {
        if (lock || actionPtr == 0) {
            return false;
        }
        auto& action = actions[--actionPtr];
        lock = true;
        action->revert();
        lock = false;
        return true;
    }

    bool redo() {
        if (lock || actionPtr == actions.size()) {
            return false;
        }
        auto& action = actions[actionPtr++];
        lock = true;
        action->apply();
        lock = false;
        return true;
    }

    void clear() {
        actionPtr = 0;
        actions.clear();
    }

    void squash(ptrdiff_t n) {
        if (n < 2) return;

        n = std::min(n, static_cast<ptrdiff_t>(actionPtr));
        std::vector<std::unique_ptr<Action>> squashing;
        for (size_t i = actionPtr - n; i < actionPtr; i++) {
            squashing.emplace_back(std::move(actions[i]));
        }
        actions.erase(actions.begin() + actionPtr - n, actions.end());
        actionPtr -= n;
        store(std::make_unique<CombinedAction>(std::move(squashing)));
    }

    size_t size() const {
        return actionPtr;
    }

    struct Combination {
        ActionsHistory& history;
        size_t historySize;

        Combination(ActionsHistory& history)
            : history(history), historySize(history.size()) {
        }

        ~Combination() {
            history.squash(history.size() - historySize);
        }
    };

    Combination beginCombination() {
        return Combination(*this);
    }
private:
    std::vector<std::unique_ptr<Action>> actions;
    size_t actionPtr = 0;
    bool lock = false;
};
