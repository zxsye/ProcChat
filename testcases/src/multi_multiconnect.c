#include "../../client.c"

#define DOMAIN "multi_multiconnect_1"
#define DOMAIN_2 "multi_multiconnect_2"

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

    Pipeline amanda;
    init_client_pipeline(&amanda, DOMAIN_2, "Amanda");
    connect_to_server(&amanda);

    Pipeline bovey;
    init_client_pipeline(&bovey, DOMAIN_2, "Bovey");
    connect_to_server(&bovey);

    Pipeline caleb;
    init_client_pipeline(&caleb, DOMAIN_2, "Caleb");
    connect_to_server(&caleb);
}