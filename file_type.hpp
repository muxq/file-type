#pragma once
#include "json/json.h"
#include <assert.h>
#include <exception>
#include <fstream>
#include <iosfwd>
#include <set>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <shlwapi.h>
#else
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

inline size_t file_size(const std::string &path) {
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad;
  if (::GetFileAttributesExA(path.c_str(), ::GetFileExInfoStandard, &fad) ==
      0) {
    return -1;
  }
  if (0 != (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
    return -1;
  }
  return (static_cast<size_t>(fad.nFileSizeHigh)
          << (sizeof(fad.nFileSizeLow) * 8)) +
         fad.nFileSizeLow;
#else // Windows
  struct stat path_stat;
  if (::stat(path.c_str(), &path_stat) != 0) {
    return -1;
  }
  if (!S_ISREG(path_stat.st_mode)) {
    return -1;
  }
  return static_cast<size_t>(path_stat.st_size);
#endif
}

inline std::string load_string_file(const std::string &path) {
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  file.open(path.c_str(), std::ios_base::binary);
  std::size_t sz = static_cast<std::size_t>(file_size(path));
  std::string data(sz, '\0');
  file.read(&data[0], sz);
  return data;
}

inline std::string program_path() {
  char buf[1024] = {0};
  GetModuleFileNameA(NULL, buf, sizeof(buf));
  return buf;
}

inline std::string current_path() {
  std::string ret = program_path();
  std::string::size_type pos = ret.find_last_of('\\');
  if (pos != std::string::npos) {
    return ret.substr(0, pos);
  }
  return ret;
}

class magic {
public:
  ~magic() {}
  static magic &instance() {
    static magic inst = {};
    return inst;
  }
  void extensions(const std::string &path) {
    std::string data = load_string_file(path);
    Json::Reader jr;
    if (!jr.parse(data, jv_)) {
      Json::Value().swap(jv_);
      throw std::logic_error("parse extersions file failed");
    }
  }
  const Json::Value &extensions() { return jv_; }

private:
  magic() {
    std::ostringstream oss;
    oss << current_path() << "\\extensions.json";
    extensions(oss.str());
  }

private:
  Json::Value jv_;
};

inline std::string bytes2hex(const std::vector<unsigned char> &bytes) {
  std::string res;
  const char hex[] = "0123456789ABCDEF";
  for (auto it = bytes.begin(); it != bytes.end(); ++it) {
    unsigned char c = static_cast<unsigned char>(*it);
    res += hex[c >> 4];
    res += hex[c & 0xf];
  }
  return res;
}

inline std::string file_type(const std::string &fp) {
  FILE *f = fopen(fp.c_str(), "rb");
  char *ext = PathFindExtensionA(fp.c_str());
  if (ext) {
    ext + 1;
  }

  if (!f) {
    return ext;
  }
  struct file_guard {
    FILE *f_;
    ~file_guard() {
      if (f_) {
        fclose(f_);
      }
    }
  } guard = {f};

  const auto &jv = magic::instance().extensions();
  std::set<std::string> types;
  for (auto const &key : jv.getMemberNames()) {
    for (const auto &item : jv[key]["signs"]) {
      int ofset = std::stoi(item.asCString());
      const char *hex = strstr(item.asCString(), ",") + 1;
      int len = strlen(hex) / 2;
      std::vector<unsigned char> data(len, '\0');
      if (0 == fseek(f, ofset, SEEK_SET) &&
          (fread(&data[0], 1, len, f) == len) &&
          0 == stricmp(bytes2hex(data).c_str(), hex)) {
        types.insert(key);
      }
    }
  }
  if (types.empty()) {
    return ext;
  }
  if (types.count(ext)) {
    return ext;
  }
  return *types.begin();
}