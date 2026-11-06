// Exercise the built DLL through this DC++ checkout's public C ABI.
// All chat and player endpoints are local test doubles; no network messages are sent.
#include <windows.h>
#include <dcpp/plugins/PluginDefs.h>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

static void check(bool value, const char* what) {
    if(!value) throw std::runtime_error(what);
}
struct Subscription { DCHOOK callback; void* common; };
static std::map<std::string, Subscription> subscriptions;
static std::map<std::string, std::string> settings;
static std::string sent, local, notice;
static Bool action;
static int hubSends, pmSends;
static DCCommandFunc menuCommand;
static bool denyUtils = false;
static DCHooks hooks{};
static DCConfig config{};
static DCHub hubs{};
static DCLog logger{};
static DCUI ui{};
static DCUtils utils{};

static DCInterfacePtr DCAPI query(const char* name, uint32_t version) {
    struct Interface { const char* name; DCInterfacePtr ptr; };
    Interface interfaces[] = {
        {DCINTF_HOOKS, reinterpret_cast<DCInterfacePtr>(&hooks)},
        {DCINTF_CONFIG, reinterpret_cast<DCInterfacePtr>(&config)},
        {DCINTF_DCPP_HUBS, reinterpret_cast<DCInterfacePtr>(&hubs)},
        {DCINTF_LOGGING, reinterpret_cast<DCInterfacePtr>(&logger)},
        {DCINTF_DCPP_UI, reinterpret_cast<DCInterfacePtr>(&ui)},
        {DCINTF_DCPP_UTILS, reinterpret_cast<DCInterfacePtr>(&utils)}
    };
    if(denyUtils && !std::strcmp(name, DCINTF_DCPP_UTILS)) return nullptr;
    for(const auto& i : interfaces)
        if(!std::strcmp(name, i.name) && version == i.ptr->apiVersion) return i.ptr;
    return nullptr;
}
static subsHandle DCAPI bind(const char* name, DCHOOK callback, void* common) {
    auto& sub = subscriptions[name];
    sub = {callback, common};
    return &sub;
}
static size_t DCAPI releaseHook(subsHandle handle) {
    for(auto i = subscriptions.begin(); i != subscriptions.end(); ++i) {
        if(&i->second == handle) { subscriptions.erase(i); return 0; }
    }
    return 0;
}
static ConfigStrPtr stringValue(const std::string& value) {
    auto str = new char[value.size() + 1];
    std::memcpy(str, value.c_str(), value.size() + 1);
    return new ConfigStr{CFG_TYPE_STRING, str};
}
static ConfigStrPtr DCAPI installPath(const char*) { return stringValue(".\\"); }
static void DCAPI setConfig(const char*, const char* name, ConfigValuePtr value) {
    check(value->type == CFG_TYPE_STRING, "unexpected config type");
    settings[name] = reinterpret_cast<ConfigStrPtr>(value)->value;
}
static ConfigValuePtr DCAPI getConfig(const char*, const char* name, ConfigType) {
    return reinterpret_cast<ConfigValuePtr>(stringValue(settings[name]));
}
static void DCAPI releaseConfig(ConfigValuePtr value) {
    auto str = reinterpret_cast<ConfigStrPtr>(value);
    delete[] str->value;
    delete str;
}
// Match PluginApiImpl's v1 length/capacity contract, including exact fits.
static size_t DCAPI toWide(wchar_t* dst, const char* src, size_t capacity) {
    int length = static_cast<int>(std::strlen(src));
    int needed = MultiByteToWideChar(CP_UTF8, 0, src, length, nullptr, 0);
    if(dst && capacity >= static_cast<size_t>(needed)) {
        MultiByteToWideChar(CP_UTF8, 0, src, length, dst, needed);
        if(capacity > static_cast<size_t>(needed)) dst[needed] = 0;
    }
    return needed;
}
static size_t DCAPI toUtf8(char* dst, const wchar_t* src, size_t capacity) {
    int length = static_cast<int>(std::wcslen(src));
    int needed = WideCharToMultiByte(CP_UTF8, 0, src, length, nullptr, 0, nullptr, nullptr);
    if(dst && capacity >= static_cast<size_t>(needed)) {
        WideCharToMultiByte(CP_UTF8, 0, src, length, dst, needed, nullptr, nullptr);
        if(capacity > static_cast<size_t>(needed)) dst[needed] = 0;
    }
    return needed;
}
static void DCAPI sendHub(HubDataPtr, const char* message, Bool thirdPerson) {
    ++hubSends; sent = message; action = thirdPerson;
}
static Bool DCAPI sendPM(UserDataPtr, const char* message, Bool thirdPerson) {
    ++pmSends; sent = message; action = thirdPerson; return True;
}
static void DCAPI localMessage(HubDataPtr, const char* message, MsgType) { local = message; }
static void DCAPI notify(const char*, const char* message) { notice = message; }
static void DCAPI addCommand(const char*, const char*, DCCommandFunc command, const char*) { menuCommand = command; }
static void DCAPI removeCommand(const char*, const char*) { menuCommand = nullptr; }
static void DCAPI logMessage(const char*) {}
static const char* DCAPI hostName() { return "DC++ Experimental compatibility test"; }
static Bool invoke(bool pm, const char* name, const char* params = "") {
    const auto& sub = subscriptions.at(pm ? HOOK_UI_CHAT_COMMAND_PM : HOOK_UI_CHAT_COMMAND);
    CommandData command{name, params};
    HubData hub{};
    UserData user{};
    Bool stop = False;
    return sub.callback(pm ? static_cast<void*>(&user) : static_cast<void*>(&hub), &command, sub.common, &stop);
}
static LRESULT CALLBACK playerProc(HWND window, UINT msg, WPARAM w, LPARAM l) {
    if(msg == WM_USER) {
        switch(l) {
        case 0: return 0x5090; // IPC_GETVERSION
        case 104: return 1; // IPC_ISPLAYING
        case 105: return w ? 240 : 120000; // IPC_GETOUTPUTTIME
        case 126: return w == 0 ? 44 : w == 1 ? 320 : 2; // IPC_GETINFO
        }
    }
    return DefWindowProcW(window, msg, w, l);
}
static HWND player(const wchar_t* className, const wchar_t* title) {
    WNDCLASSW wc{};
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = playerProc;
    wc.lpszClassName = className;
    check(RegisterClassW(&wc) != 0, "register test player");
    auto window = CreateWindowW(className, title, WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    check(window != nullptr, "create test player");
    return window;
}
static HWND dialog() { return FindWindowW(L"#32770", L"Media player plugin configuration"); }

int main(int argc, char** argv) try {
    check(argc == 2, "DLL argument required");
    check(!FindWindowW(L"Winamp v1.x", nullptr) && !FindWindowW(L"MediaPlayerClassicW", nullptr),
        "Close Winamp and MPC before running the isolated compatibility test");
    hooks.apiVersion = DCINTF_HOOKS_VER; hooks.bind_hook = bind; hooks.release_hook = releaseHook;
    config.apiVersion = DCINTF_CONFIG_VER; config.get_install_path = installPath;
    config.set_cfg = setConfig; config.get_cfg = getConfig; config.release = releaseConfig;
    hubs.apiVersion = DCINTF_DCPP_HUBS_VER; hubs.send_message = sendHub;
    hubs.send_private_message = sendPM; hubs.local_message = localMessage;
    logger.apiVersion = DCINTF_LOGGING_VER; logger.log = logMessage;
    ui.apiVersion = DCINTF_DCPP_UI_VER; ui.add_command = addCommand;
    ui.remove_command = removeCommand; ui.notify = notify;
    utils.apiVersion = DCINTF_DCPP_UTILS_VER; utils.utf8_to_wcs = toWide; utils.wcs_to_utf8 = toUtf8;
    DCCore core{}; core.apiVersion = DCAPI_CORE_VER; core.query_interface = query; core.host_name = hostName;
    auto library = LoadLibraryA(argv[1]);
    check(library != nullptr, "LoadLibrary");
    using Init = DCMAIN (DCAPI*)(MetaDataPtr);
    auto init = reinterpret_cast<Init>(GetProcAddress(library, "pluginInit"));
    check(init != nullptr, "undecorated pluginInit export");
    MetaData metadata;
    std::memset(&metadata, 0xff, sizeof(metadata));
    auto main = init(&metadata);
    check(main && metadata.apiVersion == DCAPI_CORE_VER, "API version");
    check(metadata.version == 0.5 && metadata.numDependencies == 0 && !metadata.dependencies, "metadata initialized");
    check(!init(nullptr), "null metadata rejected");
    check(!main(ON_LOAD, nullptr, nullptr), "null core rejected");
    core.apiVersion = 0;
    check(!main(ON_LOAD, &core, nullptr), "wrong API rejected");
    core.apiVersion = DCAPI_CORE_VER;
    denyUtils = true;
    check(!main(ON_LOAD_RUNTIME, &core, nullptr), "missing interface rejected");
    check(subscriptions.empty(), "failed load cleanup");
    denyUtils = false;
    check(main(ON_LOAD, &core, nullptr), "load after failure");
    check(subscriptions.size() == 2 && menuCommand, "hub/PM hooks and menu registered");
    check(!dialog(), "startup does not open configuration");
    check(!main(ON_LOAD, &core, nullptr), "duplicate load rejected");
    check(!invoke(false, "unrelated"), "unknown command passes through");
    check(invoke(false, "help", "plugins") && local.find("/winamp") != std::string::npos, "plugin help");
    check(invoke(false, "wa") && local == "Winamp is not running" && hubSends == 0, "missing Winamp stays local");
    check(invoke(true, "wa") && notice == "Winamp is not running" && pmSends == 0, "missing Winamp PM handled locally");
    auto winamp = player(L"Winamp v1.x", L"1. Artist - Song - Winamp");
    check(invoke(false, "w") && sent == "NP > Artist - Song [ 0:02:00 of 0:04:00 ]", "Winamp formatting");
    settings["WAFormat"] = "/me %[title]";
    check(invoke(true, "WINAMP") && sent == "Artist - Song" && action == True && pmSends == 1, "PM action message");
    SetWindowTextW(winamp, L"\u97f3\u697d - Winamp");
    settings["WAFormat"] = "%[title]";
    check(invoke(false, "wa") && sent == "\xe9\x9f\xb3\xe6\xa5\xbd", "Unicode title without playlist prefix");
    const std::wstring longTitle(3000, L'a');
    SetWindowTextW(winamp, longTitle.c_str());
    check(invoke(false, "wa") && sent.size() == 2047, "bounded Winamp title");
    DestroyWindow(winamp);
    auto mpc = player(L"MediaPlayerClassicW", L"Movie.mkv");
    check(invoke(false, "mpc") && sent == "NP > Movie.mkv", "MPC configured format");
    check(invoke(true, "mpc") && sent == "NP > Movie.mkv", "MPC PM");
    DestroyWindow(mpc);
    check(invoke(true, "mpc") && notice == "MPC is not running", "missing MPC handled locally");
    auto owner = CreateWindowW(L"STATIC", L"Test owner", WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    check(main(ON_CONFIGURE, &core, owner), "configure");
    auto window = dialog(); check(window != nullptr, "configuration window created");
    ShowWindow(window, SW_HIDE);
    check(GetWindow(window, GW_OWNER) == owner, "configuration owned by DC++ window");
    check(SendDlgItemMessage(window, 1001, EM_GETLIMITTEXT, 0, 0) == 2047, "edit length limit");
    check(main(ON_CONFIGURE, &core, owner) && dialog() == window, "repeat configure reuses window");
    ShowWindow(window, SW_HIDE);
    SetDlgItemTextW(window, 1001, L"%[title]");
    SendMessage(window, WM_COMMAND, IDOK, 0);
    check(settings["WAFormat"] == "%[title]" && !IsWindow(window), "short format saved and dialog destroyed");
    check(main(ON_CONFIGURE, &core, owner), "reopen dialog");
    window = dialog(); ShowWindow(window, SW_HIDE);
    SendMessage(window, WM_CLOSE, 0, 0);
    check(!IsWindow(window), "close dialog without recursion");
    check(main(ON_CONFIGURE, &core, owner), "reopen before unload");
    window = dialog(); ShowWindow(window, SW_HIDE);
    check(main(ON_UNLOAD, nullptr, nullptr), "unload");
    check(!IsWindow(window) && subscriptions.empty() && !menuCommand, "unload releases dialog hooks and menu");
    check(main(ON_INSTALL, &core, nullptr), "install after unload");
    window = dialog(); check(window != nullptr, "installation configuration"); ShowWindow(window, SW_HIDE);
    check(settings["WAFormat"] == "%[title]", "existing format preserved");
    check(main(ON_UNINSTALL, nullptr, nullptr) && !IsWindow(window), "uninstall cleanup");
    check(main(ON_LOAD_RUNTIME, &core, nullptr) && !dialog(), "runtime reload without dialog");
    check(main(ON_UNLOAD, nullptr, nullptr), "final unload");
    DestroyWindow(owner);
    check(FreeLibrary(library), "FreeLibrary");
    std::cout << "PASS: DC++ API " << DCAPI_CORE_VER << " DLL lifecycle, hub/PM commands, Unicode, formats, dialog ownership and cleanup\n";
    return 0;
} catch(const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
}
