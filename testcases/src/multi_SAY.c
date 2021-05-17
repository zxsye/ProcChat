#include "../../client.c"

#define DOMAIN "multi_SAY"
#define DOMAIN2 "multi_SAY_2"

int main() {
    // Connect to server
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

    // DOMAIN2 SAYRECV
    Pipeline amanda;
    init_client_pipeline(&amanda, DOMAIN2, "Amanda");
    connect_to_server(&amanda);
    Pipeline bovey;
    init_client_pipeline(&bovey, DOMAIN2, "Bovey");
    connect_to_server(&bovey);

    send(&amanda, "Bonjour, je m'appelle Amanda", Say, 0);
    receive(&bovey);

    send(&bovey, "Sorry I don't speak German", Say, 0);
    receive(&amanda);

    // Back to DOMAIN SAYRECV
    Pipeline joshua;
    init_client_pipeline(&joshua, DOMAIN, "Joshua");
    connect_to_server(&joshua);

    send(&joshua, "Hey guys!", Say, 0);
    receive(&colby);
    receive(&zara);

    send(&colby, "Hey Joshua, welcome to the domain :)", Say, 0);
    receive(&joshua);
    receive(&zara);
    
    send(&zara, "Thanks for joining us", Say, 0);
    receive(&joshua);
    receive(&colby);
}