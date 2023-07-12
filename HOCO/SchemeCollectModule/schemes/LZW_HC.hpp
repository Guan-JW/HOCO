
// One instance correspond to one application or one corpus.
template <typename Path, typename Offset, typename Symbol, typename Rst>
class LZW_HC : public BaseHC<Path, Offset, Symbol, Rst> {
    public:
        // Constructor
        LZW_HC(HC_Category _ctg) : BaseHC<Path, Offset, Symbol, Rst>(_ctg) {}
        LZW_HC() : BaseHC<Path, Offset, Symbol, Rst>() { 
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

            /* Plaintext input */
            ifstream file(input_file.get_uncomp_file());
            if (!file.is_open()) {
                cerr << "LZW_HC: Failed to open input file: " << input_file.get_uncomp_file() << endl;
                return false;
            }
            
            /* Binary output */
            ofstream outfile(output_file.get_comp_file(), std::ios::binary);    // output file
            if (!outfile.is_open()) {
                cerr << "LZW_HC: Failed to open output file: " << output_file.get_comp_file() << endl;
                return false;
            }
            /* Dictionary output */
            ofstream dicfile(output_file.get_dic_file(), std::ios::binary);
            if (!dicfile.is_open()) {
                cerr << "LZW_HC: Failed to open dictionary file: " << output_file.get_dic_file() << endl;
                return false;
            }
            /* Plaintext block offset output */
            ofstream blockoffFile(output_file.get_block_offset_file());
            if (!blockoffFile.is_open()) {
                cerr << "LZW_HC: Failed to open block offset file: " << output_file.get_block_offset_file() << endl;
                return false;
            }
            /* Plaintext offset output */
            ofstream offsetFile(output_file.get_offset_file());
            if (!offsetFile.is_open()) {
                cerr << "LZW_HC: Failed to open offset file: " << output_file.get_offset_file() << endl;
                return false;
            }

            if (this->compData == nullptr) {
                throw runtime_error("LZW_HC-Compress: compData not defined.");
                return false;
            }
            /* Copy split file */
            this->compData->get_compress_path()->copy_split_file(input_file.get_split_file());
            /* Load splitting information */
            this->compData->load_split_file(input_file.get_split_file());

            /* Turn input into words */ 
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);
            int no = DICMAXSIZE + numChars;
            int outputSize = 0;
            vector<int> input;
            string word;
            int max_word_len = 1;
            vector<int> tmp(2);
            while ((word = getWordFromUncomp(file)).size() > 0) {
                int x;
                if (dic.find(word) == dic.end()) {
                    x = no;
                    dic[word] = no;
                    dic_rev[no] = word;
                    tmp[0] = no;
                    tmp[1] = no;
                        
                    rules[tmp] = no;
                    
                    no ++;
                    max_word_len = word.length() > max_word_len ?
                                    word.length() : max_word_len;
                } else
                    x = dic[word];
                
                input.push_back(x);
            }
            int inputSize = input.size();
            int ruleSize = 0; // Size of rules

            vector<int> w;
            w.push_back(0);

            /* Generate Block and local offsets */
            
            int block_size = 4096 / sizeof(int);
            Offset block_charoff = 0;
            Offset block_eleoff = 0;
            Offset block_hash_numerator = 0;

            int cnt_eleoff = 0;
            Offset local_charoffset = 0;
            offsetFile << local_charoffset << " ";

            int splitCur = 0;
            Offset split_number = this->compData->get_split_number();
            const int* split_symbol = this->compData->get_split_symbol();
            int* split_location = new int[split_number];

