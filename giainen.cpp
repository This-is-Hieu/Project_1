#include<bits/stdc++.h>
#include<thread>
#include<zip.h>
#include<zlib.h>
#include<windows.h>
using namespace std;
deque<string> List;
atomic<bool> Found(false);         //Khóa kiểm tra khi tìm thấy kết quả
atomic<unsigned long long> Index(0);         //Tính số pass đã thử
set<char> charList= {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> Alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
set<char> number = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
string newCharList;
int length = 0;                     //Số kí tự trong mật khẩu
unsigned long long total;                    //Tổng số mật khẩu cần thử
string ZipFileDirectory;
string filename;
const char *zip_filename;
const char *file_name;
int num_thread = thread::hardware_concurrency()/2;                     //Số luồng, mặc định lấy 1 nửa luồng tối đa
vector<thread> testers;             //Vector chứa luồng
int choice = 0;
string PasswordFilePath;
int sizeOfList = 0;
mutex mtx;
bool theEnd = false;
unsigned long correct_crc;
int file_index = 0;

void Convert(unsigned long long tmp, string& pass) {
    pass.clear();
    while (tmp > 0) {
        pass = newCharList[tmp % sizeOfList] + pass;
        tmp /= sizeOfList;
    }
}

// Hàm xử lý tín hiệu ngắt và ghi vào file
void signalHandler(int signum) {
    int tmp;
    if (signum == SIGINT) {  //Ctrl+C
        tmp = Index.exchange(total);
        ifstream inFile("D:/code/Project1/progress.txt");
        stringstream buffer;
        buffer << inFile.rdbuf();
        string fileContents = buffer.str();
        inFile.close();


        string oldContent = ZipFileDirectory + "\n" + filename + "\n";  //cũ
        string newContent = ZipFileDirectory + "\n" + filename + "\n" + to_string(tmp) + "\n";  //mới

        // Tìm và thay thế hoặc thêm
        size_t pos = fileContents.find(oldContent);
        if (pos != string::npos) {

            fileContents.replace(pos + oldContent.length(), string::npos, to_string(tmp) + "\n");
        } else {
            fileContents += newContent;
        }

        ofstream outFile("D:/code/Project1/progress.txt", ios::trunc);
        if (outFile.is_open()) {
            outFile << fileContents;
            outFile.close();
            string temp;
            if (choice == 1) {
                cout << endl;
                cerr << "Current password: " << temp << endl;
            }

            if (choice == 2) {
                cout << "Not available " << endl;
            }
        } else {
            cerr << "Cannot open history" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
        exit(0);
    }
}

unsigned long long tmpIndex;
bool checkProgressFile() {
    ifstream progressFile("D:/code/Project1/progress.txt", ios::in);
    if (!progressFile.is_open()) {
        cerr << "Cannot open history " << endl;
        return false;
    }
    string zipFile, fileInZip;
    unsigned long long storedIndex;
    while (getline(progressFile, zipFile)) {
        getline(progressFile, fileInZip);
        progressFile >> storedIndex;
        progressFile.ignore();

        if (zipFile == ZipFileDirectory && fileInZip == filename) {
            tmpIndex = storedIndex;
            progressFile.close();
            return true;
        }
    }
    progressFile.close();
    return false;
}

bool isPassFile(const string PasswordFilePath) {
    ifstream passfile(PasswordFilePath);
    if (!passfile.is_open()) {
        return false;
    }
    passfile.close();
    return true;
}

unsigned long calculate_crc(const char *data, size_t size) {
    return crc32(0L, reinterpret_cast<const unsigned char*>(data), size);
}

bool isZipFile() {
    int err = 0;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    if (!archive) {
        return false;
    }
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    if (num_entries < 0) {
        zip_close(archive);
        return false;
    }
    return true;
}

void passFromFile(const string PasswordFilePath){
    ifstream passfile(PasswordFilePath);
    string line;
    while((!Found) && (getline(passfile, line))){
        if(!line.empty()){
            if(List.size() < 20000){
                mtx.lock();
                List.push_back(line);
                mtx.unlock();
            }
            else {
                cout << "\rTried password " << Index << flush;
                this_thread::sleep_for(chrono::milliseconds(100));

            }
        }
    }
    while(!List.empty()){};
    theEnd = true;
    passfile.close();
}

void Test2(const char *zip_filename) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    int err = 0;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    string pass;
    const char* password = pass.c_str();
    while(!Found){
        mtx.lock();
        if(!List.empty()){
            pass = List.front();
            List.pop_front();
            mtx.unlock();
            Index++;
            zip_file_t *file = zip_fopen_index_encrypted(archive,file_index, 0, password);
            if(file){
                if (file) {
                    char buffer[4096];
                    string file_data = "";
                    int bytes_read;
                    while ((bytes_read = zip_fread(file, buffer, 4096)) > 0) {
                        file_data.append(buffer, buffer + bytes_read);
                    }
                    unsigned long crc = calculate_crc(file_data.c_str(), file_data.size());

                    if (crc == correct_crc) {
                        cout << "\nRight password: " << pass << "\n";
                        Found = true;
                        zip_fclose(file);
                        zip_close(archive);
                        return;  // Mật khẩu đúng
                    }
                    zip_fclose(file);
                }
            }
        }
        else{
            mtx.unlock();
            if((theEnd)) {
                return;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
    zip_close(archive);
    return;
}



void Test1(const char *zip_filename) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    zip_t *archive = zip_open(zip_filename, 0, 0);
    string pass;
    const char* password = pass.c_str();
    char buffer[4096];
    while (!Found && (Index < total)) {
        unsigned long long tmp = Index.fetch_add(1, std::memory_order_relaxed);
        Convert(tmp, pass);

        zip_file_t *file = zip_fopen_index_encrypted(archive, file_index, 0, password);
        if (file) {
            string file_data;
            int bytes_read;
            while ((bytes_read = zip_fread(file, buffer, 4096)) > 0) {
                file_data.append(buffer, bytes_read);
            }
            if (!file_data.empty()) {
                unsigned long crc = calculate_crc(file_data.c_str(), file_data.size());
                if (crc == correct_crc) {
                    cout << "\nRight password: " << pass << "\n";
                    Found = true;
                    zip_fclose(file);
                    zip_close(archive);
                    return;
                }
            }
            zip_fclose(file);
        }
    }

    zip_close(archive);
}



void TestAndProgress(const char *zip_filename) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    zip_t *archive = zip_open(zip_filename, 0, 0);
    string pass;
    const char* password = pass.c_str();
    auto start_time = chrono::steady_clock::now();  // Store the start time
    int seconds_elapsed = 0;  // Variable to track elapsed time

    while ((!Found) && (Index < total)) {
        unsigned long long tmp = Index.fetch_add(1, memory_order_relaxed);

        auto current_time = chrono::steady_clock::now();
        chrono::duration<float> elapsed_time = current_time - start_time;

        // Update progress bar every second
        if (elapsed_time.count() >= (seconds_elapsed + 1)) {
            int width = 100;
            cout << "\r[";  // Return to the start of the line
            int pos = Index * width / total;
            for (int j = 0; j < width; ++j) {
                if (j < pos) cout << "=";
                else if (j == pos) cout << ">";
                else cout << " ";
            }
            cout << "] " << Index * 100 / total << "%  " << (int)elapsed_time.count() << "s" << flush;

            // Update seconds_elapsed to reflect the current time
            seconds_elapsed = static_cast<int>(elapsed_time.count());
        }

        Convert(tmp, pass);
        zip_file_t *file = zip_fopen_index_encrypted(archive, file_index, ZIP_RDONLY, password);
        if (file) {
            char buffer[4096];
            string file_data = "";
            int bytes_read;
            while ((bytes_read = zip_fread(file, buffer, 4096)) > 0) {
                file_data.append(buffer, buffer + bytes_read);
            }
            if (!file_data.empty()) {
                unsigned long crc = calculate_crc(file_data.c_str(), file_data.size());
                if (crc == correct_crc) {
                    cout << "\nRight password: " << pass << "\n";
                    Found = true;
                    zip_fclose(file);
                    zip_close(archive);
                    return;
                }
            }
            zip_fclose(file);
        }
    }
    zip_close(archive);
}




void instruction(){
    cout << "Giainen" << endl;
    cout << "Usage:giainen zipfilepath --bruteforce threaduse" << endl;
    cout << "or giainen zipfilepath --dictionary passwordsfilepath"<< endl;
    cout << endl;
    cout << "Modifier" << endl;
    cout << "-t <num_thread>\tNumber of threads used (default: "<< thread::hardware_concurrency()/2 << ")"<< endl;
    cout << "-l\t\tRemoving lowercase characters from characters set" << endl;
    cout << "-n\t\tAdding number to characters set" << endl;
    cout << "-u\t\tAdding uppercase characters from characters set" << endl;
    cout << "Default character set is set to lowercase character(a-z)" << endl;
    cout << endl;
    cout << "Example:" << endl;
    cout << " giainen file.zip --bruteforce 4" << endl;
}

void input(int argc, char* argv[]) {
    zip_t *archive;
    set<string> valid_flags = {"-b", "-l", "-d", "-u", "-t", "-l"};

    if (argc == 1) {
        instruction();  // Hiển thị hướng dẫn khi không có đối số
        exit(1);
    }

    ZipFileDirectory = argv[1];
    zip_filename = ZipFileDirectory.c_str();
    if(!isZipFile()){
        cerr << "Not a zip file" << endl;
        exit(1);
    }

    for (int i = 2; i < argc; ++i) {
        string flag = argv[i];
        // Kiểm tra các chế độ
        if (flag == "--bruteforce" || flag == "-b") {
            choice = 1;
            if (i + 1 < argc && isdigit(argv[i + 1][0])) {
                length = atoi(argv[++i]);
                if (length <= 0 || length > 6) {
                    cerr << "Length must be from 1 to 6" << endl;
                    exit(1);
                }
            } else {
                cerr << "Need number of character after --bruteforce" << endl;
                exit(1);
            }
        } else if (flag == "--dictionary" || flag == "-d") {
            choice = 2;
            if (i + 1 < argc) {
                PasswordFilePath = argv[++i];
                ifstream file(PasswordFilePath);
                if (!file) {
                    cerr << "Cannot open passwords file " << PasswordFilePath << endl;
                    exit(1);
                }
                else if (!isPassFile) {
                    cerr << "Not a passwords file " << endl;
                    exit(1);
                }
            } else {
                cerr << "Need passwords file path after --dictionary" << endl;
                exit(1);
            }
        } else if (flag == "-l") {
            // Xóa các ký tự chữ thường khỏi charList
            for (auto it = charList.begin(); it != charList.end(); ) {
                if (islower(*it)) {
                    it = charList.erase(it);  // Xóa ký tự nếu là chữ thường
                } else {
                    ++it;  // Tiến tới ký tự tiếp theo
                }
            }
        } else if (flag == "-u") {
            charList.insert(Alphabet.begin(), Alphabet.end());
        } else if (flag == "-n") {
            charList.insert(number.begin(), number.end());
        } else if (flag == "-t" && i + 1 < argc) {
            num_thread = atoi(argv[++i]);
            if (num_thread <= 0 || num_thread > thread::hardware_concurrency()) {
                cerr << "Thread must be from 1 to " << thread::hardware_concurrency() << endl;
                exit(1);
            }
        } else {
            cerr << "Unknown option: " << flag << endl;
        }
    }

    for (char ch : charList) {
        newCharList += ch;
    }

    if (choice == 0 || ZipFileDirectory.empty()) {
        cerr << "No method or directory found" << endl;
        exit(1);
    }

    archive = zip_open(zip_filename, 0, 0);

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    if (num_entries < 0) {
        cout << "Cannot find file in archive. Error " << num_entries << endl;
        zip_close(archive);
        exit(1);
    }

    cout << "Number of file in archive:" << endl;
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        file_name = zip_get_name(archive, i, 0);
        if (file_name) {
            cout << i + 1 << ": " << file_name << endl;
        } else {
            cout << "Cannot found file index " << i << endl;
        }
    }

    int tmp1;
    cout << "Pick a file " << endl;
    do {
        cin >> tmp1;
        if (tmp1 < 1 || tmp1 > num_entries) {
            cout << "Invalid ";
        }
    } while (tmp1 < 1 || tmp1 > num_entries);
    file_index = tmp1 - 1;
    filename = zip_get_name(archive, tmp1 - 1, 0);
    file_name = filename.c_str();
    cout << "File using " << file_name << endl;

    zip_stat_t stat;
    zip_stat(archive, file_name, 0, &stat);  // Lấy thông tin file
    correct_crc = stat.crc;  // Lấy CRC từ thông tin file
    zip_close(archive);

    if (choice == 1) {
        cout << "Bruteforce Mode" << endl;
        cout << "Number of character " << length << endl;
        cout << "Character in use: " << newCharList << endl;
    } else if (choice == 2) {
        cout << "Dictionary Mode" << endl;
        cout << "Password file path " << PasswordFilePath << endl;
    }
    cout << "Number of threads " << num_thread << endl;

    return;
}

void output(){
    cout << endl;
    if(!Found) {
        cout << "No password could be found" << endl;
    }
    cout << endl;
}

void processbruteforce(){
    total = pow(charList.size(),length);
    sizeOfList = newCharList.size();
    cout << "Total passwords " << total << endl;
    if(num_thread > 1){
        for(int i=1; i < num_thread; i++){
            testers.push_back(thread(Test1, zip_filename));
        }
    }
    TestAndProgress(zip_filename);
    for (auto &t : testers) {
        t.join();
    }
    return;
}

void processdictionary(){
    total = 9223372036854775807;
    for(int i=1; i < num_thread; i++){
        testers.push_back(thread(Test2, zip_filename));
    }
    passFromFile(PasswordFilePath);
    for (auto &t : testers) {
        t.join();
    }
    return;
}

void process() {
    if(choice == 1) {
        if(checkProgressFile()) {
            cout << "You have tried " << tmpIndex << " passwords before, Y to continue, other to restart" << endl;
            char tmp;
            cin >> tmp;
            if(tmp == 'Y'){
                Index = tmpIndex;
            }
        }
        processbruteforce();
    }
    else if(choice == 2) {
        processdictionary();
    }
    return;
}

int main(int argc, char* argv[]){
    signal(SIGINT, signalHandler);
    input(argc, argv);
    process();
    output();
    return 0;
}
