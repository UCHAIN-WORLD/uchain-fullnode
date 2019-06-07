///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2018 libbitcoin developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UC_BITCOIN_HPP
#define UC_BITCOIN_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <UChain/coin/compat.h>
#include <UChain/coin/compat.hpp>
#include <UChain/coin/constants.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/error.hpp>
#include <UChain/coin/handlers.hpp>
#include <UChain/coin/messages.hpp>
#include <UChain/coin/version.hpp>
#include <UChain/coin/chain/block.hpp>
#include <UChain/coin/chain/header.hpp>
#include <UChain/coin/chain/history.hpp>
#include <UChain/coin/chain/input.hpp>
#include <UChain/coin/chain/output.hpp>
#include <UChain/coin/chain/point.hpp>
#include <UChain/coin/chain/point_iterator.hpp>
#include <UChain/coin/chain/spend.hpp>
#include <UChain/coin/chain/stealth.hpp>
#include <UChain/coin/chain/transaction.hpp>
#include <UChain/coin/chain/script/opcode.hpp>
#include <UChain/coin/chain/script/operation.hpp>
#include <UChain/coin/chain/script/script.hpp>
#include <UChain/coin/config/authority.hpp>
#include <UChain/coin/config/base16.hpp>
#include <UChain/coin/config/base2.hpp>
#include <UChain/coin/config/base58.hpp>
#include <UChain/coin/config/base64.hpp>
#include <UChain/coin/config/checkpoint.hpp>
#include <UChain/coin/config/directory.hpp>
#include <UChain/coin/config/endpoint.hpp>
#include <UChain/coin/config/hash160.hpp>
#include <UChain/coin/config/hash256.hpp>
#include <UChain/coin/config/parameter.hpp>
#include <UChain/coin/config/parser.hpp>
#include <UChain/coin/config/printer.hpp>
#include <UChain/coin/config/sodium.hpp>
#include <UChain/coin/formats/base_10.hpp>
#include <UChain/coin/formats/base_16.hpp>
#include <UChain/coin/formats/base_58.hpp>
#include <UChain/coin/formats/base_64.hpp>
#include <UChain/coin/formats/base_85.hpp>
#include <UChain/coin/math/checksum.hpp>
#include <UChain/coin/math/crypto.hpp>
#include <UChain/coin/math/elliptic_curve.hpp>
#include <UChain/coin/math/hash.hpp>
#include <UChain/coin/math/hash_number.hpp>
#include <UChain/coin/math/script_number.hpp>
#include <UChain/coin/math/stealth.hpp>
#include <UChain/coin/math/uint256.hpp>
#include <UChain/coin/message/address.hpp>
#include <UChain/coin/message/block_msg.hpp>
#include <UChain/coin/message/block_transactions.hpp>
#include <UChain/coin/message/compact_block.hpp>
#include <UChain/coin/message/fee_filter.hpp>
#include <UChain/coin/message/filter_add.hpp>
#include <UChain/coin/message/filter_clear.hpp>
#include <UChain/coin/message/filter_load.hpp>
#include <UChain/coin/message/get_address.hpp>
#include <UChain/coin/message/get_block_transactions.hpp>
#include <UChain/coin/message/get_blocks.hpp>
#include <UChain/coin/message/get_data.hpp>
#include <UChain/coin/message/get_headers.hpp>
#include <UChain/coin/message/header_message.hpp>
#include <UChain/coin/message/headers.hpp>
#include <UChain/coin/message/heading.hpp>
#include <UChain/coin/message/inventory.hpp>
#include <UChain/coin/message/inventory_vector.hpp>
#include <UChain/coin/message/memory_pool.hpp>
#include <UChain/coin/message/merkle_block.hpp>
#include <UChain/coin/message/network_address.hpp>
#include <UChain/coin/message/not_found.hpp>
#include <UChain/coin/message/ping.hpp>
#include <UChain/coin/message/pong.hpp>
#include <UChain/coin/message/prefilled_transaction.hpp>
#include <UChain/coin/message/reject.hpp>
#include <UChain/coin/message/send_compact_blocks.hpp>
#include <UChain/coin/message/send_headers.hpp>
#include <UChain/coin/message/transaction_message.hpp>
#include <UChain/coin/message/verack.hpp>
#include <UChain/coin/message/version.hpp>
#include <UChain/coin/unicode/console_streambuf.hpp>
#include <UChain/coin/unicode/ifstream.hpp>
#include <UChain/coin/unicode/ofstream.hpp>
#include <UChain/coin/unicode/unicode.hpp>
#include <UChain/coin/unicode/unicode_istream.hpp>
#include <UChain/coin/unicode/unicode_ostream.hpp>
#include <UChain/coin/unicode/unicode_streambuf.hpp>
#include <UChain/coin/utility/array_slice.hpp>
#include <UChain/coin/utility/asio.hpp>
#include <UChain/coin/utility/assert.hpp>
#include <UChain/coin/utility/atomic.hpp>
#include <UChain/coin/utility/binary.hpp>
#include <UChain/coin/utility/collection.hpp>
#include <UChain/coin/utility/color.hpp>
#include <UChain/coin/utility/conditional_lock.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/deadline.hpp>
#include <UChain/coin/utility/decorator.hpp>
#include <UChain/coin/utility/delegates.hpp>
#include <UChain/coin/utility/deserializer.hpp>
#include <UChain/coin/utility/dispatcher.hpp>
#include <UChain/coin/utility/enable_shared_from_base.hpp>
#include <UChain/coin/utility/endian.hpp>
#include <UChain/coin/utility/exceptions.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/log.hpp>
#include <UChain/coin/utility/logging.hpp>
#include <UChain/coin/utility/monitor.hpp>
#include <UChain/coin/utility/notifier.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChain/coin/utility/png.hpp>
#include <UChain/coin/utility/random.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/resource_lock.hpp>
#include <UChain/coin/utility/resubscriber.hpp>
#include <UChain/coin/utility/scope_lock.hpp>
#include <UChain/coin/utility/serializer.hpp>
#include <UChain/coin/utility/string.hpp>
#include <UChain/coin/utility/subscriber.hpp>
#include <UChain/coin/utility/synchronizer.hpp>
#include <UChain/coin/utility/thread.hpp>
#include <UChain/coin/utility/threadpool.hpp>
#include <UChain/coin/utility/timer.hpp>
#include <UChain/coin/utility/track.hpp>
#include <UChain/coin/utility/variable_uint_size.hpp>
#include <UChain/coin/utility/work.hpp>
#include <UChain/coin/utility/writer.hpp>
#include <UChain/coin/wallet/bitcoin_uri.hpp>
#include <UChain/coin/wallet/dictionary.hpp>
#include <UChain/coin/wallet/ec_private.hpp>
#include <UChain/coin/wallet/ec_public.hpp>
#include <UChain/coin/wallet/ek_private.hpp>
#include <UChain/coin/wallet/ek_public.hpp>
#include <UChain/coin/wallet/ek_token.hpp>
#include <UChain/coin/wallet/encrypted_keys.hpp>
#include <UChain/coin/wallet/hd_private.hpp>
#include <UChain/coin/wallet/hd_public.hpp>
#include <UChain/coin/wallet/message.hpp>
#include <UChain/coin/wallet/mini_keys.hpp>
#include <UChain/coin/wallet/mnemonic.hpp>
#include <UChain/coin/wallet/payment_address.hpp>
#include <UChain/coin/wallet/qrcode.hpp>
#include <UChain/coin/wallet/select_outputs.hpp>
#include <UChain/coin/wallet/settings.hpp>
#include <UChain/coin/wallet/stealth_address.hpp>
#include <UChain/coin/wallet/uri.hpp>
#include <UChain/coin/wallet/uri_reader.hpp>

#endif
