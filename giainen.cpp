#include<bits/stdc++.h>
#include<thread>
#include<zip.h>

using namespace std;
deque<string> List;
atomic<bool> Found = false;         //Khóa kiểm tra khi tìm thấy kết quả
atomic<long long> Index(0);        //Tính số pass đã thử
queue<string> Pass;              //Queue chứa số mật khẩu gây lỗi
set<char> charList= {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> Alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
set<char> number = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
string newCharList;
int length = 0;                     //Số kí tự trong mật khẩu
long long total;                    //Tổng số mật khẩu cần thử
string ZipFileDirectory;
string filename;
const char *zip_filename;
const char *file_name;
int num_thread;                     //Số luồng
vector<thread> testers;             //Vector chứa luồng
int choice = 0;
string PasswordFilePath;
int sizeOfList = 0;
mutex mtx;
bool theEnd = false;

bool isPassFile(const string PasswordFilePath) {
    ifstream passfile(PasswordFilePath);
    if (!passfile.is_open()) {
        return false;
    }
    passfile.close();
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
                cout << "So mat khau da thu " << Index << "\n";
                this_thread::sleep_for(chrono::milliseconds(100));

            }
        }
    }
    while(!List.empty()){};
    theEnd = true;
    passfile.close();
}

void Test2(const char *zip_filename, const char *file_name) {
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
            zip_file_t *file = zip_fopen_encrypted(archive, file_name, 0, password);
            if(file){
                char buffer[1024];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 1023) {
                    cout << "Mat khau dung la " << pass << "\n";
                    Found = true;
                    zip_fclose(file);
                    zip_close(archive);
                    return;
                }
                else {
                    zip_fclose(file);
                    Pass.push(pass);
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

inline string Convert(long long tmp) {
    string pass = "";
    while(tmp > 0) {
        pass = newCharList[tmp % sizeOfList] + pass;
        tmp /= sizeOfList;
    }
    return pass;
}

void Test1(const char *zip_filename, const char *file_name) {
    int err = 0;
    long long tmp;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    string pass;
    const char* password = pass.c_str();
    while((!Found) && (Index < total)){
            long long tmp = Index.fetch_add(1, std::memory_order_relaxed);
            pass = Convert(tmp);
            zip_file_t *file = zip_fopen_encrypted(archive, file_name, 0, password);
            if(file){
                char buffer[4096];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 4095) {
                    mtx.lock();
                    Pass.push(pass);
                    mtx.unlock();
                    zip_fclose(file);
                }
                else {
                    zip_fclose(file);

                }
            }
        }
    zip_close(archive);
    return;
}

void TestAndProgress(const char *zip_filename, const char *file_name) {
    int err = 0;
    long long tmp;
    zip_t *archive = zip_open(zip_filename, 0,0);
    string pass;
    const char* password = pass.c_str();
    int time = 10000;
    while((!Found) && (Index < total)){
            long long tmp = Index.fetch_add(1, std::memory_order_relaxed);
            if(time == 0 ){
                cout << Index << "\n";
                time = 10000;
            }
            pass = Convert(tmp);
            zip_file_t *file = zip_fopen_encrypted(archive, file_name, 0, password);
            if(file){
                char buffer[4096];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 4095) {
                    mtx.lock();
                    Pass.push(pass);
                    mtx.unlock();
                    zip_fclose(file);
                }
                else {
                    zip_fclose(file);

                }
            }
            time--;
        }
    zip_close(archive);
    return;
}

void createCharList();

