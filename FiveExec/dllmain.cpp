#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <random>
#include "ResourceManager.h"
#include "ResourceCache.h"
#include "ResourceMetaDataComponent.h"
#include "VFSManager.h"
#include <regex>

using namespace fx;
using namespace vfs;

auto resourceManager = Instance<ResourceManager>::Get();

bool repla(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::string random_string()
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(str.begin(), str.end(), generator);

    return str.substr(0, 32);    // assumes 32 < number of characters in str         
}

std::vector<Resource*> GetAllResources()
{
    std::vector<Resource*> localresources;

    resourceManager->ForAllResources([&](fwRefContainer<Resource> resource)
        {
            localresources.push_back(resource.GetRef());
        });

    return localresources;
}

static int selected = NULL;


void ExecuteNewResource(std::string code) {

    _mkdir("C:\\ESD");
    _mkdir("C:\\ESD\\dump");

    std::string resourceName = random_string();
    std::string fileName = random_string();
    std::string buffer;


    std::string path;
    path += "C:\\ESD\\dump\\";
    path += fileName;
    path += ".lua";

    std::ifstream infile(code);
    //std::ifstream infile("C:\\Menus\\" + code);
    if (infile)
    {
        std::istreambuf_iterator<char> ifit( infile );
        std::ofstream outfile( path );
        std::ostreambuf_iterator<char> ofit( outfile );
        if (outfile)
        {
            outfile << "Citizen.CreateThread(function()\n";
            copy(ifit, std::istreambuf_iterator<char>(), ofit);
            outfile << "end)";
            outfile.close();
        }
        else
        {
            //cerr << "Could not open output file" << "\n";
        }
        infile.close();
    }

    std::ofstream script;
    script.open(path);
    script << buffer;
    script.close();

    std::ofstream metadata;
    metadata.open("C:\\ESD\\dump\\fxmanifest.lua");
    metadata << "fx_version 'adamant'\n";
    metadata << "game 'gta5'\n";
    metadata << "client_scripts {\n";
    metadata << "   '";
    metadata << fileName;
    metadata << ".lua'\n";
    metadata << "}";
    metadata.close();

    auto customResource = resourceManager->CreateResource(resourceName, nullptr);
    customResource->SetComponent(new ResourceCacheEntryList{});
    customResource->LoadFrom("C:\\ESD\\dump");
    customResource->Start();
    Sleep(1000);
    customResource->Stop();
    customResource->Start();

    remove(path.c_str());
    remove("C:\\ESD\\dump\\fxmanifest.lua");
    system("cls");
}


void StopResource(std::string ResourceName)
{
    if (ResourceName == "_cfx_internal")
    {
        return;
    }
    resourceManager->GetResource(ResourceName)->Stop();
}


void init(void) {

    _mkdir("C:\\ESD");
    _mkdir("C:\\ESD\\exec");

    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    SetConsoleTitleA("FiveM");


    system("cls");

    while (true) {
        system("cls");
        system("color F");
        std::string oneor2;
        printf("[1] Executor\n");
        printf("[2] Resource Stopper\n");
        printf("[3] Start Resources\n");
        printf("[4] Clear\n");
        std::getline(std::cin, oneor2);

        if (oneor2 == "1")
        {
            system("cls");
            system("color F");
            printf("Enter a File.lua FX-> [C:\\Menus\\falloutmenu.lua]\n");

            std::string command;
            std::getline(std::cin, command);

            ExecuteNewResource(command.c_str());
            system("cls");
        }
        else if (oneor2 == "2")
        {
            system("cls");
            printf("Enter a resource to stop\n");
            std::string Resource;
            std::getline(std::cin, Resource);
            resourceManager->GetResource(Resource)->Stop();
        }
        else if (oneor2 == "3")
        {
            system("cls");
            printf("Enter a resource to start!\n");
            std::string Resource;
            std::getline(std::cin, Resource);
            resourceManager->GetResource(Resource)->Start();
        }
        else if (oneor2 == "4")
        {
            system("cls");
        }


    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

