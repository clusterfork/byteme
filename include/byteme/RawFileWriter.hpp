#ifndef BYTEME_RAW_BUFFER_READER_HPP
#define BYTEME_RAW_BUFFER_READER_HPP

#include <vector>
#include <stdexcept>
#include <string>
#include <cstdio>
#include "Writer.hpp"
#include "SelfClosingFILE.hpp"

/**
 * @file RawFileWriter.hpp
 *
 * @brief Write bytes to a file without any extra transformations.
 */

namespace byteme {

/**
 * @brief Write bytes to a file.
 *
 * This class will write bytes to a file without any further transformations.
 * It is basically a simple wrapper around `FILE` structures, with correct closing and error checking.
 */
class RawFileWriter : public Writer {
public:
    /**
     * @param path Path to the file.
     * @param buffer_size Size of the buffer to use for writing.
     */
    RawFileWriter(const char* path, size_t buffer_size = 65536) : file(path, "wb"), buffer_(buffer_size) {}

    /**
     * @param path Path to the file.
     * @param buffer_size Size of the buffer to use for writing.
     */
    RawFileWriter(const std::string& path, size_t buffer_size = 65536) : RawFileWriter(path.c_str(), buffer_size) {}

public:
    void write(const unsigned char* buffer, size_t n) {
        const auto capacity = buffer_.size();
        auto ptr = buffer_.data();
        auto& handle = file.handle;

        // A bunch of choices made here to minimize the number of writes.
        if (n < capacity) {
            if (used + n < capacity) {
                std::copy(buffer, buffer + n, ptr + used);
                used += n;
            } else {
                size_t to_add = capacity - used;
                std::copy(buffer, buffer + to_add, ptr + used);
                dump(ptr, capacity);
                std::copy(buffer + to_add, buffer + n, ptr);
                used = n - to_add;
            }
        } else {
            if (used) {
                dump(ptr, used);
                used = 0;
            }
            dump(buffer, n);
        }
    }

    void finish() {
        if (file.handle) {
            if (used) {
                dump(buffer_.data(), used);
            }
            if (std::fclose(file.handle)) {
                throw std::runtime_error("failed to close raw binary file");
            }
            file.handle = nullptr;
        }
    }

private:
    SelfClosingFILE file;
    std::vector<unsigned char> buffer_;
    size_t used = 0;

    void dump(const unsigned char* ptr, size_t len) {
        size_t ok = std::fwrite(ptr, sizeof(unsigned char), len, file.handle);
        if (ok < len) {
            throw std::runtime_error("failed to write raw binary file (fwrite error " + std::to_string(std::ferror(file.handle)) + ")");
        }
        return;
    }
};

}

#endif
