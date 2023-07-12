# pragma once
# include <map>
#include "DataPath.hpp"
# include <vector>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <fstream>

template <typename Offset>
struct BlockOffset {
    Offset char_offset;
    Offset ele_offset;   // if larger than 32767 then bootstrap
    Offset hash_numerator;
    long rfid;
};

template <typename Offset>
struct Ele {
    long id;
    Offset local_char_offset;
    Ele(){id=0; local_char_offset=0;}
};

/* We deem that the data uses optimization by default */
// template <typename Offset, typename HC_Scheme>
template <typename Offset>
class CompData {
    public:
        /* Constructor 1: Given a path of a directory that contain compressed data */ 
        CompData(string _compDir) {
            /* Load compressed data */
            if (!setPath(_compDir)) 
                throw runtime_error("CompData: set path failed.");
        }

        bool setPath(string _compDir) {
            cp = new CompressPath(_compDir);
            if (cp == nullptr) {
                return false;
            }
            cp->exhibitPath();
            return true;
        }

        bool loadData() {
            if (cp == nullptr) {
                cerr << "Data path has not been set." << endl;
                return false;
            }

            load_split_file(cp->get_split_file());

            return true;
        }

        bool loadDictionary() {
            ifstream dicFile(cp->get_dic_file(), std::ios_base::binary);
            if (dicFile) {
                int dicSize, totalSize;
                dicFile.read(reinterpret_cast<char*>(&dicSize), sizeof(int));
            }
            else {
                ifstream dicFile(cp->get_dic_file());
                if (!dicFile)   return false;

                int dicSize, totalSize;
                dicFile >> dicSize;
            }
            return true;
        }

        bool isBinaryFile(string _file) {
            ifstream file(_file, std::ios_base::binary);
            if (!file) {
                return false;
            }
            return true;
        }


        bool load_split_file(string _splitFile) {
            /* Plaintext file split input */
            ifstream splitFile(_splitFile);
            if (!splitFile.is_open()) {
                cerr << "CompressData: Failed to open split file: " << _splitFile << endl;
                return false;
            }
            
            splitFile >> split_number;

            split_symbol = new int[split_number];
            memset(split_symbol, 0, split_number);
            int t;
            for (int i = 0; i < split_number; i++) {
                splitFile >> t >> split_symbol[i];
            }
            splitFile.close();

            return true;
        }

        inline void set_split_number(Offset _splitNum) {
            split_number = _splitNum;
        }

        inline Offset get_split_number() {
            return split_number;
        }

        inline const int* get_split_location() {
            if (split_location == nullptr) {
                throw runtime_error("CompressData: split location is not available.");
            }
            return split_location;
        }

        inline const int* get_split_symbol() {
            if (split_symbol == nullptr) {
                throw runtime_error("CompressData: split symbol is not available.");
            }
            // cerr << "CompressData: Got split_symbol." << endl;
            return split_symbol;
        }   

        inline void set_split_location(int* _splitLoc) {
            split_location = _splitLoc;
        }
        
        inline CompressPath* get_compress_path() { return cp; }


    protected:
        CompressPath* cp = nullptr; // Declare a pointer to CompressPath. initialize later.
        
        /* file split information */
        Offset split_number;
        int* split_location = nullptr;
        int* split_symbol = nullptr;
        int block_size = 4096 / sizeof(int);

        vector<vector<vector<Ele<Offset>>>> rule_full;
        vector<vector<BlockOffset<Offset>>> block_off;
};