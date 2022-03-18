//#include "Mediator/wamp/wamp.h"
//#include "logger.h"
//
//#include <thread>
//#include <cxxopts.hpp>
//#include <ifaddrs.h>
//#include <cstdlib>
//#include <autobahn-cpp/autobahn/wamp_arguments.hpp>
//#include <autobahn-cpp/autobahn/wamp_websocketpp_websocket_transport.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
//#include <msgpack.hpp>
//#include "base64.h"
//
//#include <arpa/inet.h>
//#include <netdb.h>
//
//#include <iomanip>
//#include <openssl/sha.h>
//#include <mcrypt.h>
//#include <ctime>
//#include "ulibudev.h"
//
//# define SHA_LONG unsigned int
//# define SHA_LBLOCK      16
//
////typedef struct SHA256state_st {
////    SHA_LONG h[8];
////    SHA_LONG Nl, Nh;
////    SHA_LONG data[SHA_LBLOCK];
////    unsigned int num, md_len;
////} SHA256_CTX;
//
//# define SHA256_DIGEST_LENGTH    32
//
//#ifdef __x86_64__
//#include <cpuid.h>
//#endif
//#ifdef __arm__
//#include <sstream>
//#include <sys/auxv.h>
//
//#define BIT_SHIFTS(n)			((1) << (n))
//#endif
//
//const std::string MASTER_KEY = "37bd7c2f0847375\0";
//
//unsigned int get_hw_hash() {
//	struct udev *context = udev_new();
//	char id[4096];
//	unsigned int hash = 0, k;
//#ifdef ARM
//	struct udev_device *device = udev_device_new_from_syspath(context, "/sys/block/mmcblk0");
//#else
//	struct udev_device *device = udev_device_new_from_syspath(context, "/sys/block/sda");
//#endif
//	snprintf(id, sizeof(id), "%s", udev_device_get_property_value(device, "ID_SERIAL"));
//    LogPrintf(spdlog::level::info, "WS: first hdd serial [%s]", id);
//	for (k = 0; k < strlen(id); k++)
//		hash = ((hash << 5) - hash) + (unsigned int)id[k];
//    LogPrintf(spdlog::level::info, "WS: first hdd serial hash [%u] [0x%08x]", hash, hash);
//	udev_device_unref(device);
//	udev_unref(context);
//	return hash;
//}
//
//int check_key(std::string &key) {
//	MCRYPT mfd;
//	std::string tmpstr;
//	int tmpint;
//	std::size_t pos1 = 0;
//	std::size_t pos2;
//	int ret;
//	char buff[2048];
//	time_t now = time(nullptr);
//
//    LogPrintf(spdlog::level::info, "Raw: %s", key.c_str());
//	std::string b64dec_key;
//    macaron::Base64::Decode(key, b64dec_key);
//	if(b64dec_key.empty()) {
//		LogPrintf(spdlog::level::err, "Base64 decoded is empty");
//		return -1;
//	}
//
//    LogPrintf(spdlog::level::warn, "Decoded: %s", b64dec_key.c_str());
//	mfd = mcrypt_module_open((char *)"xtea", nullptr, (char *)"ecb", nullptr);
//	if (mfd == MCRYPT_FAILED) {
//        LogPrintf(spdlog::level::warn, "WS: Mcrypt module open failed");
//        return -3;
//    }
//
//	ret = mcrypt_generic_init(mfd, (char *)MASTER_KEY.c_str(), MASTER_KEY.length(), nullptr);
//	if (ret < 0) {
//        LogPrintf(spdlog::level::warn, "WS: Mcrypt init failed: [%s]", mcrypt_strerror(ret));
//		return -3;
//	}
//
//	//  tmpint = sprintf(buff, "%s", b64dec_key.c_str());
//	memcpy(buff, b64dec_key.c_str(), b64dec_key.length());
//	if (mdecrypt_generic(mfd, buff, b64dec_key.length())) {
//        LogPrintf(spdlog::level::err, "xtea error");
//		return -2;
//	}
//	b64dec_key.assign(buff);
//
//    LogPrintf(spdlog::level::info, "|WS|: decrypted string %s", buff);
//	mcrypt_generic_deinit(mfd);
//	mcrypt_module_close(mfd);
//
//	pos2 = b64dec_key.find('.');
//	tmpstr.assign(b64dec_key.substr(pos1, pos2 - pos1));
//	pos1 = ++pos2;
//
//    LogPrintf(spdlog::level::info, "WS: mt_rand = [%s]", tmpstr.c_str());
//
//	pos2 = b64dec_key.find('.', pos1);
//	tmpstr.assign(b64dec_key.substr(pos1, pos2 - pos1));
//	pos1 = ++pos2;
//
//    LogPrintf(spdlog::level::info, "WS: $trm_id = [%s]", tmpstr.c_str());
//
//	pos2 = b64dec_key.find('.', pos1);
//	tmpstr.assign(b64dec_key.substr(pos1, pos2 - pos1));
//
//	pos1 = ++pos2;
//
//    LogPrintf(spdlog::level::info, "WS: $b64dec_key = [%s]", tmpstr.c_str());
//
//	if (std::strtoul(tmpstr.c_str(), nullptr, 0) == get_hw_hash()) {
//        LogPrintf(spdlog::level::info, "WS: hash check OK");
//	} else {
//        LogPrintf(spdlog::level::warn, "WS: hash check failed");
//		return -1;
//	};
//
//	pos2 = b64dec_key.find('.', pos1);
//	tmpstr.assign(b64dec_key.substr(pos1, pos2 - pos1));
//
//	pos1 = ++pos2;
//	tmpint = std::atoi(tmpstr.c_str());
//
//#ifdef KEY_DEBUG
//	LogPrintf(spdlog::level::info, "WS: $lic['v'] = %i", tmpint);
//	LogPrintf(spdlog::level::info, "WS: localtime %u", now);
//#endif
//
//	if (tmpint > now) {
//        LogPrintf(spdlog::level::info, "WS: time check OK");
//	}
//	else {
//		if (tmpint > (now + 864000)) {
//            LogPrintf(spdlog::level::warn, "WS: time check failed");
//			LogPrintf(spdlog::level::err, "License expired");
////			DB_Backup("", "");
////			terminal.set_running(false);
//			return -1;
//		}
//		else {
//            LogPrintf(spdlog::level::warn, "WS: time check failed [%i]", now - tmpint);
//			LogPrintf(spdlog::level::warn, "Time left to license expire: %i", now - tmpint);
//			return (now - tmpint);
//		};
//	};
//	LogPrintf(spdlog::level::info, "License check ok");
//	return 0;
//};
//
//std::string getMacAddress() {
//    struct ifaddrs *ifaddr;
//    int family, s;
//    char host[NI_MAXHOST];
//    std::string macAddress;
//
//    if (getifaddrs(&ifaddr) == -1) {
//        perror("getifaddrs");
//        exit(EXIT_FAILURE);
//    }
//
//    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr;
//    ifa = ifa->ifa_next) {
//        if (ifa->ifa_addr == nullptr)
//            continue;
//        family = ifa->ifa_addr->sa_family;
//        if(family == AF_INET6) {
//            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
//            if (s != 0) {
//                printf("getnameinfo() failed: %s\n", gai_strerror(s));
//                exit(EXIT_FAILURE);
//            }
//            macAddress = host;
//        }
//    }
//    freeifaddrs(ifaddr);
//    return std::move(macAddress);
//}
//
//void Wamp::wamp_io_close(const std::exception &e) {
//	std::string err = e.what();
//	LogPrintf(spdlog::level::err, "|WAMP| Close 5: {}", err.c_str());
////	set_hw_status(wamp_status, nullptr, 8, ERROR_STATUS);
//	wamp_wss_client.stop();
//}
//
//std::string GetCPUId() {
//    std::string strCPUId;
//
//#ifdef __x86_64__
//    unsigned int level = 1;
//    unsigned eax = 3, ebx = 0, ecx = 0, edx = 0;
//    __get_cpuid(level, &eax, &ebx, &ecx, &edx);
//
//    /// byte swap
//    int first = static_cast<int>(((eax >> 24) & 0xff) | ((eax << 8) & 0xff0000) | ((eax >> 8) & 0xff00) | ((eax << 24) & 0xff000000));
//    int last = static_cast<int>(((edx >> 24) & 0xff) | ((edx << 8) & 0xff0000) | ((edx >> 8) & 0xff00) | ((edx << 24) & 0xff000000));
//
//    std::stringstream ss;
//
//    ss << std::hex << first;
//    ss << std::hex << last;
//    ss >> strCPUId;
//#endif
//#ifdef __arm__
//#endif
//    return strCPUId;
//}
//
//std::string sha256(const std::string& str) {
//    unsigned char hash[SHA256_DIGEST_LENGTH];
//    SHA256_CTX sha256;
//    SHA256_Init(&sha256);
//    SHA256_Update(&sha256, str.c_str(), str.size());
//    SHA256_Final(hash, &sha256);
//    std::stringstream ss;
//    for(unsigned char i : hash)
//    {
//        ss << std::hex << std::setw(2) << std::setfill('0') << (int)i;
//    }
//    return ss.str();
//}
//
//std::string getHardwareId() {
//    std::string macAddress = getMacAddress();
//
//    if (!macAddress.empty()) {
//        macAddress = macAddress.substr(0, macAddress.find('%'));
//        std::cout << "MAC: " << macAddress << std::endl;
//    }
//    std::string cpuId = GetCPUId();
//    std::cout << "CPUID: " << cpuId << std::endl;
//    std::string decryptedStr = cpuId + macAddress;
//    std::cout << "DECRYPTED CODE: " << decryptedStr << std::endl;
//    std::string encryptedStr = sha256(decryptedStr);
//    std::cout << "ENCRYPTED CODE: " << encryptedStr << std::endl;
//    return encryptedStr;
//}
//
//Wamp::Wamp() {
//    std::random_device rd;
//	std::mt19937 mt(rd());
//	std::uniform_int_distribution<int> dist(25, 30);
//
//    wampInit(true);
//}
//
//int greeting() {
//	autobahn::wamp_kw_arguments data, res;
//
//	autobahn::wamp_call_options call_options;
//	call_options.set_timeout(std::chrono::seconds(15));
//
//	std::random_device rd;
//	std::mt19937 mt(rd());
//	std::uniform_int_distribution<int> dist(15, 20);
//
////    data["trm_id"] = trm_uid;
////    data["corev"] = core_version;
////    data["guiv"] = gui_version;
////    data["scriptv"] = script_version;
////    data["client_id"] = session_id;
//    res.clear();
////    res = wamp_call<autobahn::wamp_kw_arguments>("com.vndmanager.backend.greeting",std::make_tuple(data),call_options, wamp_session);
////    if (!wamp_session) {
////        LogPrintf(spdlog::level::debug, "Greeting loop exit");
////        return -1;
////    }
//    if (!res.empty()) {
//        LogPrintf(spdlog::level::debug, "Greeting OK");
//    }
//    else {
//        LogPrintf(spdlog::level::debug, "Wamp call timeout [greeting]");
//        sleep(dist(mt));
//    }
//	return 0;
//}
//
//int Wamp::grey_greeting() {
//	std::string grey = getHardwareId();
//	autobahn::wamp_kw_arguments data, res;
//	autobahn::wamp_call_options call_options;
//	call_options.set_timeout(std::chrono::seconds(15));
//
//    data["code"] = msgpack::object(grey.c_str());
//    data["client_id"] = msgpack::object(session_id);
//    res.clear();
//    res = wamp_call<autobahn::wamp_kw_arguments>(
//            "com.vndmanager.backend.grey_greeting",
//            std::make_tuple(data),
//            call_options,
//            wamp_session);
//    if (!wamp_session) {
//        LogPrintf(spdlog::level::debug, "Grey greeting loop exit");
//        return -1;
//    }
//    if (!res.empty()) {
//        return 0;
//    }
//    else {
//        LogPrintf(spdlog::level::debug, "Wamp call timeout [grey_greeting]");
//        return 0;
//    }
//	return 0;
//}
//
//int Wamp::get_grey_code() {
//	autobahn::wamp_kw_arguments tmp;
//	std::string tmpstr;
//	int tmpint;
//
//	std::random_device rd;
//	std::mt19937 mt(rd());
//	std::uniform_int_distribution<int> dist(15, 20);
//
//	autobahn::wamp_call_options call_options;
//	call_options.set_timeout(std::chrono::seconds(15));
//
//    tmp.clear();
//    tmp = wamp_call<autobahn::wamp_kw_arguments>(
//            "com.vndmanager.backend.get_grey_code",
//            std::make_tuple(get_hw_hash()),
//            call_options,
//            wamp_session);
//    if (!wamp_session) {
//        LogPrintf(spdlog::level::debug, "Get grey loop exit");
//        return -1;
//    }
//    if (tmp.empty()) {
//        LogPrintf(spdlog::level::debug, "Wamp call error [get_grey_code]");
//        sleep(dist(mt));
//    }
//    else {
//        msgpack::object obj = tmp["error"];
//        if (obj.is_nil()) {
//            obj = tmp["code"];
//            obj.convert(tmpint);
//            LogPrintf(spdlog::level::debug, "grey code: {}", tmpint);
//            tmpstr = std::to_string(tmpint);
//            this->mediator_->updateLicenseKet(tmpstr);
//        }
//        else {
//            obj = tmp["error"];
//            obj.convert(tmpstr);
//            LogPrintf(spdlog::level::debug, "get_grey_code error: {}", tmpstr.c_str());
//            sleep(dist(mt));
//        }
//    }
//	return 0;
//}
//
//int Wamp::check_grey_code() {
//	autobahn::wamp_kw_arguments tmp;
//	std::string tmpstr;
//	int tmpint;
//
//	std::random_device rd;
//	std::mt19937 mt(rd());
//	std::uniform_int_distribution<int> dist(15, 20);
//
//	std::string grey = this->mediator_->getLicenseKey();
//
//	autobahn::wamp_call_options call_options;
//	call_options.set_timeout(std::chrono::seconds(15));
//
//    tmp.clear();
//    tmp = wamp_call<autobahn::wamp_kw_arguments>(
//            "com.vndmanager.backend.check_grey_code",
//            std::make_tuple(stoi(grey), get_hw_hash()),
//            call_options,
//            wamp_session);
//    //	log_printf(debug, "Wamp call ready");
//    if (wamp_session) {
//        LogPrintf(spdlog::level::debug, "|WAMP| Check grey loop exit");
//        return -1;
//    }
//    if (tmp.empty()) {
//        LogPrintf(spdlog::level::debug, "Wamp call error [get_grey_code]");
//        sleep(dist(mt));
//    }
//    else {
//        msgpack::object obj = tmp["error"];
//        if (obj.is_nil()) {
//            obj = tmp["code"];
//            if (obj.is_nil()) {
//                grey = "";
//                this->mediator_->updateLicenseKet(grey);
//                get_grey_code();
//            } else {
//                obj.convert(tmpint);
//                LogPrintf(spdlog::level::debug, "grey code result: {}", tmpint);
//            }
//        } else {
//            obj = tmp["error"];
//            obj.convert(tmpstr);
//            LogPrintf(spdlog::level::debug, "get_grey_code error: {}", tmpstr.c_str());
//            sleep(dist(mt));
//        }
//    }
//	return 0;
//}
//
//void initTrm(autobahn::wamp_invocation invocation) {
////	auto res = invocation->argument<autobahn::wamp_kw_arguments>(0);
////	autobahn::wamp_kw_arguments result;
////
////	msgpack::object obj = res["password"];
////	std::string license = obj.as<std::string>();
////	obj = res["log"];
////	std::string trm_id = obj.as<std::string>();
////	obj = res["error"];
////	int tmpint;
////
////	if (obj.is_nil()) {
////		tmpint = check_key(license);
////		if (!tmpint) {
////			trm_id = getHardwareId();
////			trm_uid = std::stoi(trm_id);
////            license = this->mediator_->getLicenseKey();
////			license_key = license;
////
////			result["success"] = msgpack::object(true);
////			result["error"] = msgpack::object(NULL);
////			result["client_id"] = msgpack::object(session_id);
////		}
////		else {
////			LogPrintf(spdlog::level::debug, "Secret key check failed");
////
////			result["success"] = msgpack::object(false);
////			result["error"] = msgpack::object(fmt::format("Secret key check failed: %i", tmpint).c_str());
////			result["client_id"] = msgpack::object(session_id);
////		}
////	} else {
////		LogPrintf(spdlog::level::debug, "Error during term registration(com.vndmanager.backend.init.greycode)");
////	}
////
////	invocation->result(std::make_tuple(result));
//}
//
//void setLicense(autobahn::wamp_invocation invocation) {
//
//}
//
//void setValidTill(autobahn::wamp_invocation invocation) {
//
//}
//
//int Wamp::provide_sets() {
//	try {
//		if (wamp_provide(fmt::format("com.vndmanager.core.%i.set.license", trm_uid), setLicense))
//			return -1;
//		if (wamp_provide(fmt::format("com.vndmanager.core.%i.set.valid_till", trm_uid), setValidTill))
//			return -1;
//		}
//	catch (const std::exception &e)
//	{
//		throw;
//	}
//	return 0;
//}
//
//int Wamp::wamp_provide(const std::string &procedure, void (*func)(autobahn::wamp_invocation invocation)) {
//	boost::future<autobahn::wamp_registration> provide_future;
//
//    try {
//		autobahn::provide_options options;
//		options["ivnoke"] = msgpack::object("last");
//		if (wamp_session) {
//			LogPrintf(spdlog::level::debug, "Providing procedure: %s", procedure.c_str());
//			provide_future = wamp_session->provide(procedure, func, options);
//			auto res = provide_future.get();
//			return 0;
//		} else {
//			LogPrintf(spdlog::level::err, "Could not provide procedure since no wamp session aviable");
//			return -2;
//		}
//	} catch (const std::exception &e) {
////		auto err = e.what();
////		if(err == "wamp.error.procedure_already_exists")
////		{
////			log_printf(debug, "Procedure already exists. Unregistering: %s", procedure.c_str());
////		}
//		wamp_io_close(e);
//		throw;
//		// return -1;
//	}
//}
//
//int Wamp::provide_init() {
//	try {
//		std::string grey = getHardwareId();
//
//		switch (wamp_provide(fmt::format("com.vndmanager.core.init.%s", grey.c_str()), initTrm)) {
//		    case 0:
//		    	return 0;
//		    case -1:
//		    case -2:
//		    	return -1;
//		}
//	} catch (const std::exception &e) {
//		throw;
//	}
//}
//
//void Wamp::wampInit(bool registered) {
//	LogPrintf(spdlog::level::info, "Configuring WAMP");
//	std::vector<std::string> authmethods = {"ticket"};
//	std::string authid;
//	std::string secret;
//
//	if (registered) {
//        secret = this->mediator_->getLicenseKey();
//        authid = getHardwareId();
//	}
//	else {
//		secret = getHardwareId(); /// HARDCODE
//		authid = getHardwareId(); /// HARDCODE
//	}
//
//	try {
//		trasnport_ptr transport = std::make_shared<autobahn_client>(wamp_wss_client, wamp_address,
//						true);
//		wamp_session = std::make_shared<auth_wamp_session>(io, true, secret);
//		transport->attach(std::static_pointer_cast<autobahn::wamp_transport_handler>(wamp_session));
//
//		boost::future_status fut_status;
//		boost::future<void> connect_future;
//		boost::future<void> start_future;
//		boost::future<void> join_future;
//		boost::future<void> subscribe_future;
//		boost::future<void> leave_future;
//		boost::future<void> stop_future;
//
//		connect_future = transport->connect().then([&](boost::future<void> connected) {
//            try {
//                connected.get();
//            } catch (const std::exception &e) {
//                LogPrintf(spdlog::level::err, "|WAMP| Close 1: {}", e.what());
//                return;
//            }
//            LogPrintf(spdlog::level::debug, "|WAMP| Transport connected");
//            start_future = wamp_session->start().then([&](boost::future<void> started) {
//                try {
//                    started.get();
//                }
//                catch (const std::exception &e) {
//                    LogPrintf(spdlog::level::err, "|WAMP| Close 2: {}", e.what());
//                    return;
//                }
//                LogPrintf(spdlog::level::debug, "|WAMP| Session started");
//                join_future = wamp_session->join("vndmanager", authmethods, authid).then([&](boost::future<uint64_t> joined) {
//                    {
//                        std::string tmpstr;
//                        try {
//                            session_id = joined.get();
//                            LogPrintf(spdlog::level::info, "|WAMP| connected.Session id: %llu", session_id);
//                            if (!trm_uid) {
//                                tmpstr = this->mediator_->getLicenseKey(); // Check DB for grey_code
//                                if (tmpstr.empty()) {// If not exists then get it
//                                    LogPrintf(spdlog::level::warn, "|WAMP| Grey code not found. Calling procedure");
//                                    if (this->mediator_->getLicenseKey().empty()) {
//                                        return;
//                                    }
//                                    tmpstr = this->mediator_->getLicenseKey();
//                                }
//                                else { // If exist then check it
//                                    LogPrintf(spdlog::level::info, "|WAMP| Grey code found |{}|. Calling procedure", tmpstr.c_str());
//                                    if (check_grey_code()) {
//                                        return;
//                                    }
//                                    tmpstr = this->mediator_->getLicenseKey();
//                                }
//                            }
//                            autobahn::wamp_kw_arguments welcome_details;
//                            welcome_details = wamp_session->welcome_details();
//                            if (!welcome_details.empty()) {
//                                msgpack::object obj = welcome_details["authrole"];
//                                obj.convert(authrole);
//                                LogPrintf(spdlog::level::info, "|WAMP| Authrole: |{}|", authrole.c_str());
//                                sleep(5);
//                                if (authrole == "helper-grey") {
//                                    if (grey_greeting()) {
//                                        return;
//                                    }
//                                    if (provide_init()) {
//                                        return;
//                                    }
//                                } else if (authrole == "helper-green") {
//                                    if (provide_sets()) {
//                                        return;
//                                    }
//                                    if (greeting()) {
//                                        return;
//                                    }
//                                    sleep(3);
//                                }
//                            }
//                        } catch (const std::exception &e) {
//                            LogPrintf(spdlog::level::err, "|WAMP| Close 3: %s", e.what());
//                            wamp_wss_client.stop();
//                            return;
//                        }
//                    }
//                });
//            });
//		});
//		LogPrintf(spdlog::level::info, "|WAMP| io services start");
//		io.run();
//		LogPrintf(spdlog::level::info, "|WAMP| io services stop");
//
//		if (transport->is_connected()) {
//			transport->disconnect();
//		}
////		terminal.set_wamp_state(Wstate::wamp_disconnected);
//		wamp_session.reset();
//		io.restart();
//
//		LogPrintf(spdlog::level::info, "|WAMP| io services stopped");
//	} catch (std::exception &e) {
////		set_hw_status(wamp_status, nullptr, 8, ERROR_STATUS);
//		LogPrintf(spdlog::level::err, "|WAMP| Close 4: %s", e.what());
//		wamp_wss_client.stop();
//	}
//}