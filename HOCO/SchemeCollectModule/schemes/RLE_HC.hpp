
// One instance correspond to one application or one corpus.
template <typename Path, typename Offset, typename Symbol, typename Rst>
class RLE_HC : public BaseHC<Path, Offset, Symbol, Rst> {
    public:
        // Constructor
        RLE_HC(HC_Category _ctg) : BaseHC<Path, Offset, Symbol, Rst>(_ctg) {}
        RLE_HC() : BaseHC<Path, Offset, Symbol, Rst>() { 
            this->HC_category = FHC; 
            /* Set properties*/
            this->property_reset();
            this->set_strong_extract(true);
            this->set_strong_insert(true);
            this->set_strong_delete(true);
        }

        // Implementation of the pure virtual functions
        bool Compress(Path& input_file, Path& output_file) override {
            
            resetScheme();
            init_dic();

            /* Plaintext input */
            ifstream file(input_file.get_uncomp_file());
            if (!file.is_open()) {
                cerr << "RLE_HC: Failed to open input file: " << input_file.get_uncomp_file() << endl;
                return false;
            }
            /* Binary output */
            ofstream outfile(output_file.get_comp_file(), std::ios::binary);    // output file
            if (!outfile.is_open()) {
                cerr << "RLE_HC: Failed to open output file: " << output_file.get_comp_file() << endl;
                return false;
            }
            /* Dictionary output */
            ofstream dicfile(output_file.get_dic_file(), std::ios::binary);
            if (!dicfile.is_open()) {
                cerr << "RLE_HC: Failed to open dictionary file: " << output_file.get_dic_file() << endl;
                return false;
            }
            /* Plaintext block offset output */
            ofstream blockoffFile(output_file.get_block_offset_file());
            if (!blockoffFile.is_open()) {
                cerr << "RLE_HC: Failed to open block offset file: " << output_file.get_block_offset_file() << endl;
                return false;
            }
            /* Plaintext offset output */
            ofstream offsetFile(output_file.get_offset_file());
            if (!offsetFile.is_open()) {
                cerr << "RLE_HC: Failed to open offset file: " << output_file.get_offset_file() << endl;
                return false;
            }

            if (this->compData == nullptr) {
                throw runtime_error("RLE_HC-Compress: compData not defined.");
                return false;
            }
            /* Copy split file */
            this->compData->get_compress_path()->copy_split_file(input_file.get_split_file());
            /* Load splitting information */
            this->compData->load_split_file(input_file.get_split_file());

            /* Turn input into words */ 
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);
            int no = numChars;
            vector<int> input;
            string word;
            vector<int> tmp(2);
            int counter = 0;
            int max_word_len = 1;
            while ((word = getWordFromUncomp(file)).size() > 0) {
                int x;
                if (dic.find(word) == dic.end()) {
                    x = no;
                    dic[word] = no;
                    dic_rev[no] = word;
                    no ++;
                    max_word_len = word.length() > max_word_len ? 
                                    word.length() : max_word_len;
                } else
                    x = dic[word];
                input.push_back(x);
            }
            int inputSize = input.size();

            /* Generate Block and local offsets */
            int block_size = 4096 / sizeof(int);
            Offset block_charoffset = 0;
            Offset block_eleoff = 0;
            Offset block_hash_numerator = 0;

            Offset total_eleoff = 0;
            Offset cnt_eleoff = 0;
            Offset local_charoffset = 0;
            offsetFile << local_charoffset << " ";

            int splitCur = 0;
            Offset split_number = this->compData->get_split_number();
            const int* split_symbol = this->compData->get_split_symbol();
            int* split_location = new int[split_number];

            Offset run_size = 1;
            int c = input[0];
            int old_c = c;

            for (int i = 1; i < inputSize; i ++) {
                c = input[i];
                if (c != old_c) {
                    outfile.write(reinterpret_cast<const char*>(&old_c), sizeof(int));
                    int char_length = run_size * dic_rev[old_c].length();
                    local_charoffset += char_length;

                    block_charoffset += char_length;
                    block_hash_numerator += char_length * cnt_eleoff;
                    cnt_eleoff ++;

                    // meet the end of file, create a new block
                    if (c == split_symbol[splitCur] && splitCur < split_number) {
                        block_eleoff += cnt_eleoff;
                        blockoffFile << block_charoffset << " " << block_eleoff << " " << block_hash_numerator << endl;
                        total_eleoff += block_eleoff;

                        block_hash_numerator = 0;
                        block_charoffset = 0;
                        block_eleoff = 0;
                        cnt_eleoff = 0;
                        local_charoffset = 0;

                        split_location[splitCur] = block_eleoff + cnt_eleoff;
                        splitCur ++;
                    } 
                    else if (cnt_eleoff == block_size) {
                        // meet the end of a block
                        block_eleoff += cnt_eleoff;
                        blockoffFile << block_charoffset << " " << block_eleoff << " " << block_hash_numerator << " ";
                        block_hash_numerator = 0;
                        cnt_eleoff = 0;
                        local_charoffset = 0;
                    }
                    offsetFile << local_charoffset << " "; // write offset
                    run_size = 0;
                    old_c = c;
                }
                run_size ++;
            }
            if (cnt_eleoff) {   // remain elements, create a block
                outfile.write(reinterpret_cast<const char*>(&old_c), sizeof(int));
                int char_length = run_size * dic_rev[old_c].length();
                local_charoffset += char_length;
                offsetFile << local_charoffset; // write offset
                    
                block_charoffset += char_length;
                block_hash_numerator += char_length * cnt_eleoff;
                cnt_eleoff ++;
                block_eleoff += cnt_eleoff;
                blockoffFile << block_charoffset << " " << block_eleoff << " " << block_hash_numerator;
                total_eleoff += block_eleoff;
            }

            /* set split location */
            if (this->compData == nullptr) {
                throw runtime_error("RLE_HC-Compress: compData not defined.");
                return false;
            }
            this->compData->set_split_location(split_location);

            /* Write Dictionary */
            int dicSize = dic.size();
            dicfile.write(reinterpret_cast<const char*>(&dicSize), sizeof(int));
            dicfile.write(reinterpret_cast<const char*>(&total_eleoff), sizeof(int));
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
            /* Binary input */
            ifstream infile(this->compData->get_compress_path()->get_comp_file(), std::ios::binary);    // input file
            if (!infile.is_open()) {
                cerr << "RLE_HC: Failed to open input file: " << this->compData->get_compress_path()->get_comp_file() << endl;
                return false;
            }
            /* Dictionary input */
            ifstream dicfile(this->compData->get_compress_path()->get_dic_file(), std::ios::binary);
            if (!dicfile.is_open()) {
                cerr << "RLE_HC: Failed to open dictionary file: " << this->compData->get_compress_path()->get_dic_file() << endl;
                return false;
            }
            /* Plaintext offset input*/
            ifstream blockofffile(this->compData->get_compress_path()->get_block_offset_file());
            if (!blockofffile.is_open()) {
                cerr << "RLE_HC: Failed to open block offset file: " << this->compData->get_compress_path()->get_block_offset_file() << endl;
                return false;
            }
            /* Plaintext offset input*/
            ifstream offfile(this->compData->get_compress_path()->get_offset_file());
            if (!offfile.is_open()) {
                cerr << "RLE_HC: Failed to open offset file: " << this->compData->get_compress_path()->get_offset_file() << endl;
                return false;
            }
            /* Plaintext output */
            ofstream outfile(output_file.get_uncomp_file());
            if (!outfile.is_open()) {
                cerr << "RLE_HC: Failed to open output file: " << output_file.get_uncomp_file() << endl;
                return false;
            }

            /* Load dictionary */
            int dicSize, totalSize, maxWordLen;
            dicfile.read(reinterpret_cast<char*>(&dicSize), sizeof(int));
            dicfile.read(reinterpret_cast<char*>(&totalSize), sizeof(int));
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
            int* input = new int[totalSize];
            infile.read(reinterpret_cast<char*>(input), sizeof(int) * totalSize);
            
            Offset local_charoff = 0;
            Offset block_charoff = 0;
            int run_time = 0;

            offfile >> local_charoff;   // input
            Offset old_local_charoff = local_charoff;
            Offset old_block_charoff = block_charoff;
            

            if (this->compData == nullptr) {
                throw runtime_error("RLE_HC-Compress: compData not defined.");
                return false;
            }
            int splitCur = 0;
            Offset split_number = this->compData->get_split_number();
            const int* split_symbol = this->compData->get_split_symbol();

            Offset tmp1;
            Offset tmp2;

            int i = 0;
            for ( ; i < totalSize; i ++) {  // for each element, calculate its run_time
                // how many chars contained in the word
                int c = input[i];

                string word = dDic[c];   
                Offset char_length = word.length();

                // calculate run time
                offfile >> local_charoff;
                if (local_charoff < old_local_charoff) {
                    //  new block

                    blockofffile >> block_charoff;
                    blockofffile >> tmp1;
                    blockofffile >> tmp2;

                    if (block_charoff < old_block_charoff) {
                        // new file
                        splitCur ++;
                        old_block_charoff = 0;
                    }
                    run_time = ((int)(block_charoff - old_block_charoff - old_local_charoff) / (int)char_length);
                    
                    old_block_charoff = block_charoff;
                    for (int rt = 0; rt < run_time; rt ++) 
                        outfile << word;

                    // deal with the first word in next block
                    old_local_charoff = 0;
                }
                else {
                    run_time = ((int)(local_charoff - old_local_charoff) / (int)char_length);
                    for (int rt = 0; rt < run_time; rt ++)
                        outfile << word;

                    old_local_charoff = local_charoff;
                }

            }

            delete []input;

            /* Copy split file */
            string _split = "fileYyNO.txt";
            string dst;
            output_file.path_join(output_file.getDirectoryPath(output_file.get_uncomp_file()), _split, &dst);
            output_file.copy_split_file(this->compData->get_compress_path()->get_split_file(), dst);
            
            infile.close();
            dicfile.close();
            outfile.close();
            offfile.close();
            blockofffile.close();
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

            int no = 0;
            int numChars = sizeof(Dic_Init_Chars) / sizeof(Dic_Init_Chars[0]);

            for (int i = 0; i < numChars; i ++) {
                string charString(1, Dic_Init_Chars[i]);
                dic[charString] = no;
                dic_rev[no] = charString;
                no ++;
            }
        }

        void printName() const {
            cout << "RLE" << endl;
        }

        string getName() { return "RLE_HC"; }

        void resetScheme() override {
            BaseHC<Path, Offset, Symbol, Rst>::resetScheme();
            
            dic.clear();
            dic_rev.clear();
        }

    private:
        ska::flat_hash_map<string, int> dic;    // hashmap from a string to a interger
        map<int, string> dic_rev;
};
