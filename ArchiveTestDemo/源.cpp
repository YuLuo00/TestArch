#include <archive.h>
#include <archive_entry.h>

#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

#include <fstream>


int example_unpress()
{
    auto loc = setlocale(LC_ALL, "zh_US.utf8");
    struct archive *a;
    struct archive_entry *entry;
    int r;
    const char *password = "key\0"; // 

    const char *filePath = R"(Desktop.rar)";

    a = archive_read_new();
    r = archive_read_support_format_all(a);
    r = archive_read_support_filter_all(a);
    //archive_read_set_options(a, "7zip:use_cryptography_extension=1");
    if (r != ARCHIVE_OK) {
        exit(1);
    }

    const char *format = NULL;

    if (archive_read_add_passphrase(a, password) != ARCHIVE_OK) {
        auto i = archive_error_string(a);
        printf("Error adding passphrase: %s\n", i);
        return 1;
    }

    r = archive_read_open_filename(a, filePath, 500 * 1240); //            test.zip        
    if (r != ARCHIVE_OK) {
        auto errInfo = archive_error_string(a);
        cout << "Error opening compressed file." << endl;
        return 1;
    }

    bool is_encrypted = archive_read_has_encrypted_entries(a);

    int size = 1024 * 1024;
    char *buff = new char[size];
    while (1) {
        r = archive_read_next_header(a, &entry);
        if (r != ARCHIVE_OK) {
            auto i = archive_error_string(a);
            break;
        }

        format = archive_format_name(a);
        // 获取文件名
        //const wchar_t* wfileName = archive_entry_pathname_utf8
        LPCWSTR filename = archive_entry_pathname_w(entry); //;"tmp"
        if (filename == nullptr) {
            break;
        }
        cout << "Extracting: " << filename << endl;
        // 打开文件并写入解压数据
        auto file = CreateFile(filename, GENERIC_WRITE, 1, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE) {
            break;
        }
        int fileDescriptor = _open_osfhandle(reinterpret_cast<intptr_t>(file), _O_WRONLY | _O_TEXT);
        if (fileDescriptor == -1) {
            break;
        }

        r = archive_read_data_into_fd(a, fileDescriptor);
        if (r < 0) {
            auto i = archive_error_string(a);
            break;
        }

        if (CloseHandle(file) == 0) {
            break;
        }
    }
    delete[]buff;


    archive_read_close(a);
    archive_read_free(a);

    return 0;
}

void main() {
    example_unpress();
}