#ifndef DATACACHE_H
#define DATACACHE_H

#include "../Objects/StockData/StockData.h"
#include <unordered_map>

// On-disk binary cache layered over the text parser. On the first call for a given
// source file, parses the text and writes a sidecar `<source>.cache`. Subsequent calls
// read the binary cache directly when the source's size + mtime still match the
// header recorded in the cache. Delete `<source>.cache` to force a re-parse.
//
// Identical observable behaviour to ReadData(); only the internal path differs.
std::unordered_map<std::string, StockData> ReadDataCached(const std::string &fileName);

#endif
