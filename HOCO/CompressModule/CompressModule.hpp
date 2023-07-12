#pragma once
#include "../SchemeCollectModule/CollectModule.hpp"

template <typename Path, typename Offset, typename Symbol, typename Rst>
class CompressModule {
public:
    CompressModule() {}
    CompressModule(CompData<Offset>* _compData) : compData(_compData) {}
    CompressModule(CompData<Offset>* _compData, BaseHC<Path, Offset, Symbol, Rst>* _scheme) :
            compData(_compData), scheme(_scheme) 
            {
                /* Set the compdata pointer in scheme !!!! */
                scheme->set_compData(compData);
            }

    bool SchemeSelection() {
        return true;
    }

    bool Compress(UncompressPath _upath) {
        if (!scheme->Compress(_upath, *(compData->get_compress_path())) ) {
            cerr << "CompressModule: compression process failed." << endl;
            return false;
        }
        /* Add scheme information to compressed data for evaluation */
        if (!write_scheme_meta()) {
            throw runtime_error("CompressModule: Failed to create meta data.");
            return false;
        }
        return true;
    }

    bool Decompress(DecompressPath _dpath) {
        if (!scheme->Decompress(_dpath)) {
            cerr << "CompressModule: decompression process failed." << endl;
            return false;
        }
        return true;
    }

    bool write_scheme_meta() {

        string outputFile;
        if (!Path::join(compData->get_compress_path()->get_comp_dir(),
                                META_FILE_PATH, &outputFile)) {
            cerr << "CompressModule: Failed to create meta data." << endl;
            return false;
        }
        ofstream outfile(outputFile);
        outfile << scheme->getName();
        outfile.close();
        return true;
    }


    bool set_scheme(BaseHC<Path, Offset, Symbol, Rst>* _scheme) {
        scheme = _scheme;
        if (scheme == nullptr) {
            cerr << "Compress Module: Given scheme pointer is null." << endl;
            return false;
        }
        /* Set the compdata pointer in scheme !!!! */
        scheme->set_compData(compData);
        return true;
    }

    bool set_data(CompData<Offset>* _compData) {
        compData = _compData;
        if (compData == nullptr) {
            cerr << "Compress Module: Given data pointer is null." << endl;
            return false;
        }
        return true;
    }

    void printData() {
        cout << "comp dir: " << compData->get_compress_path()->get_comp_dir() << endl;
    }

    void printSchemeName() {
        scheme->printName();
    }

private:
    CompData<Offset> * compData = nullptr;
    BaseHC<Path, Offset, Symbol, Rst>* scheme = nullptr;
};