#pragma once
#include <Interpreters/TransactionVersionMetadata.h>
#include <boost/noncopyable.hpp>
#include <Storages/IStorage_fwd.h>
#include <Common/Stopwatch.h>

#include <list>
#include <unordered_set>

namespace DB
{

class IMergeTreeDataPart;
using DataPartPtr = std::shared_ptr<const IMergeTreeDataPart>;
using DataPartsVector = std::vector<DataPartPtr>;

class MergeTreeTransaction : public std::enable_shared_from_this<MergeTreeTransaction>, private boost::noncopyable
{
    friend class TransactionLog;
public:
    enum State
    {
        RUNNING,
        COMMITTED,
        ROLLED_BACK,
    };

    Snapshot getSnapshot() const { return snapshot; }
    State getState() const;

    const TransactionID tid;

    MergeTreeTransaction(Snapshot snapshot_, LocalTID local_tid_, UUID host_id);

    void addNewPart(const StoragePtr & storage, const DataPartPtr & new_part);
    void removeOldPart(const StoragePtr & storage, const DataPartPtr & part_to_remove, const TransactionInfoContext & context);

    void addMutation(const StoragePtr & table, const String & mutation_id);

    static void addNewPart(const StoragePtr & storage, const DataPartPtr & new_part, MergeTreeTransaction * txn);
    static void removeOldPart(const StoragePtr & storage, const DataPartPtr & part_to_remove, MergeTreeTransaction * txn);
    static void addNewPartAndRemoveCovered(const StoragePtr & storage, const DataPartPtr & new_part, const DataPartsVector & covered_parts, MergeTreeTransaction * txn);

    bool isReadOnly() const;

    void onException();

    String dumpDescription() const;

    Float64 elapsedSeconds() const { return elapsed.elapsedSeconds(); }

private:
    void beforeCommit();
    void afterCommit(CSN assigned_csn) noexcept;
    bool rollback() noexcept;
    void checkIsNotCancelled() const;

    mutable std::mutex mutex;
    Stopwatch elapsed;

    Snapshot snapshot;

    std::unordered_set<StoragePtr> storages;
    DataPartsVector creating_parts;
    DataPartsVector removing_parts;

    std::atomic<CSN> csn;

    std::list<Snapshot>::iterator snapshot_in_use_it;

    using RunningMutationsList = std::vector<std::pair<StoragePtr, String>>;
    RunningMutationsList mutations;
};

using MergeTreeTransactionPtr = std::shared_ptr<MergeTreeTransaction>;

}
