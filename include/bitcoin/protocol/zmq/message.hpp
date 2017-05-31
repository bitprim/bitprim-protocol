/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_PROTOCOL_ZMQ_MESSAGE_HPP
#define LIBBITCOIN_PROTOCOL_ZMQ_MESSAGE_HPP

#include <cstddef>
#include <string>
#include <google/protobuf/message_lite.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is not thread safe.
class BCP_API message
{
public:
    /// A zeromq route identifier is always this size.
    static const size_t address_size = 5;

    /// An identifier for message routing.
    typedef byte_array<address_size> address;

    /// Add an empty message part to the outgoing message.
    void enqueue();

    /// Move a data message part to the outgoing message.
    void enqueue(data_chunk&& value);

    /// Add a data message part to the outgoing message.
    void enqueue(const data_chunk& value);

    /// Add a text message part to the outgoing message.
    void enqueue(const std::string& value);

    /// Move an identifier message part to the outgoing message.
    void enqueue(const address& value);

    /// Add a protobuf message part to the outgoing message.
    bool enqueue_protobuf_message(const google::protobuf::MessageLite& value);

    /// Add a message part to the outgoing message.
    template <typename Unsigned>
    void enqueue_little_endian(Unsigned value)
    {
        queue_.emplace(to_chunk(to_little_endian<Unsigned>(value)));
    }

    /// Remove a message part from the top of the queue, empty if empty queue.
    data_chunk dequeue_data();
    std::string dequeue_text();

    /// Remove a part from the queue top, false if empty queue or invalid.
    bool dequeue();
    bool dequeue(uint32_t& value);
    bool dequeue(data_chunk& value);
    bool dequeue(std::string& value);
    bool dequeue(hash_digest& value);
<<<<<<< HEAD
    bool dequeue(google::protobuf::MessageLite& value);
=======
    bool dequeue(address& value);
>>>>>>> v3.2.0

    /// Clear the queue of message parts.
    void clear();

    /// True if the queue is empty.
    bool empty() const;

    /// The number of items on the queue.
    size_t size() const;

    /// Must be called on the socket thread.
    /// Send the message in parts. If a send fails the unsent parts remain.
    code send(socket& socket);

    /// Must be called on the socket thread.
    /// Receve a message (clears the queue first).
    code receive(socket& socket);

protected:
    data_queue queue_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif

