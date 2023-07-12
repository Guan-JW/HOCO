#include "Parser.hpp"

// Good sample:

std::string good_input = R"(
    TEXT_DIR = ../../_Text/Compressed_data/LZW/COV19
    ELEMENT = WORD
    CODE_SEGMENT_NUM = 2
    SEGMENT 1:
        LOCAL_STRUCTURE_INIT = {
            string word;
            Offset current_offsest = FILE_START;
        }
        END_CONDITION={
            current_offset == FILE_END;
        }
        while (!END_CONDITION) { 
            word = EXTRACT(current_offset++, 1);
            if (wordCount.find(word) == wordCount.end()) 
                wordCount[word] = 1;
            else
                wordCount[word] ++;
        }
    SEGMENT 2:
        LOCAL_STRUCTURE_INIT = {
            string word;
            Offset current_offsest = FILE_START;
        }
        END_CONDITION={
            current_offset == FILE_END;
        }
        while (!END_CONDITION) { 
            word = EXTRACT(current_offset++, 1);
            if (wordCount.find(word) == wordCount.end()) 
                wordCount[word] = 1;
            else
                wordCount[word] ++;
        }
)";

// Input sample with error:

std::string bad_input = R"(
    TEXT_DIR = /path/to/file
    ELEMENT = BYTE
    CODE_SEGMENT_NUM = 2
    SEGMENT 1:
        LOCAL_STRUCTURE_INIT = {
            string word;
            Offset current_offsest = FILE_START;
        }
        END_CONDITION={
            current_offset == FILE_END;
        }
        while (!END_CONDITION) { 
            word = EXTRACT(current_offset++, 1);
            if (wordCount.find(word) == wordCount.end()) 
                wordCount[word] = 1;
            else
                wordCount[word] ++;
        }
    SEGMENT 2:
        LOCAL_STRUCTURE_INIT = {
            string word;
            Offset current_offsest = FILE_START;
        }
        END_CONDITION={
            current_offset == FILE_END;
        }
        while (!END_CONDITION) { 
            word = EXTRACT(current_offset++, 1);
            if (wordCount.find(word) == wordCount.end()) 
                wordCount[word] = 1;
            else
                wordCount[word] ++;
        }
)";

int main() {
    /* Read DSL code */
    std::string inputFile = "input.dsl";
    ifstream input(inputFile);
    if (!input) {
        throw std::runtime_error("Failed to open file.");
    }
    std::string content((std::istreambuf_iterator<char>(input)), 
                            std::istreambuf_iterator<char>());

    /* HOCO Parser */
    if (!parse(content)) {
        throw std::runtime_error("HOCO parsing failed.");
    }

   // Try bad input
    std::cout << "Now we have some errors" << std::endl;
    parse(bad_input);

    input.close();
    return 0;
}