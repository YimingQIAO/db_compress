#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "base.h"
#include "data_io.h"
#include "model.h"

#include <vector>
#include <memory>

namespace db_compress {
   
class Compressor {
  private:
    std::string outputFile_;
    Schema schema_;
    std::unique_ptr<ModelLearner> learner_;
    std::vector< std::unique_ptr<Model> > model_;
    std::vector<size_t> attr_order_;
    std::unique_ptr<ByteWriter> byte_writer_;
    int stage_;
    size_t num_of_tuples_;
    size_t implicit_prefix_length_;
    std::vector<size_t> block_length_;
  public:
    Compressor(const char* outputFile, const Schema& schema, const CompressionConfig& config);
    void ReadTuple(const Tuple& tuple);
    bool RequireMoreIterations() const;
    bool RequireFullPass() const;
    void EndOfData();
};

}  // namespace db_compress

#endif
