#include<bits/stdc++.h>
#include<thread>
#include<zip.h>

using namespace std;

deque<string> List;                 //Deque chứa các mật khẩu để thử
atomic<bool> Found = false;         //Khóa kiểm tra khi tìm thấy kết quả
atomic<long long> tried = 0;        //Tính số pass đã thử
queue<string> ErrPass;              //Queue chứa số mật khẩu gây lỗi
int day = 0;                        //Biến đếm số lần deque đầy
int het = 0;                        //Biến đếm số lần deque trống
mutex mtx;                          //Khóa deque tránh xung đột
set<char> charList= {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> Alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
set<char> number = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int length = 0;                     //Số kí tự trong mật khẩu
long long total;
string ZipFileDirectory;
string filename;
const char *zip_filename;
const char *file_name;
int num_thread;
vector<thread> testers;
int choice = 0;
string PasswordFilePath;

void Generate(string &pass, int cur_length){
    if (cur_length == length) {
        while(List.size() > 20000) {
            int width = 100;
            cout << "\r[";
            int pos = tried * width / total;
            for (int j = 0; j < width; ++j) {
                if (j < pos) cout << "=";
                else if (j == pos) cout << ">";
                else cout << " ";
            }
            cout << "] " << tried*100/total << "%" <<flush;
            this_thread::sleep_for(chrono::milliseconds(200));
            day++;
        }
        mtx.lock();
        List.push_back(pass);
        mtx.unlock();
        return;
    }
    for(char c : charList){
        pass[cur_length] = c;
        Generate(pass, cur_length +1);
    }
}

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
                cout << "So mat khau da thu " << tried << "\n";
                this_thread::sleep_for(chrono::milliseconds(200));
                day++;
            }
        }
    }
    passfile.close();
}

