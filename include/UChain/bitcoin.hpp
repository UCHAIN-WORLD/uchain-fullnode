///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin developers (see COPYING).
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

#include <UChain/bitcoin/compat.h>
#include <UChain/bitcoin/compat.hpp>
#include <UChain/bitcoin/constants.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/error.hpp>
#include <UChain/bitcoin/handlers.hpp>
#include <UChain/bitcoin/messages.hpp>
#include <UChain/bitcoin/version.hpp>
#include <UChain/bitcoin/chain/block.hpp>
#include <UChain/bitcoin/chain/header.hpp>
#include <UChain/bitcoin/chain/history.hpp>
#include <UChain/bitcoin/chain/input.hpp>
#include <UChain/bitcoin/chain/output.hpp>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/point_iterator.hpp>
#include <UChain/bitcoin/chain/spend.hpp>
#include <UChain/bitcoin/chain/stealth.hpp>
#include <UChain/bitcoin/chain/transaction.hpp>
#include <UChain/bitcoin/chain/script/opcode.hpp>
#include <UChain/bitcoin/chain/script/operation.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/config/authority.hpp>
#include <UChain/bitcoin/config/base16.hpp>
#include <UChain/bitcoin/config/base2.hpp>
#include <UChain/bitcoin/config/base58.hpp>
#include <UChain/bitcoin/config/base64.hpp>
#include <UChain/bitcoin/config/checkpoint.hpp>
#include <UChain/bitcoin/config/directory.hpp>
#include <UChain/bitcoin/config/endpoint.hpp>
#include <UChain/bitcoin/config/hash160.hpp>
#include <UChain/bitcoin/config/hash256.hpp>
#include <UChain/bitcoin/config/parameter.hpp>
#include <UChain/bitcoin/config/parser.hpp>
#include <UChain/bitcoin/config/printer.hpp>
#include <UChain/bitcoin/config/sodium.hpp>
#include <UChain/bitcoin/formats/base_10.hpp>
#include <UChain/bitcoin/formats/base_16.hpp>
#include <UChain/bitcoin/formats/base_58.hpp>
#include <UChain/bitcoin/formats/base_64.hpp>
#include <UChain/bitcoin/formats/base_85.hpp>
#include <UChain/bitcoin/math/checksum.hpp>
#include <UChain/bitcoin/math/crypto.hpp>
#include <UChain/bitcoin/math/elliptic_curve.hpp>
#include <UChain/bitcoin/math/hash.hpp>
#include <UChain/bitcoin/math/hash_number.hpp>
#include <UChain/bitcoin/math/script_number.hpp>
#include <UChain/bitcoin/math/stealth.hpp>
#include <UChain/bitcoin/math/uint256.hpp>
#include <UChain/bitcoin/message/address.hpp>
#include <UChain/bitcoin/message/block_message.hpp>
#include <UChain/bitcoin/message/block_transactions.hpp>
#include <UChain/bitcoin/message/compact_block.hpp>
#include <UChain/bitcoin/message/fee_filter.hpp>
#include <UChain/bitcoin/message/filter_add.hpp>
#include <UChain/bitcoin/message/filter_clear.hpp>
#include <UChain/bitcoin/message/filter_load.hpp>
#include <UChain/bitcoin/message/get_address.hpp>
#include <UChain/bitcoin/message/get_block_transactions.hpp>
#include <UChain/bitcoin/message/get_blocks.hpp>
#include <UChain/bitcoin/message/get_data.hpp>
#include <UChain/bitcoin/message/get_headers.hpp>
#include <UChain/bitcoin/message/header_message.hpp>
#include <UChain/bitcoin/message/headers.hpp>
#include <UChain/bitcoin/message/heading.hpp>
#include <UChain/bitcoin/message/inventory.hpp>
#include <UChain/bitcoin/message/inventory_vector.hpp>
#include <UChain/bitcoin/message/memory_pool.hpp>
#include <UChain/bitcoin/message/merkle_block.hpp>
#include <UChain/bitcoin/message/network_address.hpp>
#include <UChain/bitcoin/message/not_found.hpp>
#include <UChain/bitcoin/message/ping.hpp>
#include <UChain/bitcoin/message/pong.hpp>
#include <UChain/bitcoin/message/prefilled_transaction.hpp>
#include <UChain/bitcoin/message/reject.hpp>
#include <UChain/bitcoin/message/send_compact_blocks.hpp>
#include <UChain/bitcoin/message/send_headers.hpp>
#include <UChain/bitcoin/message/transaction_message.hpp>
#include <UChain/bitcoin/message/verack.hpp>
#include <UChain/bitcoin/message/version.hpp>
#include <UChain/bitcoin/unicode/console_streambuf.hpp>
#include <UChain/bitcoin/unicode/ifstream.hpp>
#include <UChain/bitcoin/unicode/ofstream.hpp>
#include <UChain/bitcoin/unicode/unicode.hpp>
#include <UChain/bitcoin/unicode/unicode_istream.hpp>
#include <UChain/bitcoin/unicode/unicode_ostream.hpp>
#include <UChain/bitcoin/unicode/unicode_streambuf.hpp>
#include <UChain/bitcoin/utility/array_slice.hpp>
#include <UChain/bitcoin/utility/asio.hpp>
#include <UChain/bitcoin/utility/assert.hpp>
#include <UChain/bitcoin/utility/atomic.hpp>
#include <UChain/bitcoin/utility/binary.hpp>
#include <UChain/bitcoin/utility/collection.hpp>
#include <UChain/bitcoin/utility/color.hpp>
#include <UChain/bitcoin/utility/conditional_lock.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/data.hpp>
#include <UChain/bitcoin/utility/deadline.hpp>
#include <UChain/bitcoin/utility/decorator.hpp>
#include <UChain/bitcoin/utility/delegates.hpp>
#include <UChain/bitcoin/utility/deserializer.hpp>
#include <UChain/bitcoin/utility/dispatcher.hpp>
#include <UChain/bitcoin/utility/enable_shared_from_base.hpp>
#include <UChain/bitcoin/utility/endian.hpp>
#include <UChain/bitcoin/utility/exceptions.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/log.hpp>
#include <UChain/bitcoin/utility/logging.hpp>
#include <UChain/bitcoin/utility/monitor.hpp>
#include <UChain/bitcoin/utility/notifier.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>
#include <UChain/bitcoin/utility/png.hpp>
#include <UChain/bitcoin/utility/random.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/resource_lock.hpp>
#include <UChain/bitcoin/utility/resubscriber.hpp>
#include <UChain/bitcoin/utility/scope_lock.hpp>
#include <UChain/bitcoin/utility/serializer.hpp>
#include <UChain/bitcoin/utility/string.hpp>
#include <UChain/bitcoin/utility/subscriber.hpp>
#include <UChain/bitcoin/utility/synchronizer.hpp>
#include <UChain/bitcoin/utility/thread.hpp>
#include <UChain/bitcoin/utility/threadpool.hpp>
#include <UChain/bitcoin/utility/timer.hpp>
#include <UChain/bitcoin/utility/track.hpp>
#include <UChain/bitcoin/utility/variable_uint_size.hpp>
#include <UChain/bitcoin/utility/work.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <UChain/bitcoin/wallet/bitcoin_uri.hpp>
#include <UChain/bitcoin/wallet/dictionary.hpp>
#include <UChain/bitcoin/wallet/ec_private.hpp>
#include <UChain/bitcoin/wallet/ec_public.hpp>
#include <UChain/bitcoin/wallet/ek_private.hpp>
#include <UChain/bitcoin/wallet/ek_public.hpp>
#include <UChain/bitcoin/wallet/ek_token.hpp>
#include <UChain/bitcoin/wallet/encrypted_keys.hpp>
#include <UChain/bitcoin/wallet/hd_private.hpp>
#include <UChain/bitcoin/wallet/hd_public.hpp>
#include <UChain/bitcoin/wallet/message.hpp>
#include <UChain/bitcoin/wallet/mini_keys.hpp>
#include <UChain/bitcoin/wallet/mnemonic.hpp>
#include <UChain/bitcoin/wallet/payment_address.hpp>
#include <UChain/bitcoin/wallet/qrcode.hpp>
#include <UChain/bitcoin/wallet/select_outputs.hpp>
#include <UChain/bitcoin/wallet/settings.hpp>
#include <UChain/bitcoin/wallet/stealth_address.hpp>
#include <UChain/bitcoin/wallet/uri.hpp>
#include <UChain/bitcoin/wallet/uri_reader.hpp>

#endif
