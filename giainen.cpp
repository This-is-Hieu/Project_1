#include<bits/stdc++.h>
#include<thread>
#include<zip.h>

using namespace std;

deque<string> List;                 //Deque chứa các mật khẩu để thử
atomic<bool> Full = false;          //Khóa kiểm tra khi deque đầy
atomic<bool> Found = false;         //Khóa kiểm tra khi tìm thấy kết quả
atomic<bool> Empty = true;          //Khóa kiểm tra khi deque rỗng
queue<string> ErrPass;              //Queue chứa số mật khẩu gây lỗi
int day=0;                          //Biến đếm số lần deque đầy
int het = 0;                        //Biến đếm số lần deque trống
mutex mtx;                          //Khóa deque tránh xung đột
vector<char> charList = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
int length = 0;                     //Số kí tự trong mật khẩu
const char *zip_filename = "D:/OneDrive/Desktop/123.zip";
const char *file_name_in_zip = "123.txt";

void Check() {                      //Hàm kiểm tra trạng thái luồng
    while (true) {
        if(List.size() > 5000){
            Full = true;
            Empty = false;
            day++;
        }
        else if(List.empty()){
            Full = false;
            Empty = true;
            het++;
        }
        else{
            Full = false;
            Empty = false;
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}


void Generate(string &pass, int cur_length){
    if (cur_length == length) {
        while(List.size() > 5000) {
            this_thread::sleep_for(chrono::milliseconds(100));
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
            mtx.unlock();
        }
        else{
            mtx.unlock();
            continue;
        }
            cout << pass <<"\n";
            zip_file_t *file = zip_fopen_encrypted(archive, file_name_in_zip, 0, pass.c_str());
            if(file){
                ErrPass.push(pass);
                char buffer[1024];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    cout << "Mat khau dung la " << pass << "\n";
                    Found = true;
                    zip_fclose(file);
                    zip_close(archive);
                    return;
                }
                else zip_fclose(file);
            }
        }
    zip_close(archive);
    return;
}
void input(int &num_thread) {
    cout << "Nhap vao so luong" << endl;
    cin >> num_thread;
    cout << "Nhap vao so ky tu" << endl;
    cin >> length;
}

void init() {

}

int main(){
    int num_thread = 4;
    vector<thread> testers;
    input(num_thread);
    thread checking(Check);
    string pass(length, ' ');
    thread generator(Generate, ref(pass), 0);
    for(int i=0; i < num_thread; i++){
        testers.push_back(thread(Test, zip_filename, file_name_in_zip));
    }
    checking.join();
    generator.join();

    for (auto &t : testers) {
        t.join();
    }

    while (!ErrPass.empty()) {
        cout << ErrPass.front() << endl;
        ErrPass.pop();
    }

    cout << "So lan het la " << het <<"\n";
    cout << "So lan day la " << day <<"\n";
    return 0;
}
