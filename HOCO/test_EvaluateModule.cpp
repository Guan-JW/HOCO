#include "CompressModule/CompressModule.hpp"
#include "EvaluateModule/EvaluateModule.hpp"

#define DoCompress

void test() {
    /* Compress Module, for recovering the compressed data to the original one */
#ifdef DoCompress
    string dir = ROOT_PATH + "_Text/Compressed_data/LZW/COV19";
    CompData<size_t> data(dir);
    BaseHC<Path, size_t, string, string>*
        LZW(FACTORY(BaseHC, Path, size_t, string, string).create("LZW_HC"));

    CompressModule<Path, size_t, string, string> CM(&data, LZW);
    CM.printData();
    CM.printSchemeName();

    string splitFile = PARENT_PATH + "input/Original_data/COV19/fileYyNO.txt";
    string uncompFile = PARENT_PATH + "input/Original_data/COV19/yelp_academic_dataset_covid_features.json";
    UncompressPath upath(uncompFile, splitFile);
    CM.Compress(upath);
#endif

    string data_dir = ROOT_PATH + "_Text/Compressed_data/LZW/COV19";
    EvaluationModule<Path, size_t, string, string> EM(data_dir);
    string dsl_input = ROOT_PATH + "EvaluateModule/DSL/input.dsl";
    cout << "Testing DSL Parser..." << endl;
    if (! EM.DSL_Parser(dsl_input)) {
        throw std::runtime_error("HOCO parsing failed.");
    }
    cout << "Testing DSL Translator..." << endl;
    string gen_code_path = ROOT_PATH + "EvaluateModule/GenCode/WordCount.cpp";
    if (! EM.AST_Translator(gen_code_path)) {
        throw std::runtime_error("HOCO translation failed.");
    }

    cout << "Successfully generate code, please find refer to file 'HOCO/EvaluateModule/GenCode/WordCount.cpp'" << endl;
}


int main()
{
    FACTORY(BaseHC, Path, size_t, string, string).printRegisteredClasses();
    cout << "---" << std::endl; 

    test();

    return 0;
}
