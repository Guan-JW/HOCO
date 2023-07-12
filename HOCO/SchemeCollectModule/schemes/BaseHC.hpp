# pragma once
#include "../../DataManage/CompressData.hpp"

#define TEMP_PATH "/TempDecomp/temperal.txt"

/* Properties of HC schemes */
struct Property {
    bool direct_extract = false;
    bool direct_insert = false;
    bool direct_delete = false;
    
    bool strong_extract = false;
    bool strong_insert = false;
    bool strong_delete = false;
};

/* A counter used for Decompression-based schemes,
    to avoid repeated decompression.
    Note that the structure should be updated when used in modification ops,
    and should be cleared when an evaluation process is finished. */
struct DecompCounter {
    string decompPath = "";
    int counter = -1;   // -1 means not decompressed, 0 means no need to decomperss \
                            > 0 means updated
};


/* Base class of HC schemes */
// Rst is restricted to be string
// Symbol is restricted to be string, too
template <typename Path, typename Offset, typename Symbol, typename Rst>
class BaseHC {

public:
        /* Constructor */
        BaseHC(HC_Category _ctg) {
            HC_category = _ctg;
            if (!isValidCategory()) {
                HC_category = NOT_DEFINED;
                cerr << "Warning: Invalid category given. Set it to NOT_DEFINED by default." << endl;
            }
        }
        BaseHC() { HC_category = NOT_DEFINED; }

        /* Pure virtual functions, must be implemented by all derived classes */
        virtual bool Compress(Path& input_file, Path& output_file) = 0;
        virtual bool Decompress(Path& output_file) = 0;  // store to file by default, and keep it in memory decompData
        virtual void printName() const = 0;
        virtual string getName();

        /* Basic operations: decompress + operate (+ compress) */
        virtual bool Extract(Offset offset_start, Offset length, Rst* result) {
            string output_file = compData->get_compress_path()->get_comp_dir() + TEMP_PATH;
            DecompressPath path(output_file);
            
            /* Check if data object is available and correct */
            /* Load into memory*/
            if (!checkDataValidity()) {
                cerr << "BaseHC-Extract: No available compressed data." << endl;
                return false;
            }

            /* Decompress and store to a file */
            if (check_need_decomp(output_file)) {  // need to decompress 
                if (!Decompress(path)) {
                    throw runtime_error("BaseHC-Extract: Decomperssion failed.");
                    /* Decompress may corruptes the outputfile, so update DecompCounter */
                    dCounter.counter = -1;
                    return false;
                }
            }

            /* Extract substring with the file system in C-style coding */ 
            if (offset_start >= getFileSize(output_file)) {
                throw out_of_range("Start position is out of range.");
                return false;
            }
            *result = readSubstringFromTxTFile<Offset, Rst>(output_file, offset_start, length);

            reset_decompData(); // free decompData
            /* Delete TempDecomp directory !!!!!!*/
            
            return true;
        }


        virtual bool Insert(Offset offset_start, Symbol *str) {
            if (str == nullptr) {
                cerr << "Warning: The string pointer for insertion is null." << endl;
                return false;
            }

            /* Check if data object is available and correct */
            /* Load into memory*/
            if (!checkDataValidity()) {
                cerr << "BaseHC-Insert: No available compressed data." << endl;
                return false;
            }

            string output_file = compData->get_compress_path()->get_comp_dir() + TEMP_PATH;
            DecompressPath path(output_file);
            /* Decompress and store to a file */
            if (check_need_decomp(output_file)) {  // need to decompress 

                if (!Decompress(path)) {
                    cerr << "BaseHC-Insert: Decomperssion failed." << endl;
                    /* Decompress may corruptes the outputfile, so update DecompCounter */
                    dCounter.counter = -1;
                    return false;
                }
            }
            
            /* Insert directly into TempDecomp file */
            if (offset_start >= getFileSize(output_file)) {
                cerr << "BaseHC-Insert: Start position is out of range." << endl;
                return false;
            }
            if (!insertSubstringIntoTxTFile<Offset, Symbol>(output_file, offset_start, *str)) {
                cerr << "BaseHC-Insert: Failed to insert." << endl;
                return false;
            }

            if (!Compress(path, *(compData->get_compress_path()))) {    // cover the compressed file
                throw runtime_error("BaseHC-Insert: Compression failed.");
                return false;
            }

            /* Update decompCounter */
            dCounter.counter ++;

            reset_decompData(); // free decompData
            /* Delete TempDecomp directory !!!!!!*/
            return true;
        }

        virtual bool Delete(Offset offset_start, Offset length) {
            /* Check if data object is available and correct */
            /* Load into memory*/
            if (!checkDataValidity()) {
                cerr << "BaseHC-Delete: No available compressed data." << endl;
                return false;
            }

            // string output_file = string(TEMP_STORE_PATH) + "temperal.bin";
            string output_file = compData->get_compress_path()->get_comp_dir() + TEMP_PATH;           
            DecompressPath path(output_file);
            /* Decompress and store to a file */
            if (check_need_decomp(output_file)) {  // need to decompress 
                if (!Decompress(path)) {
                    throw runtime_error("BaseHC-Delete: Decomperssion failed.");
                    /* Decompress may corruptes the outputfile, so update DecompCounter */
                    dCounter.counter = -1;
                    return false;
                }
            }

            /* Delete directly from TempDecomp file */
            if (offset_start >= getFileSize(output_file)) {
                cerr << "BaseHC-Delete: Start position is out of range." << endl;
                return false;
            }
            if (!deleteSubstringFromTxTFile<Offset>(output_file, offset_start, length)) {
                cerr << "BaseHC-Delete: Failed to delete." << endl;
                return false;
            }

            if (!Compress(path, *(compData->get_compress_path()))) {    // cover the compressed file
                throw runtime_error("BaseHC-Delete: Compression failed.");
                return false;
            }

            /* Update decompCounter */
            dCounter.counter ++;

            /* Delete TempDecomp directory !!!!!!*/
            return true;
        }

