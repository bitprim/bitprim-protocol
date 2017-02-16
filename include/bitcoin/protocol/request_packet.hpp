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
#ifndef LIBBITCOIN_PROTOCOL_REQUEST_PACKET
#define LIBBITCOIN_PROTOCOL_REQUEST_PACKET

#include <memory>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/define.hpp>
#include <bitcoin/protocol/interface.pb.h>
#include <bitcoin/protocol/packet.hpp>
#include <bitcoin/protocol/zmq/message.hpp>

namespace libbitcoin {
namespace protocol {

class BCP_API request_packet
  : public packet
{
public:
    request_packet();

    std::shared_ptr<request> get_request() const;
    void set_request(std::shared_ptr<request> request);

protected:
    virtual bool encode_payload(zmq::message& message) const;
    virtual bool decode_payload(const data_chunk& payload);

private:
    std::shared_ptr<request> request_;
};

}
}

#endif
