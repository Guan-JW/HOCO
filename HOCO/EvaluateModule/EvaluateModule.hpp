#pragma once
#include "../DataManage/CompressData.hpp"
#include "../SchemeCollectModule/CollectModule.hpp"
#include "DSL/Parser.hpp"
#include "DSL/Skeleton.hpp"

#define WORD "WORD"
#define BYTE "BYTE"

#define PRE_READ_WORD_SIZE 5




/* Evaluation Module Implementation */
template <typename Path, typename Offset, typename Symbol, typename Rst>
class EvaluationModule {

public:
    EvaluationModule() {}
    /* Used for DSL generated code */
    EvaluationModule(string _compDir) {
        compData = new CompData<Offset>(_compDir);
        string schemeName;
        get_meta_scheme_name(schemeName);
        cerr << "EvaluationModule: Got scheme " + schemeName << "." << endl;
        
        BaseHC<Path, Offset, Symbol, Rst> *
            Scheme(FACTORY(BaseHC, Path, Offset, Symbol, Rst).create(schemeName));
        
        scheme = Scheme;
        scheme->set_compData(compData);
        scheme->get_compData()->load_split_file(compData->get_compress_path()->get_split_file());

        // Remove this line !!!!
        scheme->property_reset();
    }

    /* High-level APIs */
    Rst API_Extract(Offset offset_start, Offset length) {
        if (!check_data_validity()) {
            return "";
        }
        Rst result;
        if (get_element() == BYTE) {
            if (!scheme->Extract(offset_start, length, &result)) {
                cerr << "EvaluationModule: Extraction failed." << endl;
                return "";
            }
            cerr << "Finish extracting, counter = " << scheme->get_dcounter() << endl;
        }
        else {  // WORD
            /* Check if need basic implementation */
            if (!scheme->get_direct_extract()) {
                // cerr << "Extracting with WORD granularity and the basic implementation." << endl;
                if (compData == nullptr || scheme->get_compData() == nullptr) {
                    throw runtime_error("EvaluationModule: CompressedData pointer is nullptr.");
                }
                
                string tmpDecompFile = compData->get_compress_path()->get_comp_dir() + TEMP_PATH;
                if (!EWBuffer.modifyBuffer(tmpDecompFile, offset_start, length, scheme)) {
                    EWBuffer.clear_buffer();
                    throw runtime_error("EvaluationModule: Failed to modify buffer.");
                    return "";
                }
                return EWBuffer.getBuffer(offset_start, length);
            }
        }

        return result;
    }

    bool API_Insert(Offset offset_start, Symbol *str) {
        if (!check_data_validity())
            return false;

        if (!scheme->Insert(offset_start, str)) {   // To modify
            cerr << "EvaluationModule: Insertion failed." << endl;
            return false;
        }
        
        /* Check if WORD granularity and need basic implementation */
        if (get_element() == WORD && !scheme->get_direct_extract()) 
            EWBuffer.increment_counter();

        return true;
    }

    bool API_Delete(Offset offset_start, Offset length) {
        if (!check_data_validity())
            return false;
        
        if (!scheme->Delete(offset_start, length)) {
            cerr << "EvaluationModule: Deletion failed." << endl;
            return false;
        }
        
        /* Check if WORD granularity and need basic implementation */
        if (get_element() == WORD && !scheme->get_direct_extract()) 
            EWBuffer.increment_counter();

        return true;
    }

    /* DSL Parser */
    bool DSL_Parser(string const& input_dsl) {
        /* Load dsl into string */
        ifstream inputDSL(input_dsl);
        if (!inputDSL) {
            throw std::runtime_error("Failed to open the dsl file.");
        }
        std::string input((std::istreambuf_iterator<char>(inputDSL)), 
                            std::istreambuf_iterator<char>());
        inputDSL.close();

        /* Start parsing */
        using boost::spirit::x3::ascii::space;
        typedef std::string::const_iterator iterator_type;

        iterator_type iter = input.begin();
        iterator_type const end = input.end();

        using boost::spirit::x3::with;
        using boost::spirit::x3::error_handler_tag;
        using error_handler_type = boost::spirit::x3::error_handler<iterator_type>;

        // Our error handler
        error_handler_type error_handler(iter, end, std::cerr);

        // Our parser
        using client::parser::hococode;
        auto const parser =
            // we pass our error handler to the parser so we can access
            // it later in our on_error and on_sucess handlers
            with<error_handler_tag>(std::ref(error_handler))
            [
                hococode
            ];

        bool r = phrase_parse(iter, end, parser, space, ast);

        if (r && iter == end) { // success
            std::cout << boost::fusion::tuple_open('[');
            std::cout << boost::fusion::tuple_close(']');
            std::cout << boost::fusion::tuple_delimiter(", ");

            std::cout << "-------------------------\n";
            cout << "Parsing succeeded\n";
            std::cout << "test_dir = " << ast.test_dir << std::endl;
            std::cout << "element = " << ast.element << std::endl;
            std::cout << "seg_num = " << ast.seg_num << std::endl;
            if (ast.seg_num != ast.seg.size()) {
                std::cerr << "HOCO Parser: Given segment number mismatch." << std::endl;
                return false;
            }
            std::cout << "global_structure_init = " << ast.global_structure_init << std::endl;
            for (auto const& emp : ast.seg)
            {
                std::cout << "seg: " << emp << std::endl;
            }
            std::cout << "\n-------------------------\n";
        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
            ast.seg.clear();
            return false;
        }

        return true;
    }

