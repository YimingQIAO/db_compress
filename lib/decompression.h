// The decompression process class header

#ifndef DECOMPRESSION_H
#define DECOMPRESSION_H

#include "base.h"
#include "model.h"
#include "data_io.h"

#include <fstream>
#include <string>
#include <memory>
#include <vector>

namespace db_compress {

    class Decompressor {
    private:
        ByteReader byte_reader_;
        size_t implicit_length_, implicit_prefix_;
        Schema schema_;
        std::vector<std::unique_ptr<SquIDModel> > model_;
        std::vector<size_t> attr_order_;

        std::chrono::microseconds duration;

        void ReadTuplePrefix();

    public:
        Decompressor(const char *compressedFileName, const Schema &schema);

        void Init();

        void ReadNextTuple(Tuple *tuple);

        bool HasNext() const;

        ~Decompressor() {
            std::cout << "Decompress Time: "
                      << static_cast<double>(duration.count()) *
                         std::chrono::microseconds::period::num /
                         std::chrono::microseconds::period::den
                      << " s\n";
        }
    };

}  // namespace db_compress

#endif
