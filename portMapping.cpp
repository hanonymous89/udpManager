#include <windows.h>
#include <string>
#include <iostream>
#include <natupnp.h>
#include <objbase.h>
#include <oleauto.h>
#include <winsock.h>
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
int main(){
    CoInitialize(NULL);
    IUPnPNAT *nat;
    CoCreateInstance(CLSID_UPnPNAT,NULL,CLSCTX_ALL,IID_IUPnPNAT,(void**)&nat);
    IStaticPortMappingCollection *maps=NULL;
    nat->get_StaticPortMappingCollection(&maps);
    if(!maps){
        std::cout<<"failed get_StaticPortMappingCollection()";
        return system("pause");
    }
    constexpr int hostBufSize=63;
    char host[hostBufSize];
    wchar_t showIp;
    PHOSTENT phost;
    gethostname(host,hostBufSize);
    phost=gethostbyname(host);
    std::cout<<"yourip:"<<inet_ntoa(*(IN_ADDR*)phost->h_addr_list[0]);
    std::wstring ip;
    std::cout<<std::endl<<"ip:";
    std::wcin>>ip;
    int port;
    std::cout<<"port";
    std::cin>>port;
    BSTR local=SysAllocString(ip.c_str());
    BSTR proto=SysAllocString(L"UDP");
    IStaticPortMapping *map;
    BSTR name=SysAllocString(L"UDP_PortForward");
    maps->Add(port,proto,port,local,VARIANT_TRUE,name,&map);
    std::cout<<"wait close";
    system("pause");
    maps->Remove(port,proto);
    map->Release();
    maps->Release();
    nat->Release();
    CoUninitialize();
    return system("pause");
}