#include <string>
#include <vector>

#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext_brcm.h>
#undef EGL_EGLEXT_PROTOTYPES


class GPUStats {

  struct Counter {
    Counter(int counterid, std::string name, std::string unit, uint64_t minValue, uint64_t maxValue, uint64_t denominator)
      :counterid_(counterid), name_(name), unit_(unit), minValue_(minValue), maxValue_(maxValue), denominator_(denominator), enabled_(false)
    {}

    int counterid_;
    std::string name_;
    std::string unit_;
    uint64_t minValue_;
    uint64_t maxValue_;
    uint64_t denominator_;
    bool enabled_;
  };

  struct CounterGroup {
    CounterGroup(int groupid, std::string name, int numCounters, int maxActiveCounters)
      : groupid_(groupid), name_(name), numCounters_(numCounters), maxActiveCounters_(maxActiveCounters)
    {}

    int groupid_;
    std::string name_;
    std::vector<Counter> counters_;
    int numCounters_;
    int maxActiveCounters_;
  };


  std::vector<CounterGroup> counterGroups_;

  FILE* statsfile_ = nullptr;

  PFNEGLGETPERFCOUNTERGROUPINFOBRCMPROC m_getPerfCounterGroupInfoBRCM = nullptr;
  PFNEGLGETPERFCOUNTERINFOBRCMPROC m_getPerfCounterInfoBRCM = nullptr;
  PFNEGLSETPERFCOUNTINGBRCMPROC    m_setPerfCountingBRCM = nullptr;
  PFNEGLCHOOSEPERFCOUNTERSBRCMPROC m_choosePerfCountersBRCM = nullptr;
  PFNEGLGETPERFCOUNTERDATABRCMPROC m_getPerfCounterDataBRCM = nullptr;

  public:
    void Init();
    void Print(float timestamp);
    void Acquire();
    void Enable(int group, std::vector<int> counters);
    void Start();
    FILE* StatsFile() { return statsfile_; }
};

