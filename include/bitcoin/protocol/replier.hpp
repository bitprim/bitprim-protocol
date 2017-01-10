
#ifndef LIBBITCOIN_PROTOCOL_REPLIER_HPP
#define LIBBITCOIN_PROTOCOL_REPLIER_HPP

#include <map>
#include <mutex>
#include <string>
#include <boost/optional.hpp>
#include <google/protobuf/message_lite.h>
#include <bitcoin/bitcoin/config/endpoint.hpp>
#include <bitcoin/bitcoin/utility/asio.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/protocol/zmq/context.hpp>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {

class BCP_API replier
{
    template <typename Message, typename Handler>
    class handler_wrapper
    {
    public:
        handler_wrapper(replier* replier_ptr,
            std::string const& handler_id, Handler const& handler)
          : _replier_ptr(replier_ptr),
            _handler_id(handler_id),
            _handler(handler)
        {}

        template <typename ...Args>
        bool operator()(Args&&... args)
        {
            Message reply;
            _handler(std::forward<Args>(args)..., reply);
            _replier_ptr->send_handler_reply(_handler_id, reply);
            return true;
        }

    private:
        replier* _replier_ptr;
        std::string _handler_id;
        Handler _handler;
    };

public:
    replier(zmq::context& context);

    replier(zmq::context& context, const config::endpoint& address);

    replier(const replier&) = delete;
    void operator=(const replier&) = delete;

    ~replier();

    operator const bool() const;

    code bind(const config::endpoint& address);

    code receive(google::protobuf::MessageLite& request);

    code send(zmq::message& reply);

    template <typename Message, typename Handler>
    handler_wrapper<Message, Handler> make_handler(
        std::string const& handler_id, Handler const& handler)
    {
        publish_connect(handler_id);

        return { this, handler_id, handler };
    }

    template <typename Message, typename Handler>
    handler_wrapper<Message, Handler> make_subscription(
        std::string const& handler_id, Handler const& handler)
    {
        return make_handler<Message>(handler_id, handler);
    }

private:
    code publish_connect(std::string const& handler_id);

    void send_handler_reply(std::string const& handler_id,
        const google::protobuf::MessageLite& reply);

private:
    zmq::context& _context;
    boost::optional<zmq::socket> _socket;

    mutable std::mutex _handlers_mutex;
    asio::service _handlers_service;
    asio::thread _handlers_thread;
    asio::service::work _handlers_work;
    std::map<std::string, zmq::socket> _publish_sockets;
};

} // namespace protocol
} // namespace libbitcoin

#endif
