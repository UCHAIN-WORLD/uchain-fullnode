/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
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
#ifndef UC_BLOCKCHAIN_ORGANIZER_HPP
#define UC_BLOCKCHAIN_ORGANIZER_HPP

#include <unordered_map>
#include <atomic>
#include <cstdint>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/blockchain/define.hpp>
#include <UChain/blockchain/block_detail.hpp>
#include <UChain/blockchain/orphan_pool.hpp>
#include <UChain/blockchain/settings.hpp>
#include <UChain/blockchain/simple_chain.hpp>
#include <UChain/coin/math/hash.hpp>
#include <boost/thread.hpp>

namespace libbitcoin
{
namespace blockchain
{

// TODO: This is not an interface, collapse with organizer_impl.

/// This class is thread safe, with the exception of organize().
/// Structure which organises the blocks from the orphan pool to the blockchain.
class BCB_API organizer
{
  public:
    typedef handle0 result_handler;
    typedef std::shared_ptr<organizer> ptr;
    typedef message::block_msg::ptr_list list;
    typedef resubscriber<const code &, uint64_t, const list &, const list &>
        reorganize_subscriber;
    typedef std::function<bool(const code &, uint64_t, const list &, const list &)>
        reorganize_handler;

    /// Construct an instance.
    organizer(threadpool &pool, simple_chain &chain, const settings &settings);

    /// This method is NOT thread safe.
    virtual void organize();

    virtual void start();
    virtual void stop();
    virtual bool add(block_detail::ptr block);
    virtual void subscribe_reorganize(reorganize_handler handler);
    virtual void filter_orphans(message::get_data::ptr message);

    void fired();
    std::unordered_map<hash_digest, uint64_t> get_fork_chain_last_block_hashes();
    void add_fork_chain_hash(const hash_digest &);
    void delete_fork_chain_hash(const hash_digest &);

  protected:
    virtual bool stopped();

  private:
    typedef block_detail::list detail_list;

    static uint64_t count_inputs(const chain::block &block);

    bool strict(uint64_t fork_point) const;

    /// These methods are NOT thread safe.
    virtual code verify(uint64_t fork_index,
                        const block_detail::list &orphan_chain, uint64_t orphan_index);
    void process(block_detail::ptr process_block);
    void replace_chain(uint64_t fork_index, detail_list &orphan_chain);
    void remove_processed(block_detail::ptr remove_block);
    void clip_orphans(detail_list &orphan_chain, uint64_t orphan_index,
                      const code &invalid_reason);

    /// This method is thread safe.
    void notify_reorganize(uint64_t fork_point,
                           const detail_list &orphan_chain, const detail_list &replaced_chain);

    std::atomic<bool> stopped_;
    const bool use_testnet_rules_;
    const config::checkpoint::list checkpoints_;

    // These are protected by the caller protecting organize().
    simple_chain &chain_;
    block_detail::list process_queue_;

    // These are thread safe.
    orphan_pool orphan_pool_;
    reorganize_subscriber::ptr subscriber_;
    std::unordered_map<hash_digest, uint64_t> fork_chain_last_block_hashes_;
    boost::mutex mutex_fork_chain_last_block_hashes_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
