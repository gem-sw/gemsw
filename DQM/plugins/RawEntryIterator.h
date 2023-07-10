#ifndef gemsw_DQM_RawEntryIterator_h
#define gemsw_DQM_RawEntryIterator_h

// A copy of
// DQMServices_StreamerIO_DQMFileIterator

#include <chrono>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include <iostream>

namespace edm {
  class ParameterSet;
  class ParameterSetDescription;
} // namespace edm

class RawEntryIterator {
  public:

    struct Entry {
      std::string filename;
      std::string run_path;

      unsigned int entry_number;
      bool sec_file;

      static Entry load_entry(const std::string& run_path,
                              const std::string& filename,
                              const unsigned int entryNumber,
                              bool secFile);

      std::string get_data_path() const;
      std::string state;
    };
    
    struct EorEntry {
      bool loaded = false;
      std::string filename;
      std::string run_path;
      std::size_t n_entry;

      static EorEntry load_eor(const std::string& run_path, const std::string& filename);
    };

    enum State {
      OPEN = 0,
      EOR_CLOSING = 1, // EoR file found, but files are still pending
      EOR = 2
    };

    RawEntryIterator(edm::ParameterSet const& pset);
    ~RawEntryIterator() = default;
    void init(int run, const std::string&, const std::string&);

    State state() const { return state_; }

    /* methods to iterate over the files */
    
    /* nextEntryNumber_ is the first unprocessed entry number
     * entryReady() returns if the next entry is ready to be loaded
     * open() opens a file and advances the pointer to the next entry
    */
    
    bool entryReady();
    Entry open();

    // control
    void reset();
    void update_state();

    void delay();

    void logFileAction(const std::string& msg, const std::string& fileName = "") const;
    void logEntryState(const Entry& entry, const std::string& msg);

    unsigned int runNumber() const { return runNumber_; }
    unsigned int lastEntryFound();
    void advanceEntry(unsigned int entry, std::string reason);

    static void fillDescription(edm::ParameterSetDescription& desc);

  private:
    unsigned int runNumber_;
    std::string runInputDir_;
    std::string streamLabel_;
    unsigned long delayMillis_;
    long nextEntryTimeoutMillis_;
    long forceFileCheckTimeoutMillis_;
    bool flagScanOnce_;
    bool secFile_;

    std::string runPath_;

    EorEntry eor_;
    State state_;

    unsigned int nextEntryNumber_;
    std::map<unsigned int, Entry> entrySeen_;
    std::unordered_set<std::string> filesSeen_;

    unsigned runPathModTime_;
    std::chrono::high_resolution_clock::time_point runPathLastCollect_;

    std::chrono::high_resolution_clock::time_point lastEntryLoad_;

    unsigned mtimeHash() const;
    void collect(bool ignoreTimers);
};

#endif  // gemsw_DQM_RawEntryIterator_h
