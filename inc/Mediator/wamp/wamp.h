//#ifndef PTS2_0_WAMP_H
//#define PTS2_0_WAMP_H
//
//#include "Mediator.h"
//
//#include "autobahn-cpp/autobahn/autobahn.hpp"
//#include "autobahn-cpp/autobahn/wamp_websocketpp_websocket_transport.hpp"
//
//#ifdef WAMP_TLS
//typedef std::shared_ptr<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_tls_client>> trasnport_ptr;
//typedef autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_tls_client> autobahn_client;
//typedef websocketpp::client<websocketpp::config::asio_tls_client> websocket_client;
//#else
//typedef std::shared_ptr<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>> trasnport_ptr;
//typedef autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client> autobahn_client;
//typedef websocketpp::client<websocketpp::config::asio_client> websocket_client;
//#endif
//
//class auth_wamp_session : public autobahn::wamp_session {
//public:
//  boost::promise<autobahn::wamp_authenticate> challenge_future;
//
//  std::string m_secret;
//
//  auth_wamp_session() = default;
//
//  auth_wamp_session(
//      boost::asio::io_service &io,
//      bool debug_enabled,
//      std::string secret)
//      : autobahn::wamp_session(io, debug_enabled), m_secret(std::move(secret)) {}
//
//  boost::future<autobahn::wamp_authenticate> on_challenge(const autobahn::wamp_challenge &challenge) override {
//    std::string signature = m_secret;
//    challenge_future.set_value(autobahn::wamp_authenticate(signature));
//    return challenge_future.get_future();
//  }
//};
//
//class Wamp : public BaseComponent {
//private:
//    int trm_uid{};
//    long long session_id{};
//    std::string authrole;
//    std::string license_key;
//    boost::asio::io_service io;
//    std::shared_ptr<auth_wamp_session> wamp_session;
//    websocket_client wamp_wss_client;
//    std::string wamp_address = "wss://crossbar-eticket.marit.expert:8085/ws";
//public:
//    Wamp();
//
//    void wampInit(bool registered);
//    void wamp_io_close(const std::exception &e);
//    int wamp_provide(const std::string &procedure, void (*)(autobahn::wamp_invocation invocation));
//
//    int get_grey_code();
//    int check_grey_code();
//    int provide_init();
//    int provide_sets();
//    int greeting();
//    int grey_greeting();
//
//    template <class Response, class... List>
//    Response wamp_call(std::string procedure, const std::tuple<List...> &arguments,
//                       autobahn::wamp_call_options &call_options, const std::shared_ptr<auth_wamp_session> &session,
//                       bool print = true);
//};
//
//#endif //PTS2_0_WAMP_H
