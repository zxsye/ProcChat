#include "../../client.c"

#define DOMAIN "one_connect_again"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Zeus");
    connect_to_server(&colby);
    
    // TRY TO CONNECT AGAIN
    connect_to_server(&colby);  

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby", Say, NON_TRM);
    receive(&zara);


}