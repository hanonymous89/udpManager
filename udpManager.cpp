#include <windows.h>
#include <memory>
#include <string>
#include <iostream>
class WsaManager{
    private:
    WsaManager(){
        WSAData wsaData;
        WSAStartup(MAKEWORD(2,0),&wsaData);
    };
    virtual ~WsaManager(){
        WSACleanup();
    };
    public:
    WsaManager(const WsaManager&)=delete;
    WsaManager& operator=(const WsaManager&)=delete;
    WsaManager(WsaManager&&)=delete;
    WsaManager& operator=(WsaManager&&)=delete;
    static WsaManager& getInstance(){
        static WsaManager wsaManager;
        return wsaManager;
    }
};
class UdpInterface{
    protected:
    static int count;
    struct sockaddr_in addr;
    int sock;
    public:
    UdpInterface(const decltype(ADDR_ANY) ip,const int port){
        WsaManager::getInstance();
        sock=socket(AF_INET,SOCK_DGRAM,0);
        if(sock==INVALID_SOCKET){
            return;
        }
        addr.sin_family=AF_INET;
        addr.sin_port=htons(port);
        addr.sin_addr.S_un.S_addr=ip;
    }
    virtual ~UdpInterface(){
        if(sock==INVALID_SOCKET)return;
        closesocket(sock);
        sock=INVALID_SOCKET;
    }
};
class UdpSend:private UdpInterface{
    public:
    UdpSend(const std::string ip,const int port):UdpInterface(inet_addr(ip.c_str()),port){
    }
    auto &send(const std::string data){
        sendto(sock,data.c_str(),data.size()+1,0,(struct sockaddr*)&addr,sizeof(addr));
        return *this;
    }
};
class UdpServer:private UdpInterface{
    public:
    UdpServer(int port):UdpInterface(ADDR_ANY,port){
        bind(sock,(struct sockaddr*)&addr,sizeof(addr));
    }
    std::string recv(int bufSize){
        if(sock==INVALID_SOCKET)return "";
        std::unique_ptr<char> buf(new char[bufSize]);
        bufSize=::recv(sock,buf.get(),bufSize,0);
        return std::string(buf.get(),bufSize);
    }
};
int main(){
    std::cout<<UdpServer(46490).recv(100);
    return system("pause");
}