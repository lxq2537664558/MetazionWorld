#include "ServerApp.hpp"

#include <Metazion/Net/Network.hpp>

#include "Net/Sockets.hpp"

USING_NAMESPACE_MZ_NET

ServerApp* g_serverApp = nullptr;

ServerApp::ServerApp() {}

ServerApp::~ServerApp() {}

void ServerApp::Initialize() {
    ::printf("GatewayServer start.\n");

    Network::Startup();

    m_serverManager.Initialize();

    m_socketServer.Initialize(1024, 8);

    ::memset(m_sockets, 0, sizeof(m_sockets));
    m_socketArray.Attach(m_sockets, 1024, 0);

    const auto gatewayInfo = m_serverManager.GetGatewayInfo(1);
    ASSERT_TRUE(!NS_MZ::IsNull(gatewayInfo));

    Host hostCG;
    hostCG.FromAddress(gatewayInfo->m_publicAddress);

    ListenSocketCG* listenSocketCG = new ListenSocketCG();
    listenSocketCG->Retain();
    listenSocketCG->SetLocalHost(hostCG);
    listenSocketCG->Listen(100);
    m_socketServer.Attach(listenSocketCG);

    Host hostWG;
    hostWG.FromAddress(gatewayInfo->m_privateAddress);

    ListenSocketWG* listenSocketWG = new ListenSocketWG();
    listenSocketWG->Retain();
    listenSocketWG->SetLocalHost(hostWG);
    listenSocketWG->Listen(100);
    m_socketServer.Attach(listenSocketWG);
}

void ServerApp::Finalize() {
    m_socketArray.Detach();

    m_socketServer.Finalize();

    m_serverManager.Finalize();

    Network::Cleanup();

    ::printf("GatewayServer stop.\n");
}

void ServerApp::Tick() {
    const auto size = m_socketServer.LockSockets(m_socketFilter, m_socketArray);
    for (auto i = 0; i < size; ++i) {
        auto socket = m_socketArray[i];
        ASSERT_TRUE(!NS_MZ::IsNull(socket));
        socket->Dispatch();
    }
    m_socketServer.UnlockSockets(m_socketArray);
}