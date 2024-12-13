
#include "stdafx.h"
#include "Server.h"
#include "afxsock.h"
#include "math.h"
#include <fstream>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

using namespace std;

// Struct để lưu thông tin file
struct FileInfo {
    string name;
    long size;
};

vector<FileInfo> readFileList(const string& filename) {
    vector<FileInfo> fileList;
    ifstream file(filename);

    if (!file.is_open()) {
        printf("Không thể mở file: %s\n", filename.c_str());
        return fileList; // Trả về danh sách rỗng
    }

    string line;
    while (getline(file, line)) {
        size_t lastSpace = line.find_last_of(' ');
        if (lastSpace != string::npos) {
            string name = line.substr(0, lastSpace); // Lấy tên file
            long size = stol(line.substr(lastSpace + 1)); // Lấy kích thước file
            fileList.push_back({ name, size });
        }
        else {
            printf("Dòng không hợp lệ: %s\n", line.c_str());
        }
    }

    file.close();
    return fileList;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]) {
    int nRetCode = 0;

    // initialize MFC and print and error on failure
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
        _tprintf(_T("Fatal Error: MFC initialization failed\n"));
        nRetCode = 1;
    }
    else {
        CSocket server;
        unsigned int port = 1234;
        AfxSocketInit(NULL);

        server.Create(port);
        server.Listen(5);

        // Đọc danh sách file từ file text
        vector<FileInfo> fileList = readFileList("filelist.txt");

        printf("\n Dang lang nghe ket noi tu Client...\n");
        while (true) {
            CSocket client;
            server.Accept(client);
            printf("Da tiep nhan mot client moi\n");
            string fileListStr;
            for (const auto& file : fileList) {
                fileListStr += file.name + " " + to_string(file.size) + "MB\n";
            }

            // Gửi kích thước trước
            int fileListSize = fileListStr.size();
            client.Send(&fileListSize, sizeof(fileListSize));

            // Gửi nội dung danh sách file
            int bytesSent = client.Send(fileListStr.c_str(), fileListSize);
            if (bytesSent == SOCKET_ERROR) {
                printf("Lỗi khi gửi danh sách file tới client.\n");
            }
            // Nhận yêu cầu từ client
            char buffer[1024];
            int bytesReceived = client.Receive(buffer, 1024);
            buffer[bytesReceived] = '\0';
            string request(buffer);

            // Xử lý yêu cầu
            // Giả sử yêu cầu có dạng "filename offset"
           /* string filename;
            long offset;
            sscanf(request.c_str(), "%s %ld", &filename[0], &offset);*/

            char tempFilename[256] = { 0 }; // Mảng tạm lưu filename
            long offset;
            sscanf(request.c_str(), "%255s %ld", tempFilename, &offset); // Tránh tràn bộ đệm
            string filename(tempFilename); // Chuyển thành std::string
            // Tìm file trong danh sách
            auto it = find_if(fileList.begin(), fileList.end(), [&filename](const FileInfo& file) {
                cout << filename;
                return file.name == filename;
                });

            if (it != fileList.end()) {
                // Mở file và gửi dữ liệu từ offset
                ifstream file(it->name, ios::binary);
                if (file.is_open()) {
                    file.seekg(offset, ios::beg);
                    while (!file.eof()) {
                        file.read(buffer, sizeof(buffer));
                        client.Send(buffer, file.gcount());
                    }
                    file.close();
                }
            }

            client.Close();
        }
        server.Close();
    }
    return nRetCode;
}
