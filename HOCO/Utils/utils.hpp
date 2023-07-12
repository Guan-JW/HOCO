
# pragma once
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <chrono>
#include <mutex>
#include "flat_hash_map.hpp"
#include <cxxabi.h>
using namespace std;

typedef unsigned long ulong;
#define quote(x) #x
#define TEMP_STORE_PATH "tempStore/"
#define META_FILE_PATH "meta.data"
#define DICMAXSIZE 1000000000
#define lengthof(array) (sizeof(array) / sizeof(array[0]))

#define ROOT_PATH string(PROJECT_ROOT_PATH)
#define PARENT_PATH string(PARENT_DIR)

const char Dic_Init_Chars[] = { ' ', '\t', '\n', '\r', ',', '.' };
const size_t Start_offs[] = {0, 100, 1000, 10000};
const size_t Lengths[] = {32, 64, 128, 256};
const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

enum HC_Category : short {
    NOT_DEFINED,
    UNHC,
    LHC,
    PHC,
    FHC
};

template<typename T>
T get_file_name(T const & path, T const & delims = "/\\") {
    T subpath = path.substr(path.find_last_of(delims) + 1);
    typename T::size_type const p(subpath.find_last_of('.'));
    if (p <= 0 || p == T::npos)
        return "";
    return subpath;
}


class MyException : public exception
{
    public:
        MyException(const string& msg) : m_msg(msg) {
            cout << "MyException::MyException - set m_msg to:" << m_msg << endl;
        }
        ~MyException() {
                cout << "MyException::~MyException" << endl;
        }

        virtual const char* what() const throw () {
                cout << "MyException::what" << endl;
                return m_msg.c_str();
        }

        const string m_msg;
};


long getFileSize(const string filePath) {
    const char* cFilePath = filePath.c_str();
    FILE* file = fopen(cFilePath, "rb");
    if (file == NULL) {
        // Handle error: unable to open file
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);

    fclose(file);

    return fileSize;
}


template <typename Offset, typename Rst>
Rst readSubstringFromTxTFile(const string filePath, Offset offset, Offset length) {
    // Assume the readSubstringFromFile function reads the substring into the buffer
    const char* cFilePath = filePath.c_str();
    FILE* file = fopen(cFilePath, "r");
    if (file == NULL) {
        // Handle error: unable to open file
        throw runtime_error("readSubstringFromTxTFile: Failed to open file: " + filePath);
        // return -1;
    }

    char* buffer = (char*)malloc(length);

    fseek(file, offset, SEEK_SET);
    Offset elementsRead = fread(buffer, sizeof(char), length, file);
    if (elementsRead != length) {
        cerr << "readSubstringFromTxTFile: Failed to read the expected number of elements, got length " 
                << elementsRead << "." << endl;
        length = elementsRead;
    }

    fclose(file);

    string substring(buffer, length);

    free(buffer);
    
    if (is_same<Rst, string>::value) {
        return substring;
    } 
    else if (is_same<Rst, char*>::value) {
        // This may not happen
        cout << "Char*: " << substring.c_str() << endl;
        return substring.c_str();
    } 
    else {
        // This may not happen
        cerr << "readSubstringFromTxTFile: Rst's type is not string. Return string." << endl;
        return substring;
    }
}

template <typename Offset, typename Symbol>
bool insertSubstringIntoTxTFile(const string filename, Offset offset, Symbol newstr) {
    fstream file(filename, ios::in | ios::out | ios::binary);
    if (!file) {
        cerr << "insertSubstringIntoTxTFile: Failed to open file " << filename << "." << endl;
        return false;
    }
    
    // Move the file pointer to the desired offset
    file.seekp(offset, ios::beg);

    // Read the content after the offset
    string tailData(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));

    // Clear the content after the offset
    file.seekp(offset, ios::beg);
    // file.truncate();

    // Write the new data at the offset
    file << newstr;

    // Write back the tail data after the inserted data
    file << tailData;

    file.close();
    return true;
}

template <typename Offset>
bool deleteSubstringFromTxTFile(const string filename, Offset offset, Offset length) {
    fstream file(filename, ios::in | ios::out | ios::binary);
    if (!file) {
        cerr << "deleteSubstringFromTxTFile: Failed to open file " << filename << "." << endl;
        return false;
    }

    // calculate the remaining data size after the substring to be deleted
    streampos resultSize = streamsize(file.seekg(0, ios::end).tellg()) - length;
    streampos remainingDataSize = streamsize(file.seekg(0, ios::end).tellg()) - offset - length;

    // Seek to the offset + length
    file.seekp(offset + length, ios::beg);

    // create a buffer to hold the remaining data
    string tailData(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
    
    // Close the file
    file.close();

    // Reopen the file in write mode to truncate it
    ofstream outfile(filename, ios::binary | ios::out | ios::trunc);
    if (!outfile) {
        cerr << "deleteSubstringFromTxTFile: Failed to open file " << filename << " for writing." << endl;
        return false;
    }

    // move the file pointer to the offset position
    outfile.seekp(offset, ios::beg);

    // write the remaining data over the deleted position
    outfile << tailData;

    // cloase the output file
    outfile.close();

    // truncate the file to remove any remaining data after the intended deletion
    if (truncate(filename.c_str(), resultSize) == -1) {
        cerr << "deleteSubstringFromTxTFile: Failed to truncate file " << filename << "." << endl;
        return false;
    }

    return true;
}

struct VectorHasher { 
    int operator()(const vector<int> &V) const { // hash(vector) = vector[0]
        unsigned int hash = 0;
        return V[0];
    }
};

string generateRandomString(size_t length) {
    int chMax = characters.length();
    srand (time(NULL));

    string randomString = "";
    for (int i = 0; i < length; ++i) {
        randomString += characters[rand() % length];
    }

    return randomString;
}
