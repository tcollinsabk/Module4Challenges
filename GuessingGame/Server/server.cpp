#include <enet/enet.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

struct Client
{
    ENetAddress addr;
};
ENetAddress address;
ENetHost* server;
ENetHost* client;
ENetEvent event;
ENetPeer* peer;

std::string serverName;
char* clientName;

int number;

std::vector<ENetPeer> clients;
bool CreateServer()
{
    /* Bind the server to the default localhost.     */
   /* A specific host address can be specified by   */
   /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        2      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);
    //std::cout << "What is your name? " << std::endl;
    //std::cin >> serverName;

    std::cout << "Server created" << std::endl;

    return server != nullptr;
}

void ProcessEvents()
{
    std::string msg;
    int guess;

    while (1)
    {
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                event.peer->data = (void*)("Client information");
                //send the welcome message
                if (server->connectedPeers >= 2)
                {
                    msg = "Welcome to the guessing game. Please guess a number.";
                    ENetPacket* packet = enet_packet_create(msg.c_str(),
                        strlen(msg.c_str()) + 1,
                        ENET_PACKET_FLAG_RELIABLE);
                    enet_host_broadcast(server, 0, packet);
                }
                else
                {
                    msg = "Welcome to the guessing game. Please wait for peers to connect...";
                    ENetPacket* packet = enet_packet_create(msg.c_str(),
                        strlen(msg.c_str()) + 1,
                        ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                //for (int i = 0; i < server->connectedPeers; i++)
                //{
                //    if (&server->peers[i] != event.peer)
                //    {
                //        enet_peer_send(&server->peers[i], 0, event.packet);
                //        enet_host_flush(server);
                //    }
                //    //enet_host_broadcast(server, 0, event.packet);
                //    //enet_host_flush(server);
                //}
         
                guess = ((int)(*event.packet->data)) - 48;
                if (guess == number)
                {
                    msg = "Congrats! you were correct! Press q to quit.";
                    ENetPacket* packet = enet_packet_create(msg.c_str(),
                        strlen(msg.c_str()) + 1,
                        ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);

                    /*std::string* ptr = static_cast<std::string*>(event.packet->userData);
                    std::string name = *ptr;
                    delete ptr;*/
                 
                    msg = "Your opponent guessed the correct answer. It was ";
                    msg.append(std::to_string(number));
                    msg.append(". Press q to quit");
                    
                    packet = enet_packet_create(msg.c_str(),
                        strlen(msg.c_str()) + 1,
                        ENET_PACKET_FLAG_RELIABLE);

                    for (int i = 0; i < server->connectedPeers; i++)
                    {
                        if (&server->peers[i] != event.peer)
                        {
                            enet_peer_send(&server->peers[i], 0, packet);
                            enet_host_flush(server);
                        }

                        enet_peer_disconnect(&server->peers[i], 0);
                        enet_peer_reset(&server->peers[i]);
       
                    }
                }
                else
                {
                    msg = "Sorry, Incorrect";
                    ENetPacket* packet = enet_packet_create(msg.c_str(),
                        strlen(msg.c_str()) + 1,
                        ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);

                }
                //std::cout << event.packet->data << std::endl;
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                //printf("%s disconnected.\n", event.peer->data);
                /* Reset the peer's client information. */
                std::cout << "\nPeer Disconnected" << std::endl;
                event.peer->data = NULL;
            }
        }
    }
}

void ConnectClient()
{
    while (enet_host_service(server, &event, 5000) > 0)
    {
        switch (event.type)
        {

        }
    }
}


int main()
{
    srand(time(NULL));
    number = rand()%7 + 1;

    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    if (!CreateServer())
    {
        std::cout << "an error occurred while trying to create a server" << std::endl;
        exit(EXIT_FAILURE);
    }

    //TODO: add wait for client connection
    //ProcessEvents();

    //ConnectClient();
    /*for (auto client : clients)
    {
        std::cout << client.address.host << std::endl;
        std::cout << client.address.port << std::endl;
    }*/
    //std::thread* handleEvents = new std::thread(ProcessEvents);
    //handleEvents->join();

    //std::string input = "";

    //while (input.compare("q") != 0)
    //{
    //    std::cout << serverName << ": ";
    //    std::cin >> input;

    //    if (input.compare("q") != 0)
    //    {
    //        std::string msg = serverName + ':'+' ' + input;
    //        /* Create a reliable packet of size 7 containing "packet\0" */
    //        ENetPacket* packet = enet_packet_create(msg.c_str(),
    //            strlen(msg.c_str()) + 1,
    //            ENET_PACKET_FLAG_RELIABLE);
    //        /* Send the packet to the peer over channel id 0. */
    //        /* One could also broadcast the packet by         */
    //        enet_host_broadcast (server, 0, packet); 
    //        //enet_peer_send(peer, 0, packet);
    //        /* One could just use enet_host_service() instead. */
    //        enet_host_flush(server);

    //    }
    //}

    ProcessEvents();
    return EXIT_SUCCESS;
}