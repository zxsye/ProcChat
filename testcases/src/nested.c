#include "../../client.c"

#define DOMAIN "nested"
#define DOMAIN_NEST "nested/HAHA"

int main() {
    // DOMAIN 1
    Pipeline colby;
    init_client_pipeline(&colby, DOMAIN, "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, DOMAIN, "Zara");
    connect_to_server(&zara);

    Pipeline jack;
    init_client_pipeline(&jack, DOMAIN_NEST, "Jack");
    connect_to_server(&jack);

    Pipeline peter;
    init_client_pipeline(&peter, DOMAIN_NEST, "Peter");
    connect_to_server(&peter);

    // CHECK MAIN DOMAIN
    send(&colby, "Hi I'm Colby", Say, 0);
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you", Say, 0);
    receive(&colby);

    // NEST COMMUNICATION
    send(&jack, "Hi I'm Jack", Say, 0);
    receive(&peter);

    send(&peter, "Hi my name's Peter, nice to meet you", Say, 0);
    receive(&jack);

    // CHECK MESSAGE DOES NOT LEAK OUT
    send(&colby, "Hi I'm Colby xoxo", Say, 0);
    receive(&zara);

    send(&zara, "Hi my name's Zara, hehe xd", Say, 0);
    receive(&colby);

}