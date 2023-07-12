#pragma once
#include "sequitur_wrapper.hpp"

// One instance correspond to one application or one corpus.
template <typename Path, typename Offset, typename Symbol, typename Rst>
class TADOC_HC : public BaseHC<Path, Offset, Symbol, Rst> {
    public:
        // Constructor
        TADOC_HC(HC_Category _ctg) : BaseHC<Path, Offset, Symbol, Rst>(_ctg) {}
        TADOC_HC() : BaseHC<Path, Offset, Symbol, Rst>() { 
            this->HC_category = PHC;
            /* Set properties*/
            this->property_reset();
            this->set_strong_extract(true);
            this->set_direct_insert(true);
            this->set_direct_delete(true); 
        }

        // Implementation of the pure virtual functions
        bool Compress(Path& input_file, Path& output_file) override {
            resetScheme();
            init_dic();

            /* Create input file descriptor */
            ifstream file(input_file.get_uncomp_file());
            if (!file.is_open()) {
                cerr << "TADOC-HC: Failed to open input file: " << input_file.get_uncomp_file() << endl;
                return false;
            }
            /* Create a file for dictionary output */
            ofstream dicfile(output_file.get_dic_file());
            if (!dicfile.is_open()) {
                cerr << "TADOC-HC: Failed to open dictionary file: " << output_file.get_dic_file() << endl;
                return false;
            }
            /* Plaintext block offset output */
            ofstream blockoffFile(output_file.get_block_offset_file());
            if (!blockoffFile.is_open()) {
                cerr << "TADOC_HC: Failed to open block offset file: " << output_file.get_block_offset_file() << endl;
                return false;
            }
            /* Plaintext offset output */
            ofstream offsetFile(output_file.get_offset_file());
            if (!offsetFile.is_open()) {
                cerr << "TADOC_HC: Failed to open offset file: " << output_file.get_offset_file() << endl;
                return false;
            }

            if (this->compData == nullptr) {
                throw runtime_error("TADOC_HC-Compress: compData not defined.");
                return false;
            }
            /* Copy split file */
            this->compData->get_compress_path()->copy_split_file(input_file.get_split_file());
            /* Load splitting information */
            this->compData->load_split_file(input_file.get_split_file());

            /* Get words */
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);
            int no = numChars;
            vector<int> input;
            string word;
            int max_word_len = 1;
            while ((word = getWordFromUncomp(file)).size() > 0) {
                if (dic.find(word) == dic.end()) {
                    dic[word] = no;
                    dic_rev[no] = word;
                    no ++;
                    max_word_len = word.length() > max_word_len ?
                                    word.length() : max_word_len;
                }
                input.push_back(dic[word]);
            }
            int inputSize = input.size();

            /* Write Dictionary */
            int dicSize = dic.size();
            dicfile.write(reinterpret_cast<const char*>(&dicSize), sizeof(int));
            dicfile.write(reinterpret_cast<const char*>(&max_word_len), sizeof(int));
            for (auto it : dic_rev) {
                dicfile.write(reinterpret_cast<const char*>(&(it.first)), sizeof(int));
                int size = (it.second).size();
                dicfile.write(reinterpret_cast<const char*>(&size), sizeof(int));
                dicfile.write(reinterpret_cast<const char*>((it.second).c_str()), sizeof(char) * (it.second).size());
            }

            /* Compress and write to file */
            int **row, **col;
            int *ruleNum = (int*)malloc(sizeof(int));
            row = (int**)malloc(sizeof(int*));
            col = (int**)malloc(sizeof(int*));
            sequitur(output_file.get_comp_file(), dic, input, row, col, ruleNum, 1);

            /* Write binary output */
            int word_num = dic.size();
            int rule_num = *ruleNum;

