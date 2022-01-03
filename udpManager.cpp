#include <windows.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>
#include <thread>
namespace h{
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
        UdpInterface(const decltype(INADDR_ANY) ip,const int port){
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
            if(sock==INVALID_SOCKET)return *this;
            sendto(sock,data.c_str(),data.size()+1,0,(struct sockaddr*)&addr,sizeof(addr));
            return *this;
        }
    };
    class UdpServer:private UdpInterface{
        public:
        UdpServer(int port):UdpInterface(INADDR_ANY,port){
            if(sock==INVALID_SOCKET)return;
            bind(sock,(struct sockaddr*)&addr,sizeof(addr));
        }
        auto &blocking(bool is_Block){
            u_long block=!is_Block;
            ioctlsocket(sock,FIONBIO,&block);
            return *this;
        }
        std::string recvfrom(const int bufSize){
            if(sock==INVALID_SOCKET)return "";
            std::unique_ptr<char> buf(new char[bufSize]);
            struct sockaddr_in address;
            int addlen=sizeof(address);
            ::recvfrom(sock,buf.get(),bufSize,0,(struct sockaddr*)&address,&addlen);
            return std::string(inet_ntoa(address.sin_addr))+"/"+std::to_string(ntohs(address.sin_port))+":"+std::string(buf.get());
        }
        std::string recv(const int bufSize){
            if(sock==INVALID_SOCKET)return "";
            std::unique_ptr<char> buf(new char[bufSize]);
            if(::recv(sock,buf.get(),bufSize,0)==-1)return "";
            return std::string(buf.get());
        }
    };
};
int main(){
    h::UdpServer server(50000);
    std::string input;
    std::cout<<"ip(port:50000):";
    std::cin>>input;
    h::UdpSend client(input.c_str(),50000);
    auto func=[&]{
        while(input!="end"){
            std::cout<<server.recvfrom(65535)<<std::endl;
        }
    };
    std::thread thread(func);  
    while(getline(std::cin,input)){
        client.send(input);
        if(input=="end")break;
    }
    thread.join();
    return 0;
}