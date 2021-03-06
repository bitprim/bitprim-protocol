#include <bitcoin/protocol/requester_simple.hpp>

#include <boost/thread/latch.hpp>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/poller.hpp>
#include <bitcoin/protocol/zmq/zeromq.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace libbitcoin {
namespace protocol {

code requester_simple::simple_req_connect(const config::endpoint& address)
{
    return connect(address);
}
code requester_simple::simple_req_send(const google::protobuf::MessageLite& request, google::protobuf::MessageLite& reply)
{
    return send(request, reply);
}

requester_simple::requester_simple(zmq::context& context)
        : _context(context),
          _io_service(),
          _io_work(_io_service)
{}

requester_simple::requester_simple(zmq::context& context, const config::endpoint& address)
        : requester_simple(context)
{
    code ec = connect(address);
    if (ec) throw std::system_error(ec);
}

requester_simple::~requester_simple()
{
    disconnect();
}

requester_simple::operator const bool() const
{
    return _socket.is_initialized();
}

code requester_simple::connect(const config::endpoint& address)
{
    _io_service.reset();

    code ec;
    {
        boost::latch latch(2);

        _io_thread = asio::thread([&] {
            ec = do_connect(address);
            latch.count_down();

            zmq::poller poller;
            poller.add(*_subscriber_socket);
            while (!_io_service.stopped())
            {
                _io_service.poll();
            }
        });
        latch.count_down_and_wait();
    }
    return ec;
}

code requester_simple::disconnect()
{
    _io_service.stop();

    if (_io_thread.joinable())
        _io_thread.join();

    _socket = boost::none;

    _handlers.clear();
    _subscriber_socket = boost::none;
    _subscriber_endpoint.clear();

    return error::success;
}

code requester_simple::send(const google::protobuf::MessageLite& request,
                     google::protobuf::MessageLite& reply)
{
    BITCOIN_ASSERT(_socket);

    code ec;
    {
        boost::latch latch(2);
        _io_service.dispatch([&] () {
            ec = do_send(request, reply);
            latch.count_down();
        });
        latch.count_down_and_wait();
    }
    return ec;
}

code requester_simple::do_connect(const config::endpoint& address)
{
    _socket = boost::in_place(
            std::ref(_context), zmq::socket::role::requester);
    if (!*_socket)
        return zmq::get_last_error();

    code ec = _socket->connect(address);
    if (ec)
        return ec;

    _subscriber_socket = boost::in_place(
            std::ref(_context), zmq::socket::role::pair);
    if (!*_subscriber_socket)
        return zmq::get_last_error();

    ec = _subscriber_socket->bind({ "tcp://127.0.0.1:0" });
    if (ec)
        return ec;

    _subscriber_socket->get_last_endpoint(_subscriber_endpoint);
    _subscriber_endpoint = _subscriber_endpoint.substr(sizeof("tcp://")-1);

    return ec;
}

code requester_simple::do_send(const google::protobuf::MessageLite& request,
                        google::protobuf::MessageLite& reply)
{
    zmq::message message;
    message.enqueue_protobuf_message(request);

    code ec = _socket->send(message);
    if (ec) return ec;

    zmq::message response;
    ec = _socket->receive(response);
    if (ec) return ec;

    if (!response.dequeue(reply))
        return error::bad_stream;

    return error::success;
}

std::string requester_simple::add_handler(const std::string& message_name,
                                   handler_type handler)
{
    std::lock_guard<std::mutex> lock(_handlers_mutex);

    const std::string handler_id =
            message_name + '/' + std::to_string(++_next_handler_id);
    _handlers[handler_id] = std::move(handler);

    return _subscriber_endpoint + '/' + handler_id;
}

}}