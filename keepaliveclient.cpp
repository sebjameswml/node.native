#include <iostream>
#include <memory>
#include <string>
#include <native/native.h>
using namespace native;
using namespace std;

#include <fcntl.h>

int main() {

    auto client = net::tcp::create();

    client->connect("127.0.0.1", 8081, [=](error e){
        // ------------------------------------------------------------------
        // First write to the server
        // ------------------------------------------------------------------
        client->write("GET / HTTP/1.1\r\n\r\n", [=](error e){
            shared_ptr<string> response(new string);
            client->read_start([=](const char* buf, ssize_t len){
                if(len < 0) // EOF
                {
                    cout << "Closing connection after first response" << endl;
                    client->close([](){});
                }
                else
                {
                    response->append(string(buf, len));
                    cout << "\nFirst response:\n---------------\n" << buf << "\n---------------\n";
                    // ------------------------------------------------------------------
                    // Second write to the server
                    // ------------------------------------------------------------------
                    // As we're expecting to need to send another message before the
                    // connection will be closed by keepaliveserver.cpp, let's do so now:
                    response->clear();
                    string postmsg("POST / HTTP/1.1\r\n"
                                   "Content-Length: 52\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "User-Agent: node.native/keepaliveclient\r\n\r\n"
                                   "Post message body, as written by keepaliveclient.cpp");
                    client->write(postmsg, [=](error e){
                        shared_ptr<string> response(new string);
                        client->read_start([=](const char* buf, ssize_t len){
                            if(len < 0) // EOF
                            {
                                cout << "Closing connection after second response." << endl;
                                client->close([](){});
                            }
                            else
                            {
                                response->append(string(buf, len));
                                cout << "\nSecond response:\n----------------\n" << buf
                                     << "\n----------------\n";
                            }
                        });
                    });
                    //-------------------------------------------------------------------
                }
            });
        });
    });

    return run();
}