            /* Load data into rule structures, and write to file */
            ofstream outfile(output_file.get_comp_file(), std::ios::binary);
            outfile << 0 << " " << word_num << " " << rule_num <<endl;
            struct RULE<Offset>* rule_full = new struct RULE<Offset>[rule_num];
            for (int i = 0; i < rule_num; i ++) {
                int ruleSize = (*row)[i+1] - (*row)[i];
                outfile << ruleSize << " ";
                for (int j = (*row)[i]; j < (*row)[i+1]; j ++) {
                    Ele<Offset> tmp;
                    tmp.id = (*col)[j];
                    rule_full[i].eleIdx.push_back(tmp); // jth element in rule i
                    outfile << (*col)[j] << " ";
                }
            }
            outfile.close();

            /* Generate Block and local offsets */
            int splitCur = 0;
            Offset split_number = this->compData->get_split_number();
            const int* split_symbol = this->compData->get_split_symbol();
            int* split_location = new int[split_number];
            vector<vector<BlockOffset<Offset>>> block_off;
            block_off.resize(rule_num + split_number);

            /* Set split location */
            for (int j = 0; j < rule_full[0].eleIdx.size(); j ++) {
                int id = rule_full[0].eleIdx[j].id;
                if (id == split_symbol[splitCur] && splitCur < split_number) {
                    split_location[splitCur] = j;
                    splitCur ++;
                }
            }
            this->compData->set_split_location(split_location);

            /* 3. Generate offsets */
            struct Ele<Offset> tmp;
            int block_size = 4096 / sizeof(tmp);

            rule_char_length = new Offset[rule_num - 1]();
            memset(rule_char_length, 0, rule_num - 1);
            
            int start = 0, end = 0;
            for (int file = 0; file <= split_number; file ++) {
                if (file != 0)  start = end;
                end = split_location[file];
                if (file == split_number)   
                    end = rule_full[0].eleIdx.size();
                Offset global_char_offset = 0;
                Offset local_char_offset = 0;
                Offset hash_numerator = 0;

                vector<BlockOffset<Offset>> rule_offset;
                for (int i = start; i < end; i ++) {
                    if (i != start && (i - start) % block_size == 0) {  // create block
                        BlockOffset<Offset> tmp;
                        global_char_offset += local_char_offset;
                        tmp.char_offset = global_char_offset;   // char offset inside the file
                        tmp.ele_offset = i - start; // element offset inside the file
                        tmp.hash_numerator = hash_numerator;
                        rule_offset.push_back(tmp);

                        local_char_offset = 0;
                        hash_numerator = 0;
                    }
                    int eleId = rule_full[0].eleIdx[i].id;
                    if (eleId < word_num) {
                        rule_full[0].eleIdx[i].local_char_offset = local_char_offset;
                        local_char_offset += dic_rev[eleId].length();
                        hash_numerator += dic_rev[eleId].length() * ((i-start) % block_size);
                    }
                    else {
                        if(rule_char_length[eleId - word_num - 1] == 0) {
                            dfs(eleId, rule_full, block_off, block_size, word_num, split_number);
                        }
                        rule_full[0].eleIdx[i].local_char_offset = local_char_offset;
                        local_char_offset += rule_char_length[eleId - word_num - 1];
                        hash_numerator += rule_char_length[eleId - word_num - 1] * ((i-start) % block_size);
                    }
                }
                BlockOffset<Offset> tmp;
                global_char_offset += local_char_offset;
                tmp.char_offset = global_char_offset; // char offset inside the file
                tmp.ele_offset = end - start;   // element offset inside the file
                tmp.hash_numerator = hash_numerator;
                rule_offset.push_back(tmp);

                block_off[file] = rule_offset;
            }

