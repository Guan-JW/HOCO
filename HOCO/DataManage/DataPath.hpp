# pragma once
# include "../Utils/utils.hpp"
#include <string.h>
#include <fstream>

class Path {
    public:
        Path() {}
        Path(string _tmpPath) : tempPath(_tmpPath) {}

        static string getDirectoryPath(const string &file_path) {
            size_t lastSlash = file_path.find_last_of("/"); // remove trailing slashes
            size_t lastBackslash = file_path.find_last_of("\\");

            size_t lastSeparator = file_path.length();
            if (lastSlash != string::npos && lastBackslash != string::npos) {
                lastSeparator = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
            }
            else if (lastSlash != string::npos) {
                lastSeparator = lastSlash;
            }
            else if (lastBackslash != string::npos) {
                lastSeparator = lastBackslash;
            }
            
            if (lastSeparator != file_path.length()) {
                string dir_path = file_path.substr(0, lastSeparator);
                return dir_path;
            }
            else {
                cerr << "File_path (" + file_path + ") doesn't have an upper directory." << endl;
                return file_path;
            }
        }

        // be invoked after checking the existence of dir
        static bool path_join(const string &dir, const string &file, string* file_path) {
            if (path_exist(dir) != 0) {
                throw runtime_error("path_join: Directory(" + dir + ") doesn't exist.");
            }
            size_t lastPos = dir.find_last_not_of(" "); // remove trailing spaces
            *file_path = dir;
            if (lastPos != string::npos) {
                *file_path = dir.substr(0, lastPos + 1);
            }

            lastPos = (*file_path).find_last_not_of("/"); // remove trailing slashes
            if (lastPos != string::npos) {
                *file_path = (*file_path).substr(0, lastPos + 1);
            }

            *file_path += "/" + get_file_name<string>(file);
            if (path_exist(*file_path) != 1) {
                cerr << "Warning: File " + *file_path + " doesn't exist." << endl;
                return false;
            }
            return true;
        }

        // just get a joined file name.
        static bool join(const string &dir, const string &file, string* file_path) {
            if (path_exist(dir) != 0) {
                throw runtime_error("path_join: Directory(" + dir + ") doesn't exist.");
            }
            size_t lastPos = dir.find_last_not_of(" "); // remove trailing spaces
            *file_path = dir;
            if (lastPos != string::npos) {
                *file_path = dir.substr(0, lastPos + 1);
            }

            lastPos = (*file_path).find_last_not_of("/"); // remove trailing slashes
            if (lastPos != string::npos) {
                *file_path = (*file_path).substr(0, lastPos + 1);
            }

            *file_path += "/" + get_file_name<string>(file);
            if (path_exist(*file_path) != 1) {
                cerr << "Warning: File " + *file_path + " doesn't exist." << endl;
            }
            return true;
        }

        // be invoked after checking the existence of dir
        inline bool dir_path_join(const string &dir, const string &subdir, string* dir_path) {
            if (path_exist(dir) != 0) {
                throw runtime_error("path_join: Directory(" + dir + ") doesn't exist.");
            }
            size_t lastPos = dir.find_last_not_of(" "); // remove trailing spaces
            *dir_path = dir;
            if (lastPos != string::npos) {
                *dir_path = dir.substr(0, lastPos + 1);
            }

            lastPos = (*dir_path).find_last_not_of("/"); // remove trailing slashes
            if (lastPos != string::npos) {
                *dir_path = (*dir_path).substr(0, lastPos + 1);
            }

            *dir_path += "/" + subdir;
            if (path_exist(*dir_path) != 0) {
                cerr << "Warning: Directory " + *dir_path + " doesn't exist." << endl;
                return false;
            }
            return true;
        }

        static int path_exist (const string& name) {
            struct stat buffer;   
            if (stat (name.c_str(), &buffer) == 0){
                if (S_ISDIR(buffer.st_mode))   // directory
                    return 0;
                if (S_ISREG(buffer.st_mode))   // regular file
                    return 1;
            }   
            return -1;  // path not exist, or path is not a directory or a regular file
        }

