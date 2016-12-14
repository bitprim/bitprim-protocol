
#include <bitcoin/protocol/replier.hpp>

#include <functional>
#include <string>
#include <system_error>
#include <tuple>
#include <boost/thread/latch.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <google/protobuf/message_lite.h>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/zeromq.hpp>

namespace libbitcoin {
namespace protocol {

replier::replier(zmq::context& context)
  : _context(context)
{}

replier::replier(zmq::context& context, const config::endpoint& address)
  : replier(context)
{
    code ec = bind(address);
    if (ec) throw std::system_error(ec);
}

replier::~replier()
{}

replier::operator const bool() const
{
    return _socket.is_initialized();
}

code replier::bind(const config::endpoint& address)
{
    _socket = boost::in_place(
        std::ref(_context), zmq::socket::role::replier);
    if (!*_socket)
        return zmq::get_last_error();

    return _socket->bind(address);
}

code replier::receive(google::protobuf::MessageLite& request)
{
    BITCOIN_ASSERT(_socket);

    zmq::message message;
    auto ec = _socket->receive(message);
    if (ec)
        return ec;

    if (!message.dequeue(request))
        return error::bad_stream;

    return error::success;
}

code replier::send(zmq::message& reply)
{
    BITCOIN_ASSERT(_socket);

    code ec = _socket->send(reply);
    if (ec) return ec;

    return error::success;
}

code replier::publish_connect(std::string const& handler_id)
{
    const std::string endpoint =
        handler_id.substr(0, handler_id.find_first_of('/'));

    if (_publish_sockets.count(endpoint))
        return error::success;

    auto iter = _publish_sockets.emplace(std::piecewise_construct,
        std::forward_as_tuple(endpoint),
        std::forward_as_tuple(std::ref(_context), zmq::socket::role::pair)).first;
    auto& socket = iter->second;

    return socket.connect("tcp://" + endpoint);
}

code replier::send_handler_reply(std::string const& handler_id,
    const google::protobuf::MessageLite& reply)
{
    const auto separator = handler_id.find_first_of('/');
    const std::string endpoint = handler_id.substr(0, separator);
    const std::string id = handler_id.substr(separator + 1);

    auto publish_iter = _publish_sockets.find(endpoint);
    BITCOIN_ASSERT(publish_iter != _publish_sockets.end());

    zmq::message message;
    message.enqueue(id);
    message.enqueue_protobuf_message(reply);
    return publish_iter->second.send(message);
}

}
}
