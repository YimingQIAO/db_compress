/*
 * The header file for categorical SquID and SquIDModel
 */

#ifndef CATEGORICAL_MODEL_H
#define CATEGORICAL_MODEL_H

#include "model.h"
#include "base.h"
#include "utility.h"

#include <vector>

namespace db_compress {

class EnumAttrValue: public AttrValue {
  private:
    size_t value_;
  public:
    EnumAttrValue() {}
    EnumAttrValue(size_t val) : value_(val) {}
    inline void Set(size_t val) { value_ = val; }
    inline size_t Value() const { return value_; }
};

struct CategoricalStats {
    std::vector<int> count;
    std::vector<Prob> prob;
};

class CategoricalSquID : public SquID {
  private:
    int choice_;
    EnumAttrValue attr;
  public:
    inline void Init(const std::vector<Prob>& prob_segs);
    bool HasNextBranch() const { return choice_ == -1; }
    void GenerateNextBranch() {}
    int GetNextBranch(const AttrValue* attr) const;
    void ChooseNextBranch(int branch) { choice_ = branch; }
    const AttrValue* GetResultAttr() { 
        attr.Set(choice_);
        return &attr;
    }
};

class TableCategorical : public SquIDModel {
  private:
    std::vector<const AttrInterpreter*> predictor_interpreter_;
    size_t target_range_;
    size_t cell_size_;
    double err_;
    double model_cost_;
    CategoricalSquID squid_;

    // Each vector consists of k-1 probability segment boundary
    DynamicList<CategoricalStats> dynamic_list_;
    void GetDynamicListIndex(const Tuple& tuple, std::vector<size_t>* index);
   
  public:
    TableCategorical(const Schema& schema, const std::vector<size_t>& predictor_list, 
                    size_t target_var, double err);
    SquID* GetSquID(const Tuple& tuple);
    int GetModelCost() const { return model_cost_; }
    void FeedTuple(const Tuple& tuple);
    void EndOfData();
    int GetModelDescriptionLength() const;
    void WriteModel(ByteWriter* byte_writer, size_t block_index) const;
    static SquIDModel* ReadModel(ByteReader* byte_reader, const Schema& schema, size_t index);
};

class TableCategoricalCreator : public ModelCreator {
  private:
    const size_t MAX_TABLE_SIZE = 1000;
  public:
    SquIDModel* ReadModel(ByteReader* byte_reader, const Schema& schema, size_t index);
    SquIDModel* CreateModel(const Schema& schema, const std::vector<size_t>& predictor, 
                       size_t index, double err);
};

} // namespace db_compress

#endif
