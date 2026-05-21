#include "DataCache.h"
#include "DataReader.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace {

constexpr char kMagic[16] = {'B','T','C','A','C','H','E','v','1','\0','\0','\0','\0','\0','\0','\0'};
constexpr size_t kIoBufBytes = 8 * 1024 * 1024;

struct CacheHeader {
    char     magic[16];
    uint64_t sourceSize;
    int64_t  sourceMtimeNs;
    uint64_t tickerCount;
};

int64_t fileMtimeNs(const fs::path& p) {
    auto t = fs::last_write_time(p);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();
}

template <typename T>
bool writePod(std::ofstream& os, const T& value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    return static_cast<bool>(os);
}

template <typename T>
bool readPod(std::ifstream& is, T& value) {
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
    return static_cast<bool>(is);
}

bool writeBytes(std::ofstream& os, const void* data, size_t bytes) {
    os.write(reinterpret_cast<const char*>(data), bytes);
    return static_cast<bool>(os);
}

bool readBytes(std::ifstream& is, void* data, size_t bytes) {
    is.read(reinterpret_cast<char*>(data), bytes);
    return static_cast<bool>(is);
}

bool writeDoubleVec(std::ofstream& os, const std::vector<double>& v, uint64_t expectedLen) {
    if (v.size() != expectedLen) return false;
    return writeBytes(os, v.data(), v.size() * sizeof(double));
}

bool readDoubleVec(std::ifstream& is, std::vector<double>& out, uint64_t len) {
    out.resize(len);
    if (len == 0) return true;
    return readBytes(is, out.data(), len * sizeof(double));
}

bool writeCache(const fs::path& cachePath, const fs::path& sourcePath,
                const std::unordered_map<std::string, StockData>& data) {
    fs::path tmpPath = cachePath;
    tmpPath += ".tmp";

    std::error_code ec;
    uint64_t sourceSize = static_cast<uint64_t>(fs::file_size(sourcePath, ec));
    if (ec) return false;
    int64_t mtimeNs = fileMtimeNs(sourcePath);

    std::ofstream os;
    auto buf = std::make_unique<char[]>(kIoBufBytes);
    os.rdbuf()->pubsetbuf(buf.get(), kIoBufBytes);
    os.open(tmpPath, std::ios::binary | std::ios::trunc);
    if (!os.is_open()) return false;

    CacheHeader header{};
    std::memcpy(header.magic, kMagic, sizeof(kMagic));
    header.sourceSize    = sourceSize;
    header.sourceMtimeNs = mtimeNs;
    header.tickerCount   = data.size();
    if (!writePod(os, header)) return false;

    for (const auto& [ticker, sd] : data) {
        uint32_t nameLen = static_cast<uint32_t>(ticker.size());
        if (!writePod(os, nameLen)) return false;
        if (!writeBytes(os, ticker.data(), nameLen)) return false;
        if (!writePod(os, sd.contractSize)) return false;
        if (!writePod(os, sd.frictionPerRoundTrip)) return false;

        uint64_t L = sd.open.size();
        if (!writePod(os, L)) return false;
        if (!writeDoubleVec(os, sd.open,   L)) return false;
        if (!writeDoubleVec(os, sd.close,  L)) return false;
        if (!writeDoubleVec(os, sd.high,   L)) return false;
        if (!writeDoubleVec(os, sd.low,    L)) return false;
        if (!writeDoubleVec(os, sd.volume, L)) return false;

        if (sd.date.size() != L) return false;
        for (uint64_t i = 0; i < L; ++i) {
            const std::string& d = sd.date[i];
            uint32_t dLen = static_cast<uint32_t>(d.size());
            if (!writePod(os, dLen)) return false;
            if (dLen && !writeBytes(os, d.data(), dLen)) return false;
        }
    }

    os.flush();
    os.close();
    if (!os) {
        fs::remove(tmpPath, ec);
        return false;
    }

    fs::rename(tmpPath, cachePath, ec);
    if (ec) {
        fs::remove(tmpPath, ec);
        return false;
    }
    return true;
}

// Returns std::nullopt on any failure (cache missing, stale, truncated, corrupt). Caller
// falls back to text parsing on nullopt — never crashes on a bad cache.
std::optional<std::unordered_map<std::string, StockData>>
tryReadCache(const fs::path& cachePath, const fs::path& sourcePath) {
    std::error_code ec;
    if (!fs::exists(cachePath, ec) || ec) return std::nullopt;
    if (!fs::exists(sourcePath, ec) || ec) return std::nullopt;

    uint64_t sourceSize = static_cast<uint64_t>(fs::file_size(sourcePath, ec));
    if (ec) return std::nullopt;
    int64_t mtimeNs = fileMtimeNs(sourcePath);

    std::ifstream is;
    auto buf = std::make_unique<char[]>(kIoBufBytes);
    is.rdbuf()->pubsetbuf(buf.get(), kIoBufBytes);
    is.open(cachePath, std::ios::binary);
    if (!is.is_open()) return std::nullopt;

    CacheHeader header{};
    if (!readPod(is, header)) return std::nullopt;
    if (std::memcmp(header.magic, kMagic, sizeof(kMagic)) != 0) return std::nullopt;
    if (header.sourceSize != sourceSize || header.sourceMtimeNs != mtimeNs) return std::nullopt;

    std::unordered_map<std::string, StockData> out;
    out.reserve(header.tickerCount);

    for (uint64_t t = 0; t < header.tickerCount; ++t) {
        uint32_t nameLen = 0;
        if (!readPod(is, nameLen)) return std::nullopt;
        std::string ticker(nameLen, '\0');
        if (nameLen && !readBytes(is, ticker.data(), nameLen)) return std::nullopt;

        StockData sd;
        if (!readPod(is, sd.contractSize)) return std::nullopt;
        if (!readPod(is, sd.frictionPerRoundTrip)) return std::nullopt;

        uint64_t L = 0;
        if (!readPod(is, L)) return std::nullopt;
        if (!readDoubleVec(is, sd.open,   L)) return std::nullopt;
        if (!readDoubleVec(is, sd.close,  L)) return std::nullopt;
        if (!readDoubleVec(is, sd.high,   L)) return std::nullopt;
        if (!readDoubleVec(is, sd.low,    L)) return std::nullopt;
        if (!readDoubleVec(is, sd.volume, L)) return std::nullopt;

        sd.date.reserve(L);
        for (uint64_t i = 0; i < L; ++i) {
            uint32_t dLen = 0;
            if (!readPod(is, dLen)) return std::nullopt;
            std::string d(dLen, '\0');
            if (dLen && !readBytes(is, d.data(), dLen)) return std::nullopt;
            sd.date.emplace_back(std::move(d));
        }

        out.emplace(std::move(ticker), std::move(sd));
    }

    return out;
}

} // namespace

std::unordered_map<std::string, StockData> ReadDataCached(const std::string &fileName) {
    fs::path source = fileName;
    fs::path cache  = source;
    cache += ".cache";

    if (auto cached = tryReadCache(cache, source)) {
        return std::move(*cached);
    }

    auto parsed = ReadData(fileName);
    if (parsed.empty()) return parsed;

    if (!writeCache(cache, source, parsed)) {
        std::cerr << "[Warning] Failed to write data cache: " << cache.string()
                  << " (next run will re-parse the text file)" << std::endl;
    }
    return parsed;
}
