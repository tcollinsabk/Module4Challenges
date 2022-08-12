// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
#include <enet/enet.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <thread>
#include <string>

//ENET STUFF
ENetAddress address;
ENetHost* server;
ENetHost* client;
ENetPeer* peer;
ENetEvent event;

bool CreateClient();
void ConnectHost();
void ProcessEvents();
void DisconnectServer();

char clientName[50];
std::thread* getMessage;

//WINDOWS STUFF
constexpr int SEND = 1;
constexpr int NAME = 2;
// Global variables
HWND txtBox;
HWND outbox;
HWND nameBox;
HWND secondWN;
// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Chatbot");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddControls(HWND hWind);
void CreateDialogBox(HINSTANCE inst);
void displayDialog(HWND hWind);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    CreateDialogBox(hInstance);

    if (enet_initialize() != 0)
    {
        //fprintf(stderr, "An error occurred while initializing ENet.\n");
        //std::cout << "An error occurred while initializing ENet." << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);
    secondWN = hWnd;
    //getMessage = new std::thread(ProcessEvents);
    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // Here your application is laid out.
        // For this introduction, we just print out "Hello, Windows desktop!"
        // in the top left corner.
        AddControls(hWnd);
        displayDialog(hWnd);
        // End application-specific layout section.

        EndPaint(hWnd, &ps);
        break;
    case WM_COMMAND:
        switch (wParam)
        {
        case SEND:
            char output[204];
            char ogText[5000];
            char msg[5000];
            strcpy_s(msg, clientName);
            strcat_s(msg, ": ");
            
            GetWindowTextA(outbox, ogText, 5000);
            GetWindowTextA(txtBox, output, 200);
            if (strcmp(output, "q") == 0)
            {
                DisconnectServer();
                return DefWindowProc(hWnd, WM_CLOSE, wParam, lParam);
            }
            strcat_s(msg, output);
            strcat_s(output, "\r\n");
            if ((!strcmp(output, "") == 0)&&(strcmp(output,"q")!=0))
            {
                strcat_s(ogText, clientName);
                strcat_s(ogText, ": ");
                strcat_s(ogText, output);
                SetWindowTextA(outbox, ogText);
            }
            SetWindowTextA(txtBox, "");
            std::string str = " ";
            /* Create a reliable packet of size 7 containing "packet\0" */
            strcat_s(msg, str.c_str());
            ENetPacket* packet = enet_packet_create(msg,
                strlen(msg) + 1,
                ENET_PACKET_FLAG_RELIABLE);
            /* Send the packet to the peer over channel id 0. */
            /* One could also broadcast the packet by         */
            /* enet_host_broadcast (host, 0, packet);         */
            enet_peer_send(peer, 0, packet);
            /* One could just use enet_host_service() instead. */
            enet_host_flush(client);
            break;
        }
        
        break;
    case WM_CLOSE:
       // DisconnectServer();
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        //getMessage->join();
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

void AddControls(HWND hWind)
{
    txtBox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL, 10, 400, 400, 50, hWind, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Send", WS_VISIBLE | WS_CHILD, 420, 400, 50, 50, hWind, (HMENU)SEND, NULL, NULL);
    outbox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL|ES_READONLY, 10, 10, 460, 350, hWind, NULL, NULL, NULL);
}

LRESULT CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        switch (wParam)
        {
        case NAME:
            CreateClient();
            DestroyWindow(hWnd);
            ConnectHost();
            getMessage = new std::thread(ProcessEvents);
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void CreateDialogBox(HINSTANCE inst)
{
    WNDCLASSW dialog = { 0 };
    dialog.hbrBackground = (HBRUSH)COLOR_WINDOW;
    dialog.hCursor = LoadCursor(NULL, IDC_CROSS);
    dialog.hInstance = inst;
    dialog.lpszClassName = L"EnterYourName";
    dialog.lpfnWndProc = DialogProc;

    RegisterClassW(&dialog);
}

void displayDialog(HWND hWind)
{
    HWND dia = CreateWindowW(L"EnterYourName", L"Enter your name", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 400, 400, 280, 200, hWind, NULL, NULL, NULL);

    nameBox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 55, 20, 150, 30, dia, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Enter", WS_VISIBLE | WS_CHILD, 80, 70, 100, 40, dia, (HMENU)NAME, NULL, NULL);
}

bool CreateClient()
{
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    GetWindowTextA(nameBox, clientName, 50);
    std::string str(clientName);
    str.append("'s chat box");
    SetWindowTextA(secondWN, str.c_str());
    //SetWindowTextA(outbox, clientName);

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
        //fprintf(stderr,
         //   "No available peers for initiating an ENet connection.\n");
        //std::cout << "Server was unavailable." << std::endl;
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        //SetWindowTextA(outbox, "Connection to localhost:1234 succeeded");
        //std::cout << "Connection to localhost:1234 succeeded." << std::endl;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
       // std::cout << "Connection to localhost:1234 failed." << std::endl;
    }
}

void ProcessEvents()
{
    char ogText[5000];
    int text;
    /* Wait up to 1000 milliseconds for an event. */
    while (1)
    {
        while (enet_host_service(client, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                //char ogText[5000];
                char data[204];
                strcpy_s(data, (char*)event.packet->data);
     
                GetWindowTextA(outbox, ogText, 5000);
                strcat_s(data, "\r\n");
                text = GetWindowTextLength(outbox);
                if ((!strcmp(data, "") == 0) && (strcmp(data, "q") != 0))
                {
                    strcat_s(ogText, data);
                    SetWindowTextA(outbox, ogText);
                }
                
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                //printf("%s disconnected.\n", event.peer->data);
                //char ogText[5000];

                GetWindowTextA(outbox, ogText, 5000);
                strcat_s(ogText, "\r\nPeer disconnected");
                SetWindowTextA(outbox, ogText);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }
}

void DisconnectServer()
{
    std::string msg = (std::string)clientName + " disconnected";
    ENetPacket* packet = enet_packet_create(msg.c_str(),
        strlen(msg.c_str()) + 1,
        ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush(client);
    Sleep(2000);
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
            //std::cout << "Disconnection succeeded." << std::endl;
            return;
        }
    }
    /* We've arrived here, so the disconnect attempt didn't */
    /* succeed yet.  Force the connection down.             */
    enet_peer_reset(peer);
}