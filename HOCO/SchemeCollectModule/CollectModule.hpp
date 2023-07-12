#pragma once
#include "schemes/include_list.txt"
#include "Factory.hpp"
#include "../DataManage/DataPath.hpp"

template <typename Path, typename Offset, typename Symbol, typename Rst>
class SchemeCollectModule {
public:
    SchemeCollectModule()  {
        load_uncomp_data();
        load_decomp_data();
        backup_dir = test_dir + "/" + scheme->getName() + "/Backup_data";
    }
    SchemeCollectModule(BaseHC<Path, Offset, Symbol, Rst>* _scheme) : scheme(_scheme) {
        string dir = test_dir + "/" + scheme->getName() + "/Compressed_data";
        compData = new CompData<Offset>(dir);
        scheme->set_compData(compData);

        load_uncomp_data();
        load_decomp_data();
        backup_dir = test_dir + "/" + scheme->getName() + "/Backup_data";
    }

    void load_uncomp_data() {
        string file = PARENT_PATH + "input/Original_data/COV19/yelp_academic_dataset_covid_features.json";
        string splitfile = PARENT_PATH + "input/Original_data/COV19/fileYyNO.txt";
        upath = new UncompressPath(file, splitfile);
    }

    void load_decomp_data() {
        string decompFile = test_dir + "/" + scheme->getName() + "/Decompressed_data/decomp.txt";
        dpath = new DecompressPath(decompFile);
    }

    void load_backupUncomp_data() {
        if (upath == nullptr) {
            cerr << "SchemeCollectModule: upath is nullptr. Create it." << endl;
            load_uncomp_data();
        }
        bupath = new BackupUncompPath(backup_dir, upath->get_split_file());
        string backup_path = bupath->get_uncomp_file();
        backupFile(upath->get_uncomp_file(), backup_path);
    }

    bool DeComp_correctness() {
        // If scheme set
        if (scheme == nullptr) {
            cerr << "SchemeCollectModule: No scheme available." << endl;
            return false;
        }
        // Compress
        if (!scheme->Compress(*upath, *(compData->get_compress_path())) ) {
            cerr << "SchemeCollectModule: Compression process failed." << endl;
            return false;
        }
        // Decompress
        if (!scheme->Decompress(*dpath)) {
            cerr << "SchemeCollectModule: Decompression process failed." << endl;
            return false;
        }
        // Same
        if (!compareFiles(upath->get_uncomp_file(), dpath->get_uncomp_file())) {
            return false;
        }

        /* Clear all the data structures */
        return true;
    }

    bool Evaluation_Check() {

        if (scheme == nullptr) {
            cerr << "SchemeCollectModule: No scheme available." << endl;
            return false;
        }

        /* Backup the original file */
        load_backupUncomp_data();
        string backup_path = bupath->get_uncomp_file();

        /* First generate compressed data */
        if (!scheme->Compress(*upath, *(compData->get_compress_path())) ) {
            cerr << "SchemeCollectModule: Compression process failed." << endl;
            return false;
        }

        scheme->set_compData(compData);

        /* Extract */
        if (!Evaluation_Check_Extract()) {
            cerr << "SchemeCollectModule: Extract check failed." << endl;
            return false;
        }

        // /* Insert */
        if (!Evaluation_Check_Insert()) {
            cerr << "SchemeCollectModule: Insert check failed." << endl;
        }

        /* Delete */
        if (!Evaluation_Check_Delete()) {
            cerr << "SchemeCollectModule: Delete check failed." << endl;
        }

        scheme->judge_category();

        return true;
    }

