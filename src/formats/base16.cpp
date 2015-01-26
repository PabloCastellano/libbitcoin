/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/bitcoin/formats/base16.hpp>

#include <algorithm>
#include <ctype.h>
#include <iomanip>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {

std::string encode_base16(data_slice data)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int val: data)
        ss << std::setw(2) << val;
    return ss.str();
}

static unsigned from_hex(const char c)
{
    if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    return c - '0';
}

bool decode_base16_private(uint8_t* out, size_t out_size, const char* in)
{
    if (!std::all_of(in, in + 2 * out_size, isxdigit))
        return false;

    for (size_t i = 0; i < out_size; ++i)
    {
        out[i] = (from_hex(in[0]) << 4) + from_hex(in[1]);
        in += 2;
    }
    return true;
}

bool decode_base16(data_chunk& out, const std::string& in)
{
    // This prevents a last odd character from being ignored:
    if (in.size() % 2 != 0)
        return false;

    data_chunk result(in.size() / 2);
    if (!decode_base16_private(result.data(), result.size(), in.data()))
        return false;

    out = result;
    return true;
}

// Bitcoin hash format (these are all reversed):
std::string encode_hash(hash_digest hash)
{
    std::reverse(hash.begin(), hash.end());
    return encode_base16(hash);
}

bool decode_hash(hash_digest& out, const std::string& in)
{
    if (in.size() != 2 * hash_size)
        return false;

    hash_digest result;
    if (!decode_base16_private(result.data(), result.size(), in.data()))
        return false;

    // Reverse:
    std::reverse_copy(result.begin(), result.end(), out.begin());
    return true;
}

hash_digest hash_literal(const char (&string)[2 * hash_size + 1])
{
    hash_digest out;
    DEBUG_ONLY(const auto success =) decode_base16_private(out.data(),
        out.size(), string);
    BITCOIN_ASSERT(success);
    std::reverse(out.begin(), out.end());
    return out;
}

// Old names for backwards-compatibility:
std::string encode_hex(data_slice in)
{
    return encode_base16(in);
}

data_chunk decode_hex(std::string in)
{
    // Trim the fat:
    boost::algorithm::trim(in);

    data_chunk out;
    decode_base16(out, in);
    return out;
}

} // namespace libbitcoin