        static bool isDirExist(const std::string& path) {
            struct stat info;
            if (stat(path.c_str(), &info) != 0)
            {
                return false;
            }
            return (info.st_mode & S_IFDIR) != 0;
        }

        static bool makePath(const std::string& path, mode_t mode = 0755) {
            int ret = mkdir(path.c_str(), mode);
            if (ret == 0)
                return true;

            switch (errno)
            {
            case ENOENT:
                // parent didn't exist, try to create it
                {
                    int pos = path.find_last_of('/');
                    if (pos == std::string::npos)
                        return false;
                    if (!makePath( path.substr(0, pos) ))
                        return false;
                }
                // now, try to create again
                return 0 == mkdir(path.c_str(), mode);

            case EEXIST:
                // done!
                return Path::isDirExist(path);

            default:
                return false;
            }
        }

        inline bool create_file(const string& path, std::ios_base::openmode mode) {
            ofstream file(path, mode);
            if (!file) {
                throw runtime_error("Failed to create file: " + path);
                return false; 
            }
            file.close();
            return true;
        }

        inline bool copy_split_file(string _src) {
            if (fileSplitPath == "") {
                if (compDir == "") {
                    cerr << "DataPath: No compress directory available." << endl;
                    return false;
                }
                string _split = "fileYyNO.txt";
                if ( !path_join(compDir, _split, &fileSplitPath) ) {    // set split file
                    cerr << "CompressPath: File splitting information doesn't exist." << endl;
                    create_file(fileSplitPath, std::ios_base::out);
                }
            }
            string cmd = std::string("cp '") + _src + "' '" + fileSplitPath + "'";
            if (system(cmd.c_str())) {
                cerr << "DataPath-copy_split_file: Error in copy file. src = " << _src << " ; dst = " << fileSplitPath << endl;
                fileSplitPath = _src;
                return false;
            }
            return true;
        }

        inline bool copy_split_file(string _src, string _dst) {
            string cmd = std::string("cp '") + _src + "' '" + _dst + "'";
            if (system(cmd.c_str())) {
                cerr << "DataPath: Error in copy file. src = " << _src << " ; dst = " << _dst  << endl;
                fileSplitPath = _src;
                return false;
            }
            fileSplitPath = _dst;   // set split file
            return true;
        }

        inline string get_comp_dir() { return compDir; }
        inline string get_uncomp_file() { return filePath; }
        inline string get_split_file() { return fileSplitPath; }
        inline string get_comp_file() { return compressFilePath; }
        inline string get_dic_file() { return dictionaryPath; }
        inline string get_block_offset_file() { return blockoffsetPath; }
        inline string get_offset_file() { return offsetPath; }

        inline void set_split_file(string _split) { fileSplitPath = _split; }
    
        string tempPath = "";
    
    protected:
        string compDir;
        string compressFilePath;
        string dictionaryPath;
        string blockoffsetPath;
        string offsetPath;

        string filePath;
        string fileSplitPath;

};



class CompressPath : public Path {
    public:
        CompressPath() {}

