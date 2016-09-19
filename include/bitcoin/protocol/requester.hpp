
#ifndef LIBBITCOIN_PROTOCOL_REQUESTER_HPP
#define LIBBITCOIN_PROTOCOL_REQUESTER_HPP

#include <boost/optional.hpp>
#include <google/protobuf/message_lite.h>
#include <bitcoin/bitcoin/config/endpoint.hpp>
#include <bitcoin/bitcoin/utility/asio.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/protocol/zmq/context.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {

class BCP_API requester
{
public:
    requester(zmq::context& context);

    requester(zmq::context& context, const config::endpoint& address);

    requester(const requester&) = delete;
    void operator=(const requester&) = delete;

    ~requester();

    operator const bool() const;

    code connect(const config::endpoint& address);

    code send(const google::protobuf::MessageLite& request,
        google::protobuf::MessageLite& reply);

private:
    code send_recv(const google::protobuf::MessageLite& request,
        google::protobuf::MessageLite& reply);

    zmq::context& _context;
    asio::service _io_service;
    asio::service::work _io_work;
    boost::optional<zmq::socket> _socket;
    asio::thread _io_thread;
};

} // namespace protocol
} // namespace libbitcoin

#endif