            for (int i = 0; i < inputSize; i ++) {
                int c = input[i];
                vector<int> wc(w);
                wc.push_back(c);
                wc[0] += c; // make the vector hashable
                
                if (rules.count(wc))
                    // c in dic, add it and come to next iteration
                    w = wc;
                else {
                    // c not in dic, add wc to dic and write w to file
                    int value = rules[w];
                    outfile.write(reinterpret_cast<const char*>(&value), sizeof(int));
                    
                    Offset length = 0;
                    for (Offset i = 0; i < w.size(); i ++)
                        length += dic_rev[w[i] - DICMAXSIZE].length();
                    local_charoffset += length;
                    offsetFile << local_charoffset << " "; // write offset
                    block_charoff += length;
                    block_hash_numerator += length * cnt_eleoff;
                    rules[wc] = ruleSize ++;
                    w.clear();
                    outputSize ++;
                    cnt_eleoff ++;

                    if (c - DICMAXSIZE == split_symbol[splitCur] && splitCur < split_number) {
                        // if meet the end of a file
                        // create new block
                        block_eleoff += cnt_eleoff;

                        blockoffFile << block_charoff << " " << block_eleoff << " " << block_hash_numerator << endl;

                        // reset block list
                        block_charoff = 0;
                        block_eleoff = 0;
                        block_hash_numerator = 0;
                        cnt_eleoff = 0;
                        local_charoffset = 0;

                        split_location[splitCur] = block_eleoff;
                        splitCur ++;
                    }
                    else {
                        if (cnt_eleoff == block_size) { // create new block
                            block_eleoff += cnt_eleoff;
                            blockoffFile << block_charoff << " " << block_eleoff << " " << block_hash_numerator << " ";
                            block_hash_numerator = 0;
                            cnt_eleoff = 0;
                            local_charoffset = 0;
                        }
                    }
                    vector<int> tmp(2);
                    tmp[0] = c;
                    tmp[1] = c;
                    w = tmp;
                }
            }
            // if end with a word in dictionary
            if (!w.empty()) {
                int value = rules[w];
                outfile.write(reinterpret_cast<const char*>(&value), sizeof(int));
                outputSize ++;

                cnt_eleoff ++;
                block_eleoff += cnt_eleoff;
                blockoffFile << block_charoff << " " << block_eleoff << " " << block_hash_numerator;
                Offset length = 0;
                for (Offset i = 0; i < w.size(); i ++)
                    length += dic_rev[w[i]].length();
                local_charoffset += length;
                offsetFile << local_charoffset;
            }
            /* set split location */
            this->compData->set_split_location(split_location);

            /* Write Dictionary */
            int dicSize = dic.size();
            dicfile.write(reinterpret_cast<const char*>(&dicSize), sizeof(int));
            dicfile.write(reinterpret_cast<const char*>(&outputSize), sizeof(int));
            dicfile.write(reinterpret_cast<const char*>(&ruleSize), sizeof(int));
            dicfile.write(reinterpret_cast<const char*>(&max_word_len), sizeof(int));

            for (ska::flat_hash_map<string, int>::iterator i = dic.begin(); 
                    i != dic.end(); i ++) {
                dicfile.write(reinterpret_cast<const char*>(&(i->second)), sizeof(int));
                int size = (i->first).size();
                dicfile.write(reinterpret_cast<const char*>(&size), sizeof(int));
                dicfile.write(reinterpret_cast<const char*>((i->first).c_str()), sizeof(char) * (i->first).size());
                
            }

            /* Calculate compression ratio */
            struct stat s{};
            stat(input_file.get_uncomp_file().c_str(), &s);
            Offset original_length = s.st_size;

            stat(output_file.get_dic_file().c_str(), &s);
            Offset dic_length = s.st_size;
            stat(output_file.get_comp_file().c_str(), &s);
            Offset output_length = s.st_size;
            stat(output_file.get_block_offset_file().c_str(), &s);
            Offset block_length = s.st_size;
            stat(output_file.get_offset_file().c_str(), &s);
            Offset offset_length = s.st_size;

            output_length += dic_length; 
            cout << "Orginal size : " << original_length << " " << ", Compressed size : " << output_length << ", Dictionary size : " << dic_length << ", Block size : " << block_length  << endl;
            cout << "Compression ratio (text+dic) : " << (double)original_length / (output_length + dic_length) << endl;
            cout << "Compression ratio (text+dic+block) : " << (double)original_length / (output_length + dic_length + block_length) << endl;
            cout << "Dictionary occ : " << (double)dic_length / (output_length + dic_length + block_length) << endl;
            cout << "Block occ : " << (double)block_length / (output_length + dic_length + block_length) << endl;
            cout << endl;

            file.close();
            outfile.close();
            dicfile.close();
            offsetFile.close();
            blockoffFile.close();

