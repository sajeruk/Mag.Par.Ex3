#include "cache.h"

#include <iostream>
#include <random>
#include <ctime>

//__declspec(align(16))
// __restrict
namespace {

    struct TCacheData {
        TCacheEmulator::ECacheType Type;
        size_t CacheSize;
        size_t WaysOfAssociativity;
        size_t LineSize;

        TCacheData(TCacheEmulator::ECacheType type,
            size_t log2CacheSize,
            size_t waysOfAssociativity,
            size_t lineSize = 64)
                : Type(type)
                , CacheSize(1 << log2CacheSize)
                , WaysOfAssociativity(waysOfAssociativity)
                , LineSize(lineSize)
        {}

        friend std::ostream& operator<<(std::ostream& os, TCacheData& st);
    };

    std::ostream& operator<<(std::ostream& os, const TCacheData& data) {
        os << "CacheSize: " << data.CacheSize << " WaysOfAssociativity: " <<
            data.WaysOfAssociativity << " LineSize " << data.LineSize << " Type: " <<
            ((data.Type == TCacheEmulator::RANDOM) ? "RANDOM" : "FIFO");

        return os;
    }

    void MultSimple(const float* __restrict a, const float* __restrict b, float* __restrict c, int n, const TCacheData& data)
    {
        std::cout << data << std::endl;
        TCacheEmulator cache(data.CacheSize, data.WaysOfAssociativity, data.LineSize, data.Type);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                c[i * n + j] = 0.f;
                cache.Access(c + i * n + j);
                for (int k = 0; k < n; ++k) {
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                    cache.Access(a + i * n + k);
                    cache.Access(b + k * n + j);
                    cache.Access(c + i * n + j);

                }
            }
        }

        std::cout << "Stats for comp process" << std::endl;
        std::cout << cache.GetStats(0);

    }

    void FillRandom(float* a, int n)
    {
        std::default_random_engine eng;
        std::uniform_real_distribution<float> dist;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = dist(eng);
            }
        }
    }
}

int main(int /*argc*/, char* argv[])
{
    const int n = atoi(argv[1]);
    std::cerr << "n = " << n << std::endl;
    const int log2CacheSize = atoi(argv[2]);
    const int waysOfAssociativity = atoi(argv[3]);
    const int random = atoi(argv[4]);

    TCacheData data(random ? TCacheEmulator::RANDOM : TCacheEmulator::LA_FO,
        log2CacheSize, waysOfAssociativity);

    float* a = new float[n * n];
    float* b = new float[n * n];
    float* c = new float[n * n];

    FillRandom(a, n);
    FillRandom(b, n);

    {
        const auto startTime = std::clock();
        MultSimple(a, b, c, n, data);
        const auto endTime = std::clock();

        std::cerr << "timeSimple: " << double(endTime - startTime) / CLOCKS_PER_SEC << '\n';
    }

    delete[] a;
    delete[] b;
    delete[] c;
}

