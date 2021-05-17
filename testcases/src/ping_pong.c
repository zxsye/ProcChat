#include "../../client.c"

#define DOMAIN "ping_pong"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    sleep(16);
}