        virtual bool Symbol_cmp(Symbol s1, Symbol s2) {
            return (s1 == s2);
        }

        void loadTextFile(const string& filePath, string* content) {
            ifstream file(filePath);
            if (!file.is_open()) {
                *content = "";
                throw runtime_error("BaseHC: Failed to open decompressed data.");
            }
            *content = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
        }

        inline short get_ctg() { return HC_category; }

        void reset_decompData() {
            if (decompData != nullptr) {
                delete decompData;
                decompData = nullptr;
            }
        }

        inline void set_compData(CompData<Offset> * _compData) { compData = _compData; }
        
        /* Property related functions */
        inline void property_reset() { struct Property p; Ppt = p; }
        inline void set_direct_extract(bool state) { Ppt.direct_extract = state; }
        inline void set_direct_insert(bool state) { Ppt.direct_insert = state; }
        inline void set_direct_delete(bool state) { Ppt.direct_delete = state; }
        inline bool get_direct_extract() { return Ppt.direct_extract; }
        inline bool get_direct_insert() { return Ppt.direct_insert; }
        inline bool get_direct_delete() { return Ppt.direct_delete; }
        inline void set_strong_extract(bool state) { Ppt.direct_extract = state; Ppt.strong_extract = state; }
        inline void set_strong_insert(bool state) { Ppt.direct_insert = state; Ppt.strong_insert = state; }
        inline void set_strong_delete(bool state) { Ppt.direct_delete = state; Ppt.strong_delete = state; }
        inline bool get_strong_extract() { return Ppt.strong_extract && Ppt.direct_extract; }
        inline bool get_strong_insert() { return Ppt.strong_insert && Ppt.direct_insert; }
        inline bool get_strong_delete() { return Ppt.strong_delete && Ppt.direct_delete; }
        inline struct Property get_property()   { return Ppt; }

        /* Category related functions */
        inline void reset_category() { HC_category = NOT_DEFINED; }
        inline void set_UNHC() { HC_category = UNHC; }
        inline void set_LHC() { HC_category = LHC; }
        inline void set_PHC() { HC_category = PHC; }
        inline void set_FHC() { HC_category = FHC; }
        inline string get_category() {
            switch (HC_category) {
                case UNHC:
                    return "UNHC";
                case LHC:
                    return "LHC";
                case PHC:
                    return "PHC";
                case FHC:
                    return "FHC";
                default: 
                    return "NOT_DEFINED";
            }
        }
        
        virtual inline void judge_category() final {    // not allowed to override
            // strong homomorphism must based on directness
            bool strong_extract = Ppt.strong_extract && Ppt.direct_extract;
            bool strong_insert = Ppt.strong_insert && Ppt.direct_insert;
            bool strong_delete = Ppt.strong_delete && Ppt.direct_delete;
            
            if (strong_extract && strong_insert && strong_delete)
                set_FHC();  // correct, direct, and strong homomorphic for all ops
            else if (strong_extract || strong_insert || strong_delete)
                set_PHC();  // correct, direct, and strong homomorphic for partial ops
            else if (Ppt.direct_extract || Ppt.direct_insert || Ppt.direct_delete)
                set_LHC();  // correct and direct for partial ops
            else
                set_UNHC(); // correct, not direct -- need decompression
        }

        /* DecompCounter related functions */
        bool check_need_decomp(string decompPath) {     // only called before decompression
            /* Case 1: new target path */
            if (dCounter.decompPath != decompPath) {    
                dCounter.decompPath = decompPath;
                dCounter.counter = 0;   // Will perform a new decompression, set it to zero
                return true;
            }
            /* Case 2: compressed file has been updated */
            if (dCounter.counter != 0) {
                dCounter.counter = 0;   // Will perform a new decompression, set it to zero
                return true;
            }

            return false;
        }
        int get_dcounter() {
            return dCounter.counter;
        }
        void resetDecompCounter() {
            dCounter.decompPath = "";
            dCounter.counter = -1;
        }

        // Just clear all the structures that contain data, \
            save the category and property information.
        virtual void resetScheme() {
            /* Reset DecompCounter */
            resetDecompCounter();
        }

        CompData<Offset> * get_compData() { return compData; }

protected:
        /* protected members can be accessed by derived classes */
        HC_Category HC_category;
        // HC_CompressData HC_compData;
        CompData<Offset> * compData = nullptr;      // A pointer to compressed data

        string * decompData = nullptr;  // A pointer to decompressed data \
                                            for temperarily store decompressed data

        struct Property Ppt;    // property


private:
        bool isValidCategory() {
            return HC_category >= NOT_DEFINED && HC_category <= FHC;
        }

        bool checkDataValidity() {
            /* Add checking on if data resides in memory. */
            if (compData == nullptr)    return false;
            if (compData->get_compress_path() == nullptr)   return false;
            /* Add checks !!! */
            return true;
        }

        DecompCounter dCounter;
};

