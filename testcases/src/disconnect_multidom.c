#include "../../client.c"

#define DOMAIN "disconnect_multidom"
#define DOMAIN_2 "disconnect_multidom_2"

int main() {
    // DOMAIN 1
    Pipeline dono;
    init_client_pipeline(&dono, DOMAIN, "Dono");
    connect_to_server(&dono);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    send(&dono, "Hi I'm Dono", Say, 0);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, 0);
    receive(&dono);

    send(&dono, " ", Disconnect, 0);
    send(&zara, " ", Disconnect, 0);

    // DOMAIN 2
    Pipeline amanda;
    init_client_pipeline(&amanda, DOMAIN_2, "Amanda");
    connect_to_server(&amanda);

    Pipeline bovey;
    init_client_pipeline(&bovey, DOMAIN_2, "Bovey");
    connect_to_server(&bovey);

    send(&amanda, "Bonjour, je m'appelle Amanda...", Saycont, NON_TRM);
    receive(&bovey);
    send(&amanda, ".. that means my name is Amanda...", Saycont, NON_TRM);
    receive(&bovey);
    send(&amanda, ".. what's your name?", Saycont, TRM);
    receive(&bovey);

    send(&amanda, " ", Disconnect, 0);
    send(&bovey, " ", Disconnect, 0);



}