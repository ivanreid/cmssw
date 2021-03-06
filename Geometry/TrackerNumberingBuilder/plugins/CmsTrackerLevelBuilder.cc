#include "DetectorDescription/Core/interface/DDFilteredView.h"
#include "DetectorDescription/DDCMS/interface/DDFilteredView.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/TrackerNumberingBuilder/plugins/CmsTrackerLevelBuilder.h"
#include "Geometry/TrackerNumberingBuilder/plugins/ExtractStringFromDDD.h"

#include <cmath>

#define DEBUG false

bool CmsTrackerLevelBuilderHelper::subDetByType(const GeometricDet* a, const GeometricDet* b) {
  // it relies on the fact that the GeometricDet::GDEnumType enumerators used
  // to identify the subdetectors in the upgrade geometries are equal to the
  // ones of the present detector + n*100
  return a->type() % 100 < b->type() % 100;
}

// NP** Phase2 BarrelEndcap
bool CmsTrackerLevelBuilderHelper::phiSortNP(const GeometricDet* a, const GeometricDet* b) {
  if (std::abs(a->translation().rho() - b->translation().rho()) < 0.01 &&
      (std::abs(a->translation().phi() - b->translation().phi()) < 0.01 ||
       std::abs(a->translation().phi() - b->translation().phi()) > 6.27) &&
      a->translation().z() * b->translation().z() > 0.0) {
    return (std::abs(a->translation().z()) < std::abs(b->translation().z()));
  } else
    return false;
}

bool CmsTrackerLevelBuilderHelper::isLessZ(const GeometricDet* a, const GeometricDet* b) {
  // NP** change for Phase 2 Tracker
  if (a->translation().z() == b->translation().z()) {
    return a->translation().rho() < b->translation().rho();
  } else {
    // Original version
    return a->translation().z() < b->translation().z();
  }
}

bool CmsTrackerLevelBuilderHelper::isLessModZ(const GeometricDet* a, const GeometricDet* b) {
  return std::abs(a->translation().z()) < std::abs(b->translation().z());
}

double CmsTrackerLevelBuilderHelper::getPhi(const GeometricDet* a) {
  double phi = a->phi();
  return (phi >= 0 ? phi : phi + 2 * M_PI);
}

double CmsTrackerLevelBuilderHelper::getPhiModule(const GeometricDet* a) {
  std::vector<const GeometricDet*> const& comp = a->components().back()->components();
  float phi = 0.;
  bool sum = true;

  for (auto i : comp) {
    if (std::abs(i->phi()) > M_PI / 2.) {
      sum = false;
      break;
    }
  }

  if (sum) {
    for (auto i : comp) {
      phi += i->phi();
    }

    double temp = phi / float(comp.size()) < 0. ? 2 * M_PI + phi / float(comp.size()) : phi / float(comp.size());
    return temp;

  } else {
    for (auto i : comp) {
      double phi1 = i->phi() >= 0 ? i->phi() : i->phi() + 2 * M_PI;
      phi += phi1;
    }

    double com = comp.front()->phi() >= 0 ? comp.front()->phi() : 2 * M_PI + comp.front()->phi();
    double temp =
        std::abs(phi / float(comp.size()) - com) > 2. ? M_PI - phi / float(comp.size()) : phi / float(comp.size());
    temp = temp >= 0 ? temp : 2 * M_PI + temp;
    return temp;
  }
}

double CmsTrackerLevelBuilderHelper::getPhiGluedModule(const GeometricDet* a) {
  std::vector<const GeometricDet*> comp;
  a->deepComponents(comp);
  float phi = 0.;
  bool sum = true;

  for (auto& i : comp) {
    if (std::abs(i->phi()) > M_PI / 2.) {
      sum = false;
      break;
    }
  }

  if (sum) {
    for (auto& i : comp) {
      phi += i->phi();
    }

    double temp = phi / float(comp.size()) < 0. ? 2 * M_PI + phi / float(comp.size()) : phi / float(comp.size());
    return temp;

  } else {
    for (auto& i : comp) {
      double phi1 = i->phi() >= 0 ? i->phi() : i->translation().phi() + 2 * M_PI;
      phi += phi1;
    }

    double com = comp.front()->phi() >= 0 ? comp.front()->phi() : 2 * M_PI + comp.front()->phi();
    double temp =
        std::abs(phi / float(comp.size()) - com) > 2. ? M_PI - phi / float(comp.size()) : phi / float(comp.size());
    temp = temp >= 0 ? temp : 2 * M_PI + temp;
    return temp;
  }
}

