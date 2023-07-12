TEXT_DIR = ../../_Text/Compressed_data/LZW/COV19
ELEMENT = WORD
CODE_SEGMENT_NUM = 1

GLOBAL_STRUCTURE_INIT = {
    map < string , int > wordCount;
}

SEGMENT 1: 
        LOCAL_STRUCTURE_INIT = {
            string word;
            Offset current_offset = FILE_START;
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