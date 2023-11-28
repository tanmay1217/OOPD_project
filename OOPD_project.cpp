#include <iostream>
#include <string>
#include <windows.h>

using namespace std;

class Shell {
public:
    void displayPrompt() {
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        cout << currentDir << " $ ";
    }

    void listFilesOfTheDirectory() {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA("*", &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "Error opening directory." << endl;
        }
        else {
            do {
                cout << findFileData.cFileName << endl;
            } while (FindNextFileA(hFind, &findFileData) != 0);

            FindClose(hFind);
        }
    }

    void changeDirectory(const string& dirName) {
        if (SetCurrentDirectoryA(dirName.c_str()) == 0) {
            cerr << "Error changing directory." << endl;
        }
    }
};

class FileOperations:public Shell {
public:
    bool DirectoryExists(const std::string& path) {
        DWORD attributes = GetFileAttributesA(path.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
    }

    static bool removeFile(const string& fileName) {
        if (DeleteFileA(fileName.c_str()) == 0) {
            cerr << "Error removing file: " << fileName << endl;
            return false;
        }
        return true;
    }

    static bool removeDirectory(const string& dirName) {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((dirName + "/*").c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            // No files found, directly remove the directory
            if (RemoveDirectoryA(dirName.c_str()) == 0) {
                cerr << "Error removing directory: " << dirName << endl;
                return false;
            }
            return true;
        }

        do {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
                string filePath = dirName + "/" + findFileData.cFileName;
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    // Recursively remove subdirectories
                    if (!removeDirectory(filePath)) {
                        FindClose(hFind);
                        return false;
                    }
                }
                else {
                    // Remove file
                    if (DeleteFileA(filePath.c_str()) == 0) {
                        cerr << "Error removing file: " << filePath << endl;
                        FindClose(hFind);
                        return false;
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);

        // Remove the empty directory
        if (RemoveDirectoryA(dirName.c_str()) == 0) {
            cerr << "Error removing directory: " << dirName << endl;
            return false;
        }

        return true;
    }

    static bool copyFile(const string& source, const string& destination) {
        if (CopyFileA(source.c_str(), destination.c_str(), FALSE) == 0) {
            cerr << "Error copying file: " << source << endl;
            return false;
        }
        return true;
    }

    static bool copyDirectory(const string& source, const string& destination) {
        if (!CreateDirectoryA(destination.c_str(), nullptr)) {
            cerr << "Error creating destination directory: " << destination << endl;
            return false;
        }

        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((source + "/*").c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "Error opening source directory: " << source << endl;
            return false;
        }

        do {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
                string sourceFilePath = source + "/" + findFileData.cFileName;
                string destinationFilePath = destination + "/" + findFileData.cFileName;

                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    // Recursively copy subdirectories
                    if (!copyDirectory(sourceFilePath, destinationFilePath)) {
                        FindClose(hFind);
                        return false;
                    }
                }
                else {
                    // Copy file
                    if (!copyFile(sourceFilePath, destinationFilePath)) {
                        FindClose(hFind);
                        return false;
                    }
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
        return true;
    }
};
class TakeInputRm {
public:
    string takeInput(string userInput) {
        string target = userInput.substr(3);
        if (target.find("\"") == 0 && target.rfind("\"") == target.length() - 1) {
            // Remove quotes from the target (if present)
            target = target.substr(1, target.length() - 2);
        }
        return target;
    }
};
class MoveOperation:public  FileOperations {
public:
    bool firstCopyThenDeleteSrc(string absoluteSource,string absoluteDestination) {
        FileOperations obj;
        if (obj.DirectoryExists(absoluteSource)) {
            // Copy directory and its contents
            obj.copyDirectory(absoluteSource, absoluteDestination);
        }
        else {
            // Copy file
            obj.copyFile(absoluteSource, absoluteDestination);
        }
        TakeInputRm takeipObject;
        
        string target = absoluteSource;
        
        if (obj.DirectoryExists(target)) {
            // Remove directory and its contents
            obj.removeDirectory(target);
        }
        else {
            // Remove file
            obj.removeFile(target);
        }
        return true;
    }
};

int main() {
    Shell shell;
    FileOperations fileOperations;
    MoveOperation MoveOperationObject;
    TakeInputRm TakeInputRmObject;
    while (true) {
        shell.displayPrompt();
        string userInput;
        getline(cin, userInput);

        if (userInput == "ls") {
            shell.listFilesOfTheDirectory();
        }
        else if (userInput.substr(0, 3) == "cd ") {
            string directoryName = userInput.substr(3);
            shell.changeDirectory(directoryName);
        }
        else if (userInput.substr(0, 3) == "rm ") {
            string target = TakeInputRmObject.takeInput(userInput);
            if (fileOperations.DirectoryExists(target)) {
                // Remove directory and its contents
                fileOperations.removeDirectory(target);
            }
            else {
                // Remove file
                fileOperations.removeFile(target);
            }
        }
        else if (userInput.substr(0, 3) == "cp ") {
            // Syntax for cp command: cp source destination
            //C:/Users/asus/Desktop/OOPD_assignment/OOPD_5/12.txt
            size_t spaceIndex = userInput.find(" ");
            if (spaceIndex != string::npos) {
                string file_name_sep_by_space = userInput.substr(3, spaceIndex - 3);
                size_t spacePos = file_name_sep_by_space.find(' ');
                string source = file_name_sep_by_space.substr(0, spacePos);
                string destination = file_name_sep_by_space.substr(spacePos + 1);

                // Construct absolute paths for source and destination
                char currentDir[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, currentDir);
                string absoluteSource = currentDir;
                string absoluteDestination;

                // If the destination is an absolute path, use it as is
                if (destination.size() >= 2 && destination[1] == ':') {
                    absoluteDestination = destination;
                }
                else {
                    // Otherwise, construct the absolute destination path
                    absoluteDestination = currentDir;
                    if (!destination.empty() && destination[0] != '\\') {
                        absoluteDestination += "\\";
                    }
                    absoluteDestination += destination;
                }

                if (!source.empty() && source[0] != '\\') {
                    absoluteSource += "\\";
                }
                absoluteSource += source;

                if (fileOperations.DirectoryExists(absoluteSource)) {
                    // Copy directory and its contents
                    fileOperations.copyDirectory(absoluteSource, absoluteDestination);
                }
                else {
                    // Copy file
                    fileOperations.copyFile(absoluteSource, absoluteDestination);
                }
            }
            else {
                cout << "Invalid syntax for cp command. Use 'cp source destination'.\n";
            }
        }
        else if (userInput.substr(0, 3) == "mv ") {
            //MoveOperationObject
            //first cp operation is done then del option of src is done
            size_t spaceIndex = userInput.find(" ");
            if (spaceIndex != string::npos) {
                string file_name_sep_by_space = userInput.substr(3, spaceIndex - 3);
                size_t spacePos = file_name_sep_by_space.find(' ');
                string source = file_name_sep_by_space.substr(0, spacePos);
                string destination = file_name_sep_by_space.substr(spacePos + 1);

                // Construct absolute paths for source and destination
                char currentDir[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, currentDir);
                string absoluteSource = currentDir;
                string absoluteDestination;

                // If the destination is an absolute path, use it as is
                if (destination.size() >= 2 && destination[1] == ':') {
                    absoluteDestination = destination;
                }
                else {
                    // Otherwise, construct the absolute destination path
                    absoluteDestination = currentDir;
                    if (!destination.empty() && destination[0] != '\\') {
                        absoluteDestination += "\\";
                    }
                    absoluteDestination += destination;
                }

                if (!source.empty() && source[0] != '\\') {
                    absoluteSource += "\\";
                }
                absoluteSource += source;
                MoveOperationObject.firstCopyThenDeleteSrc(absoluteSource, absoluteDestination);
            }
            else {
                cout << "Invalid syntax for mv command. Use 'mv source destination'.\n";
            }
        }
        else if (userInput == "clear") {
            system("cls");
        }
        else if (userInput == "exit") {
            cout << "Exiting program. Goodbye!\n";
            break;
        }
    }

    return 0;
}