            /* Write to files */
            for (int i = 0; i < rule_num; i ++) {
                for (int j = 0; j < rule_full[i].eleIdx.size(); j ++) {
                    offsetFile << rule_full[i].eleIdx[j].local_char_offset << " ";
                }
                offsetFile << endl;
            }
            for (int i = 0; i < rule_num + split_number; i ++) {
                for(int j = 0; j < block_off[i].size(); j ++) {
                    blockoffFile << block_off[i][j].char_offset << " " 
                            << block_off[i][j].ele_offset << " " 
                            << block_off[i][j].hash_numerator << " ";
                }
                blockoffFile << endl;
            }

            delete []rule_full;

            file.close();
            dicfile.close();
            offsetFile.close();
            blockoffFile.close();

            return true;
        }

        bool Decompress(Path& output_file) override {
            /* Binary input */
            ifstream infile(this->compData->get_compress_path()->get_comp_file(), std::ios::binary);    // input file
            if (!infile.is_open()) {
                cerr << "TADOC_HC: Failed to open input file: " << this->compData->get_compress_path()->get_comp_file() << endl;
                return false;
            }
            /* Dictionary input (Plaintext) */
            ifstream dicfile(this->compData->get_compress_path()->get_dic_file());
            if (!dicfile.is_open()) {
                cerr << "TADOC_HC: Failed to open dictionary file: " << this->compData->get_compress_path()->get_dic_file() << endl;
                return false;
            }
            /* Plaintext output */
            ofstream outfile(output_file.get_uncomp_file());
            if (!outfile.is_open()) {
                cerr << "TADOC_HC: Failed to open output file: " << output_file.get_uncomp_file() << endl;
                return false;
            }

            /* Load dictionary */
            int dicSize, totalSize, ruleSize, maxWordLen;
            dicfile.read(reinterpret_cast<char*>(&dicSize), sizeof(int));
            dicfile.read(reinterpret_cast<char*>(&maxWordLen), sizeof(int));

            vector<string> dDic(dicSize);
            char wordStr[maxWordLen + 1];
            for (int i = 0; i < dicSize; i ++) {
                int id, size;
                dicfile.read(reinterpret_cast<char*>(&id), sizeof(int));
                dicfile.read(reinterpret_cast<char*>(&size), sizeof(int));
                dicfile.read(wordStr, sizeof(char) * size);
                wordStr[size]='\0';
                dDic[id]=string(wordStr, size);
            }

            /* Load compressed data */
            int file_id, word_num, rule_num;
            infile >> file_id >> word_num >> rule_num;
            struct RULE<Offset>* rule_full = new struct RULE<Offset>[rule_num];
            for (int i = 0; i < rule_num; i ++) {
                int ruleSize;
                infile >> ruleSize;
                for (int j = 0; j < ruleSize; j ++) {
                    Ele<Offset> tmp;
                    infile >> tmp.id;
                    rule_full[i].eleIdx.push_back(tmp);
                }
            }

            for (int i = 0; i < rule_full[0].eleIdx.size(); i ++) {
                int eleId = rule_full[0].eleIdx[i].id;
                if (eleId < word_num) { // word
                    outfile << dDic[eleId];
                }
                else {
                    dDFS(eleId, word_num, dDic, rule_full, outfile);
                }
            }

            delete []rule_full;

            /* Copy split file */
            string _split = "fileYyNO.txt";
            string dst;
            output_file.path_join(output_file.getDirectoryPath(output_file.get_uncomp_file()), _split, &dst);
            output_file.copy_split_file(this->compData->get_compress_path()->get_split_file(), dst);
            
            infile.close();
            dicfile.close();
            outfile.close();
            return true;
        }

        // For compress
        void dfs(int rule, struct RULE<Offset>* rule_full, 
                    vector<vector<BlockOffset<Offset>>> & block_off,
                    int block_size, int word_num, int split_number) {
            int rule_id = rule - word_num;
            Offset global_char_offset = 0;
            Offset local_char_offset = 0;
            Offset hash_numerator = 0;
            vector<BlockOffset<Offset>> rule_offset;

            for (int i = 0; i < rule_full[rule_id].eleIdx.size(); i ++) {
                if(i != 0 && i % block_size == 0) {    // create new block
                    BlockOffset<Offset> tmp;
                    global_char_offset += local_char_offset;
                    tmp.char_offset = global_char_offset; // char offset inside the rule
                    tmp.ele_offset = i;   // element offset inside the rule
                    tmp.hash_numerator = hash_numerator;
                    rule_offset.push_back(tmp);

                    local_char_offset = 0;
                    hash_numerator = 0;
                }
                int eleId = rule_full[rule_id].eleIdx[i].id;
                if(eleId < word_num) {
                    rule_full[rule_id].eleIdx[i].local_char_offset = local_char_offset;
                    local_char_offset += dic_rev[eleId].length();
                    hash_numerator += dic_rev[eleId].length() * (i % block_size);
                }
                else {
                    if(rule_char_length[eleId - word_num - 1] == 0) {
                        dfs(eleId, rule_full, block_off, block_size, word_num, split_number);
                    }
                    rule_full[rule_id].eleIdx[i].local_char_offset = local_char_offset;
                    local_char_offset += rule_char_length[eleId - word_num - 1];
                    hash_numerator += rule_char_length[eleId - word_num - 1] * (i % block_size);
                }
            }
            // add the end offset
            BlockOffset<Offset> tmp;
            global_char_offset += local_char_offset;
            tmp.char_offset = global_char_offset; // char offset inside the rule
            tmp.ele_offset = rule_full[rule_id].eleIdx.size();   // element offset inside the rule
            tmp.hash_numerator = hash_numerator;
            rule_offset.push_back(tmp);
            block_off[rule_id + split_number] = rule_offset;

            rule_char_length[rule_id - 1] = global_char_offset;
        }

        // For decompress
        void dDFS(int rule, int word_num, vector<string> dDic, 
                struct RULE<Offset>* rule_full, ofstream& outfile) {
            int rule_id = rule - word_num;
            for (int i = 0; i < rule_full[rule_id].eleIdx.size(); i ++) {
                int eleId = rule_full[rule_id].eleIdx[i].id;
                if (eleId < word_num) { // word
                    outfile << dDic[eleId];
                }
                else {
                    dDFS(eleId, word_num, dDic, rule_full, outfile);
                }
            }
        }

        string getWordFromUncomp(ifstream& file) {
            string word;
            char c;     // current char
            static bool indicator = false;
            static char old;    // last char
            if (indicator == true) {    // last time meet a normal char
                indicator = false;
                return string(1, old);  // return the old char as a string
            }
            // every time get a single char
            while (file.get(c)) {
                if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == '.')) {
                    // if not a boundary just return the char
                    word += c;          // add c to the buffer
                    indicator = true;   
                }
                else {
                    // arrive the boundary c between words, last time not meet a normal
                    if (!indicator) 
                        return string(1, c);
                    // if have met a boundary, last time meet a normal char
                    old = c;
                    return word;
                }
            }

            return word;
        }

        /* Dictionary */
        void init_dic() {
            dic.clear();
            dic_rev.clear();
            
            int no = 0;
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);

            vector<int> tmp(2);
            for (int i = 0; i < numChars; i ++) {
                string charString(1, Dic_Init_Chars[i]);
                dic[charString] = no;
                dic_rev[no] = charString;
                no ++;
            }
        }

        void printName() const {
            cout << "TADOC" << endl;
        }
        string getName() { return "TADOC_HC"; }

        void resetScheme() override {
            BaseHC<Path, Offset, Symbol, Rst>::resetScheme();
            
            dic.clear();
            dic_rev.clear();
            if (rule_char_length != nullptr) {
                delete []rule_char_length;  // ok?
                rule_char_length = nullptr;
            }
        }
        
    private:
        map<string, int> dic;    // hashmap from a string to a interger        
        map<int, string> dic_rev;
        Offset* rule_char_length = nullptr;
};
