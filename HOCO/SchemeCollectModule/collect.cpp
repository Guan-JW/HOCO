#include "CollectModule.hpp"

bool test_LZW() {
    BaseHC<Path, size_t, string, string>*
        LZW(FACTORY(BaseHC, Path, size_t, string, string).create("LZW_HC"));
    SchemeCollectModule<Path, size_t, string, string> SCM(LZW);

    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        if (SCM.Evaluation_Check() ) { // evaluation correct
            return true;
        }
    }

    /* Unregister */
    return false;
}

bool test_RLE() {
    BaseHC<Path, size_t, string, string>*
        RLE(FACTORY(BaseHC, Path, size_t, string, string).create("RLE_HC"));
    SchemeCollectModule<Path, size_t, string, string> SCM(RLE);

    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        if (SCM.Evaluation_Check()) // evaluation correct
            return true;
    }

    /* Unregister */
    return false;
}

bool test_TADOC() {
    BaseHC<Path, size_t, string, string>*
        TADOC(FACTORY(BaseHC, Path, size_t, string, string).create("TADOC_HC"));
    SchemeCollectModule<Path, size_t, string, string> SCM(TADOC);

    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        if (SCM.Evaluation_Check()) // evaluation correct
            return true;
    }

    /* Unregister */
    return false;
}

int main() {

    /* Test register */
    REGISTER(BaseHC, RLE_HC, Path, size_t, string, string);
    REGISTER(BaseHC, LZW_HC, Path, size_t, string, string);
    REGISTER(BaseHC, TADOC_HC, Path, size_t, string, string);
    FACTORY(BaseHC, Path, size_t, string, string).printRegisteredClasses();
    cout << "---" << std::endl; 

    /* LZW */
    if (!test_LZW()) {
        cerr << "LZW_HC failed. Unregister it." << endl;
        UNREGISTER(BaseHC, LZW_HC, Path, size_t, string, string);
    }

    /* RLE */
    if (!test_RLE()) {
        cerr << "RLE_HC failed. Unregister it." << endl;
        UNREGISTER(BaseHC, RLE_HC, Path, size_t, string, string);
    }

    cout << "---" << std::endl; 
    FACTORY(BaseHC, Path, size_t, string, string).printRegisteredClasses();

    return 0;
}