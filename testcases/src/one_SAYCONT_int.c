#include "../../client.c"

#define DOMAIN "one_SAYCONT_int"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    Pipeline joshua;
    init_client_pipeline(&joshua, DOMAIN, "Joshua");
    connect_to_server(&joshua);

    // ====== SENDING MESSAGES ======
    send(&joshua, "Hey guys!...", Saycont, NON_TRM);
    receive(&colby);
    receive(&zara);

    send(&colby, "Hey Joshua, welcome to the domain :)", Say, 0);
    receive(&joshua);
    receive(&zara);
    
    send(&joshua, ".. Rude, I'm leaving. Next time don't interrupt", Saycont, TRM);
    receive(&colby);
    receive(&zara);
}