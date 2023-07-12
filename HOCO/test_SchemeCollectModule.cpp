#include "SchemeCollectModule/CollectModule.hpp"

bool test_LZW() {
    BaseHC<Path, size_t, string, string>*
        LZW(FACTORY(BaseHC, Path, size_t, string, string).create("LZW_HC"));
    SchemeCollectModule<Path, size_t, string, string> SCM(LZW);

    cout << "Testing (De)compression correctness of the LZW HC scheme..." << endl;
    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        cout << "Testing directness and strong homomorphism..." << endl;
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

    cout << "Testing (De)compression correctness of the RLE HC scheme..." << endl;
    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        cout << "Testing directness and strong homomorphism..." << endl;
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

    cout << "Testing (De)compression correctness of the TADOC HC scheme..." << endl;
    if ( SCM.DeComp_correctness() ) {   // (de)compression correct
        cout << "Testing directness and strong homomorphism..." << endl;
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
    cout << "Registerd HC schemes:" << endl;
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

    return 0;
}