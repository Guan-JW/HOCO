#include "CompressModule/CompressModule.hpp"

void test_LZW() {
    string dir = ROOT_PATH + "_Text/Compressed_data/LZW/COV19";
    CompData<size_t> data(dir);
    BaseHC<Path, size_t, string, string>*
        LZW(FACTORY(BaseHC, Path, size_t, string, string).create("LZW_HC"));

    CompressModule<Path, size_t, string, string> CM(&data, LZW);
    CM.printData();
    CM.printSchemeName();

    cout << "Compressing with LZW..." << endl;
    string splitFile = PARENT_PATH + "input/Original_data/COV19/fileYyNO.txt";
    string uncompFile = PARENT_PATH + "input/Original_data/COV19/yelp_academic_dataset_covid_features.json";
    UncompressPath upath(uncompFile, splitFile);
    CM.Compress(upath);

    cout << "Decompressing with LZW..." << endl;
    string decompFile = ROOT_PATH + "_Text/Decompressed_data/LZW/COV19/decomp.txt";
    DecompressPath dpath(decompFile);
    CM.Decompress(dpath);
}

void test_RLE() {
    string dir = ROOT_PATH + "_Text/Compressed_data/RLE/COV19";
    CompData<size_t> data(dir);
    BaseHC<Path, size_t, string, string>*
        RLE(FACTORY(BaseHC, Path, size_t, string, string).create("RLE_HC"));

    CompressModule<Path, size_t, string, string> CM(&data, RLE);
    CM.printData();
    CM.printSchemeName();

    cout << "Compressing with RLE..." << endl;
    string splitFile = PARENT_PATH + "input/Original_data/COV19/fileYyNO.txt";
    string uncompFile = PARENT_PATH + "input/Original_data/COV19/yelp_academic_dataset_covid_features.json";
    UncompressPath upath(uncompFile, splitFile);
    CM.Compress(upath);

    cout << "Decompressing with LZW..." << endl;
    string decompFile = ROOT_PATH + "_Text/Decompressed_data/RLE/COV19/decomp.txt";
    DecompressPath dpath(decompFile);
    CM.Decompress(dpath);

}

void test_TADOC() {
    string dir = ROOT_PATH + "_Text/Compressed_data/TADOC/COV19";
    CompData<size_t> data(dir);
    BaseHC<Path, size_t, string, string>*
        TADOC(FACTORY(BaseHC, Path, size_t, string, string).create("TADOC_HC"));

    CompressModule<Path, size_t, string, string> CM(&data, TADOC);
    CM.printData();
    CM.printSchemeName();

    cout << "Compressing with TADOC..." << endl;
    string splitFile = PARENT_PATH + "input/Original_data/COV19/fileYyNO.txt";
    string uncompFile = PARENT_PATH + "input/Original_data/COV19/yelp_academic_dataset_covid_features.json";
    UncompressPath upath(uncompFile, splitFile);
    CM.Compress(upath);
}

int main() {

    /* Test register */
    REGISTER(BaseHC, RLE_HC, Path, size_t, string, string);
    REGISTER(BaseHC, LZW_HC, Path, size_t, string, string);
    REGISTER(BaseHC, TADOC_HC, Path, size_t, string, string);
    cout << "Registered HC schemes:" << endl;
    FACTORY(BaseHC, Path, size_t, string, string).printRegisteredClasses();
    cout << "---" << std::endl; 
    
    /* LZW */
    test_LZW();

    /* RLE */
    test_RLE();
    
    /* TADOC */
    test_TADOC();

    return 0;
}