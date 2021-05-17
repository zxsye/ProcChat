#include "../../client.c"

#define DOMAIN "one_connect"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Zeus");
    connect_to_server(&colby);
}