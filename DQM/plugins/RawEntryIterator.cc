#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/TimeOfDay.h"

#include <filesystem>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <boost/regex.hpp>

#include <fmt/printf.h>

#include "RawEntryIterator.h"

RawEntryIterator::Entry
RawEntryIterator::Entry::load_entry(const std::string& run_path,
                                   const std::string& filename1,
                                   const std::string& filename2,
                                   const unsigned int entryNumber,
                                   bool secFile) {

  Entry entry;
  entry.run_path = run_path;
  entry.sec_file = secFile;
  
  entry.entry_number = entryNumber;

  std::filesystem::path p(run_path);
  entry.rawfile = p / filename1;
  entry.secrawfile = p / filename2;

  return entry;
}

std::string 
RawEntryIterator::Entry::get_entry_path() const {
  // if (boost::starts_with(datafn, "/"))
  //  return datafn;

  std::filesystem::path p(run_path);
  return p.string();
}

RawEntryIterator::EorEntry
RawEntryIterator::EorEntry::load_eor(const std::string& run_path,
                                    const std::string& filename) {
    
  EorEntry eor;
  eor.filename = filename;
  eor.run_path = run_path;
  
  eor.loaded = true;
  return eor;
}

RawEntryIterator::RawEntryIterator(edm::ParameterSet const& pset)
  : state_(EOR) 
{
  runNumber_ = pset.getUntrackedParameter<unsigned int>("runNumber");
  runInputDir_ = pset.getUntrackedParameter<std::string>("runInputDir");
  streamLabel_ = pset.getUntrackedParameter<std::string>("streamLabel");
  secFile_ = pset.getUntrackedParameter<bool>("secFile");
  flagScanOnce_ = pset.getUntrackedParameter<bool>("scanOnce");
  delayMillis_ = pset.getUntrackedParameter<uint32_t>("delayMillis");
  nextEntryTimeoutMillis_ = pset.getUntrackedParameter<int32_t>("nextEntryTimeoutMillis");
  
  forceFileCheckTimeoutMillis_ = 5015;
  reset();
}

void RawEntryIterator::reset() {
  
  runPath_ = fmt::sprintf("%s/%08d", runInputDir_, runNumber_);
  eor_.loaded = false;
  state_ = State::OPEN;
  nextEntryNumber_ = 0;
  entrySeen_.clear();
  filesSeen_.clear();

  lastEntryLoad_ = std::chrono::high_resolution_clock::now();

  collect(true);
  update_state();
}

RawEntryIterator::Entry RawEntryIterator::open() {
  Entry& entry = entrySeen_[nextEntryNumber_];
  advanceEntry(nextEntryNumber_ + 1, "open: file iterator");
  return entry;
}

bool RawEntryIterator::entryReady() {
  if (entrySeen_.find(nextEntryNumber_) != entrySeen_.end()) {
    return true;
  }
  return false;
}

bool RawEntryIterator::nextEntryReady() {
  if (entrySeen_.find(nextEntryNumber_ + 1) != entrySeen_.end()) {
    return true;
  }
  return false;
}

unsigned int RawEntryIterator::lastEntryFound() {
  if (!entrySeen_.empty())
    return entrySeen_.rbegin()->first;
  return 1;
}

void RawEntryIterator::advanceEntry(unsigned int entry, std::string reason) {
  unsigned int currentEntry = nextEntryNumber_;

  nextEntryNumber_ = entry;
  lastEntryLoad_ = std::chrono::high_resolution_clock::now();

  auto iter = entrySeen_.lower_bound(currentEntry);

  while ((iter != entrySeen_.end()) && (iter->first < nextEntryNumber_)) {
    iter->second.state = reason;
    ++iter;
  }
}

unsigned RawEntryIterator::mtimeHash() const {
  unsigned mtime_now =0;
  if (!std::filesystem::exists(runPath_))
    return mtime_now;
  
  auto write_time = std::filesystem::last_write_time(runPath_);
  
  mtime_now = 
      mtime_now ^ std::chrono::duration_cast<std::chrono::microseconds>(write_time.time_since_epoch()).count();
  
  return mtime_now;
}

