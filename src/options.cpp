// Options objects for various depth-related algorithms,
// and policies for feature selection, etc.
// Author: Xiaohan Fei (feixh@cs.ucla.edu)
#include "options.h"
#include "feature.h"
#include "param.h"

namespace xivo {

bool Criteria::Candidate(FeaturePtr f) {
  ParameterServer& P{*ParameterServer::instance()};
  number_t zmin = P.get("min_depth", 0.05).asDouble();
  number_t zmax = P.get("max_depth", 5.0).asDouble();
  number_t max_subfilter_outlier = P.get("max_subfilter_outlier", 0.01).asDouble();

  bool good = (f->status() == FeatureStatus::READY ||
          f->status() == FeatureStatus::INITIALIZING) &&
         (f->outlier_counter() < max_subfilter_outlier);
  good = good && (f->z() > zmin && f->z() < zmax);
  return good;
}

bool Criteria::CandidateStrict(FeaturePtr f) {
  ParameterServer& P{*ParameterServer::instance()};
  number_t zmin = P.get("min_depth", 0.05).asDouble();
  number_t zmax = P.get("max_depth", 5.0).asDouble();
  number_t max_subfilter_outlier = P.get("max_subfilter_outlier", 0.01).asDouble();

  bool good = f->status() == FeatureStatus::READY &&
         (f->outlier_counter() < max_subfilter_outlier);
  good = good && (f->z() > zmin && f->z() < zmax);
  return good;
}

bool Criteria::CandidateComparison(FeaturePtr f1, FeaturePtr f2) {
  ParameterServer& P{*ParameterServer::instance()};
  std::string score_type = P.get("comparison_score_type", "DepthUncertainty").asString();

  int s1 = as_integer(f1->status());
  int s2 = as_integer(f2->status());

  number_t score1, score2;
  if (score_type == "DepthUncertainty") {
    score1 = -1.0 * (f1->P())(2,2);
    score2 = -1.0 * (f2->P())(2,2);
  }
  else if (score_type == "CovarianceDiagNorm") {
    score1 = -1.0 * f1->P().diagonal().norm();
    score2 = -1.0 * f2->P().diagonal().norm();
  }
  else if (score_type == "CovarianceDiagNormPlusOutlierCount") {
    // This is the one that is implemented in Corvis
    score1 = -1.0 * (f1->P().diagonal().norm() + f1->outlier_counter());
    score2 = -1.0 * (f2->P().diagonal().norm() + f2->outlier_counter());
  }
  else {
    LOG(ERROR) << "Invalid feature score type";
  }

  return (s1 > s2) || (s1 == s2 && f1->score() > f2->score());
}

} // namespace xivo
