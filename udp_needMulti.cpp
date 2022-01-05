#include <windows.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>
#include <thread>
#include <fstream>
#include <algorithm>
namespace h{
    constexpr int udpMaxSize=65535;
    constexpr int usePort=50000;
    inline auto split(std::string str,const std::string cut) noexcept(false){
        std::vector<std::string> data;
        for(auto pos=str.find(cut);pos!=std::string::npos;pos=str.find(cut)){
            data.push_back(str.substr(0,pos));
            str=str.substr(pos+cut.size());
        }
        return data;
    }
    class File{
       private:
       std::string name,
                   content;
       public:
       inline File(const std::string name)noexcept(true):name(name){
       }
       inline auto &setName(const std::string name)noexcept(true){
           this->name=name;
           return *this;
       }
       inline auto &getContent()const noexcept(true) {
           return content;
       }
       inline File &read() noexcept(false){
           std::fstream file(name);
           if(file.fail())return *this;
           content=std::string(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
           return *this;
       }
       inline auto write(const std::string str,const bool reset=false) const noexcept(false){
           std::ofstream file(name,reset?std::ios_base::trunc:std::ios_base::app);
           file<<str;
           file.close();
           return *this;
       }
    };
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
        SOCKADDR_IN addr;
        SOCKET sock;
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
        std::string recvfrom(){
            if(sock==INVALID_SOCKET)return "";
            std::unique_ptr<char> buf(new char[udpMaxSize]);
            SOCKADDR_IN address;
            int addlen=sizeof(address);
            ::recvfrom(sock,buf.get(),udpMaxSize,0,(struct sockaddr*)&address,&addlen);
            return std::string(inet_ntoa(address.sin_addr))+"/"+std::to_string(ntohs(address.sin_port))+":"+std::string(buf.get());
        }
        template <class T>
        auto recvfrom(T func){
            if(sock==INVALID_SOCKET){
                return func("127.0.0.1","");    
            }
            std::unique_ptr<char> buf(new char[udpMaxSize]);
            struct sockaddr_in address;
            int addlen=sizeof(address);
            ::recvfrom(sock,buf.get(),udpMaxSize,0,(struct sockaddr*)&address,&addlen);
            return func(inet_ntoa(address.sin_addr),std::string(buf.get()));
        }
        std::string recv(){
            if(sock==INVALID_SOCKET)return "";
            std::unique_ptr<char> buf(new char[udpMaxSize]);
            if(::recv(sock,buf.get(),udpMaxSize,0)==-1)return "";
            return std::string(buf.get());
        }
    };
};
int main(){
    h::UdpServer server(h::usePort);
    h::File file("users.ip");//portは50000こてい
    std::vector<std::string> ips=h::split(file.read().getContent(),"\n");
    while(true){           
        server.blocking(true).recvfrom([&](std::string ip,std::string content){
            for(auto &ip:ips){
                h::UdpSend(ip,h::usePort).send(content);
            }
            ips.push_back(ip);
            ips.erase(std::unique(ips.begin(),ips.end()),ips.end());
            if(content=="end"){
                std::string data;
                for(auto &ip:ips){
                    data+=ip+"\n";
                }
                file.write(data,true);
                std::exit(0);
            }
    });
    }
    return 0;
}//中央集権サーバープログラム