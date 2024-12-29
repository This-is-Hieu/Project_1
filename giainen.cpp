#include<bits/stdc++.h>
#include<thread>
#include<zip.h>

using namespace std;

deque<string> List;                 //Deque chứa các mật khẩu để thử
atomic<bool> Full = false;          //Khóa kiểm tra khi deque đầy
atomic<bool> Found = false;         //Khóa kiểm tra khi tìm thấy kết quả
atomic<bool> Empty = true;          //Khóa kiểm tra khi deque rỗng
atomic<long long> tried = 0;
queue<string> ErrPass;              //Queue chứa số mật khẩu gây lỗi
int day=0;                          //Biến đếm số lần deque đầy
int het = 0;                        //Biến đếm số lần deque trống
mutex mtx;                          //Khóa deque tránh xung đột
set<char> charList= {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
set<char> Alphabet = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
set<char> number = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int length = 0;                     //Số kí tự trong mật khẩu
const char *zip_filename = "D:/OneDrive/Desktop/123.zip";
const char *file_name_in_zip = "123.txt";


void Generate(string &pass, int cur_length){
    if (cur_length == length) {
        while(List.size() > 5000) {
            this_thread::sleep_for(chrono::milliseconds(50));
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

void Test(const char *zip_filename, const char *file_name_in_zip) {
    int err = 0;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    if (!archive) {
        cout << "Khong the mo file zip  " << err << "\n";
        return;
    }
    string pass ;
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
            continue;
        }
            zip_file_t *file = zip_fopen_encrypted(archive, file_name_in_zip, 0, pass.c_str());
            if(file){
                char buffer[1024];
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

void input(int &num_thread) {
    string tmp;
    bool flag;

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
    return;
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
        if(tmp == '2') {
            charList.insert(Alphabet.begin(), Alphabet.end());
            continue;
        }
        if(tmp == '3') {
            charList.insert(number.begin(), Alphabet.end());
            continue;
        }
        if(tmp != '0') charList.insert(tmp);
    }
    return;
}

void progress_bar() {
    long long total = pow(charList.size(),length);
    int width = 100;
    cout << "So mat khau can xu li: " << total <<endl;
    cout << "Cac ky tu se su dung" << endl;
    for(char c : charList){
        cout << c << " ";
    }
    cout << endl;
    while(!Found){
        cout << "\r[";
        int pos = tried *width / total;
        for (int j = 0; j < width; ++j) {
            if (j < pos) cout << "=";
            else if (j == pos) cout << "";
            else cout << " ";
        }
        cout << "] " << tried*100/total << "%" <<flush;
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    cout << endl;
}

int main(){
    int num_thread;
    vector<thread> testers;
    input(num_thread);

    string pass(length, ' ');
    thread generator(Generate, ref(pass), 0);
    for(int i=0; i < num_thread; i++){
        testers.push_back(thread(Test, zip_filename, file_name_in_zip));
    }
    progress_bar();

    generator.join();
    for (auto &t : testers) {
        t.join();
    }

    cout << endl;
    cout << "Danh sach mat khau gay loi CRC" << endl;
    while (!ErrPass.empty()) {
        cout << ErrPass.front() << endl;
        ErrPass.pop();
    }

    cout << endl;
    cout << "So lan het la " << het <<"\n";
    cout << "So lan day la " << day <<"\n";
    return 0;
}