    static void removeBraces(std::string& codeString) {
        size_t startPos = codeString.find_first_not_of(" \t\r\n"); // Find the first non-whitespace character
        size_t endPos = codeString.find_last_not_of(" \t\r\n"); // Find the last non-whitespace character

        if (startPos != std::string::npos && endPos != std::string::npos &&
            codeString[startPos] == '{' && codeString[endPos] == '}') {
            codeString = codeString.substr(startPos + 1, endPos - startPos - 1);
        }
    }

    static void trimString(string& str) {
        // Find the position of the first non-whitespace character
        size_t startPos = str.find_first_not_of(" \t\r\n");

        // Find the position of the last non-whitespace character
        size_t endPos = str.find_last_not_of(" \t\r\n");

        // If startPos is not equal to std::string::npos, there are non-whitespace characters
        if (startPos != std::string::npos) {
            // Extract the substring between the first non-whitespace character and the last non-whitespace character
            str = str.substr(startPos, endPos - startPos + 1);
        } else {
            // If startPos is equal to std::string::npos, the string is empty or contains only whitespace characters
            str.clear();
        }
    }

    /* Translate the AST (generated by parser) into a C++ file */
    bool AST_Translator(const string& gen_fileName) {
        if (ast.test_dir == "") {
            cerr << "EvaluationModule-AST_Translator: AST not defined." << endl;
            return false;
        }

        /* Create the target directory */
        string dir = Path::getDirectoryPath(gen_fileName);
        int existence = Path::path_exist(dir);
        if (existence < 0) {
            cerr << "EvaluationModule-AST_Translator: Given directory (" + dir + ") doesn't exist, creat it." << endl;
            if (!Path::makePath(dir)) {
                cerr << "EvaluationModule-AST_Translator: Couldn't create directory (" + dir + "." << endl;
                return false;
            }
        } else if (existence == 1) {
            cerr << "EvaluationModule-AST_Translator: Given directory (" + dir + ") is a file name." << endl;
            return false;
        }

        /* Generate code */
        ofstream genCode(gen_fileName);

        /* 1. File header */
        genCode << skeleton_HEADER;
        
        /* 2. Global structure init */
        string placeHolder = "/* AST code here */";
        string injectStr;
        Offset position = skeleton_GLOBAL_STRUCT_INIT.find(placeHolder) + placeHolder.length();
        string code = skeleton_GLOBAL_STRUCT_INIT;
        if (position != string::npos) {
            injectStr = ast.global_structure_init;
            removeBraces(injectStr);
            code.insert(position, "\n" + injectStr);
        }
        genCode << code;

        /* 3. Action functions definition */
        regex segNum_Regex(R"(\{NUM\})");
        for (int i = 1; i <= ast.seg_num; i ++) {
            /* Get function name */
            code = skeleton_SEGMENT;
            code = regex_replace(code, segNum_Regex, to_string(i));

            /* Inject code */
            position = 0;

            /* 3.1 Local sturcture init */
            if ((position = code.find(placeHolder, position)) != string::npos) {
                injectStr = ast.seg[i-1].local_structure_init;
                removeBraces(injectStr);
                // code.insert(position, "\n" + injectStr);
                code.replace(position, placeHolder.length(), injectStr);
                position += injectStr.length();
            }
            /* 3.2 End condition */
            if ((position = code.find(placeHolder, position)) != string::npos) {
                injectStr = ast.seg[i-1].end_condition;
                removeBraces(injectStr);
                code.replace(position, placeHolder.length(), injectStr);
                // code.insert(position, injectStr);
                position += injectStr.length();
            }
            /* 3.3 Action */
            if ((position = code.find(placeHolder, position)) != string::npos) {
                injectStr = ast.seg[i-1].while_action;
                removeBraces(injectStr);
                // code.insert(position, "\n" + injectStr);
                code.replace(position, placeHolder.length(), injectStr);
                position += injectStr.length();
            }
            /* 3.4 Update end condition */
            if ((position = code.find(placeHolder, position)) != string::npos) {
                injectStr = ast.seg[i-1].end_condition;
                removeBraces(injectStr);
                // code.insert(position, injectStr);
                code.replace(position, placeHolder.length(), injectStr);
                position += injectStr.length();
            }
            genCode << code;
        }

        /* 4. Main */
        code = skeleton_MAIN_BEFORE_ACTION;
        position = 0;
        /* test_dir */
        if ((position = code.find(placeHolder)) != string::npos) {
            injectStr = ast.test_dir;
            trimString(injectStr);
            removeBraces(injectStr);
            code.replace(position, placeHolder.length(), '"' + injectStr + '"');
            position += injectStr.length();
        }
        /* element */
        regex element_Regex(R"(\{ELEMENT\})");
        injectStr = ast.element;
        trimString(injectStr);
        code = regex_replace(code, element_Regex, ast.element);
        genCode << code;

        /* Call functions */
        for (int i = 1; i <= ast.seg_num; i ++) {
            /* Get function name */
            code = skeleton_MAIN_ACTIONS;
            code = regex_replace(code, segNum_Regex, to_string(i));
            genCode << code;
        }

        /* End */
        genCode << skeleton_MAIN_END;

        genCode.close();
        return true;
    }

