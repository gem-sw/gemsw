#ifndef QC8Harvestor_H
#define QC8Harvestor_H

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class QC8Harvestor : public MuonGEMBaseHarvestor {
public :
  explicit QC8Harvestor(const edm::ParameterSet&);
  ~QC8Harvestor();
  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;
  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
private:
  bool isMC_;
};

#endif
