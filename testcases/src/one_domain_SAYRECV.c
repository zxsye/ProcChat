#include "../../client.c"

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, "Rutherford", "Colby");
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, "Rutherford", "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby");
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you");
    receive(&colby);

    Pipeline joshua;
    init_client_pipeline(&joshua, "Rutherford", "Joshua");
    connect_to_server(&joshua);

    send(&joshua, "Hey guys!");
    receive(&colby);
    receive(&zara);

    send(&colby, "Hey Joshua, welcome to the domain :)");
    receive(&joshua);
    receive(&zara);
    
    send(&zara, "Thanks for joining us");
    receive(&colby);
    receive(&joshua);
}