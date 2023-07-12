#include <string>
using namespace std;



string skeleton_HEADER = R"(
#include "../EvaluateModule.hpp"

#define EXTRACT(offset_start, length) EM->API_Extract(offset_start, length)
#define FILE_START EM->get_file_start()
#define FILE_END EM->get_file_end()

)";

string skeleton_GLOBAL_STRUCT_INIT = R"(
/* Global structure init */
/* AST code here */
)";

string skeleton_SEGMENT = R"(
template <typename Path, typename Offset, typename Symbol, typename Rst>
void func_SEGMENT{NUM}(EvaluationModule<Path, Offset, Symbol, Rst>* EM) {
    /* Local structure init */
    /* AST code here */

    /* End condition */
    bool end_condition = /* AST code here */

    /* Actions */
    while (!end_condition) {
        /* AST code here */

        /* Update end condition here */
        end_condition = /* AST code here */;
    }
}

)";

string skeleton_MAIN_BEFORE_ACTION = R"(

int main() {
    string test_dir = /* AST code here */;
    EvaluationModule<Path, size_t, string, string> EM(test_dir);
    EM.set_element({ELEMENT});

    /* Code segments */
)";

string skeleton_MAIN_ACTIONS = R"(
    func_SEGMENT{NUM}<Path, size_t, string, string>(&EM);
)";

string skeleton_MAIN_END = R"(
    /* Write results to file */

    /* Finish */
    return 0;
}
)";