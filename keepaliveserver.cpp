/*
 * This is a sample of a server which makes a connection from a client
 * into a keep-alive connection. It uses a variable in the scope of
 * the main function to store a number which is linked to each
 * client_context (client_context is held as a member of the
 * http::response). This number is a counter. On the first request
 * from the client, a response is sent with Connection: Keep-alive. On
 * a second request from the client, a response is sent with
 * Connection: Close.
 *
 * This is controlled here with
 * http::response::set_keep_alive_timeout.
 *
 * Note also, that in this example, if the client request contains
 * Expect: 100-continue, then a 100-continue is sent prior to the
 * response (controlled with http::response::set_send_continue_first.
 */

#include <iostream>
#include <sstream>
#include <native/native.h>

using namespace native::http;
using namespace std;

int main() {
    http server;
    int port = 8081;

    // key: response id (pointer address), value "stage counter"
    std::map<native::http::response*, int> msg_number;

    if(!server.listen("0.0.0.0", port, [=,&msg_number](request& req, response& res) {

        cout << "User-registered callback START." << endl;

        // Using the memory address of response to identify
        // the connection - for each response in a kept-alive
        // connection, the response object is re-used, hence
        // its memory address will not change.
        try {
            msg_number[&res]++;
        } catch (std::out_of_range& e) {
            stringstream ss;
            ss << "Failed to record a message number count for the response with id 0x"
               << hex << &res << endl;
            throw runtime_error (ss.str());
        }

        cout << " msg_number: " << msg_number.at(&res)
             << " for response ID 0x" << hex << &res << endl;

        res.set_header("Content-Type", "text/html");
        res.set_status(200);

        if (req.get_header("Expect") == "100-continue") {
            res.set_send_continue_first(true);
        }

        if (msg_number[&res] <= 1) {
            // On the first message of the client connection, enable keep-alive
            res.set_keep_alive_timeout(3);
        } else {
            // On second message, disable keep-alive and reset the msg_number counter.
            res.set_keep_alive_timeout(0);
            msg_number[&res] = 0;
        }

        // We'll echo the body back in the response.
        string body = req.get_body();
        if (body.empty()) {
            body = "no body to echo back";
        }
        cout << " Setting body to '" << body << "'" << endl;
        res.end(body);

        cout << "User-registered callback END" << endl;

    })) {
        return 1; // Failed to run server.
    }

    cout << "Server running at http://0.0.0.0:" << port << "/" << endl;
    return native::run();
}
