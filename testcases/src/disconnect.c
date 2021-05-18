#include "../../client.c"

#define DOMAIN "disconnect"

int main() {
    // DOMAIN 1
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby", Say, 0);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, 0);
    receive(&colby);

    send(&colby, " ", Disconnect, 0);
    send(&zara, " ", Disconnect, 0);

    // CHECK THAT GEVENT IS STILL RUNNING
    Pipeline andrew;
    init_client_pipeline(&andrew, DOMAIN, "Andrew");
    connect_to_server(&andrew)
;
    Pipeline helen;
    init_client_pipeline(&helen, DOMAIN, "Helen");
    connect_to_server(&helen);

    send(&andrew, "Hi I'm Andrew", Say, 0);
    receive(&helen);

    send(&helen, "Hi my name's Helen, nice to meet you", Say, 0);
    receive(&andrew);


}