    bool Evaluation_Check_Extract() {
        /* Correctness, directness, strong homomorphism, category*/
        scheme->set_strong_extract(false);   // set direct and strong homomorphic to false

        Rst rst;
        int num_checks = sizeof(Start_offs) / sizeof(Start_offs[0]);

        for (int i = 0; i < num_checks; i ++) {
            Offset offset_start = Start_offs[i];
            Offset length = Lengths[i];
            if (offset_start >= getFileSize(upath->get_uncomp_file())) {
                cerr << "SchemeCollectModule: File is shorter than offset_start " << offset_start << endl;
                continue;
            }
            auto start = chrono::high_resolution_clock::now();
            Rst rst_org = readSubstringFromTxTFile<Offset, Rst>(upath->get_uncomp_file(), offset_start, length);
            auto org_end = chrono::high_resolution_clock::now();
            scheme->Extract(offset_start, length, &rst);
            auto sch_end = chrono::high_resolution_clock::now();
            
            // correctness
            if (rst_org != rst) {
                cerr << "SchemeCollectModule: Extract gives wrong result. Set scheme to UNHC." << endl;
                // set scheme to UNHC
                scheme->set_strong_extract(false);
                scheme->set_strong_insert(false);
                scheme->set_strong_delete(false);
                return false;   // extract is the most important, return false directly
            }

            // directness, strong homomorphism
            auto duration1 = chrono::duration_cast<chrono::microseconds>(org_end - start);
            auto duration2 = chrono::duration_cast<chrono::microseconds>(sch_end - org_end);
            if (duration1 * direct_tolerance_rate < duration2) {
                cerr << "SchemeCollectModule: Extract doesn't satisfy directness. Time(uncompressed): " << duration1.count() 
                        << " ms, time(" << scheme->getName() << "): " << duration2.count() <<" ms." << endl; 

                scheme->set_strong_extract(false);  // If extract is not direct, insert and delete are likely the same \
                                                         For extract, direct == strong, since no compression needed
                return true;
            }
            scheme->set_strong_extract(true);   
        }

        return true;
    }
    
    bool Evaluation_Check_Insert() {
        /* If not loaded, backup file -- means the first evaluation check */
        if (bupath == nullptr) {
            cerr << "Insert loading backup data.." << endl;
            load_backupUncomp_data();
        }
        string backup_path = bupath->get_uncomp_file();
        
        scheme->set_strong_insert(false);   // set direct and strong homomorphic to false

        int num_checks = sizeof(Start_offs) / sizeof(Start_offs[0]);
        for (int i = 0; i < num_checks; i ++) {
            Offset offset_start = Start_offs[i];
            Offset length = Lengths[i];

            string insert_str = generateRandomString(length);
            if (offset_start >= getFileSize(upath->get_uncomp_file())) {
                cerr << "SchemeCollectModule: File is shorter than offset_start " << offset_start << endl;
                continue;
            }

            /* Insert into plaintext file */
            auto start = chrono::high_resolution_clock::now();
            insertSubstringIntoTxTFile<Offset, Symbol>(backup_path, offset_start, insert_str);
            auto org_end = chrono::high_resolution_clock::now();
            
            /* Insert into compressed file */
            scheme->Insert(offset_start, &insert_str);
            auto sch_end = chrono::high_resolution_clock::now();

            /* Correctness check */ 
            /* Decompress */
            if (!scheme->Decompress(*dpath)) {
                cerr << "SchemeCollectModule: Decompression process failed." << endl;
                return false;
            }
            if (!compareFiles(backup_path, dpath->get_uncomp_file())) { // reject the insert function
                scheme->set_strong_insert(false);
                return false;
            }

            /* Directness check */
            auto duration1 = chrono::duration_cast<chrono::microseconds>(org_end - start);
            auto duration2 = chrono::duration_cast<chrono::microseconds>(sch_end - org_end);
            if (duration1 * direct_tolerance_rate < duration2) {
                cerr << "SchemeCollectModule: Insert doesn't satisfy directness. Time(uncompressed): " << duration1.count() 
                        << " ms, time(" << scheme->getName() << "): " << duration2.count() <<" ms." << endl; 
                
                scheme->set_strong_insert(false);  // direct and strong, all set to false
                return true;    // if one test doesn't satisfy, then return
            }

            /* Strong homomorphism check */
            /* Compress modified text */
            string backup_compDir = backup_dir + "/Compressed_data";
            CompData<Offset> ccompData(backup_compDir);
            if (!scheme->Compress(*bupath, *ccompData.get_compress_path())) {
                cerr << "SchemeCollectModule: Modified plaintext compression failed." << endl;
                return false;
            }
            if (compareFiles(ccompData.get_compress_path()->get_comp_file(), compData->get_compress_path()->get_comp_file())) {
                scheme->set_strong_insert(true);
            } else {
                scheme->set_strong_insert(false);   // this set both direct and strong to false
                scheme->set_direct_insert(true);    // this reset direct to true
            }

        }
        return true;
    }

