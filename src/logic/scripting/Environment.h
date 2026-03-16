#ifndef LOGIC_SCRIPTING_ENVIRONMENT_H_
#define LOGIC_SCRIPTING_ENVIRONMENT_H_

namespace scripting {
    class Environment {
    private:
        int env;
    public:
        Environment(int env);
        ~Environment();

        int getId() const;

        void release();
    };
}

#endif // LOGIC_SCRIPTING_ENVIRONMENT_H_
