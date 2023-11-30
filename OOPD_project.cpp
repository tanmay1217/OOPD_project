#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
using namespace std;

class Shell {
public:
    void replaceBackslashes(char* filePath) {
        for (int i = 0; i < std::strlen(filePath); ++i) {
            if (filePath[i] == '\\') {
                filePath[i] = '/'; // Replace backslash with forward slash
            }
        }
    }
    void displayPrompt(string cmd) {
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        if (cmd == "pwd") {
            replaceBackslashes(currentDir);
            cout << currentDir << std::endl;

        }
        else {
            replaceBackslashes(currentDir);
            cout << currentDir << " $ ";
        } 
    }
    bool createFile(const std::string& filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file.close();
            return true;
        }
        else {
            std::cerr << "Error creating file '" << filename << "'." << std::endl;
            return false;
        }
    }
    void printFileContents(const std::string& filename) {
        std::ifstream file(filename);

        if (file.is_open()) {
            std::string line;

            while (std::getline(file, line)) {
                std::cout << line << std::endl;
            }

            file.close();
        }
        else {
            std::cerr << "Error opening file '" << filename << "'." << std::endl;
        }
    }
    void createDirectory(const char* folderPath) {
    // Convert narrow character string to wide character string
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, folderPath, -1, nullptr, 0);
    wchar_t* wideFolderPath = new wchar_t[wideCharLength];
    MultiByteToWideChar(CP_UTF8, 0, folderPath, -1, wideFolderPath, wideCharLength);

    // Create the new folder
    if (CreateDirectory(wideFolderPath, NULL) != 0) {
        std::wcout << L"Folder created successfully." << std::endl;
    } else {
        std::wcerr << L"Error creating folder. Error code: " << GetLastError() << std::endl;
    }

    delete[] wideFolderPath;
}
    void listFilesOfTheDirectory(bool humanReadable = false) {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA("*", &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "Error opening directory." << endl;
        }
        else {
            do {
                cout << findFileData.cFileName;

                if (humanReadable) {
                    LARGE_INTEGER fileSize;
                    fileSize.LowPart = findFileData.nFileSizeLow;
                    fileSize.HighPart = findFileData.nFileSizeHigh;

                    double sizeInKB = static_cast<double>(fileSize.QuadPart) / 1024;
                    double sizeInMB = sizeInKB / 1024;

                    if (sizeInMB >= 1.0) {
                        cout << "\t\t - " << fixed << setprecision(2) << sizeInMB << " MB";
                    }
                    else {
                        cout << "\t\t - " << fixed << setprecision(2) << sizeInKB << " KB";
                    }
                }

                cout << endl;

            } while (FindNextFileA(hFind, &findFileData) != 0);

            FindClose(hFind);
        }
    }
    void listFilesRecursively(const string& directory) {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((directory + "\\*").c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "Error opening directory." << endl;
            return;
        }

        do {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
                cout << directory + "\\" + findFileData.cFileName << endl;

                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    listFilesRecursively(directory + "\\" + findFileData.cFileName);
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    void listFilesOfTheDirectory_rec() {
        listFilesRecursively(".");
    }

    void changeDirectory(const string& dirName) {
        if (SetCurrentDirectoryA(dirName.c_str()) == 0) {
            cerr << "Error changing directory." << endl;
        }
    }
};

class FileOperations:public Shell {
public:
    void updateProgressBar(int progress, int total) {
        const int barWidth = 50;

        float ratio = static_cast<float>(progress) / total;
        int barLength = static_cast<int>(ratio * barWidth);

        cout << "\r[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < barLength) cout << "=";
            else cout << " ";
        }

        cout << "] " << setw(3) << static_cast<int>(ratio * 100.0) << "%";
        cout.flush();
    }
    void callprogress() {
        const int totalSteps = 100;

        

        for (int i = 0; i <= totalSteps; ++i) {
            updateProgressBar(i, totalSteps);
            // Simulate work by sleeping for a short duration
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
    
    bool DirectoryExists(const std::string& path) {
        DWORD attributes = GetFileAttributesA(path.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
    }

    static bool removeFile(const string& fileName) {
        std::istringstream iss(fileName);
        std::vector<std::string> values;
        std::string value;

        while (iss >> value) {
            values.push_back(value);
        }
        for (const auto& val : values) {
            if (DeleteFileA(val.c_str()) == 0) {
                cerr << "Error removing file: " << val << endl;
                return false;
            }
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
    bool fileExists(const std::string& filePath) {
        DWORD fileAttributes = GetFileAttributesA(filePath.c_str());

        return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
            !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
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
        string target="";
        if (userInput.substr(0, 6) == "rmdir ") {
            string filename = userInput.substr(6);
            target = filename;
        }
        else { 
            target = userInput.substr(3);
        }
        
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
class Calendar {
public :
    
    void printCalendar(int year, int month) {
        const char* monthNames[] = { "", "January", "February", "March", "April", "May", "June", "July",
                                    "August", "September", "October", "November", "December" };

        std::cout << std::setw(5) << monthNames[month] << " " << year << "\n\n";
        std::cout << " Sun Mon Tue Wed Thu Fri Sat\n";

        // Use localtime_s instead of localtime
        time_t now = time(0);
        struct tm timeStruct;
        if (localtime_s(&timeStruct, &now) != 0) {
            std::cerr << "Error getting local time." << std::endl;
            return;
        }

        // Get the first day of the month
        timeStruct.tm_mday = 1;
        std::mktime(&timeStruct);

        // Get the day of the week for the first day
        int startingDay = timeStruct.tm_wday;

        // Get the number of days in the month
        int daysInMonth = 31;  // Assume maximum days for simplicity
        if (month == 4 || month == 6 || month == 9 || month == 11) {
            daysInMonth = 30;
        }
        else if (month == 2) {
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                daysInMonth = 29;  // Leap year
            }
            else {
                daysInMonth = 28;
            }
        }

        // Print the calendar
        int day = 1;

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                if (i == 0 && j < startingDay) {
                    std::cout << std::setw(5) << " ";
                }
                else if (day <= daysInMonth) {
                    std::cout << std::setw(5) << day;
                    ++day;
                }
                if (day > daysInMonth) {
                    break;
                }
            }
            std::cout << std::endl;
        }
    }
    int inputForCalendar() {
        time_t now = time(0);
        struct tm timeStruct;

        // Use localtime_s instead of localtime
        if (localtime_s(&timeStruct, &now) != 0) {
            std::cerr << "Error getting local time." << std::endl;
            return 1;
        }

        int year = timeStruct.tm_year + 1900;
        int month = timeStruct.tm_mon + 1;  // Months are zero-based

        printCalendar(year, month);
        return 1;
    }
};
class PrintHelp {
public:
    void ls(){
        cout << "ls[option][file / directory]"<< endl;
        cout << "-r	known as reverse order which is used to reverse the default order of listing.."<<endl;
        cout << "-h	Print file sizes in human-readable format (e.g., 1K, 234M, 2G)." << endl;
    }
    void cd() {
        cout << "cd [directory]" << endl;
        cout << "cd Documents" << endl;
    }
};
int main() {
    Shell shell;
    FileOperations fileOperations;
    MoveOperation MoveOperationObject;
    TakeInputRm TakeInputRmObject;
    Calendar calObject;
    PrintHelp PrintHelpObject;
    while (true) {
        shell.displayPrompt("none");
        string userInput;
        getline(cin, userInput);

        if (userInput == "ls") {
            shell.listFilesOfTheDirectory();
        }

        else if (userInput == "ls -h") {
            shell.listFilesOfTheDirectory(true);
        }
        else if (userInput == "ls -r") {
            shell.listFilesOfTheDirectory_rec();
        }
        else if (userInput == "ls --help") {
            PrintHelpObject.ls();
        }

        else if (userInput.substr(0, 3) == "cd ") {
            string directoryName = userInput.substr(3);
            shell.changeDirectory(directoryName);
        }
        else if (userInput == "cd --help") {
            PrintHelpObject.cd();
        }

        else if (userInput.substr(0, 6) == "rmdir " ) {
            char lastChar = userInput.at(userInput.length() - 1);
            string target,oldtarget;
            if (lastChar != 'r') {
                 target = TakeInputRmObject.takeInput(userInput);
            }
            else {
                oldtarget = TakeInputRmObject.takeInput(userInput);
                istringstream iss(oldtarget);
                string firstWord;
                iss >> firstWord;
                target = firstWord;
            }
            /*char minus_sign = target.at(0);
            char rsign = target.at(1);*/

            if (fileOperations.DirectoryExists(target)) {
                // Remove directory and its contents
                fileOperations.removeDirectory(target);
            }
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
            //handle-rCase();
            //fileOperations.callprogress();
            char lastChar = userInput.at(userInput.length() - 1);
            
            
            size_t spaceIndex = userInput.find(" ");
            bool force=false;
            if (spaceIndex != string::npos) {
                string file_name_sep_by_space;
                string source;
                string olddestination;
                size_t spacePos;
                size_t anynewspace;
                string destination;
                if (lastChar == 'v' or lastChar =='f' or lastChar=='i') {
                    if (lastChar == 'v') {
                        fileOperations.callprogress();
                        cout << endl;
                    }
                    if (lastChar == 'f') {
                        force = false;
                    }
                    if (lastChar == 'i') {
                        force = false;
                    }
                    file_name_sep_by_space = userInput.substr(3, spaceIndex - 3);
                    spacePos = file_name_sep_by_space.find(' ');
                    source = file_name_sep_by_space.substr(0, spacePos);
                    olddestination = file_name_sep_by_space.substr(spacePos + 1);
                    anynewspace = olddestination.find(' ');
                    destination = olddestination.substr(0, anynewspace);
                }
                else {
                    file_name_sep_by_space = userInput.substr(3, spaceIndex - 3);
                    spacePos = file_name_sep_by_space.find(' ');
                    source = file_name_sep_by_space.substr(0, spacePos);
                    destination = file_name_sep_by_space.substr(spacePos + 1);
                }
                

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
                    if (force) {
                        fileOperations.copyFile(absoluteSource, absoluteDestination);
                    }
                    else {
                        if (fileOperations.fileExists(absoluteDestination)) {
                            cout << "File already existed." << endl;
                        }
                        else
                            fileOperations.copyFile(absoluteSource, absoluteDestination);
                    }
                    

                    
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
        else if (userInput == "pwd") {

            shell.displayPrompt(userInput);
        }
        else if (userInput == "cal") {
            calObject.inputForCalendar();
        }
        else if (userInput.substr(0, 6) == "touch ") {
            string filename = userInput.substr(6);
            shell.createFile(filename);
        }
        else if (userInput.substr(0, 4) == "cat ") {
            string filename = userInput.substr(4);
            shell.printFileContents(filename);
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
