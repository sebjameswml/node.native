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
#include <native/native.h>

using namespace native::http;
using namespace std;

int main() {
    http server;
    int port = 8080;

    std::map<native::http::response*, int> stage; // For response id first, set the stage to second.

    if(!server.listen("0.0.0.0", port, [=,&stage](request& req, response& res) {

        printf ("webserver - user-registered callback START.\n");

        // Using the memory address of response to identify
        // the connection - for each response in a kept-alive
        // connection, the response object is re-used, hence
        // its memory address will not change.
        try {
            stage[&res]++;
        } catch (std::out_of_range& e) {
            //stage.emplace(std::make_pair(&res, static_cast<int>(0)));
            stage[&res] = 0;
        }
        printf ("webserver -- stage: %d for response ID 0x%x\n", stage.at(&res), &res);

        string body = req.get_body(); // Now you can write a custom handler for the body content.

        res.set_header("Content-Type", "text/html");
        res.set_status(200);

        if (req.get_header("Expect") == "100-continue") {
            res.set_send_continue_first(true);
        }

        if (stage[&res] > 1) {
            res.set_keep_alive_timeout (0);
            stage[&res] = 0;
        } else {
            res.set_keep_alive_timeout (30); // or 30 for keep alive == true.
        }

        // We're echoing back the body of the request:
        res.end(body);

        printf ("webserver - user-registered callback END\n");
    })) {
        return 1; // Failed to run server.
    }

    cout << "Server running at http://0.0.0.0:" << port << "/" << endl;
    return native::run();
}
