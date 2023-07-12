#include "CompressData.hpp"

int main() {
    /* Test LZW */
    string dir = "../../input/Compressed_data/LZW/COV19";
    CompData<size_t> data(dir);

    /* Test RLE */
    dir = "../../input/Compressed_data/RLE/COV19";
    CompData<size_t> data_rle(dir);

    /* Test TADOC */
    dir = "../../input/Compressed_data/TADOC/COV19";
    CompData<size_t> data_tadoc(dir);
    

    return 0;
}