double CmsTrackerLevelBuilderHelper::getPhiMirror(const GeometricDet* a) {
  double phi = a->phi();
  phi = (phi >= 0 ? phi : phi + 2 * M_PI);                              // (-pi,pi] --> [0,2pi)
  return ((M_PI - phi) >= 0 ? (M_PI - phi) : (M_PI - phi) + 2 * M_PI);  // (-pi,pi] --> [0,2pi)
}

double CmsTrackerLevelBuilderHelper::getPhiModuleMirror(const GeometricDet* a) {
  double phi = getPhiModule(a);                // [0,2pi)
  phi = (phi <= M_PI ? phi : phi - 2 * M_PI);  // (-pi,pi]
  return (M_PI - phi);
}

double CmsTrackerLevelBuilderHelper::getPhiGluedModuleMirror(const GeometricDet* a) {
  double phi = getPhiGluedModule(a);           // [0,2pi)
  phi = (phi <= M_PI ? phi : phi - 2 * M_PI);  // (-pi,pi]
  return (M_PI - phi);
}

bool CmsTrackerLevelBuilderHelper::isLessRModule(const GeometricDet* a, const GeometricDet* b) {
  return a->deepComponents().front()->rho() < b->deepComponents().front()->rho();
}

bool CmsTrackerLevelBuilderHelper::isLessR(const GeometricDet* a, const GeometricDet* b) { return a->rho() < b->rho(); }

/*
 * Old DD navigation.
 */
template <>
void CmsTrackerLevelBuilder<DDFilteredView>::build(DDFilteredView& fv,
                                                   GeometricDet* tracker,
                                                   const std::string& attribute) {
  if (DEBUG) {
    edm::LogVerbatim("CmsTrackerLevelBuilder")
        << "CmsTrackerLevelBuilder::build called on volume"
        << " tracker->name() = " << tracker->name() << " tracker->path() = " << fv.geoHistory()
        << " tracker->type() = " << tracker->type();
  }

  // Go down one level in hierarchy
  bool doLayers = fv.firstChild();

  while (doLayers) {
    if (DEBUG) {
      edm::LogVerbatim("CmsTrackerLevelBuilder") << "Calling buildComponent, before";
    }

    // Call build on sibling
    buildComponent(fv, tracker, attribute);

    if (DEBUG) {
      edm::LogVerbatim("CmsTrackerLevelBuilder") << "Calling buildComponent, after";
    }

    // Go to next sibling
    doLayers = fv.nextSibling();
  }

  // Come back up
  fv.parent();

  // Now that all GeometricDets of a given hierarchy level are built, sort them!
  sortNS(fv, tracker);

  if (DEBUG) {
    edm::LogVerbatim("CmsTrackerLevelBuilder")
        << "CmsTrackerLevelBuilder::build: Exit, finished all buildComponents calls.";
  }
}

/*
 * DD4hep navigation.
 */
template <>
void CmsTrackerLevelBuilder<cms::DDFilteredView>::build(cms::DDFilteredView& fv,
                                                        GeometricDet* tracker,
                                                        const std::string& attribute) {
  if (DEBUG) {
    edm::LogVerbatim("CmsTrackerLevelBuilder")
        << "CmsTrackerLevelBuilder::build called on volume"
        << " tracker->name() = " << tracker->name() << " tracker->path() = " << fv.geoHistory()
        << " tracker->type() = " << tracker->type();
  }

  // Go down one level in hierarchy
  fv.firstChild();

  // This is the siblings hierarchy level
  const int level = fv.level();
  const std::string& type = ExtractStringFromDDD<cms::DDFilteredView>::getString(attribute, &fv);

  // Treat all siblings of a given hierarchy level
  while (fv.level() == level) {
    if (DEBUG) {
      edm::LogVerbatim("CmsTrackerLevelBuilder")
          << "Calling buildComponent, before."
          << " level = " << level << " type = " << type << " fv.level() = " << fv.level()
          << " fv.type() = " << ExtractStringFromDDD<cms::DDFilteredView>::getString(attribute, &fv);
    }

    // Call build on sibling
    buildComponent(fv, tracker, attribute);

    if (DEBUG) {
      edm::LogVerbatim("CmsTrackerLevelBuilder")
          << "Calling buildComponent, after."
          << " level = " << level << " type = " << type << " fv.level() = " << fv.level()
          << " fv.type() = " << ExtractStringFromDDD<cms::DDFilteredView>::getString(attribute, &fv);
    }
  }

  // Now that all GeometricDets of a given hierarchy level are built, sort them!
  sortNS(fv, tracker);

  if (DEBUG) {
    edm::LogVerbatim("CmsTrackerLevelBuilder")
        << "CmsTrackerLevelBuilder::build: Exit, finished all buildComponents calls.";
  }
}

template class CmsTrackerLevelBuilder<DDFilteredView>;
template class CmsTrackerLevelBuilder<cms::DDFilteredView>;
