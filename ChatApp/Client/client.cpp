#include <enet/enet.h>
#include <iostream>
#include <thread>
#include <Windows.h>
#include <string>
#include <conio.h>

ENetAddress address;
ENetHost* server;
ENetHost* client;
ENetPeer* peer;
ENetEvent event;

std::string clientName;
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
bool enable = SetConsoleMode(console, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
bool help = SetConsoleMode(console, ENABLE_PROCESSED_OUTPUT);

bool CreateClient()
{
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    std::cout << "What is your name? " << std::endl;
    std::cin >> clientName;

    return client != nullptr;
}

void ConnectHost()
{
    enet_address_set_host(&address, "localhost");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        std::cout << "Server was unavailable." << std::endl;
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connection to localhost:1234 succeeded." << std::endl;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        std::cout << "Connection to localhost:1234 failed." << std::endl;
    }
}

void ProcessEvents()
{
    /* Wait up to 1000 milliseconds for an event. */
    while (1)
    {
        while (enet_host_service(client, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                system(" ");
                SetConsoleTextAttribute(console, 5);
                std::cout << "\x1b[s";
                std::cout << "\x1b" << 1 << "L";
                std::cout << "\x1b[G";
                std::cout << event.packet->data << std::endl;
                std::cout << "\x1b[u";
                std::cout << "\x1b[" << 100 << "D";
                std::cout << "\x1b[" << 1 << "B";
                SetConsoleTextAttribute(console, 7);
                std::cout << clientName << ": ";
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }
}

void DisconnectServer()
{
    enet_peer_disconnect(peer, 0);
    /* Allow up to 3 seconds for the disconnect to succeed
     * and drop any packets received packets.
     */
    while (enet_host_service(client, &event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Disconnection succeeded." << std::endl;
            return;
        }
    }
    /* We've arrived here, so the disconnect attempt didn't */
    /* succeed yet.  Force the connection down.             */
    enet_peer_reset(peer);
}

int main()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    if (!CreateClient())
    {
        std::cout << "an error occurred while trying to create a client" << std::endl;
        exit(EXIT_FAILURE);
    }

    ConnectHost();
    std::thread* handEvents = new std::thread(ProcessEvents);
    std::string input;
    while (input.compare("q") != 0)
    {
        std::getline(std::cin, input);
        std::cout << clientName << ": ";

        if (input.compare("q") != 0 && !input.empty())
        {
            std::string msg = clientName + ':' + ' ' + input;
            /* Create a reliable packet of size 7 containing "packet\0" */
            ENetPacket* packet = enet_packet_create(msg.c_str(),
                strlen(msg.c_str()) + 1,
                ENET_PACKET_FLAG_RELIABLE);
            /* Send the packet to the peer over channel id 0. */
            /* One could also broadcast the packet by         */
            /* enet_host_broadcast (host, 0, packet);         */
            enet_peer_send(peer, 0, packet);
            /* One could just use enet_host_service() instead. */
            enet_host_flush(client);

        }
    }

    if (input.compare("q") == 0)
    {
        DisconnectServer();
    }

    handEvents->join();
    return EXIT_SUCCESS;
}