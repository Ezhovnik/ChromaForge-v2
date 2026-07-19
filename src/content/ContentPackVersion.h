#include <string>

#include <content/ContentPack.h>

class Version {
public:
    int major;
    int minor;
    int patch;

    Version(const std::string& version);

    bool operator==(const Version& other) const {
        return major == other.major &&
            minor == other.minor &&
            patch == other.patch;
    }

    bool operator<(const Version& other) const {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        return patch < other.patch;
    }

    bool operator>(const Version& other) const {
        return other < *this;
    }

    bool operator>=(const Version& other) const {
        return !(*this < other);
    }

    bool operator<=(const Version& other) const {
        return !(*this > other);
    }

    bool process_operator(const std::string& op, const Version& other) const {
        auto dep_op = Version::string_to_operator(op);

        switch(dep_op) {
            case DependencyVersionOperator::Equal: return *this == other;
            case DependencyVersionOperator::More: return *this > other;
            case DependencyVersionOperator::Less: return *this < other;
            case DependencyVersionOperator::LessOrEqual: return *this <= other;
            case DependencyVersionOperator::MoreOrEqual: return *this >= other;
            default: return false;
        }
    }

    static DependencyVersionOperator string_to_operator(const std::string& op);
    static bool matches_pattern(const std::string& version);
};