    bool get_meta_scheme_name(string& schemeName) {
        string metaFile;
        if (!Path::path_join(compData->get_compress_path()->get_comp_dir(),
                                META_FILE_PATH, &metaFile)) {
            throw runtime_error("EvaluationModule: No scheme meta data exist.");
            return false;
        }

        ifstream metaInput(metaFile);
        metaInput >> schemeName;
        metaInput.close();
        return true;
    }

    bool check_data_validity() {
        if (scheme == nullptr || compData == nullptr) {
            cerr << "EvaluationModule: Scheme or compData not set." << endl;
            return false;
        } 
        if (scheme->get_compData() == nullptr) {
            cerr << "EvaluationModule: Scheme's compData not set, set it." << endl;
            scheme->set_compData(compData);
        }
        return true;
    }

    bool set_element(const string wordByte) {
        if (wordByte == WORD)
            ast.element = WORD;
        else if (wordByte == BYTE)
            ast.element = BYTE;
        else {
            ast.element = BYTE;
            cerr << "No such element (" << wordByte << ") allowed, set it to BYTE." << endl;
            return false;
        }
        return true;
    }

    string get_element() {
        if (ast.element == "") {
            throw runtime_error("EvaluateModule: ast.element not known.");
        }
        
        if (ast.element == WORD || ast.element == BYTE)
            return ast.element;
        
        throw runtime_error("EvaluateModule: ast.element not known.");
    }

    BaseHC<Path, Offset, Symbol, Rst>* get_scheme() {
        return scheme; 
    }


private:
    CompData<Offset> * compData = nullptr;
    BaseHC<Path, Offset, Symbol, Rst>* scheme = nullptr;
    string decompPath = "";

    client::ast::hococode ast;

private:
    struct ExtractWordBuffer {
        string fileName;
        vector<Rst> wordBuffer;
        Offset offset_end = 0;    // the ending offset of the last word (byte)
        int modifyCounter = 0;
        bool fileEOF = false;
        mutex bufferMutex;

