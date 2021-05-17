#include "../../client.c"

#define DOMAIN "one_SAY"

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

    Pipeline joshua;
    init_client_pipeline(&joshua, DOMAIN, "Joshua");
    connect_to_server(&joshua);

    send(&joshua, "Hey guys!", Say, NON_TRM);
    receive(&colby);
    receive(&zara);

    send(&colby, "Hey Joshua, welcome to the domain :)", Say, NON_TRM);
    receive(&joshua);
    receive(&zara);
    
    send(&zara, "Thanks for joining us", Say, NON_TRM);
    receive(&colby);
    receive(&joshua);
}