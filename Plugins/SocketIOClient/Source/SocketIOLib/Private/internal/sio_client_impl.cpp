// Modifications Copyright 2018-current Getnamo. All Rights Reserved

//
//  sio_client_impl.cpp
//  SioChatDemo
//
//  Created by Melo Yao on 4/3/15.
//  Copyright (c) 2015 Melo Yao. All rights reserved.
//

/* This disables two things:
   1) error 4503 where MSVC complains about
	  decorated names being too long. There's no way around
	  this.
   2) We also disable a security error triggered by
	  websocketpp not using checked iterators.
*/
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#define _SCL_SECURE_NO_WARNINGS
#endif

/* For this code, we will use standalone ASIO
   and websocketpp in C++11 mode only */
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STL_

#include "sio_client_impl.h"
#include <sstream>
#include <mutex>
#include <cmath>

// Comment this out to disable handshake logging to stdout
#define SIO_LIB_DEBUG 0

using namespace std;

#if defined(SIO_LIB_DEBUG) && SIO_LIB_DEBUG
#define DEBUG_LOG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(CategoryName, Verbosity, Format, ...)
#endif

#define LOG(x)

using std::chrono::milliseconds;

namespace sio
{
    /*************************public:*************************/
    template<typename client_type>
    client_impl<client_type>::client_impl() :
        m_ping_interval(0),
        m_ping_timeout(0),
        m_network_thread(),
        m_con_state(con_closed),
        m_reconn_delay(5000),
        m_reconn_delay_max(25000),
        m_reconn_attempts(0xFFFFFFFF),
        m_reconn_made(0),
        m_path("socket.io")
    {
        using websocketpp::log::alevel;
#ifndef DEBUG
        m_client.clear_access_channels(alevel::all);
        m_client.set_access_channels(alevel::connect | alevel::disconnect | alevel::app);
#endif
        // Initialize the Asio transport policy
        m_client.init_asio();

        // Bind the clients we are using
        using std::placeholders::_1;
        using std::placeholders::_2;

        m_client.set_open_handler(std::bind(&client_impl<client_type>::on_open, this, _1));
        m_client.set_close_handler(std::bind(&client_impl<client_type>::on_close, this, _1));
        m_client.set_fail_handler(std::bind(&client_impl<client_type>::on_fail, this, _1));
        m_client.set_message_handler(std::bind(&client_impl<client_type>::on_message, this, _1, _2));
        m_packet_mgr.set_decode_callback(std::bind(&client_impl<client_type>::on_decode, this, _1));
        m_packet_mgr.set_encode_callback(std::bind(&client_impl<client_type>::on_encode, this, _1, _2));
        template_init();
    }

    template<typename client_type>
    client_impl<client_type>::~client_impl()
    {
        this->sockets_invoke_void(socket_on_close());
        sync_close();
    }

