// Copyright (c) 2014 Sandstorm Development Group, Inc. and contributors
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/*
 * Compatibility layer for stdlib iostream
 */

#pragma once

#if defined(__GNUC__) && !KJ_HEADER_WARNINGS
#pragma GCC system_header
#endif

#include <kj/io.h>
#include <iostream>

namespace kj {
    namespace std {

        class StringPipe: public kj::BufferedInputStream, public kj::OutputStream {
        public:
            StringPipe()
                    : preferredReadSize(kj::maxValue), readPos(0) {}
            explicit StringPipe(::std::string& data_)
                    : preferredReadSize(kj::maxValue), readPos(0), data(data_) {}
            explicit StringPipe(size_t preferredReadSize)
                    : preferredReadSize(preferredReadSize), readPos(0) {}
            ~StringPipe() {}

            const ::std::string& getData() { return data; }

            kj::ArrayPtr<const byte> getArray() {
                return kj::arrayPtr(reinterpret_cast<const byte*>(data.data()), data.size());
            }

            void resetRead(size_t preferredReadSize = kj::maxValue) {
                readPos = 0;
                this->preferredReadSize = preferredReadSize;
            }

            bool allRead() {
                return readPos == data.size();
            }

            void clear(size_t preferredReadSize = kj::maxValue) {
                resetRead(preferredReadSize);
                data.clear();
            }

            void write(const void* buffer, size_t size) override {
                data.append(reinterpret_cast<const char*>(buffer), size);
            }

            size_t tryRead(void* buffer, size_t minBytes, size_t maxBytes) override {
//                KJ_ASSERT(maxBytes <= data.size() - readPos, "Overran end of stream.");
                size_t amount = kj::min(maxBytes, kj::max(minBytes, preferredReadSize));
                memcpy(buffer, data.data() + readPos, amount);
                readPos += amount;
                return amount;
            }

            void skip(size_t bytes) override {
//                KJ_ASSERT(bytes <= data.size() - readPos, "Overran end of stream.");
                readPos += bytes;
            }

            kj::ArrayPtr<const byte> tryGetReadBuffer() override {
                size_t amount = kj::min(data.size() - readPos, preferredReadSize);
                return kj::arrayPtr(reinterpret_cast<const byte*>(data.data() + readPos), amount);
            }

        private:
            size_t preferredReadSize;
            ::std::string data;
            ::std::string::size_type readPos;
        };
    }  // namespace std
}  // namespace kj
