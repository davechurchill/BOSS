#include "CombatSearch_BucketData.h"

using namespace BOSS;


// Combat Search Bucketing
// 
// Computes and stores the build order which maximizes an evaluation function up to a given time interval [t0-t1]
// The number of buckets and the frame limit determine the size of the buckets


CombatSearch_BucketData::CombatSearch_BucketData(const int frameLimit, const size_t numBuckets)
        : m_buckets(numBuckets, BucketData())
        , m_frameLimit(frameLimit)
{
    
}

const size_t CombatSearch_BucketData::numBuckets() const
{
    return m_buckets.size();
}

const size_t CombatSearch_BucketData::getBucketIndex(const GameState & state) const
{
    return (size_t)(((double)state.getCurrentFrame() / (double)m_frameLimit) * m_buckets.size());
}

void CombatSearch_BucketData::update(const GameState & state, const BuildOrder & buildOrder)
{
    if (state.getCurrentFrame() >= m_frameLimit)
    {
        return;
    }

    BOSS_ASSERT(state.getCurrentFrame() <= m_frameLimit, "State's frame exceeds bucket frame limit: (%d %d)", (int)state.getCurrentFrame(), (int)m_frameLimit);

    // get the bucket index corresponding to the time of the current state finishing
    size_t bucketIndex = getBucketIndex(state);

    // evaluate the state with whatever value we want
    double eval = Eval::ArmyTotalResourceSum(state);

    // update the data if we have a new best value for this bucket
    BucketData & bucket = m_buckets[bucketIndex];
    if ((eval > bucket.eval) || ((eval == bucket.eval) && Eval::BuildOrderBetter(buildOrder, bucket.buildOrder)))
    {
        // update every bucket for which this is a new record
        for (size_t b=bucketIndex; b < m_buckets.size(); ++b)
        {
            if (m_buckets[b].eval >= eval)
            {
                break;
            }

            m_buckets[b].eval = eval;
            m_buckets[b].buildOrder = buildOrder;
            m_buckets[b].state = state;
        }
    }
}

bool CombatSearch_BucketData::isDominated(const GameState & state)
{
    return Eval::StateDominates(getBucketData(state).state, state);
}

BucketData & CombatSearch_BucketData::getBucketData(const GameState & state)
{
    BOSS_ASSERT(getBucketIndex(state) < m_buckets.size(), "State goes over bucket limit");

    return m_buckets[getBucketIndex(state)];
}

void CombatSearch_BucketData::print() const
{
    std::cout << "\n\nFinal CombatBucket results\n";
    std::cout << "\n  Frame     Sec     ArmyEval   BuildOrder\n";
    double maxEval = 0;

    for (size_t b(0); b<m_buckets.size(); ++b)
    {
        if (m_buckets[b].eval > maxEval)
        {
            maxEval = m_buckets[b].eval;

            double frame = ((double)b / m_buckets.size()) * m_frameLimit;
            double sec   = frame / 24;

            printf("%7d %7d %12.2lf   ", (int)frame, (int)sec, m_buckets[b].eval);
            std::cout << m_buckets[b].buildOrder.getNameString(2) << std::endl;
        }
    }
}

const BucketData & CombatSearch_BucketData::getBucket(const size_t index) const
{
    return m_buckets[index];
}

std::string CombatSearch_BucketData::getBucketResultsString()
{
    std::stringstream ss;

    ss << "0 0" << std::endl;

    double maxEval = 0;
    for (size_t b(0); b<m_buckets.size(); ++b)
    {
        if (m_buckets[b].eval > maxEval)
        {
            maxEval = m_buckets[b].eval;

            double frame = ((double)b / m_buckets.size()) * m_frameLimit;
            double sec   = frame / 24;

            ss << frame << " " << m_buckets[b].eval << std::endl;
        }
    }

    return ss.str();
}