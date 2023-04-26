#include <archive.h>
#include <archive_entry.h>

#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
using namespace std;

class LogDisk {
public:
    LogDisk(){};
    template <class T>
    const LogDisk& operator<<(T t) const {
        return *(this);
    };
};
LogDisk loginfo;

#include <fstream>

void extract(const char* filename, int flags);

const char* pass_cb(struct archive* a, void* _client_data) {
    if (archive_read_add_passphrase(a, "key") != ARCHIVE_OK) {
        auto i = archive_error_string(a);
        printf("Error adding passphrase: %s\n", i);
        return "key";
    }
}

int example_unpress() {
    auto                  loc = setlocale(LC_ALL, "zh_US.utf8");
    struct archive*       a;
    struct archive_entry* entry;
    int                   r;
    const char*           password = "key";  //

    const char* filePath = R"(Desktop.rar)";

    a = archive_read_new();
    r = archive_read_support_format_all(a);
    r = archive_read_support_filter_all(a);
    // archive_read_set_options(a, "7zip:use_cryptography_extension=1");
    if (r != ARCHIVE_OK) {
        exit(1);
    }

    const char* format = NULL;

    if (archive_read_add_passphrase(a, password) != ARCHIVE_OK) {
        auto i = archive_error_string(a);
        printf("Error adding passphrase: %s\n", i);
        return 1;
    }

    if (archive_read_set_passphrase_callback(a, NULL, pass_cb) != ARCHIVE_OK) {
        auto i = archive_error_string(a);
        printf("Error adding passphrase: %s\n", i);
        return 1;
    }

    r = archive_read_open_filename(a, filePath,
                                   500 * 1240);  //            test.zip
    if (r != ARCHIVE_OK) {
        auto errInfo = archive_error_string(a);
        cout << "Error opening compressed file." << endl;
        return 1;
    }

    bool is_encrypted = archive_read_has_encrypted_entries(a);

    int   size = 1024 * 1024;
    char* buff = new char[ size ];
    while (1) {
        r = archive_read_next_header(a, &entry);
        if (r != ARCHIVE_OK) {
            auto i = archive_error_string(a);
            break;
        }

        format = archive_format_name(a);
        // 获取文件名
        // const wchar_t* wfileName = archive_entry_pathname_utf8
        LPCWSTR filename = archive_entry_pathname_w(entry);  //;"tmp"
        if (filename == nullptr) {
            break;
        }
        cout << "Extracting: " << filename << endl;
        // 打开文件并写入解压数据
        HANDLE file = NULL;
        file        = CreateFile(filename, GENERIC_WRITE, 1, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
    delete[] buff;

    archive_read_close(a);
    archive_read_free(a);

    return 0;
}

void main() {
    // example_unpress();

    const char* filename = "Desktop.rar";
    int         flags    = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    extract(filename, flags);
}

static int copy_data(struct archive* ar, struct archive* aw) {
    int         r;
    const void* buff;
    size_t      size;
    int64_t offset;

    const char* format = archive_format_name(ar);
    bool is_encrypted = archive_read_has_encrypted_entries(ar);
    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF || r != ARCHIVE_OK) {
            auto i = archive_error_string(ar);
            loginfo << "" << r;
            return r;
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            auto i = archive_error_string(aw);
            loginfo << "archive_write_data_block()" << i;
            return (r);
        }
    }
}

static void extract(const char* filename, int flags) {

    auto                  loc = setlocale(LC_ALL, "zh_US.utf8");
    struct archive*       a;
    struct archive*       ext;
    struct archive_entry* entry;
    int                   r;

    a   = archive_read_new();
    ext = archive_write_disk_new();

    archive_write_disk_set_options(ext, flags);
    /*
     * Note: archive_write_disk_set_standard_lookup() is useful
     * here, but it requires library routines that can add 500k or
     * more to a static executable.
     */
    r = archive_read_support_format_all(a);
    r = archive_read_support_filter_all(a);
    r = archive_read_add_passphrase(a, "123");
    archive_read_set_passphrase_callback(a, NULL, pass_cb);
    /*
     * On my system, enabling other archive formats adds 20k-30k
     * each.  Enabling gzip decompression adds about 20k.
     * Enabling bzip2 is more expensive because the libbz2 library
     * isn't very well factored.
     */

    if ((r = archive_read_open_filename(a, filename, 1 * 1024))) {
        ("archive_read_open_filename()", archive_error_string(a), r);
    }
    // 遍历读取
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        else if (r == ARCHIVE_FAILED) {
            auto i = archive_error_string(a);
            loginfo << "archive_read_next_header()" << i;
            continue;
        }
        else if(r != ARCHIVE_OK) {
            auto i = archive_error_string(a);
            loginfo << "archive_read_next_header()" << i;
        }

        auto outPath = archive_entry_pathname(entry);

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            auto i = archive_error_string(ext);
            loginfo << "archive_write_header()" << i;
        }

        const char* format       = archive_format_name(a);
        bool        is_encrypted = archive_read_has_encrypted_entries(a);

        // 循环解压
        for (;;) {
            const void* buff;
            size_t      size;
            int64_t     offset;
            r = archive_read_data_block(a, &buff, &size, &offset);
            if (r == ARCHIVE_EOF) {
                break;
            }
            else if (r == ARCHIVE_FAILED) {
                auto i = archive_error_string(a);
                loginfo << "archive_read_next_header()" << i;
                continue;
            }
            else if (r != ARCHIVE_OK) {
                auto i = archive_error_string(a);
                loginfo << "archive_read_next_header()" << i;
            }

            r = archive_write_data_block(ext, buff, size, offset);
            if (r != ARCHIVE_OK) {
                auto i = archive_error_string(ext);
                loginfo << "archive_write_data_block()" << i;
            }
        }

        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            auto i = archive_error_string(ext);
            loginfo << "archive_write_finish_entry()" << i;
        }
    }
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);
}
