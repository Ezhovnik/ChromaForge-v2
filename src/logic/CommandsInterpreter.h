#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include <data/dv.h>

namespace cmd {
    enum class ArgType {
        Number, 
        Integer,
        Enumvalue,
        Selector,
        String
    };

    inline std::string argtype_name(ArgType type) {
        switch (type) {
            case ArgType::Number: return "number";
            case ArgType::Integer: return "integer";
            case ArgType::Enumvalue: return "enumeration";
            case ArgType::Selector: return "selector";
            case ArgType::String: return "string";
            default: return "<unknown>";
        }
    }

    struct Argument {
        std::string name;
        ArgType type;
        bool optional;
        dv::value def;
        dv::value origin;
        std::string enumname;
    };

    class Command;
    class CommandsInterpreter;

    struct Prompt {
        Command* command;
        dv::value args;
        dv::value kwargs;
    };

    using executor_func = std::function<dv::value(
        CommandsInterpreter*,
        const dv::value& args,
        const dv::value& kwargs
    )>;

    class Command {
    private:
        std::string name;
        std::string description;
        std::vector<Argument> args;
        std::unordered_map<std::string, Argument> kwargs;
        executor_func executor;
    public:
        Command() = default;

        Command(
            std::string name,
            std::vector<Argument> args,
            std::unordered_map<std::string, Argument> kwargs,
            std::string description,
            executor_func executor
        ) : name(name),
            args(std::move(args)),
            kwargs(std::move(kwargs)),
            description(description),
            executor(executor) {}

        Argument* getArgument(size_t index) {
            if (index >= args.size()) return nullptr;
            return &args[index];
        }

        Argument* getArgument(const std::string& keyword) {
            auto found = kwargs.find(keyword);
            if (found == kwargs.end()) return nullptr;
            return &found->second;
        }

        dv::value execute(CommandsInterpreter* interpreter, const Prompt& prompt) {
            return executor(interpreter, prompt.args, prompt.kwargs);
        }

        const std::string& getName() const {
            return name;
        }

        const std::vector<Argument>& getArgs() const {
            return args;
        }

        const std::unordered_map<std::string, Argument>& getKwArgs() const {
            return kwargs;
        }

        const std::string& getDescription() const {
            return description;
        }

        static Command create(
            std::string_view scheme, 
            std::string_view description, 
            executor_func
        );
    };

    class CommandsRepository {
    private:
        std::unordered_map<std::string, Command> commands;
    public:
        void add(
            std::string_view scheme, 
            std::string_view description, 
            executor_func
        );
        Command* get(const std::string& name);
        const std::unordered_map<std::string, Command>& getCommands() const {
            return commands;
        }
    };

    class CommandsInterpreter {
    private:
        std::unique_ptr<CommandsRepository> repository;
        std::unordered_map<std::string, dv::value> variables;
    public:
        CommandsInterpreter() : repository(std::make_unique<CommandsRepository>()) {}
        CommandsInterpreter(const CommandsInterpreter&) = delete;
        CommandsInterpreter(std::unique_ptr<CommandsRepository> repository) : repository(std::move(repository)){}

        Prompt parse(std::string_view text);

        dv::value& operator[](const std::string& name) {
            return variables[name];
        }

        dv::value execute(std::string_view input) {
            return execute(parse(input));
        }

        dv::value execute(const Prompt& prompt) {
            return prompt.command->execute(this, prompt);
        }

        CommandsRepository* getRepository() const {
            return repository.get();
        }
    };
}
