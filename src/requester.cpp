
#include <bitcoin/protocol/requester.hpp>

#include <mutex>
#include <functional>
#include <string>
#include <system_error>
#include <boost/thread/latch.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <google/protobuf/message_lite.h>
#include <bitcoin/protocol/interface.pb.h>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/poller.hpp>
#include <bitcoin/protocol/zmq/zeromq.hpp>

namespace libbitcoin {
namespace protocol {

requester::requester(zmq::context& context)
  : _context(context),
    _io_service(),
    _io_work(_io_service),
    _handlers_service(),
    _handlers_work(_handlers_service)
{}

requester::requester(zmq::context& context, const config::endpoint& address)
  : requester(context)
{
    code ec = connect(address);
    if (ec) throw std::system_error(ec);
}

requester::~requester()
{
    disconnect();
}

requester::operator const bool() const
{
    return _socket.is_initialized();
}

code requester::connect(const config::endpoint& address)
{
    _io_service.reset();

    _handlers_service.reset();
    _handlers_thread = asio::thread([&] {
        _handlers_service.run();
    });

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
                while (_io_service.poll()) {}

                auto const& ids = poller.wait(100); // ms
                if (ids.contains(_subscriber_socket->id()))
                {
                    zmq::message message;
                    _subscriber_socket->receive(message);

                    std::string const id = message.dequeue_text();
                    data_chunk const payload = message.dequeue_data();
                    call_handler(id, payload);
                }
            }
        });
        latch.count_down_and_wait();
    }
    return ec;
}

void requester::call_handler(const std::string& id,
     const data_chunk& payload)
{
    std::function<code(const data_chunk&)> callback;
    {
        std::lock_guard<std::mutex> lock(_handlers_mutex);
        auto handler_iter = _handlers.find(id);
        BITCOIN_ASSERT(handler_iter != _handlers.end());

        auto& handler = handler_iter->second;
        if (handler.single)
        {
            callback = std::move(handler_iter->second.function);
            _handlers.erase(handler_iter);
        } else {
            void_reply end_message;
            if (end_message.ParseFromArray(payload.data(), payload.size()))
            {
                _handlers.erase(handler_iter);
            } else {
                callback = std::ref(handler_iter->second.function);
            }
        }
    }

    _handlers_service.dispatch([=]{
        callback(payload);
    });
}

code requester::disconnect()
{
    _io_service.stop();
    if (_io_thread.joinable())
        _io_thread.join();

    _socket = boost::none;

    _handlers_service.stop();
    if (_handlers_thread.joinable())
        _handlers_thread.join();
    _handlers.clear();
    _subscriber_socket = boost::none;
    _subscriber_endpoint.clear();

    return error::success;
}

code requester::send(const google::protobuf::MessageLite& request,
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

code requester::do_connect(const config::endpoint& address)
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

code requester::do_send(const google::protobuf::MessageLite& request,
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

std::string requester::add_handler(const std::string& message_name,
    handler_type handler)
{
    std::lock_guard<std::mutex> lock(_handlers_mutex);

    const std::string handler_id =
        message_name + '/' + std::to_string(++_next_handler_id);
    _handlers[handler_id] = std::move(handler);

    return _subscriber_endpoint + '/' + handler_id;
}

}
}