    bool Evaluation_Check_Delete() {
        /* If not loaded, backup file -- means the first evaluation check */
        if (bupath == nullptr) {
            cerr << "Delete loading backup data.." << endl;
            load_backupUncomp_data();
        }
        string backup_path = bupath->get_uncomp_file();  // get backup file path
        
        scheme->set_strong_delete(false);   // set direct and strong homomorphic to false        
        
        int num_checks = sizeof(Start_offs) / sizeof(Start_offs[0]);
        for (int i = 0; i < num_checks; i ++) {
            Offset offset_start = Start_offs[i];
            Offset length = Lengths[i];

            if (offset_start >= getFileSize(upath->get_uncomp_file())) {
                cerr << "SchemeCollectModule: File is shorter than offset_start " << offset_start << endl;
                continue;
            }

            /* Delete from plaintext file */
            auto start = chrono::high_resolution_clock::now();
            deleteSubstringFromTxTFile<Offset>(backup_path, offset_start, length);
            auto org_end = chrono::high_resolution_clock::now();
            
            /* Delete from compressed file */
            scheme->Delete(offset_start, length);
            auto sch_end = chrono::high_resolution_clock::now();

            /* Correctness check */ 
            /* Decompress */
            if (!scheme->Decompress(*dpath)) {
                cerr << "SchemeCollectModule: Decompression process failed." << endl;
                return false;
            }
            if (!compareFiles(backup_path, dpath->get_uncomp_file())) { // reject the insert function
                scheme->set_strong_insert(false);
                return false;
            }

            /* Directness check */
            auto duration1 = chrono::duration_cast<chrono::microseconds>(org_end - start);
            auto duration2 = chrono::duration_cast<chrono::microseconds>(sch_end - org_end);
            if (duration1 * direct_tolerance_rate < duration2) {
                cerr << "SchemeCollectModule: Delete doesn't satisfy directness. Time(uncompressed): " << duration1.count() 
                        << " ms, time(" << scheme->getName() << "): " << duration2.count() <<" ms." << endl; 
                
                scheme->set_strong_delete(false);  // direct and strong, all set to false
                return true;    // if one test doesn't satisfy, then return
            }

            /* Strong homomorphism check */
            /* Compress modified text */
            string backup_compDir = backup_dir + "/Compressed_data";
            CompData<Offset> ccompData(backup_compDir);
            if (!scheme->Compress(*bupath, *ccompData.get_compress_path())) {
                cerr << "SchemeCollectModule: Modified plaintext compression failed." << endl;
                return false;
            }
            if (compareFiles(ccompData.get_compress_path()->get_comp_file(), compData->get_compress_path()->get_comp_file())) {
                scheme->set_strong_insert(true);
            } else {
                scheme->set_strong_insert(false);   // this set both direct and strong to false
                scheme->set_direct_insert(true);    // this reset direct to true
            }
        }

        return true;
    }


    bool compareFiles(const string& file1, const string& file2) {
        // Open the files in binary mode \
            and positions the file pointers at the end of the files
        ifstream stream1(file1, ios::binary | ios::ate);
        ifstream stream2(file2, ios::binary | ios::ate);

        if (!stream1 || !stream2) {
            // Error opening files
            cerr << "SchemeCollectModule: Error opening files." << endl;
            return false;
        }

        auto size1 = stream1.tellg();
        auto size2 = stream2.tellg();
        if (size1 != size2) {
            // Files have different sizes
            cerr << "SchemeCollectModule: Files have different sizes." << endl;
            return false;
        }

        stream1.seekg(0);
        stream2.seekg(0);

        if (equal(istreambuf_iterator<char>(stream1),
                    istreambuf_iterator<char>(),
                    istreambuf_iterator<char>(stream2))) {
            // Files have the same contents
            return true;
        }

        // Files have different contents
        cerr << "SchemeCollectModule: Files have different contents." << endl;
        return false;
    }

    bool backupFile(const string& originalFile, const string& backupFile) {
        ifstream src(originalFile, ios::binary);
        ofstream dst(backupFile, ios::binary);
        if (!src) {
            cerr << "CollectModule - backupFile: Couldn't open the source file(" << originalFile << ")." << endl;
            return false;
        }
        if (!dst) {
            cerr << "CollectModule - backupFile: Couldn't open the backup file(" << backupFile << ")." << endl;
            return false;
        }
        dst << src.rdbuf();
        return true;
    }


private:    
    string test_dir = "testoutput";
    string backup_dir = "testoutput/Backup";

    UncompressPath * upath = nullptr;
    DecompressPath * dpath = nullptr;
    BackupUncompPath * bupath = nullptr;
    
    CompData<Offset> * compData = nullptr;
    BaseHC<Path, Offset, Symbol, Rst>* scheme = nullptr;

    int direct_tolerance_rate = 1;
};