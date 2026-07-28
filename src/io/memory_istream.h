#pragma once

#include <istream>
#include <util/Buffer.h>

class memory_streambuf : public std::streambuf {
public:
    explicit memory_streambuf(util::Buffer<char> buffer)
        : buffer(std::move(buffer)) {
        char* base = this->buffer.data();
        char* end = base + this->buffer.size();
        setg(base, base, end);
    }

    memory_streambuf(const memory_streambuf&) = delete;
    memory_streambuf& operator=(const memory_streambuf&) = delete;
protected:
    int_type underflow() override {
        return traits_type::eof();
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        if (!(which & std::ios_base::in)) {
            return pos_type(off_type(-1));
        }
        char* base = eback();
        char* end = egptr();
        char* pos;
        switch (dir) {
            case std::ios_base::beg: pos = base + off; break;
            case std::ios_base::cur: pos = gptr() + off; break;
            case std::ios_base::end: pos = end + off; break;
            default: return pos_type(off_type(-1));
        }
        if (pos < base || pos > end) {
            return pos_type(off_type(-1));
        }
        setg(base, pos, end);
        return pos_type(pos - base);
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return seekoff(static_cast<off_type>(pos), std::ios_base::beg, which);
    }
private:
    util::Buffer<char> buffer;
};

class memory_istream : private memory_streambuf, public std::istream {
public:
    explicit memory_istream(util::Buffer<char> buffer)
        : memory_streambuf(std::move(buffer)), std::istream(this) {}
};