void Test(const char *zip_filename, const char *file_name) {
    int err = 0;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    string pass;
    while(!Found){
        mtx.lock();
        if(!List.empty()){
            pass = List.front();
            List.pop_front();
            tried++;
            mtx.unlock();
        }
        else{
            het++;
            mtx.unlock();
            if(tried == total) {
                return;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
            zip_file_t *file = zip_fopen_encrypted(archive, file_name, 0, pass.c_str());

            if(file){
                char buffer[16];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    cout << "Mat khau dung la " << pass << "\n";
                    Found = true;
                    zip_fclose(file);
                    zip_close(archive);
                    return;
                }
                else {
                    zip_fclose(file);
                    ErrPass.push(pass);
                }
            }
        }
    zip_close(archive);
    return;
}

void createCharList();

void input() {
    string tmp;
    bool flag;
    int err = 0;
    zip_t *archive;

    cout << "Nhap vao duong dan" << endl;
    do {
        cin >> ZipFileDirectory;
        zip_filename = ZipFileDirectory.c_str();
        archive = zip_open(zip_filename, 0, &err);
        if (!archive) {
            cout << "Khong tim thay duong dan Error " << err << endl;
            cout << "Hay nhap lai duong dan" << endl;
        }
    } while (!archive);

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

    cout << "Chon file trong ZIP bang cach nhap so thu tu: " << endl;
    do {
        cin >> choice;
        if (choice < 1 || choice > num_entries) {
            cout << "Lua chon khong hop le. Vui long nhap lai: ";
        }
    } while (choice < 1 || choice > num_entries);
    filename = zip_get_name(archive, choice - 1, 0);
    file_name = filename.c_str();
    cout << "Ban da chon file: " << file_name << endl;
    zip_close(archive);

    cout << "Nhap vao so luong" << endl;
    do {
        cin >> tmp;
        try {
        num_thread = stoi(tmp);
        flag = true;
        }
        catch (const invalid_argument&) {
            cout << "Du lieu khong hop le. Vui long nhap lai (0-16): " << endl;
            num_thread = -1;
            flag = false;
        }
        catch (const out_of_range&) {
            cout << "Gia tri qua lon. Vui long nhap lai (0-16): " << endl;
            num_thread = -1;
            flag = false;
        }
        if (((num_thread < 0) || (num_thread >16)) && (flag == true)) {
            cout << "Gia tri qua lon. Vui long nhap lai (0-16): " << endl;
        }
    }
    while((num_thread < 0) || (num_thread >16));

    cout <<"Chon cach thuc" << endl;
    cout <<"1.Bruteforce"<< endl;
    cout <<"2.Su dung danh sach mat khau" << endl;
    do {
        cin >> choice;
        if((choice != 1) && (choice != 2)) {
            cout << "Xin chon 1 lua chon khac" << endl;
        }
    } while((choice != 1) && (choice != 2));
    if(choice == 1){
        cout << "Nhap vao so ky tu" << endl;
        do {
            cin >> tmp;
            try {
            length = stoi(tmp);
            flag = true;
            }
            catch (const invalid_argument&) {
                cout << "Du lieu khong hop le. Vui long nhap lai (0-6): " << endl;
                length = -1;
                flag = false;
            }
            catch (const out_of_range&) {
                cout << "Gia tri qua lon. Vui long nhap lai (0-6): " << endl;
                length = -1;
                flag = false;
            }
            if (((length < 0) || (length >6)) && (flag == true)) {
                cout << "Gia tri qua lon. Vui long nhap lai (0-6): " << endl;
            }
        }
        while((num_thread < 0) || (num_thread >6));
        createCharList();
        return;
    }
    else if (choice == 2) {
        cout << "Nhap vao duong dan file chua mat khau" << endl;
        while (!isPassFile(PasswordFilePath)) {
            cin >> PasswordFilePath;
            if(!isPassFile(PasswordFilePath)) {
                cout << "Khong the mo tep, hay nhap lai " << PasswordFilePath << endl;
            }
        return;
        }
    }
}

void createCharList(){
    char tmp;
    cout << "Nhap 1 de su dung bang chu cai thuong, mac dinh se su dung bang chu cai thuong" << endl;
    cout << "Nhap 2 de su dung bang chu cai hoa" << endl;
    cout << "Nhap 3 de su dung so" << endl;
    cout << "Cac ki tu con lai co the nhap tu ban phim" << endl;
    cout << "Nhap 0 de bat dau chay" << endl;
    while(tmp != '0'){
        cin >> tmp;
        if(tmp == '1') {
            charList.insert(alphabet.begin(), alphabet.end());
            continue;
        }
        else if(tmp == '2') {
            charList.insert(Alphabet.begin(), Alphabet.end());
            continue;
        }
        else if(tmp == '3') {
            charList.insert(number.begin(),number.end());
            continue;
        }
        else if(tmp != '0') charList.insert(tmp);
    }
    return;
}

void output(){
    cout << endl;
    if(!Found) {
        cout << "Khong tim thay mat khau dung" << endl;
    }
    cout << "Danh sach mat khau gay loi CRC" << endl;
    while (!ErrPass.empty()) {
        cout << ErrPass.front() << endl;
        ErrPass.pop();
    }

    cout << endl;
    cout << "So lan het la " << het <<"\n";
    cout << "So lan day la " << day <<"\n";
}

void process(){
    cout << choice << endl;
    string pass(length, ' ');
    if(choice == 1) {
        total = pow(charList.size(),length);
        cout << "So pass can thu " << total << endl;
    }

    for(int i=0; i < num_thread; i++){
        testers.push_back(thread(Test, zip_filename, file_name));
    }

    if(choice == 1) {
        cout << "Bat dau bruteforce" << endl;
        Generate(pass, 0);
    }
    else if(choice == 2) {
        passFromFile(PasswordFilePath);
    }
    for (auto &t : testers) {
        t.join();
    }
    return;
}

int main(){
    string terminate;
    while(true) {
        input();
        process();
        output();
        return 0;
        getline(cin, terminate);
        if(terminate.empty()) break;
    }
}
