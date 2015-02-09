#include "utility.h"

#include <cmath>
#include <vector>
#include <iostream>

namespace db_compress {

void TestDynamicList() {
    std::vector<size_t> index;
    index.push_back(0); index.push_back(1); index.push_back(2);
    DynamicList<int> dynamic_list(3);
    dynamic_list[index] = 1;
    index[2] = 3;
    dynamic_list[index] = 2;
    if (dynamic_list.size() != 2)
        std::cerr << "Dynamic List Unit Test Failed!\n";
    if (dynamic_list[index] != 2 || dynamic_list[1] != 2)
        std::cerr << "Dynamic List Unit Test Failed!\n";
    const DynamicList<int>& another = dynamic_list;
    if (another[index] != 2)
        std::cerr << "Dynamic List Unit Test Failed!\n";
    index[2] = 2;
    if (dynamic_list[index] != 1)
        std::cerr << "Dynamic List Unit Test Failed!\n";
    dynamic_list[index] = 3;
    if (dynamic_list.size() != 2 || another[index] != 3)
        std::cerr << "Dynamic List Unit Test Failed!\n";   
}

void TestQuantization() {
    // Todo:
}

void TestGetProbInterval() {
    double l = 0.2, r = 0.6;
    std::vector<unsigned char> emit_bytes;
    GetProbSubinterval(l, r, 0, 0.1, &l, &r, &emit_bytes);
    if (fabs(l - 0.2) > 0.001 || fabs(r - 0.24) > 0.001 || emit_bytes.size() != 0)
        std::cerr << "Get Prob Subinterval Unit Test Failed!\n";
    GetProbSubinterval(l, r, 0.1, 0.15, &l, &r, NULL);
    if (fabs(l - 0.204) > 0.001 || fabs(r - 0.206) > 0.001)
        std::cerr << "Get Prob Subinterval Unit Test Failed!\n";
    GetProbSubinterval(l, r, 0, 1, &l, &r, &emit_bytes);
    if (emit_bytes.size() != 1 || emit_bytes[0] != 52 || 
        fabs(l - 0.224) > 0.001 || fabs(r - 0.736) > 0.001)
        std::cerr << "Get Prob Subinterval Unit Test Failed!\n"; 
}

void TestLaplace() {
    if (fabs(GetMidValueFromExponential(2, 0, -1) - 1.386) > 0.001)
        std::cerr << "Laplace Unit Test Failed!\n";
    if (fabs(GetMidValueFromExponential(2, 0, 1000000) - 1.386) > 0.001)
        std::cerr << "Laplace Unit Test Failed!\n";
    double result, l, r;
    GetProbIntervalFromExponential(0.6, 0.5, 0.22, false, &result, &l, &r);
    if (fabs(result - 0.62) > 0.02 || fabs(l - 0.5) > 0.01 || fabs(r - 0.75) > 0.01)
        std::cerr << "Laplace Unit Test Failed!\n";
    GetProbIntervalFromExponential(0.6, 1, 0, true, &result, &l, &r);
    if (fabs(result - 1) > 0.02 || fabs(l - 0.5) > 0.01 || fabs(r - 0.75) > 0.01)
        std::cerr << "Laplace Unit Test Failed!\n";
}

void TestFloatQuantization() {
    unsigned char bytes[4];
    ConvertSinglePrecision(0.1, bytes);
    if (bytes[0] != 0x3d || bytes[1] != 0xcc || bytes[2] != 0xcc || bytes[3] != 0xcd)
        std::cerr << "Float QUantization Unit Test Failed!\n";
}

void TestBitString() {
    if (GetByte(0x7fdecba0, 4) != 0xfd) 
        std::cerr << "Bit String Unit Test Failed!\n";
    BitString str;
    str.Clear();
    StrCat(&str, 0xff);
    if (str.length != 8 || str.bits[0] != 0xff000000)
        std::cerr << "Bit String Unit Test Failed!\n";
    StrCat(&str, 0x7fec, 12);
    if (str.length != 20 || str.bits[0] != 0xfffec000)
        std::cerr << "Bit String Unit Test Failed!\n";
    BitString copy(str);
    StrCat(&str, copy);
    if (str.length != 40 || str.bits.size() != 2 || 
        str.bits[0] != 0xfffecfff || str.bits[1] != 0xec000000)
        std::cerr << "Bit String Unit Test Failed!\n";
    if (ComputePrefix(str, 12) != 0xfff)
        std::cerr << "Bit String Unit Test Failed!\n";
    GetBitStringFromProbInterval(&str, 0.66, 0.68);
    if (str.length != 7 || str.bits[0] != 0xaa000000)
        std::cerr << "Bit String Unit Test Failed!\n";
}

void Test() {
    TestDynamicList();
    TestQuantization();
    TestGetProbInterval();
    TestLaplace();
    TestFloatQuantization();
    TestBitString();
}

}  // namespace db_compress

int main() {
    db_compress::Test();
}
