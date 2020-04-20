#pragma once
#include <iostream>
#include <string>
namespace EgcCommon {
namespace EgcGrpcLog {
bool init(std::string logfile);
void unInit();
void log(const std::string& message, bool verbose = false);
void setVerbose(bool verbose);
std::ostream& get();
}  // namespace EgcGrpcLog
}  // namespace EgcCommon
