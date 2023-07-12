
#include "../EvaluateModule.hpp"

#define EXTRACT(offset_start, length) EM->API_Extract(offset_start, length)
#define FILE_START EM->get_file_start()
#define FILE_END EM->get_file_end()


/* Global structure init */
/* AST code here */

    map < string , int > wordCount;


template <typename Path, typename Offset, typename Symbol, typename Rst>
void func_SEGMENT1(EvaluationModule<Path, Offset, Symbol, Rst>* EM) {
    /* Local structure init */
    
            string word;
            Offset current_offset = FILE_START;
        

    /* End condition */
    bool end_condition = 
            current_offset == FILE_END;
        

    /* Actions */
    while (!end_condition) {
         
            word = EXTRACT(current_offset++, 1);
            if (wordCount.find(word) == wordCount.end()) 
                wordCount[word] = 1;
            else
                wordCount[word] ++;
        

        /* Update end condition here */
        end_condition = 
            current_offset == FILE_END;
        ;
    }
}



int main() {
    string test_dir = "../../_Text/Compressed_data/LZW/COV19";
    EvaluationModule<Path, size_t, string, string> EM(test_dir);
    EM.set_element(WORD);

    /* Code segments */

    func_SEGMENT1<Path, size_t, string, string>(&EM);

    /* Write results to file */

    /* Finish */
    return 0;
}