void input(int argc, char* argv[]) {
    zip_t *archive;
    set<string> valid_flags = {"-b", "-l", "-d", "-u", "-f"};
    if (argc == 1) {
        cerr << "Usage: Giainen -b(-d) [flags(-l, -u, -n)] -f[filename]" << endl;
        exit(1);
    }
    for (int i = 1; i < argc; ++i) {
        string flag = argv[i];
        if (flag == "-b") {
            choice = 1;
        } else if (flag == "-d") {
            choice = 2;
        } else if (flag == "-l") {
            charList.insert(alphabet.begin(), alphabet.end());
        } else if (flag == "-u") {
            charList.insert(Alphabet.begin(), Alphabet.end());
        } else if (flag == "-n") {
            charList.insert(number.begin(), number.end());
        } else if (flag == "-f" && i + 1 < argc) {
            ZipFileDirectory = argv[++i];
            zip_filename = ZipFileDirectory.c_str();
            archive = zip_open(zip_filename, 0, 0);
        } else {
            cerr << "Unknown option: " << flag << endl;
        }
    }
    for (char ch : charList) {
        newCharList += ch;
    }

    if (choice == 0 || ZipFileDirectory.empty()) {
        cerr << "Error: Missing required flags or filename." << endl;
        exit(1);
    }


        filename = ZipFileDirectory;
        zip_filename = ZipFileDirectory.c_str();
        archive = zip_open(zip_filename, 0, 0);

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    if (num_entries < 0) {
        cout << "Khong the lay danh sach file trong ZIP Error " << num_entries << endl;
        zip_close(archive);
        return;
    }

    cout << "Danh sach cac file trong ZIP:" << endl;
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        file_name = zip_get_name(archive, i, 0);
        if (file_name) {
            cout << i + 1 << ": " << file_name << endl;
        } else {
            cout << "Khong the lay ten file tai vi tri " << i << endl;
        }
    }
    int tmp1;
    cout << "Chon file trong ZIP bang cach nhap so thu tu: " << endl;
    do {
        cin >> tmp1;
        if (tmp1 < 1 || tmp1 > num_entries) {
            cout << "Lua chon khong hop le. Vui long nhap lai: ";
        }
    } while (tmp1 < 1 || tmp1 > num_entries);
    filename = zip_get_name(archive, tmp1 - 1, 0);
    file_name = filename.c_str();
    cout << "Ban da chon file: " << file_name << endl;
    zip_close(archive);
    return;
}


void output(){
    cout << endl;
    if(!Found) {
        cout << "Khong tim thay mat khau dung" << endl;
    }
    cout << "Danh sach mat khau co the su dung: " << endl;
    while (!Pass.empty()) {
        cout << Pass.front() << endl;
        Pass.pop();
    }

    cout << endl;
}

void processbruteforce(){
    total = pow(charList.size(),length);
    sizeOfList = newCharList.size();
    cout << "So pass can thu " << total << endl;
    if(num_thread > 1){
        for(int i=1; i < num_thread; i++){
            testers.push_back(thread(Test1, zip_filename, file_name));
        }
    }
    TestAndProgress(zip_filename, file_name);
    for (auto &t : testers) {
        t.join();
    }
    return;
}


void processdictionary(){
    total = 9223372036854775807;
    cout << "Ban da chon su dung tu dien" << endl;
    for(int i=1; i < num_thread; i++){
        testers.push_back(thread(Test2, zip_filename, file_name));
    }
    passFromFile(PasswordFilePath);
    for (auto &t : testers) {
        t.join();
    }
    return;
}

void process() {
    cout << zip_filename;
    cout << ZipFileDirectory;
    if(choice == 1) {
        cout << "May ban hien co " << std::thread::hardware_concurrency() << " luong" << endl;
        do {
            cout << "Nhap so luong muon su dung: ";
            cin >> num_thread;
            if ((num_thread <= 0)||(num_thread >=16)) {
                cout << "So luong luong phai lon hon 0. Vui long nhap lai." << endl;
            }
        }
        while ((num_thread <= 0)||(num_thread >=16));
        cout << "Nhap vao so ky tu" << endl;
        do {
            cin >> length;
            if ((length < 0) || (length >6)) {
                cout << "Gia tri qua lon. Vui long nhap lai (0-6): " << endl;
            }
        }
        while((length < 0) || (length> 6));
        processbruteforce();
    }
    else if(choice == 2) {
        cout << "May ban hien co " << std::thread::hardware_concurrency() << " luong" << endl;
        do {
            cout << "Nhap so luong muon su dung: ";
            cin >> num_thread;
            if ((num_thread <= 0)||(num_thread >=16)) {
                cout << "So luong luong phai lon hon 0. Vui long nhap lai." << endl;
            }
        }
        while ((num_thread <= 0)||(num_thread >=16));
        cout << "Nhap duong dan toi file tu dien"; fflush(stdin); getline(cin,PasswordFilePath);
        processdictionary();
    }
    return;
}

int main(int argc, char* argv[]){
    input(argc, argv);
    process();
    output();
    return 0;
}
