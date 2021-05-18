#include "../../client.c"

#define DOMAIN "ping_pong"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby", Say, NON_TRM);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, NON_TRM);
    receive(&colby);

    sleep(15);
    receive(&colby);
    receive(&zara);

    send(&colby, " ", Pong, 0);
    send(&zara, " ", Pong, 0);
}