            return true;
        }


        bool Decompress(Path& output_file) override {
            /* Allocate a map for decoding */
            unordered_map< int,string> rules;

            /* Binary input */
            ifstream infile(this->compData->get_compress_path()->get_comp_file(), std::ios::binary);    // input file
            if (!infile.is_open()) {
                cerr << "LZW_HC: Failed to open input file: " << this->compData->get_compress_path()->get_comp_file() << endl;
                return false;
            }
            /* Dictionary input */
            ifstream dicfile(this->compData->get_compress_path()->get_dic_file(), std::ios::binary);
            if (!dicfile.is_open()) {
                cerr << "LZW_HC: Failed to open dictionary file: " << this->compData->get_compress_path()->get_dic_file() << endl;
                return false;
            }
            /* Plaintext output */
            ofstream outfile(output_file.get_uncomp_file());
            if (!outfile.is_open()) {
                cerr << "LZW_HC: Failed to open output file: " << output_file.get_uncomp_file() << endl;
                return false;
            }

            /* Load dictionary */
            int dicSize, totalSize, ruleSize, maxWordLen;
            dicfile.read(reinterpret_cast<char*>(&dicSize), sizeof(int));
            dicfile.read(reinterpret_cast<char*>(&totalSize), sizeof(int));
            dicfile.read(reinterpret_cast<char*>(&ruleSize), sizeof(int));
            dicfile.read(reinterpret_cast<char*>(&maxWordLen), sizeof(int));

            vector<string> dDic(dicSize);
            char wordStr[maxWordLen + 1];
            for (int i = 0; i < dicSize; i ++) {
                int id, size;
                dicfile.read(reinterpret_cast<char*>(&id), sizeof(int));
                dicfile.read(reinterpret_cast<char*>(&size), sizeof(int));
                dicfile.read(wordStr, sizeof(char) * size);
                wordStr[size]='\0';
                dDic[id - DICMAXSIZE]=string(wordStr, size);
            }

            /* Load compressed data */
            int* input = new int[totalSize];
            infile.read(reinterpret_cast<char*>(input), sizeof(int) * totalSize);

            /* Decompress */
            char prev[10000];
            char output[10000];
            int* p = (int*)input; 

            // Get the first code
            int code = p[0];
            memcpy(output, (char*)input, sizeof(int));
            output[sizeof(int)] = '\0';
            p = (int*)output;
            outfile << dDic[*p - DICMAXSIZE];

            p = input;
            int preSize, ruleID = 0, lastOutputSize = sizeof(int);
            for (int i = 1; i < totalSize; i ++) {
                code = p[i];
                memcpy(prev, output, lastOutputSize);
                prev[lastOutputSize + sizeof(int)] = '\0';

                // decode new rule
                string st(prev, lastOutputSize + sizeof(int));
                rules[ruleID++] = st;
                if (code >= DICMAXSIZE) {   // if not rule
                    memcpy(output, (char*)&p[i], sizeof(int));  // set prev to the new code
                    preSize = sizeof(int);
                }
                else {
                    memcpy(output, (char*)rules[code].c_str(), rules[code].size());
                    preSize = rules[code].size();
                }

                for (int j = lastOutputSize; j < lastOutputSize + sizeof(int); j ++) {
                    prev[j] = output[j - lastOutputSize];
                }
                string st2(prev, lastOutputSize + sizeof(int));
                rules[ruleID-1] = st2;

                if (code < DICMAXSIZE) {
                    memcpy(output, (char*)rules[code].c_str(), rules[code].size());
                }

                for (int j = 0; j < preSize; j += 4) {
                    int* p = (int*)&output[j];
                    outfile << dDic[*p - DICMAXSIZE];
                }
                lastOutputSize = preSize;
                
            }

            delete[] input;

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

            int no = DICMAXSIZE;
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);

            vector<int> tmp(2);
            for (int i = 0; i < numChars; i ++) {
                string charString(1, Dic_Init_Chars[i]);
                dic[charString] = no;
                dic_rev[no] = charString;
                tmp[0] = no;
                tmp[1] = no;
                rules[tmp] = no++;
            }
        }

        void printName() const {
            cout << "LZW" << endl;
        }

        string getName() { return "LZW_HC"; }

        //
        void resetScheme() override {
            BaseHC<Path, Offset, Symbol, Rst>::resetScheme();
            
            dic.clear();
            dic_rev.clear();
            rules.clear();
        }

    private:
        ska::flat_hash_map<string, int> dic;    // hashmap from a string to a interger
        map<int, string> dic_rev;
        unordered_map<vector<int>, int, VectorHasher> rules;    // hashmap from a vector to a interger
};
