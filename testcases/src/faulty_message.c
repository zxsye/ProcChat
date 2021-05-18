#include "../../client.c"

#define DOMAIN "faulty_message"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);
    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);
    
    send(&colby, "Hi I'm Colby", Say, 0);
    send(&colby, "NEVER READ THIS", 99, 0);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, 0);
    receive(&colby);

}