#include "lib/base.h"
#include "lib/model.h"
#include "lib/categorical_model.h"
#include "lib/numerical_model.h"
#include "lib/string_model.h"
#include "lib/compression.h"
#include "lib/decompression.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

char del = ',';
std::vector<db_compress::BiMap> enum_map;

std::ifstream::pos_type filesize(const char *filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

int EnumTranslate(const std::string &str, int attr) {
    db_compress::BiMap &map = enum_map[attr];
    auto &enum2idx = map.enum2idx;
    auto &enums = map.enums;

    auto it = enum2idx.find(str);
    if (it != enum2idx.end())
        return it->second;
    else {
        enum2idx[str] = enums.size();
        enums.push_back(str);
        return enums.size() - 1;
    }
}

class SimpleCategoricalInterpreter : public db_compress::AttrInterpreter {
private:
    int cap_;
public:
    SimpleCategoricalInterpreter(int cap) : cap_(cap) {}

    bool EnumInterpretable() const { return true; }

    int EnumCap() const { return cap_; }

    int EnumInterpret(const db_compress::AttrValue *attr) const {
        return static_cast<const db_compress::EnumAttrValue *>(attr)->Value();
    }
};

const int NonFullPassStopPoint = 2000;

char inputFileName[100], outputFileName[100], configFileName[100];
bool compress;
db_compress::Schema schema;
db_compress::CompressionConfig config;
std::vector<int> attr_type;
std::vector<db_compress::EnumAttrValue> enum_vec;
std::vector<db_compress::IntegerAttrValue> int_vec;
std::vector<db_compress::DoubleAttrValue> double_vec;
std::vector<db_compress::StringAttrValue> str_vec;

void PrintHelpInfo() {
    std::cout << "Usage:\n";
    std::cout << "Compression: sample -c input_file config_file is_special_del\n";
    std::cout << "Decompression: sample -d input_file config_file is_special_del\n";
}

// Read inputFileName, outputFileName, configFileName and whether to
// compress or decompress. Return false if failed to recognize params.
bool ReadParameter(int argc, char **argv) {
    if (argc != 5) return false;
    if (strcmp(argv[1], "-c") == 0) compress = true;
    else if (strcmp(argv[1], "-d") == 0) compress = false;
    else return false;

    strcpy(inputFileName, argv[2]);
    strcpy(outputFileName, inputFileName);
    strcat(outputFileName, compress ? ".squish" : ".rec");
    strcpy(configFileName, argv[3]);
    int is_special = std::stoi(argv[4]);
    if (is_special) del = '|';
    std::cout << "Delimiter: " << del << "\n";
    return true;
}

void ReadConfig(char *configFileName) {
    std::ifstream fin(configFileName);
    std::string str;
    std::vector<int> type;
    std::vector<double> err;
    attr_type.clear();
    config.sort_by_attr = 0;

    while (std::getline(fin, str)) {
        std::vector<std::string> vec;
        std::string item;
        std::stringstream sstream(str);
        while (std::getline(sstream, item, ' ')) {
            if (item.back() == '\r') item.pop_back();
            vec.push_back(item);
        }
        if (vec[0] == "SORT") {
            continue;
        } else {
            int type_ = type.size();
            type.push_back(type_);
            if (vec[0] == "ENUM") {
                RegisterAttrModel(type_, new db_compress::TableCategoricalCreator());
                RegisterAttrInterpreter(type_, new SimpleCategoricalInterpreter(std::stoi(vec[1])));
                err.push_back(std::stod(vec[2]));
                attr_type.push_back(0);
            } else if (vec[0] == "INTEGER") {
                RegisterAttrModel(type_, new db_compress::TableLaplaceIntCreator());
                RegisterAttrInterpreter(type_, new db_compress::AttrInterpreter());
                err.push_back(std::stod(vec[1]));
                attr_type.push_back(1);
            } else if (vec[0] == "DOUBLE") {
                RegisterAttrModel(type_, new db_compress::TableLaplaceRealCreator());
                RegisterAttrInterpreter(type_, new db_compress::AttrInterpreter());
                err.push_back(std::stod(vec[1]));
                attr_type.push_back(2);
            } else if (vec[0] == "STRING") {
                RegisterAttrModel(type_, new db_compress::StringModelCreator());
                RegisterAttrInterpreter(type_, new db_compress::AttrInterpreter());
                err.push_back(0);
                attr_type.push_back(3);
            } else
                std::cerr << "Config File Error!\n";
        }
    }

    if (attr_type.size() == 0)
        std::cerr << "Config File Error!\n";
    schema = db_compress::Schema(type);
    config.allowed_err = err;

    enum_vec.resize(attr_type.size());
    int_vec.resize(attr_type.size());
    double_vec.resize(attr_type.size());
    str_vec.resize(attr_type.size());

    enum_map.resize(attr_type.size());
}

inline void AppendAttr(db_compress::Tuple *tuple, const std::string &str,
                       int attr_type, int index) {
    switch (attr_type) {
        case 0:
            enum_vec[index].Set(EnumTranslate(str, index));
            tuple->attr[index] = &enum_vec[index];
            break;
        case 1:
            int_vec[index].Set(std::stoi(str));
            tuple->attr[index] = &int_vec[index];
            break;
        case 2:
            double_vec[index].Set(std::stod(str));
            tuple->attr[index] = &double_vec[index];
            break;
        case 3:
            str_vec[index].Set(str);
            tuple->attr[index] = &str_vec[index];
            break;
    }
}

inline std::string ExtractAttr(const db_compress::Tuple &tuple, int attr_type, int index) {
    std::string ret;
    const db_compress::AttrValue *attr = tuple.attr[index];
    switch (attr_type) {
        case 0:
            ret = enum_map[index].enums[static_cast<const db_compress::EnumAttrValue *>(attr)->Value()];
            // ret = std::to_string(static_cast<const db_compress::EnumAttrValue *>(attr)->Value());
            break;
        case 1:
            ret = std::to_string(static_cast<const db_compress::IntegerAttrValue *>(attr)->Value());
            break;
        case 2:
            ret = std::to_string(static_cast<const db_compress::DoubleAttrValue *>(attr)->Value());
            break;
        case 3:
            ret = static_cast<const db_compress::StringAttrValue *>(attr)->Value();
            break;
    }
    return ret;
}

int main(int argc, char **argv) {
    if (argc == 1)
        PrintHelpInfo();
    else {
        if (!ReadParameter(argc, argv)) {
            std::cerr << "Bad Parameters.\n";
            return 1;
        }
        std::ios::sync_with_stdio(false);
        ReadConfig(configFileName);
        if (compress) {
            // Compress
            db_compress::Compressor compressor(outputFileName, schema, config);
            int iter_cnt = 0;
            while (1) {
                std::cout << "Iteration " << ++iter_cnt << " Starts" << std::endl;
                std::ifstream inFile(inputFileName);
                std::string str;
                int tuple_cnt = 0;
                while (std::getline(inFile, str)) {
                    std::stringstream sstream(str);
                    std::string item;
                    db_compress::Tuple tuple(schema.attr_type.size());

                    size_t count = 0;
                    while (std::getline(sstream, item, del)) {
                        if (del == ',') {
                            if (!item.empty() && item[0] == '"') {
                                item = item.substr(1);
                                item += ',';
                                std::string item2;
                                if (std::getline(sstream, item2, '"'))
                                    item += item2;

                                std::getline(sstream, item2, del);
                            }
                        }
                        AppendAttr(&tuple, item, attr_type[count], count);
                        ++count;
                    }
                    // The last item might be empty string
                    if (str[str.length() - 1] == del) {
                        AppendAttr(&tuple, "", attr_type[count], count);
                        ++count;
                    }
                    if (count != attr_type.size()) {
                        std::cerr << "File Format Error!" << "\t" << count << "\t" << attr_type.size() << std::endl;
                    }
                    compressor.ReadTuple(tuple);

                    if (!compressor.RequireFullPass() &&
                        ++tuple_cnt >= NonFullPassStopPoint) {
                        break;
                    }
                }
                compressor.EndOfData();
                if (!compressor.RequireMoreIterations())
                    break;
            }
            int32_t file_size = filesize(outputFileName);
            std::cout << "File Size: " << file_size << "byte" << std::endl;

            // write down enum attrs.
            db_compress::Write(enum_map, std::string());
        } else {
            // Decompress
            // Load enum values
            db_compress::Read(enum_map, std::string());

            db_compress::Decompressor decompressor(inputFileName, schema);
            std::ofstream outFile(outputFileName);
            decompressor.Init();
            while (decompressor.HasNext()) {
                db_compress::Tuple tuple(attr_type.size());
                decompressor.ReadNextTuple(&tuple);

                for (size_t i = 0; i < attr_type.size(); ++i) {
                    std::string str = ExtractAttr(tuple, attr_type[i], i);
                    outFile << str << (i == attr_type.size() - 1 ? '\n' : del);
                }
            }
        }
    }
    return 0;
}
