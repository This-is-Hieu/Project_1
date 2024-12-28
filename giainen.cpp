#include<bits/stdc++.h>
#include<thread>
#include<zip.h>

using namespace std;

deque<string> List;
atomic<bool> Full = false;
atomic<bool> Found = false;
atomic<bool> Empty = true;
queue<string> ErrPass;
int day=0;
int het = 0;
mutex mtx;

void Check() {
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

void Generate(){
    char i,j,k,l;
    string password = "0000";
    for (i = 'a'; i <= 'z'; ++i) {
        for (j = 'a'; j <= 'z'; ++j) {
            for (k = 'a'; k <= 'z'; ++k) {
                for (l = 'a'; l <= 'z'; ++l) {
                    if(Full){
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }
                    else{
                        password[0] = i;
                        password[1] = j;
                        password[2] = k;
                        password[3] = l;
                        mtx.lock();
                        List.push_back(password);
                        mtx.unlock();
                    }
                }
            }
        }
    }
    return;
}

void Test(const char *zip_filename, const char *file_name_in_zip) {
    int err = 0;
    zip_t *archive = zip_open(zip_filename, 0, &err);
    if (!archive) {
        cout << "Khong the mo file zip  " << err << "\n";
        return;
    }
    const char* password = "0000";
    while(!Found){
        mtx.lock();
        if(!Empty){
            password = List.front().c_str();
            List.pop_front();
            mtx.unlock();
        }
        else{
            mtx.unlock();
            continue;
        }
            cout << password <<"\n";
            zip_file_t *file = zip_fopen_encrypted(archive, file_name_in_zip, 0, password);
            if(file){
                ErrPass.push(password);
                char buffer[1024];
                int bytes_read = zip_fread(file, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    cout << "Mat khau dung la " << password << "\n";
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
int main(){
    const char *zip_filename = "D:/OneDrive/Desktop/123.zip";
    const char *file_name_in_zip = "123.txt";
    thread checking(Check);
    thread generator(Generate);
    thread tester1(Test,zip_filename, file_name_in_zip);
    thread tester2(Test,zip_filename, file_name_in_zip);
    thread tester3(Test,zip_filename, file_name_in_zip);
    thread tester4(Test,zip_filename, file_name_in_zip);
    checking.join();
    generator.join();
    tester1.join();
    tester2.join();
    tester3.join();
    tester4.join();
    while (!ErrPass.empty()) {
        cout << ErrPass.front() << endl;
        ErrPass.pop();
    }

    cout << "So lan het la " << het <<"\n";
    cout << "So lan day la " << day <<"\n";
    return 0;
}