        // _fileName derives from compData, already checked existence
        bool modifyBuffer(const string& _fileName, Offset _elementOffset, Offset _wordNum,
            BaseHC<Path, Offset, Symbol, Rst>* scheme) {
            lock_guard<mutex> lock(this->bufferMutex);

            /* New file or the file's modified */
            if (this->fileName != _fileName || this->modifyCounter > 0) {
                /* Decompress */
                DecompressPath path(_fileName);
                if (!scheme->Decompress(path)) {
                    throw runtime_error("EvaluateModule: Decomperssion failed.");
                    return false;
                }

                /* Reset members */
                this->wordBuffer.clear();
                this->fileName = _fileName;
                this->modifyCounter = 0;
                this->fileEOF = false;

                /* Read words from file */
                ifstream file(this->fileName, ios::in);
                if (!file) {
                    cerr << "EvaluateModule: Failed to open file " << this->fileName << "." << endl;
                    return false;
                }
                // Get words from the beginning, with pre read
                for (Offset i = 0; i < _elementOffset + _wordNum + PRE_READ_WORD_SIZE; i ++) {
                    Rst word = getWordFromUncomp(file);   // update the offset_end member
                    if (file.eof()) {   // meet the end of file
                        this->fileEOF = true;
                        if (i < _elementOffset - 1) 
                            cerr << "EvaluateModule: Given offset exceeds the total number of words contained in the file." << endl;
                        else if (i < _elementOffset + _wordNum - 1)
                            cerr << "EvaluateModule: No enough words." << endl;
                        else if (i < _elementOffset + _wordNum + PRE_READ_WORD_SIZE - 1)   // Just pre read some words
                            break;  // true here
                        file.close();
                        return false;
                    }
                    this->wordBuffer.push_back(word);

                }
                file.close();
                return true;
            }
            
            /* If the given element_offset is larger than the number of words saved in the buffer, read more */
            if (_elementOffset + _wordNum > wordBuffer.size()) {
                if (this->fileEOF) { // already end of file, no more words for reading
                    cerr << "EvaluateModule: Meet the end of file before the given offset." << endl;
                    return true;  // true here
                }

                ifstream file(this->fileName, ios::in);
                if (!file) {
                    cerr << "EvaluateModule: Failed to open file " << this->fileName << "." << endl;
                    return false;
                }
                // Move the file pointer to the desired offset
                file.seekg(this->offset_end, ios::beg);

                // Get words from offset_end
                for (Offset i = wordBuffer.size(); i < _elementOffset + _wordNum + PRE_READ_WORD_SIZE; i ++) {
                    Rst word = getWordFromUncomp(file);   // update the offset_end member
                    if (file.eof()) {   // meet the end of file
                        this->fileEOF = true;
                        if (i < _elementOffset - 1) 
                            cerr << "EvaluateModule: Given offset exceeds the total number of words contained in the file." << endl;
                        else if (i < _elementOffset + _wordNum - 1)
                            cerr << "EvaluateModule: No enough words." << endl;
                        else if (i <  _elementOffset + _wordNum + PRE_READ_WORD_SIZE - 1)   // Just pre read some words
                            break;  // true here
                        file.close();
                        return false;
                    }
                    this->wordBuffer.push_back(word);
                }
                file.close();
                return true;
            }
            
            /* Required words are already obtained */
            return true;
        }
        
        /* Must be called after modifyBuffer checks, the buffer contains the desired words by default */
        /* Multiple words devided by "." */
        Rst getBuffer(Offset _elementOffset, Offset _wordNum) {
            lock_guard<mutex> lock(this->bufferMutex);
            Rst result = "";
            for (Offset i = _elementOffset; i < _elementOffset + _wordNum; i ++) {
                result += this->wordBuffer[i];
            }
            return result;
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
                this->offset_end ++;  // add one
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

        void clear_buffer() {
            this->fileName = "";
            this->wordBuffer.clear();
            this->offset_end = 0;    // the ending offset of the last word (byte)
            this->modifyCounter = 0;
            this->fileEOF = false;
        }

        void increment_counter() {
            lock_guard<mutex> lock(this->bufferMutex);
            this->modifyCounter ++;
        }
    };

    class FileEndCondition {
    private:
        EvaluationModule<Path, Offset, Symbol, Rst> * EM;
    public:
        FileEndCondition(EvaluationModule<Path, Offset, Symbol, Rst> * _EM)
            {
                EM = _EM;
            }

        /* Overload the == operator */
        template <typename T>
        bool operator==(const T& other) const { // other is right-hand-side value of ==
            return isFileEnd(other);
        }
        /* Function to check if it's the FILE_END condition */
        template <typename T>
        bool isFileEnd(const T& value) const {
            /* Check if scheme exist */
            if (!EM->get_scheme()) {
                cerr << "EvaluationModule-FileEndCondition: No scheme available." << endl;
                return false;
            }
            /* Decompress-based method */
            if (!EM->get_scheme()->get_direct_extract()) {  // Just consider extract here
                return EM->get_extract_word_buffer()->fileEOF;
            }
            /* TODO: Compressed-based meethod */
            return false;
        }
    };

    ExtractWordBuffer EWBuffer;

public: 

    ExtractWordBuffer* get_extract_word_buffer() { return &EWBuffer; }

    FileEndCondition get_file_end() {
        return FileEndCondition(this);
    }

    Offset get_file_start() {
        EWBuffer.clear_buffer();
        return Offset(0);
    }

    /* Overload == operator for FILE_END, value is left-hand-side value of == */
    template <typename T>
    friend bool operator==(const T& value, const FileEndCondition& rhs) {
        return rhs.isFileEnd(value);
    }

};


/* Register */
REGISTER(BaseHC, RLE_HC, Path, size_t, string, string);
REGISTER(BaseHC, LZW_HC, Path, size_t, string, string);
REGISTER(BaseHC, TADOC_HC, Path, size_t, string, string);