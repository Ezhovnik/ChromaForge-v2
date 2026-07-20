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

    bool processOperator(VersionOperator op, const Version& other) const {
        switch(op) {
            case VersionOperator::Equal: return *this == other;
            case VersionOperator::Greather: return *this > other;
            case VersionOperator::Less: return *this < other;
            case VersionOperator::LessOrEqual: return *this <= other;
            case VersionOperator::GreatherOrEqual: return *this >= other;
            default: return false;
        }
    }

    static bool matchesPattern(const std::string& version);
};
