#include "../../client.c"

#define DOMAIN "multi_SAYCONT"
#define DOMAIN2 "multi_SAYCONT_2"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);
    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby and I have something to say...", Saycont, NON_TRM);
    receive(&zara);

    send(&colby, "...hold on one sec...", Saycont, NON_TRM);
    receive(&zara);

    send(&colby, "...I am very cool", Saycont, TRM);
    receive(&zara);
    

    // DOMAIN2 SAYRECV
    Pipeline amanda;
    init_client_pipeline(&amanda, DOMAIN2, "Amanda");
    connect_to_server(&amanda);
    Pipeline bovey;
    init_client_pipeline(&bovey, DOMAIN2, "Bovey");
    connect_to_server(&bovey);

    send(&amanda, "Bonjour, je m'appelle Amanda...", Saycont, NON_TRM);
    receive(&bovey);
    send(&amanda, ".. that means my name is Amanda...", Saycont, NON_TRM);
    receive(&bovey);
    send(&amanda, ".. what's your name?", Saycont, TRM);
    receive(&bovey);

    // DOMAIN SAYRECV
    Pipeline joshua;
    init_client_pipeline(&joshua, DOMAIN, "Joshua");
    connect_to_server(&joshua);

    send(&joshua, "Hey guys!...", Saycont, NON_TRM);
    receive(&colby);
    receive(&zara);
    send(&joshua, "... I'm joshua...", Saycont, NON_TRM);
    receive(&colby);
    receive(&zara);
    send(&joshua, "... can I join in?", Saycont, TRM);
    receive(&colby);
    receive(&zara);

}