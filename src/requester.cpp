
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

requester::requester(zmq::context& context)
  : _context(context),
    _io_service(),
    _io_work(_io_service)
{}

requester::requester(zmq::context& context, const config::endpoint& address)
  : requester(context)
{
    code ec = connect(address);
    if (ec) throw std::system_error(ec);
}

requester::~requester()
{
    _io_service.stop();

    if (_io_thread.joinable())
        _io_thread.join();
}

requester::operator const bool() const
{
    return _socket.is_initialized();
}

code requester::connect(const config::endpoint& address)
{
    code ec;
    {
        boost::latch latch(1);
        _io_thread = asio::thread([&] {
            _socket = boost::in_place(std::ref(_context), zmq::socket::role::requester);
            if (!*_socket)
                ec = zmq::get_last_error();

            if (!ec)
                ec = _socket->connect(address);

            latch.count_down();

            _io_service.run();
        });
        latch.wait();
    }
    return ec;
}

code requester::send(const google::protobuf::MessageLite& request,
    google::protobuf::MessageLite& reply)
{
    BITCOIN_ASSERT(_socket);

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

code requester::send_recv(const google::protobuf::MessageLite& request,
    google::protobuf::MessageLite& reply)
{
    zmq::message message;
    message.enqueue(request.SerializeAsString());

    code ec = _socket->send(message);
    if (ec) return ec;

    zmq::message response;
    ec = _socket->receive(response);
    if (ec) return ec;

    data_chunk payload;
    response.dequeue(payload);
    if (!reply.ParseFromArray(payload.data(), static_cast<int>(payload.size())))
        return error::bad_stream;

    return error::success;
}

}
}