    template<typename client_type>
    void client_impl<client_type>::connect(const string& uri, const map<string, string>& query, const map<string, string>& headers, const message::ptr& auth, const std::string& path /*= "socket.io"*/)
    {

        if (m_reconn_timer)
        {
            m_reconn_timer->cancel();
            m_reconn_timer.reset();
        }
        if (m_network_thread)
        {
            if (m_con_state == con_closing || m_con_state == con_closed)
            {
                //if client is closing, join to wait.
                //if client is closed, still need to join,
                //but in closed case,join will return immediately.
                m_network_thread->join();
                m_network_thread.reset();//defensive
            }
            else
            {
                //if we are connected, do nothing.
                return;
            }
        }
        m_con_state = con_opening;
        m_reconn_made = 0;

        if (!uri.empty())
        {
            m_base_url = uri;
        }

        string query_str;
        for (map<string, string>::const_iterator it = query.begin(); it != query.end(); ++it) {
            query_str.append("&");
            query_str.append(it->first);
            query_str.append("=");
            string query_str_value = encode_query_string(it->second);
            query_str.append(query_str_value);
        }
        m_query_string = std::move(query_str);

        m_http_headers = headers;
		m_auth = auth;

        if (path != ""){
            m_path = path;
        }

        this->reset_states();
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::connect_impl, this, m_base_url, m_query_string));
        m_network_thread.reset(new thread(std::bind(&client_impl<client_type>::run_loop, this)));//uri lifecycle?

    }

    template<typename client_type>
    socket::ptr const& client_impl<client_type>::socket(string const& nsp)
    {
        lock_guard<mutex> guard(m_socket_mutex);
        string aux;
        if (nsp == "")
        {
            aux = "/";
        }
        else if (nsp[0] != '/')
        {
            aux.append("/", 1);
            aux.append(nsp);
        }
        else
        {
            aux = nsp;
        }

        auto it = m_sockets.find(aux);
        if (it != m_sockets.end())
        {
            return it->second;
        }
        else
        {
            shared_ptr<sio::socket> newSocket = shared_ptr<sio::socket>(new sio::socket(this, aux, m_auth));
            pair<const string, socket::ptr> p(aux, newSocket);
            return (m_sockets.insert(p).first)->second;
        }
    }

    template<typename client_type>
    void client_impl<client_type>::close()
    {
        m_con_state = con_closing;
        this->sockets_invoke_void(&sio::socket::close);
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::close_impl, this, close::status::normal, "End by user"));
    }

    template<typename client_type>
    void client_impl<client_type>::sync_close()
    {
        m_con_state = con_closing;
        this->sockets_invoke_void(&sio::socket::close);
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::close_impl, this, close::status::normal, "End by user"));
        if (m_network_thread)
        {
            m_network_thread->join();
            m_network_thread.reset();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::set_logs_default()
    {
        m_client.clear_access_channels(websocketpp::log::alevel::all);
        m_client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect | websocketpp::log::alevel::app);
    }

    template<typename client_type>
    void client_impl<client_type>::set_logs_quiet()
    {
        m_client.clear_access_channels(websocketpp::log::alevel::all);
    }

    template<typename client_type>
    void client_impl<client_type>::set_logs_verbose()
    {
        m_client.set_access_channels(websocketpp::log::alevel::all);
    }

    /*************************protected:*************************/
    template<typename client_type>
    void client_impl<client_type>::send(packet& p)
    {
        m_packet_mgr.encode(p);
    }

    template<typename client_type>
    void client_impl<client_type>::remove_socket(string const& nsp)
    {
        lock_guard<mutex> guard(m_socket_mutex);
        auto it = m_sockets.find(nsp);
        if (it != m_sockets.end())
        {
            m_sockets.erase(it);
        }
    }

    template<typename client_type>
    asio_sockio::io_service& client_impl<client_type>::get_io_service()
    {
        return m_client.get_io_service();
    }

    template<typename client_type>
    void client_impl<client_type>::on_socket_closed(string const& nsp)
    {
        if (m_socket_close_listener)m_socket_close_listener(nsp);
    }

    template<typename client_type>
    void client_impl<client_type>::on_socket_opened(string const& nsp)
    {
        if (m_socket_open_listener)m_socket_open_listener(nsp);
    }

    /*************************private:*************************/
    template<typename client_type>
    void client_impl<client_type>::run_loop()
    {

        m_client.run();
        m_client.reset();
        m_client.get_alog().write(websocketpp::log::alevel::devel,
            "run loop end");
    }

    template<typename client_type>
    void client_impl<client_type>::connect_impl(const string& uri, const string& queryString)
    {
        do {
            websocketpp::uri uo(uri);
            ostringstream ss;

            if (is_tls(uri))
            {
                // This requires SIO_TLS to have been compiled in.
                ss << "wss://";
            }
            else
            {
                ss << "ws://";
            }

            const std::string host(uo.get_host());
            // As per RFC2732, literal IPv6 address should be enclosed in "[" and "]".
            if (host.find(':') != std::string::npos) {
                ss << "[" << uo.get_host() << "]";
            }
            else {
                ss << uo.get_host();
            }

            // If a resource path was included in the URI, use that, otherwise
            // use the default /socket.io/.
            
            std::string path(uo.get_resource() == "/" ? "/socket.io/" : uo.get_resource());
			
            //override if m_path is set
			if (m_path != "socket.io")
			{
                path = "/" + m_path + "/";
			}

            ss << ":" << uo.get_port() << path << "?EIO=4&transport=websocket";
            if (m_sid.size() > 0) {
                ss << "&sid=" << m_sid;
            }
            ss << "&t=" << time(NULL) << queryString;
            lib::error_code ec;
            typename client_type::connection_ptr con = m_client.get_connection(ss.str(), ec);
            if (ec) {
                m_client.get_alog().write(websocketpp::log::alevel::app,
                    "Get Connection Error: " + ec.message());
                break;
            }

            for (auto&& header : m_http_headers) {
                con->replace_header(header.first, header.second);
            }

            m_client.connect(con);
            return;
        }         while (0);
        if (m_fail_listener)
        {
            m_fail_listener();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::close_impl(close::status::value const& code, string const& reason)
    {
        DEBUG_LOG(LogTemp, Log, TEXT("Close by reason: %s"), *FString(reason.c_str()));
        if (m_reconn_timer)
        {
            m_reconn_timer->cancel();
            m_reconn_timer.reset();
        }
        if (m_con.expired())
        {
            DEBUG_LOG(LogTemp, Warning, TEXT("close_impl::Error: No active session: %s"), *FString(reason.c_str()));
        }
        else
        {
            lib::error_code ec;
            m_client.close(m_con, code, reason, ec);
        }
    }

    template<typename client_type>
    void client_impl<client_type>::send_impl(shared_ptr<const string> const& payload_ptr, frame::opcode::value opcode)
    {
        if (m_con_state == con_opened)
        {
            lib::error_code ec;
            m_client.send(m_con, *payload_ptr, opcode, ec);
            if (ec)
            {
                //DEBUG_LOG(LogTemp, Warning, TEXT("close_impl::Error: Send failed,reason: %s"), *FString(ec.message().c_str()));
            }
        }
    }

    template<typename client_type>
    void client_impl<client_type>::ping(const asio_sockio::error_code& ec)
    {
        if (ec || m_con.expired())
        {
            if (ec != asio_sockio::error::operation_aborted)
                //LOG("ping exit,con is expired?" << m_con.expired() << ",ec:" << ec.message() << endl);
            return;
        }
        packet p(packet::frame_ping);
        m_packet_mgr.encode(p, [&](bool /*isBin*/, shared_ptr<const string> payload)
            {
                lib::error_code ec;
                this->m_client.send(this->m_con, *payload, frame::opcode::text, ec);
            });
        if (!m_ping_timeout_timer)
        {
            m_ping_timeout_timer.reset(new asio_sockio::steady_timer(m_client.get_io_service()));
            std::error_code timeout_ec;
            m_ping_timeout_timer->expires_from_now(milliseconds(m_ping_timeout), timeout_ec);
            m_ping_timeout_timer->async_wait(std::bind(&client_impl<client_type>::timeout_pong, this, std::placeholders::_1));
        }
    }

    template<typename client_type>
    void client_impl<client_type>::timeout_pong(const asio_sockio::error_code& ec)
    {
        if (ec)
        {
            return;
        }
        LOG("Pong timeout" << endl);
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::close_impl, this, close::status::policy_violation, "Pong timeout"));
    }

    template<typename client_type>
    void client_impl<client_type>::timeout_reconnect(asio_sockio::error_code const& ec)
    {
        if (ec)
        {
            return;
        }
        if (m_con_state == con_closed)
        {
            m_con_state = con_opening;
            m_reconn_made++;
            this->reset_states();
            LOG("Reconnecting..." << endl);
            if (m_reconnecting_listener) m_reconnecting_listener();
            m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::connect_impl, this, m_base_url, m_query_string));
        }
    }

    template<typename client_type>
    unsigned client_impl<client_type>::next_delay() const
    {
        //no jitter, fixed power root.
        unsigned reconn_made = min<unsigned>(m_reconn_made, 32);//protect the pow result to be too big.
        return static_cast<unsigned>(min<double>(m_reconn_delay * pow(1.5, reconn_made), m_reconn_delay_max));
    }

    template<typename client_type>
    socket::ptr client_impl<client_type>::get_socket_locked(string const& nsp)
    {
        lock_guard<mutex> guard(m_socket_mutex);
        auto it = m_sockets.find(nsp);
        if (it != m_sockets.end())
        {
            return it->second;
        }
        else
        {
            return socket::ptr();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::sockets_invoke_void(void (sio::socket::* fn)(void))
    {
        map<const string, socket::ptr> socks;
        {
            lock_guard<mutex> guard(m_socket_mutex);
            socks.insert(m_sockets.begin(), m_sockets.end());
        }
        for (auto it = socks.begin(); it != socks.end(); ++it) {
            ((*(it->second)).*fn)();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::on_fail(connection_hdl)
    {
        if (m_con_state == con_closing) {
            LOG("Connection failed while closing." << endl);
            this->close();
            return;
        }

        m_con.reset();
        m_con_state = con_closed;
        this->sockets_invoke_void(socket_on_disconnect());
        LOG("Connection failed." << endl);
        if (m_reconn_made < m_reconn_attempts)
        {
            LOG("Reconnect for attempt:" << m_reconn_made << endl);
            unsigned delay = this->next_delay();
            if (m_reconnect_listener) m_reconnect_listener(m_reconn_made, delay);
            m_reconn_timer.reset(new asio_sockio::steady_timer(m_client.get_io_service()));
            asio_sockio::error_code ec;
            m_reconn_timer->expires_from_now(milliseconds(delay), ec);
            m_reconn_timer->async_wait(std::bind(&client_impl<client_type>::timeout_reconnect, this, std::placeholders::_1));
        }
        else
        {
            if (m_fail_listener)m_fail_listener();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::on_open(connection_hdl con)
    {
        if (m_con_state == con_closing) {
            LOG("Connection opened while closing." << endl);
            this->close();
            return;
        }

        LOG("Connected." << endl);
        m_con_state = con_opened;
        m_con = con;
        m_reconn_made = 0;
        this->sockets_invoke_void(socket_on_open());
        this->socket("");
        if (m_open_listener)m_open_listener();
    }

    template<typename client_type>
    void client_impl<client_type>::on_close(connection_hdl con)
    {
        LOG("Client Disconnected." << endl);
        con_state m_con_state_was = m_con_state;
        m_con_state = con_closed;
        lib::error_code ec;
        close::status::value code = close::status::normal;
        typename client_type::connection_ptr conn_ptr = m_client.get_con_from_hdl(con, ec);
        if (ec) {
            LOG("OnClose get conn failed" << ec << endl);
        }
        else
        {
            code = conn_ptr->get_local_close_code();
        }

        m_con.reset();
        this->clear_timers();
        client::close_reason reason;

        // If we initiated the close, no matter what the close status was,
        // we'll consider it a normal close. (When using TLS, we can
        // sometimes get a TLS Short Read error when closing.)
        if (code == close::status::normal || m_con_state_was == con_closing)
        {
            this->sockets_invoke_void(socket_on_disconnect());
            reason = client::close_reason_normal;
        }
        else
        {
            this->sockets_invoke_void(socket_on_disconnect());
            if (m_reconn_made < m_reconn_attempts)
            {
                LOG("Reconnect for attempt:" << m_reconn_made << endl);
                unsigned delay = this->next_delay();
                if (m_reconnect_listener) m_reconnect_listener(m_reconn_made, delay);
                m_reconn_timer.reset(new asio_sockio::steady_timer(m_client.get_io_service()));
                asio_sockio::error_code ec2;
                m_reconn_timer->expires_from_now(milliseconds(delay), ec2);
                m_reconn_timer->async_wait(std::bind(&client_impl<client_type>::timeout_reconnect, this, std::placeholders::_1));
                return;
            }
            reason = client::close_reason_drop;
        }

        if (m_close_listener)
        {
            m_close_listener(reason);
        }
    }

    template<typename client_type>
    void client_impl<client_type>::on_message(connection_hdl, message_ptr msg)
    {
        if (m_ping_timeout_timer) {
            asio_sockio::error_code ec;
            m_ping_timeout_timer->expires_from_now(milliseconds(m_ping_timeout), ec);
            m_ping_timeout_timer->async_wait(std::bind(&client_impl<client_type>::timeout_pong, this, std::placeholders::_1));
        }
        // Parse the incoming message according to socket.IO rules
        m_packet_mgr.put_payload(msg->get_payload());
    }

    template<typename client_type>
    void client_impl<client_type>::on_handshake(message::ptr const& message)
    {
        if (message && message->get_flag() == message::flag_object)
        {
            const object_message* obj_ptr = static_cast<object_message*>(message.get());
            const map<string, message::ptr>* values = &(obj_ptr->get_map());
            auto it = values->find("sid");
            if (it != values->end()) {
                m_sid = static_pointer_cast<string_message>(it->second)->get_string();
            }
            else
            {
                goto failed;
            }
            it = values->find("pingInterval");
            if (it != values->end() && it->second->get_flag() == message::flag_integer) {
                m_ping_interval = (unsigned)static_pointer_cast<int_message>(it->second)->get_int();
            }
            else
            {
                m_ping_interval = 25000;
            }
            it = values->find("pingTimeout");

            if (it != values->end() && it->second->get_flag() == message::flag_integer) {
                m_ping_timeout = (unsigned)static_pointer_cast<int_message>(it->second)->get_int();
            }
            else
            {
                m_ping_timeout = 60000;
            }

            return;
        }
    failed:
        //just close it.
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::close_impl, this, close::status::policy_violation, "Handshake error"));
    }

    template<typename client_type>
    void client_impl<client_type>::on_ping()
    {
        packet p(packet::frame_pong);
        m_packet_mgr.encode(p, [&](bool /*isBin*/, shared_ptr<const string> payload)
            {
                this->m_client.send(this->m_con, *payload, frame::opcode::text);
            });

        if (m_ping_timeout_timer)
        {
            m_ping_timeout_timer->cancel();
            m_ping_timeout_timer.reset();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::on_decode(packet const& p)
    {
        switch (p.get_frame())
        {
        case packet::frame_message:
        {
            socket::ptr so_ptr = get_socket_locked(p.get_nsp());
            if (so_ptr)socket_on_message_packet(so_ptr, p);
            break;
        }
        case packet::frame_open:
            this->on_handshake(p.get_message());
            break;
        case packet::frame_close:
            //FIXME how to deal?
            this->close_impl(close::status::abnormal_close, "End by server");
            break;
        case packet::frame_ping:
            this->on_ping();
            break;

        default:
            break;
        }
    }

    template<typename client_type>
    void client_impl<client_type>::on_encode(bool isBinary, shared_ptr<const string> const& payload)
    {
        LOG("encoded payload length:" << payload->length() << endl);
        m_client.get_io_service().dispatch(std::bind(&client_impl<client_type>::send_impl, this, payload, isBinary ? frame::opcode::binary : frame::opcode::text));
    }

    template<typename client_type>
    void client_impl<client_type>::clear_timers()
    {
        LOG("clear timers" << endl);
        asio_sockio::error_code ec;
        if (m_ping_timeout_timer)
        {
            m_ping_timeout_timer->cancel(ec);
            m_ping_timeout_timer.reset();
        }
    }

    template<typename client_type>
    void client_impl<client_type>::reset_states()
    {
        m_client.reset();
        m_sid.clear();
        m_packet_mgr.reset();
    }

    template<>
    void client_impl<client_type_no_tls>::template_init()
    {
    }

#if SIO_TLS
    typedef websocketpp::lib::shared_ptr<asio_sockio::ssl::context> context_ptr;
    static context_ptr on_tls_init(int verify_mode, connection_hdl conn)
    {
        context_ptr ctx = context_ptr(new  asio_sockio::ssl::context(asio_sockio::ssl::context::tlsv12));
        asio_sockio::error_code ec;
        ctx->set_options(asio_sockio::ssl::context::default_workarounds |
            asio_sockio::ssl::context::no_sslv2 |
            asio_sockio::ssl::context::single_dh_use, ec);
        if (ec)
        {
            cerr << "Init tls failed,reason:" << ec.message() << endl;
        }

        if (verify_mode >= 0)
        {
            ctx->set_verify_mode(verify_mode);
        }

        return ctx;
    }

    template<typename client_type>
    void client_impl<client_type>::set_verify_mode(int mode)
    {
        verify_mode = mode;
    }

    template<>
    void client_impl<client_type_tls>::template_init()
    {
        m_client.set_tls_init_handler(std::bind(&on_tls_init, verify_mode, std::placeholders::_1));
    }
#endif

    bool client_impl_base::is_tls(const string& uri)
    {
        websocketpp::uri uo(uri);
        if (uo.get_scheme() == "http" || uo.get_scheme() == "ws")
        {
            return false;
        }
        else if (uo.get_scheme() == "https" || uo.get_scheme() == "wss")
        {
#if SIO_TLS
            return true;
#else
            return false;
#endif
        }
        else
        {
            throw std::runtime_error("unsupported URI scheme");
        }
    }

    socket* client_impl_base::new_socket(const string& nsp,const message::ptr& auth)
    {
        return new sio::socket(this, nsp, auth);
    }

    void client_impl_base::socket_on_message_packet(socket::ptr& s, const packet& p)
    {
        s->on_message_packet(p);
    }

    template class client_impl<client_type_no_tls>;
#if SIO_TLS
    template class client_impl<client_type_tls>;
#endif

    template<typename client_type>
    std::string client_impl<client_type>::encode_query_string(const std::string& query) {
        ostringstream ss;
        ss << std::hex;
        // Percent-encode (RFC3986) non-alphanumeric characters.
        for (const char c : query) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                ss << c;
            }
            else {
                ss << '%' << std::uppercase << std::setw(2) << int((unsigned char)c) << std::nouppercase;
            }
        }
        ss << std::dec;
        return ss.str();
    }
}