void RawEntryIterator::collect(bool ignoreTimers) {
  // search fylesystem to find available files
  
  auto now = std::chrono::high_resolution_clock::now();
  auto last_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - runPathLastCollect_).count();

  if ((!ignoreTimers) && (last_ms >= 0) && (last_ms < 100)) {
    return;
  }

  // check if directory changed
  auto mtime_now = mtimeHash();

  if ((!ignoreTimers) && (last_ms < forceFileCheckTimeoutMillis_) && (mtime_now == runPathModTime_))
    return;

  runPathModTime_ = mtime_now;
  runPathLastCollect_ = now;

  using std::filesystem::directory_entry;
  using std::filesystem::directory_iterator;

  std::string fn_eor;
  
  if (!std::filesystem::exists(runPath_)) {
    logFileAction("Directory does not exist: ", runPath_);
    state_ = State::EOR;
    return;
  }
  
  std::string filename1;
  std::string filename2;
  directory_iterator dend;
  unsigned int fragment;
  for (directory_iterator diter(runPath_); diter != dend; ++diter) {
    const boost::regex fn_re("run-([a-zA-Z0-9:]+)?(\\l+)-index(\\d+)\\.raw\\.zst");
    const boost::regex eor_re("EoR\\.jsn");

    const std::string filename = diter->path().filename().string();
    const std::string file = diter->path().string();

    if (filesSeen_.find(filename) != filesSeen_.end()) {
      continue;
    }

    boost::smatch result, eor_match;
    // check if file is EoR
    if (boost::regex_match(filename, eor_match, eor_re)) {
      if (!eor_.loaded) {
        fn_eor = file;
        continue;
      }
    }
   
    if (boost::regex_match(filename, result, fn_re)) {
      std::string filelabel = result[1]; 
      std::string type = result[2];
      fragment = std::stoi(result[3]);

      if (entrySeen_.find(fragment) != entrySeen_.end()) {
        continue;
      }
      
      if (secFile_) {
        if (type == "a") {
          filename2 = "run-" + filelabel + "b-index" + result[3] + ".raw.zst";
        }
        else {
          filename2 = "run-" + filelabel + "a-index" + result[3] + ".raw.zst";
        }
        filesSeen_.insert(filename2); 
      }
      
      filesSeen_.insert(filename);
      filename1 = filename;
    }
    else {
      continue;
    }
 
    if (!secFile_) {
      Entry entry = Entry::load_entry(runPath_, filename1, "", fragment, secFile_);
      entrySeen_.emplace(fragment, entry);
      logFileAction("Found file: ", filename1);
      continue; 
    }

    Entry entry = Entry::load_entry(runPath_, filename1, filename2, fragment, secFile_);
    entrySeen_.emplace(fragment, entry);
    logFileAction("Found file: ", filename1);
    logFileAction("Found file: ", filename2); 
    fragment += 1; 
  }

  if ((!fn_eor.empty()) || flagScanOnce_ ) {
    if (!fn_eor.empty()) {
      logFileAction("EoR file found: ", fn_eor);
    }
    eor_.loaded = true;

    if (entrySeen_.empty()) {
      eor_.n_entry = 0;
    } else {
      eor_.n_entry = entrySeen_.rbegin()->first;
    }
  }
}

void RawEntryIterator::change_state(State state) {
  state_ = state;
}

void RawEntryIterator::update_state() {
  
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  State old_state = state_;

  // in scanOnce mode no repeated scans
  // whatever found at reset is used

  if (!flagScanOnce_)
    collect(false);
  
  if ((state_ == State::OPEN) && (eor_.loaded)) {
    state_ = State::EOR_CLOSING;
  }

  if ((state_ != State::EOR) && (nextEntryTimeoutMillis_ >= 0)) {
    auto iter = entrySeen_.lower_bound(nextEntryNumber_);
    if ((iter != entrySeen_.end()) && iter->first != nextEntryNumber_) {
      auto elapsed = high_resolution_clock::now() - lastEntryLoad_;
      auto elapsed_ms = duration_cast<milliseconds>(elapsed).count();

      if (elapsed_ms >= nextEntryTimeoutMillis_) {
        // Timeout reached
        std::string msg("Timeout reached, skipping entry ");
        msg += std::to_string(nextEntryNumber_) + " .. " + std::to_string(iter->first -1);
        msg += ", nextEntryNumber_ is now " + std::to_string(iter->first);
        logFileAction(msg);

        advanceEntry(iter->first, "skipped: timeout");
      }
    }
  }

  if (state_ == State::EOR_CLOSING) {
    if (nextEntryNumber_ > eor_.n_entry) {
      state_ = State::EOR;
    }
  }

  if (state_ != old_state)
    logFileAction("State changed: ", std::to_string(old_state) + " -> " + std::to_string(state_));
}

void RawEntryIterator::delay() {
  usleep(delayMillis_ * 1000);
}

void RawEntryIterator::logFileAction(const std::string& msg, const std::string& fileName) const {
  edm::LogAbsolute("fileAction") << std::setprecision(0) << edm::TimeOfDay() << "  " << msg << fileName;
  edm::FlushMessageLog();
}

void RawEntryIterator::logEntryState(const Entry& entry, const std::string& msg) {
  if (entrySeen_.find(entry.entry_number) != entrySeen_.end()) {
    entrySeen_[entry.entry_number].state = msg;
  } else {
    logFileAction("Internal error: referenced lumi is not the map.");
  }
}

void RawEntryIterator::fillDescription(edm::ParameterSetDescription& desc) {
  desc.addUntracked<unsigned int>("runNumber")->setComment("Run number passed via config");
  desc.addUntracked<std::string>("streamLabel")->setComment("Stream Label");
  desc.addUntracked<std::string>("runInputDir")->setComment("Directory for the raw files");
  desc.addUntracked<bool>("secFile")->setComment("Secondary file");
  desc.addUntracked<bool>("scanOnce")->setComment("Do not repeat directory scans");
  desc.addUntracked<uint32_t>("delayMillis")->setComment("Delay to wait between file checks in ms");
  desc.addUntracked<int32_t>("nextEntryTimeoutMillis", -1)->setComment("Time (in ms) to wait before moving to the next entry, -1 to disable it");
}
