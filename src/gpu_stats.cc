#include "gpu_stats.h"

void GPUStats::Init() {
  EGLint groupid = 0;
  EGLint counterid = 0;
  int res = 0;

  m_getPerfCounterGroupInfoBRCM = (PFNEGLGETPERFCOUNTERGROUPINFOBRCMPROC) eglGetProcAddress("eglGetPerfCounterGroupInfoBRCM");
  m_getPerfCounterInfoBRCM = (PFNEGLGETPERFCOUNTERINFOBRCMPROC)eglGetProcAddress("eglGetPerfCounterInfoBRCM");
  m_setPerfCountingBRCM = (PFNEGLSETPERFCOUNTINGBRCMPROC)eglGetProcAddress("eglSetPerfCountingBRCM");
  m_choosePerfCountersBRCM = (PFNEGLCHOOSEPERFCOUNTERSBRCMPROC)eglGetProcAddress("eglChoosePerfCountersBRCM");
  m_getPerfCounterDataBRCM = (PFNEGLGETPERFCOUNTERDATABRCMPROC)eglGetProcAddress("eglGetPerfCounterDataBRCM");

  statsfile_ = fopen("/tmp/statsfile_", "w");

  do {
    EGLint gnameStrSize = 128;
    char groupNameStr[gnameStrSize];
    EGLint numCounters;
    EGLint maxActiveCounters;

    res = m_getPerfCounterGroupInfoBRCM(groupid, gnameStrSize, groupNameStr, &numCounters, &maxActiveCounters);
    // printf("XXXX: groupid: %d, %s, %d/%d\n", groupid, groupNameStr, numCounters, maxActiveCounters);

    if (res != 0) {
      CounterGroup group(groupid, groupNameStr, numCounters, maxActiveCounters);

      int ires = 0;
      counterid = 0;

      do {
        EGLuint64BRCM minValue;
        EGLuint64BRCM maxValue;
        EGLuint64BRCM denominator;
        EGLint nameStrSize = 128;
        char nameStr[nameStrSize];
        EGLint unitStrSize = 128;
        char unitStr[unitStrSize];

        ires = m_getPerfCounterInfoBRCM(groupid, counterid, &minValue, &maxValue, &denominator, nameStrSize, nameStr, unitStrSize, unitStr);

        if (res != 0) {
          // printf("XXXX: groupid: %d counterid: %d value %llu/%llu/%llu n: %s, u: %s\n", groupid, counterid, minValue, maxValue, denominator, nameStr, unitStr);

          Counter counter(counterid, nameStr, unitStr, minValue, maxValue, denominator);
          group.counters_.emplace(group.counters_.end(), std::move(counter));
          counterid++;
        }

      } while (ires != 0);

      counterGroups_.emplace(counterGroups_.end(), std::move(group));
    }

    groupid++;

  } while (res != 0);

  printf("===================================\n");
  {
    for (auto i : counterGroups_) {
      printf("XXXX: gid: %d, %s max active:%d total:%d\n", i.groupid_, i.name_.c_str(), i.maxActiveCounters_, i.counters_.size());

      for (auto j : i.counters_) {
        printf("XXXX: %02d/%02d %40s %llu/%llu/%llu u:%s\n", i.groupid_, j.counterid_, j.name_.c_str(), j.minValue_, j.maxValue_, j.denominator_, j.unit_.c_str() );
      }
    }
  }

}

void GPUStats::Acquire() {
  m_setPerfCountingBRCM(EGL_ACQUIRE_COUNTERS_BRCM);
  for (auto i : counterGroups_) {
    m_choosePerfCountersBRCM(false, i.groupid_, 0, NULL);
  }
}

void GPUStats::Start() {
  m_setPerfCountingBRCM(EGL_START_COUNTERS_BRCM);
}

void GPUStats::Enable(int group, std::vector<int> counterList) {
  m_choosePerfCountersBRCM(false, group, 0, NULL);
  m_choosePerfCountersBRCM(true, group, counterList.size(), counterList.data());
  for (auto i: counterList) {
    counterGroups_[group].counters_[i].enabled_ = true;
  }

  {
    for (auto i : counterGroups_) {
      for (auto j : i.counters_) {
        printf("XXXX: %02d/%02d %40s %d\n", i.groupid_, j.counterid_, j.name_.c_str(), j.enabled_);
      }
    }
  }
}


void GPUStats::Print(float timestamp) {
  typedef struct PerfCounter {
    uint32_t uiGroupIndex;
    uint32_t uiCounterIndex;
    uint64_t uiValue;
  } PerfCounter;
  static int counter = 0;
  uint8_t *buf = NULL;
  int32_t numBytes = 65536;
  buf = new uint8_t[65536];
  memset(buf, 0, 65536);
  int res = m_getPerfCounterDataBRCM(65536, buf, &numBytes, true);   // TRUE - reset counters
  PerfCounter* pc = (PerfCounter*) buf;
  fprintf(statsfile_,"gpustats: ts: %07.3f ", timestamp);
  for (int i = 0; i < numBytes/sizeof(PerfCounter); i++) {
    // printf("%03d/%08d: %02d %02d: %lld\n", i, numBytes, pc[i].uiGroupIndex, pc[i].uiCounterIndex, pc[i].uiValue);
    auto gid = pc[i].uiGroupIndex;
    auto cid = pc[i].uiCounterIndex;
    auto val = pc[i].uiValue;
    if (counterGroups_[gid].counters_[cid].enabled_) {
      fprintf(statsfile_,"%s: %3lld ", counterGroups_[gid].counters_[cid].name_.c_str(), val);
    }
  }
  fprintf(statsfile_,"\n");
  counter++;
  if (counter % 50 == 0) {
    fflush(statsfile_);
  }
  delete [] buf;
}