        // Constructor 1: parse from an absolute path of data directory
        CompressPath(const string _absolute_dir_path)  {
            compDir = _absolute_dir_path;
            if (path_exist(compDir) != 0) {
                if (!makePath(compDir)) {
                    throw runtime_error("CompressPath: Couldn't create directory " + compDir + ".");
                }
            }

            string _comp = "file.bin";
            if ( !path_join(_absolute_dir_path, _comp, &compressFilePath) ) {
                cerr << "CompressPath: Compressed text doesn't exist. Create it." << endl;
                create_file(compressFilePath, std::ios_base::binary);
            }

            string _dir = "dictionary.bin";
            if ( !path_join(_absolute_dir_path, _dir, &dictionaryPath) ) {
                cerr << "CompressPath: Dictionary doesn't exist." << endl;
                create_file(dictionaryPath, std::ios_base::binary);
            }

            string _split = "fileYyNO.txt";
            if ( !path_join(_absolute_dir_path, _split, &fileSplitPath) ) {
                cerr << "CompressPath: File splitting information doesn't exist." << endl;
                create_file(fileSplitPath, std::ios_base::out);
            }

            string _blockoff = "block_offset.txt";
            if ( !path_join(_absolute_dir_path, _blockoff, &blockoffsetPath) ) {
                optimize = false;
                cerr << "CompressPath: Block offset file doesn't exist." << endl;
                create_file(blockoffsetPath, std::ios_base::out);
            }

            string _off = "offset.txt";
            if (!path_join(_absolute_dir_path, _off, &offsetPath) ) {
                cerr << "CompressPath: Offset file doesn't exist." << endl;
                create_file(offsetPath, std::ios_base::out);
            }
            
            optimize = true;
        }
        
        // Constructor 2: without optimization 
        CompressPath(const string &_comp, const string &_dic, const string &_split)
                    {
                        compressFilePath = _comp;
                        dictionaryPath = _dic;
                        fileSplitPath = _split;
                        compDir = getDirectoryPath(_comp);
                        optimize = false; 
                    }
        
        // Constructor 3: with optimization
        CompressPath(const string &_comp, const string &_dic, const string &_split,
                    const string &_blockoff, const string &_off) 
                    { 
                        compressFilePath = _comp;
                        dictionaryPath = _dic;
                        fileSplitPath = _split;
                        blockoffsetPath = _blockoff;
                        offsetPath = _off;
                        compDir = getDirectoryPath(_comp);
                        optimize = true; 
                    }
        
        void exhibitPath() {
            cout << "Compress directory: " << compDir << endl;
            cout << "\tCompressed file path: " <<  compressFilePath << endl;
            cout << "\tDictionary file path: " << dictionaryPath << endl;
            cout << "\tSplit file path: " << fileSplitPath << endl;
            cout << "\tBlock offset path: " << blockoffsetPath << endl;
            cout << "\tOffset path: " << offsetPath << endl;
        }

        bool optimize = false;
};


class DecompressPath: public Path {
    public:
        DecompressPath() {}
        DecompressPath(const string &_file) {
            if (path_exist(_file) == 0) {
                throw runtime_error("DecompressPath: given path is a directory.");
            }
            
            string dir = getDirectoryPath(_file);
            if (path_exist(dir) != 1) {
                mode_t mode = 0777;
                if (!makePath(dir, mode)) {
                    throw runtime_error("DecompressPath: failed to create dir" + dir + ".");
                }
            }
            filePath = _file;
            path_join(dir, "fileYyNO.txt", &fileSplitPath);
        }
};



class UncompressPath : public Path {
    public:
        UncompressPath() {}
        UncompressPath(const string &_file, const string &_split) 
                        {
                            filePath = _file;
                            fileSplitPath = _split;
                            if (path_exist(filePath) != 1 || path_exist(fileSplitPath) != 1) {
                                throw runtime_error("UncompressPath: Given filepath (" + filePath + ") doesn't exist.");
                            }
                        }
        
};

// Just create file (or directory), waiting for backup copy
// Specify a directory for backuping uncompressed file, \
   and specify the split file. \
   This means that the data file and the split file can be in different directories.
class BackupUncompPath: public Path {
    public:
        BackupUncompPath() {}
        BackupUncompPath(string _dir, string _split) {
            if (path_exist(_dir) != 0) {
                cerr << "BackupUncompPath: Backup uncompressed directory doesn't exist. Create it." << endl;
                mode_t mode = 0777;
                makePath(_dir, mode);
            }
            path_join(_dir, "/backup.txt", &filePath);

            if (path_exist(_split) != 1)
                throw runtime_error("BackupUncompPath: Given split file path (" + _split + ") doesn't exist.");
            fileSplitPath = _split;
        }
};