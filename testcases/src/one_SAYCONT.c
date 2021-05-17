#include "../../client.c"

#define DOMAIN "one_SAYCONT"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);


    // ====== SENDING MESSAGES ======
    send(&colby, "Hi I'm Colby...", Saycont, NON_TRM);
    receive(&zara);
    
    send(&colby, "... sorry I stuttered", Saycont, TRM);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, 0);
    receive(&colby);
}