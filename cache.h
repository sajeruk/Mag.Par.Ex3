#pragma once

#include "treap.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

struct TStats {
    size_t CacheMisses = 0;
    float Fraction;
    size_t MaxSeries = 0;
    typedef std::map<size_t, size_t> map;

    map SeriesIfInCache;
    map SeriesIfOutOfCache;
    map Series;

    static void MergeMap(map& m1, const map& m2) {
        for (const auto& entry : m2) {
            m1[entry.first] += entry.second;
        }
    }

    TStats& operator += (const TStats& stats) {
        CacheMisses += stats.CacheMisses;
        MergeMap(SeriesIfInCache, stats.SeriesIfInCache);
        MergeMap(SeriesIfOutOfCache, stats.SeriesIfOutOfCache);
        MergeMap(Series, stats.Series);
        return *this;
    }

    void OutStats(std::ostream& os) const {
        os << "Count (hits)" << std::endl;
        OutHm(os, SeriesIfInCache, MaxSeries);
        os << "Count (misses)" << std::endl;
        OutHm(os, SeriesIfOutOfCache, MaxSeries);
        os << "Count (total)" << std::endl;
        OutHm(os, Series, MaxSeries);
    }

    static void OutHm(std::ostream& os, const map& m, size_t max) {
        for (const auto& item : m) {
            if (max && item.first > max) {
                break;
            }
            os << "\t" << item.first << " -> " << item.second << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, TStats& st);
};

std::ostream& operator<<(std::ostream& os, const TStats& st) {
    os << "Cache Misses: " << st.CacheMisses << " (" << (st.Fraction * 100.f) << "%)" << std::endl;
    st.OutStats(os);
    return os;
}

//__declspec(align(16))
// __restrict
class TCacheEmulator {
public:
    enum ECacheType {
        RANDOM = 0,
        LA_FO = 1
    };

public:
    TCacheEmulator(
        size_t cacheSize,
        size_t channelCount,
        size_t lineSize,
        ECacheType type = LA_FO)
            : CacheSize(cacheSize)
            , ChannelCount(channelCount)
            , LineSize(lineSize)
            , Type(type)
            , TickNumber(0)
    {
        OffsetBits = size_t(ceil(log2f(float(lineSize))));
        const size_t realLineSize = 1 << OffsetBits;
        const size_t indexSize = cacheSize / (channelCount * realLineSize);
        const size_t indexesBits = (size_t)log2f(float(indexSize));
        const size_t realIndexSize = (1 << indexesBits);
        Index.resize(realIndexSize, TIndex(channelCount, type));

        TagMask = (~0 << (OffsetBits + indexesBits));
        IndexMask = (~0 << OffsetBits) & ~TagMask;
    }

    void Access(const float* a) {
        ++TickNumber;
        size_t index, tag;
        GetAddressData(a, index, tag);
        Index[index].Access(TickNumber, tag);
    }

    TStats GetStats(size_t maxSeries) const {
        TStats stats;
        for (const auto& ind : Index) {
            stats += ind.IndexStats;
        }
        stats.MaxSeries = maxSeries;
        stats.Fraction = float(stats.CacheMisses) / TickNumber;
        return stats;
    }

private:
    class TIndex {
        struct TChannel {
            size_t Tag = 1;
            size_t LastAccessTime = 0;
            TChannel() {
            }

            TChannel(size_t tag, size_t lat)
                : Tag(tag)
                , LastAccessTime(lat)
            {
            }
        };

        struct TGreater {
            bool operator () (const TChannel& f, const TChannel& s) {
                return f.LastAccessTime > s.LastAccessTime;
            }
        };

    public:
        TIndex(size_t channelCount, ECacheType type)
            : CacheType(type)
            , ChannelCount(channelCount)
        {
        }

        void Access(size_t tick, size_t tag) {
            auto it = GetChannelByTag(tag);
            bool cacheMiss = it == Channels.end();

            if (cacheMiss) {
                auto& channel = GetChannelByPolicy();
                channel.Tag = tag;
                channel.LastAccessTime = tick;
            } else { // cache hit
                it->LastAccessTime = tick;
                if (CacheType == TCacheEmulator::LA_FO) {
                    std::make_heap(Channels.begin(), Channels.end(), TGreater());
                }
            }

            UpdateChannelsByPolicy(tick, tag, cacheMiss);
        }

    private:
        std::vector<TChannel>::iterator GetChannelByTag(size_t tag) {
            return std::find_if(Channels.begin(), Channels.end(),
               [tag](const TChannel& ch) { return tag == ch.Tag; });
        }

        TChannel& GetChannelByPolicy() {
            if (Channels.size() < ChannelCount) { // index not filled
                Channels.push_back(TChannel());
                return Channels.back();
            }

            if (CacheType == TCacheEmulator::RANDOM) {
                return Channels[rand() % Channels.size()];
            } else if (CacheType == TCacheEmulator::LA_FO) {
               std::pop_heap(Channels.begin(), Channels.end(), TGreater());
               return Channels.back();
            }
        }

        void UpdateChannelsByPolicy(size_t tick, size_t tag, bool miss) {
           if (CacheType == TCacheEmulator::LA_FO) {
               std::push_heap(Channels.begin(), Channels.end(), TGreater());
           }

            IndexStats.CacheMisses += miss ? 1 : 0;

            auto& oldTick = TagToTime[tag];
            if (oldTick) { // else never was in cache
                const size_t vertexGreater = TreapOfAt.NumOfVertexGreater(oldTick) + 1;
                auto& mp = miss ? IndexStats.SeriesIfOutOfCache : IndexStats.SeriesIfInCache;
                mp[vertexGreater] += 1;
                IndexStats.Series[vertexGreater] += 1;

                TreapOfAt.Remove(oldTick);
            }

            oldTick = tick;
            TreapOfAt.Insert(tick);
        }

    public:
        TStats IndexStats;

    private:
        std::vector<TChannel> Channels;
        ECacheType CacheType;

        size_t ChannelCount;

        std::map<size_t, size_t> TagToTime;
        TTreap TreapOfAt;
    };

private:
    void GetAddressData(const float* a, size_t& index, size_t& tag) {
        index = ((size_t)(a) & IndexMask) >> OffsetBits;
        tag = (size_t)(a) & TagMask;
    }

private:
    size_t IndexMask;
    size_t TagMask;

    size_t OffsetBits;
    size_t CacheSize;
    size_t ChannelCount;
    size_t LineSize;
    ECacheType Type;

    std::vector<TIndex> Index;
    size_t TickNumber;
};
