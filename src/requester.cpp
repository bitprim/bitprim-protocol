
#include <bitcoin/protocol/requester.hpp>

#include <functional>
#include <system_error>
#include <boost/thread/latch.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <google/protobuf/message_lite.h>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/zeromq.hpp>

namespace libbitcoin {
namespace protocol {

requester::requester(zmq::context& context, const config::endpoint& address)
  : _context(context),
    _io_service(),
    _io_work(_io_service)
{
    code ec;
    {
        boost::latch latch(1);
        _io_thread = asio::thread([&] {
            ec = connect(context, address);
            latch.count_down();

            _io_service.run();
        });
        latch.wait();
    }
    if (ec) throw std::system_error(ec);
}

requester::~requester()
{
    _io_service.stop();

    if (_io_thread.joinable())
        _io_thread.join();
}

code requester::send(const google::protobuf::MessageLite& request,
    google::protobuf::MessageLite& reply)
{
    code ec;
    {
        boost::latch latch(1);
        _io_service.dispatch([&] () {
            ec = send_recv(request, reply);
            latch.count_down();
        });
        latch.wait();
    }
    return ec;
}

code requester::connect(zmq::context& context, const config::endpoint& address)
{
    _socket = boost::in_place(std::ref(context), zmq::socket::role::requester);
    if (!*_socket)
        return zmq::get_last_error();

    return _socket->connect(address);
}

code requester::send_recv(const google::protobuf::MessageLite& request,
    google::protobuf::MessageLite& reply)
{
    BITCOIN_ASSERT(_socket);

    zmq::message message;
    message.enqueue(request.SerializeAsString());

    code ec = _socket->send(message);
    if (ec) return ec;

    zmq::message response;
    ec = _socket->receive(response);
    if (ec) return ec;

    data_chunk payload;
    response.dequeue(payload);
    reply.ParseFromArray(payload.data(), static_cast<int>(payload.size()));

    return error::success;
}

}
}
