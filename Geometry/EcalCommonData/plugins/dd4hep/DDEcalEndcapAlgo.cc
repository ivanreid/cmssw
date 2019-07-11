#include "DD4hep/DetFactoryHelper.h"
#include "DetectorDescription/DDCMS/interface/DDPlugins.h"
#include "DetectorDescription/DDCMS/interface/BenchmarkGrd.h"
#include "Geometry/EcalCommonData/interface/DDEcalEndcapTrap.h"
#include <CLHEP/Geometry/Transform3D.h>

#include <string>
#include <vector>

using namespace std;
using namespace cms;
using namespace dd4hep;
// FIXME: use local definition until dd4hep fixes the uniits
// using namespace geant_units::operators;

constexpr long double piRadians(M_PIl);              // M_PIl is long double version of pi
constexpr long double degPerRad = 180. / piRadians;  // Degrees per radian
constexpr double operator"" _deg(long double deg) { return deg / degPerRad; }
constexpr double operator"" _deg(unsigned long long int deg) { return deg / degPerRad; }

// Length
constexpr double operator"" _mm(long double length) { return length * 0.1; }
constexpr double operator"" _mm(unsigned long long int length) { return length * 0.1; }
constexpr double operator"" _cm(unsigned long long int length) { return length * 1; }

// // Header files for endcap supercrystal geometry
// #include "DDEcalEndcapTrap.h"

using DDTranslation = ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double> >;
using DDRotation = ROOT::Math::Rotation3D;

namespace {
  struct Endcap {
    string mat;
    double zOff;
    
    string quaName;
    string quaMat;
    
    string crysMat;
    string wallMat;
    
    double crysLength;
    double crysRear;
    double crysFront;
    double sCELength;
    double sCERear;
    double sCEFront;
    double sCALength;
    double sCARear;
    double sCAFront;
    double sCAWall;
    double sCHLength;
    double sCHSide;
    
    double nSCTypes;
    vector<double> vecEESCProf;
    double nColumns;
    vector<double> vecEEShape;
    double nSCCutaway;
    vector<double> vecEESCCutaway;
    double nSCquad;
    vector<double> vecEESCCtrs;
    double nCRSC;
    vector<double> vecEECRCtrs;
    
    array<double, 3> cutParms;
    string cutBoxName;
    
    string envName;
    string alvName;
    string intName;
    string cryName;
    
    DDTranslation cryFCtr[5][5];
    DDTranslation cryRCtr[5][5];
    DDTranslation scrFCtr[10][10];
    DDTranslation scrRCtr[10][10];
    
    double pFHalf;
    double pFFifth;
    double pF45;
    
    vector<double> vecEESCLims;
    
    double iLength;
    double iXYOff;
    double cryZOff;
    double zFront;
  };

  const Rotation3D& myrot(cms::DDNamespace& ns, const string& nam, const Rotation3D& r) {
    ns.addRotation(nam, r);
    return ns.rotation(ns.prepend(nam));
  }

  string_view mynamespace(string_view input) {
    string_view v = input;
    auto trim_pos = v.find(':');
    if(trim_pos != v.npos)
      v.remove_suffix(v.size() - (trim_pos + 1));
    return v;
  }
}

static long algorithm(dd4hep::Detector& /* description */,
                      cms::DDParsingContext& ctxt,
                      xml_h e,
                      dd4hep::SensitiveDetector& /* sens */) {

  BenchmarkGrd counter("DDEcalEndcapAlgo");
  cms::DDNamespace ns(ctxt, e, true);
  cms::DDAlgoArguments args(ctxt, e);

  // TRICK!
  string myns{ mynamespace(args.parentName()).data(), mynamespace(args.parentName()).size()};
  
  Endcap ee;
  ee.mat = args.str("EEMat");
  ee.zOff = args.dble("EEzOff");

  ee.quaName = args.str("EEQuaName");
  ee.quaMat = args.str("EEQuaMat");
  ee.crysMat = args.str("EECrysMat");
  ee.wallMat = args.str("EEWallMat");
  ee.crysLength = args.dble("EECrysLength");
  ee.crysRear = args.dble("EECrysRear");
  ee.crysFront = args.dble("EECrysFront");
  ee.sCELength = args.dble("EESCELength");
  ee.sCERear = args.dble("EESCERear");
  ee.sCEFront = args.dble("EESCEFront");
  ee.sCALength = args.dble("EESCALength");
  ee.sCARear = args.dble("EESCARear");
  ee.sCAFront = args.dble("EESCAFront");
  ee.sCAWall = args.dble("EESCAWall");
  ee.sCHLength = args.dble("EESCHLength");
  ee.sCHSide = args.dble("EESCHSide");
  ee.nSCTypes = args.dble("EEnSCTypes");
  ee.nColumns = args.dble("EEnColumns");
  ee.nSCCutaway = args.dble("EEnSCCutaway");
  ee.nSCquad = args.dble("EEnSCquad");
  ee.nCRSC = args.dble("EEnCRSC");
  ee.vecEESCProf = args.vecDble("EESCProf");
  ee.vecEEShape = args.vecDble("EEShape");
  ee.vecEESCCutaway = args.vecDble("EESCCutaway");
  ee.vecEESCCtrs = args.vecDble("EESCCtrs");
  ee.vecEECRCtrs = args.vecDble("EECRCtrs");

  ee.cutBoxName = args.str("EECutBoxName");

  ee.envName = args.str("EEEnvName");
  ee.alvName = args.str("EEAlvName");
  ee.intName = args.str("EEIntName");
  ee.cryName = args.str("EECryName");

  ee.pFHalf = args.dble("EEPFHalf");
  ee.pFFifth = args.dble("EEPFFifth");
  ee.pF45 = args.dble("EEPF45");

  ee.vecEESCLims = args.vecDble("EESCLims");
  ee.iLength = args.dble("EEiLength");
  ee.iXYOff = args.dble("EEiXYOff");
  ee.cryZOff = args.dble("EECryZOff");
  ee.zFront = args.dble("EEzFront");

// DDRotation DDEcalEndcapAlgo::myrot(const std::string& s, const DDRotationMatrix& r) const {
//   return DDrot(ddname(m_idNameSpace + ":" + s), std::make_unique<DDRotationMatrix>(r));
// }

// DDMaterial DDEcalEndcapAlgo::ddmat(const std::string& s) const { return DDMaterial(ddname(s)); }

// DDName DDEcalEndcapAlgo::ddname(const std::string& s) const {
//   const pair<std::string, std::string> temp(DDSplit(s));
//   if (temp.second.empty()) {
//     return DDName(temp.first, m_idNameSpace);
//   } else {
//     return DDName(temp.first, temp.second);
//   }
// }

  //  Position supercrystals in EE Quadrant

  //********************************* cutbox for trimming edge SCs
  const double cutWid(ee.sCERear / sqrt(2.));
  ee.cutParms[0] = cutWid;
  ee.cutParms[1] = cutWid;
  ee.cutParms[2] = ee.sCELength / sqrt(2.);
  Solid eeCutBox = Box(ee.cutBoxName, ee.cutParms[0], ee.cutParms[1], ee.cutParms[3]);
  //**************************************************************

  const double zFix(ee.zFront - 3172_mm);  // fix for changing z offset

  //** fill supercrystal front and rear center positions from xml input
  for (unsigned int iC(0); iC != (unsigned int)ee.nSCquad; ++iC) {
    const unsigned int iOff(8 * iC);
    const unsigned int ix((unsigned int)ee.vecEESCCtrs[iOff + 0]);
    const unsigned int iy((unsigned int)ee.vecEESCCtrs[iOff + 1]);

    assert(ix > 0 && ix < 11 && iy > 0 && iy < 11);

    ee.scrFCtr[ix - 1][iy - 1] =
      DDTranslation(ee.vecEESCCtrs[iOff + 2], ee.vecEESCCtrs[iOff + 4], ee.vecEESCCtrs[iOff + 6] + zFix);

    ee.scrRCtr[ix - 1][iy - 1] =
      DDTranslation(ee.vecEESCCtrs[iOff + 3], ee.vecEESCCtrs[iOff + 5], ee.vecEESCCtrs[iOff + 7] + zFix);
  }

  //** fill crystal front and rear center positions from xml input
  for (unsigned int iC(0); iC != 25; ++iC) {
    const unsigned int iOff(8 * iC);
    const unsigned int ix((unsigned int)ee.vecEECRCtrs[iOff + 0]);
    const unsigned int iy((unsigned int)ee.vecEECRCtrs[iOff + 1]);

    assert(ix > 0 && ix < 6 && iy > 0 && iy < 6);

    ee.cryFCtr[ix - 1][iy - 1] =
      DDTranslation(ee.vecEECRCtrs[iOff + 2], ee.vecEECRCtrs[iOff + 4], ee.vecEECRCtrs[iOff + 6]);

    ee.cryRCtr[ix - 1][iy - 1] =
      DDTranslation(ee.vecEECRCtrs[iOff + 3], ee.vecEECRCtrs[iOff + 5], ee.vecEECRCtrs[iOff + 7]);
  }

  Solid eeCRSolid = Trap(ee.cryName,
			 0.5 * ee.crysLength,
			 atan((ee.crysRear - ee.crysFront) / (sqrt(2.) * ee.crysLength)),
			 45._deg,
			 0.5 * ee.crysFront,
			 0.5 * ee.crysFront,
			 0.5 * ee.crysFront,
			 0._deg,
			 0.5 * ee.crysRear,
			 0.5 * ee.crysRear,
			 0.5 * ee.crysRear,
			 0._deg);
  Volume eeCRLog = Volume(ee.cryName, eeCRSolid, ns.material(ee.crysMat));

  for (unsigned int isc(0); isc < ee.nSCTypes; ++isc) {
    unsigned int iSCType = isc + 1;
    const string anum(std::to_string(iSCType));
    const double eFront(0.5 * ee.sCEFront);
    const double eRear(0.5 * ee.sCERear);
    const double eAng(atan((ee.sCERear - ee.sCEFront) / (sqrt(2.) * ee.sCELength)));
    const double ffived(45_deg);
    const double zerod(0_deg);
    
    Solid eeSCEnv = Trap((1 == iSCType ? ee.envName + std::to_string(iSCType) : (ee.envName + std::to_string(iSCType) + "Tmp")),
			 0.5 * ee.sCELength,
			 eAng,
			 ffived,
			 eFront,
			 eFront,
			 eFront,
			 zerod,
			 eRear,
			 eRear,
			 eRear,
			 zerod);
    
    const double aFront(0.5 * ee.sCAFront);
    const double aRear(0.5 * ee.sCARear);
    const double aAng(atan((ee.sCARear - ee.sCAFront) / (sqrt(2.) * ee.sCALength)));
    Solid eeSCAlv = Trap((1 == iSCType ? ee.alvName + std::to_string(iSCType) : (ee.alvName + std::to_string(iSCType) + "Tmp")),
			 0.5 * ee.sCALength,
			 aAng,
			 ffived,
			 aFront,
			 aFront,
			 aFront,
			 zerod,
			 aRear,
			 aRear,
			 aRear,
			 zerod);
    
    const double dwall(ee.sCAWall);
    const double iFront(aFront - dwall);
    const double iRear(iFront);    //aRear  - dwall ) ;
    const double iLen(ee.iLength);  //0.075*eeSCALength() ) ;
    Solid eeSCInt = Trap((1 == iSCType ? ee.intName + std::to_string(iSCType) : (ee.intName + std::to_string(iSCType) + "Tmp")),
			 iLen / 2.,
			 atan((ee.sCARear - ee.sCAFront) / (sqrt(2.) * ee.sCALength)),
			 ffived,
			 iFront,
			 iFront,
			 iFront,
			 zerod,
			 iRear,
			 iRear,
			 iRear,
			 zerod);
    
    const double dz(-0.5 * (ee.sCELength - ee.sCALength));
    const double dxy(0.5 * dz * (ee.sCERear - ee.sCEFront) / ee.sCELength);
    const double zIOff(-(ee.sCALength - iLen) / 2.);
    const double xyIOff(ee.iXYOff);
    
    Volume eeSCELog;
    Volume eeSCALog;
    Volume eeSCILog;
    
    if (1 == iSCType) { // standard SC in this block
      eeSCELog = ns.addVolumeNS(Volume(myns + ee.envName + std::to_string(iSCType), eeSCEnv, ns.material(ee.mat)));
      eeSCALog = Volume(ee.alvName + std::to_string(iSCType), eeSCAlv, ns.material(ee.wallMat));
      eeSCILog = Volume(ee.intName + std::to_string(iSCType), eeSCInt, ns.material(ee.mat));
    } else { // partial SCs this block: create subtraction volumes as appropriate
      const double half(ee.cutParms[0] - ee.pFHalf * ee.crysRear);
      const double fifth(ee.cutParms[0] + ee.pFFifth * ee.crysRear);
      const double fac(ee.pF45);
      
      const double zmm(0_mm);
      
      DDTranslation cutTra(2 == iSCType ? DDTranslation(zmm, half, zmm)
			   : (3 == iSCType ? DDTranslation(half, zmm, zmm)
			      : (4 == iSCType ? DDTranslation(zmm, -fifth, zmm)
				 : (5 == iSCType ? DDTranslation(-half * fac, -half * fac, zmm)
				    : DDTranslation(-fifth, zmm, zmm)))));
      
      const CLHEP::HepRotationZ cutm(ffived);
      
      Rotation3D cutRot(5 != iSCType ? Rotation3D()
			: myrot(ns, "EECry5Rot",
				Rotation3D(cutm.xx(),
					   cutm.xy(),
					   cutm.xz(),
					   cutm.yx(),
					   cutm.yy(),
					   cutm.yz(),
					   cutm.zx(),
					   cutm.zy(),
					   cutm.zz())));
      
      Solid eeCutEnv = SubtractionSolid(ee.envName + std::to_string(iSCType), eeSCEnv,
					eeCutBox, Transform3D(cutRot, cutTra));
      
      const DDTranslation extra(dxy, dxy, dz);
      
      Solid eeCutAlv = SubtractionSolid(ee.alvName + std::to_string(iSCType), eeSCAlv,
					eeCutBox, Transform3D(cutRot, cutTra - extra));
      
      const double mySign(iSCType < 4 ? +1. : -1.);
      
      const DDTranslation extraI(xyIOff + mySign * 2_mm, xyIOff + mySign * 2_mm, zIOff);
      
      Solid eeCutInt = SubtractionSolid(ee.intName + std::to_string(iSCType), eeSCInt,
					eeCutBox, Transform3D(cutRot, cutTra - extraI));
      
      eeSCELog = ns.addVolumeNS(Volume(myns + ee.envName + std::to_string(iSCType), eeCutEnv, ns.material(ee.mat)));
      eeSCALog = Volume(ee.alvName + std::to_string(iSCType), eeCutAlv, ns.material(ee.wallMat));
      eeSCILog = Volume(ee.intName + std::to_string(iSCType), eeCutInt, ns.material(ee.mat));
    }
    eeSCELog.placeVolume(eeSCALog, iSCType * 100 + 1, Position(dxy, dxy, dz));
    eeSCALog.placeVolume(eeSCILog, iSCType * 100 + 1, Position(xyIOff, xyIOff, zIOff));
    
    DDTranslation croffset(0., 0., 0.);
    // EEPositionCRs(ee.alvName + std::to_string(iSCType), croffset, iSCType, cpv);
    // Position crystals within parent supercrystal interior volume
    static const unsigned int ncol(5);

    if (iSCType > 0 && iSCType <= ee.nSCTypes) {
      const unsigned int icoffset((iSCType - 1) * ncol - 1);
      
      // Loop over columns of SC
      for (unsigned int icol(1); icol <= ncol; ++icol) {
	// Get column limits for this SC type from xml input
	const int ncrcol((int)ee.vecEESCProf[icoffset + icol]);
	
	const int imin(0 < ncrcol ? 1 : (0 > ncrcol ? ncol + ncrcol + 1 : 0));
	const int imax(0 < ncrcol ? ncrcol : (0 > ncrcol ? ncol : 0));
	
	if (imax > 0) {
	  // Loop over crystals in this row
	  for (int irow(imin); irow <= imax; ++irow) {
	    // Create crystal as a DDEcalEndcapTrap object and calculate rotation and
	    // translation required to position it in the SC.
	    DDEcalEndcapTrap crystal(1, ee.crysFront, ee.crysRear, ee.crysLength);
	    
	    crystal.moveto(ee.cryFCtr[irow-1][icol-1], ee.cryRCtr[irow-1][icol-1]);
	    
	    string rname("EECrRoC" + std::to_string(icol) + "R" + std::to_string(irow));
	    
	    eeSCALog.placeVolume(eeCRLog,
				 100 * iSCType + 10 * (icol - 1) + (irow - 1),
				 Transform3D(myrot(ns, rname, crystal.rotation()),
					     Position(crystal.centrePos().x(), crystal.centrePos().y(),
						      crystal.centrePos().z() - ee.cryZOff)));
	  }
	}
      }
    }
  }

  //const std::vector<double>& colLimits(ee.vecEEShape);
  //** Loop over endcap columns
  for (int icol = 1; icol <= int(ee.nColumns); icol++) {
    //**  Loop over SCs in column, using limits from xml input
    for (int irow = int(ee.vecEEShape[2 * icol - 2]); irow <= int(ee.vecEEShape[2 * icol - 1]); ++irow) {
      if (ee.vecEESCLims[0] <= icol && ee.vecEESCLims[1] >= icol && ee.vecEESCLims[2] <= irow &&
	  ee.vecEESCLims[3] >= irow) {
	// Find SC type (complete or partial) for this location
	unsigned int isctype = 1;

	for (unsigned int ii = 0; ii < (unsigned int)(ee.nSCCutaway); ++ii) {
	  if ((ee.vecEESCCutaway[3 * ii] == icol) && (ee.vecEESCCutaway[3 * ii + 1] == irow)) {
	    isctype = int(ee.vecEESCCutaway[3 * ii + 2]);
	  }
	}

	// Create SC as a DDEcalEndcapTrap object and calculate rotation and
	// translation required to position it in the endcap.
	DDEcalEndcapTrap scrys(1, ee.sCEFront, ee.sCERear, ee.sCELength);
	scrys.moveto(ee.scrFCtr[icol - 1][irow - 1], ee.scrRCtr[icol - 1][irow - 1]);
	scrys.translate(DDTranslation(0., 0., -ee.zOff));

	string rname(ee.envName + std::to_string(isctype) + std::to_string(icol) + "R" + std::to_string(irow));
	// Position SC in endcap
	Volume quaLog = ns.volume(ee.quaName);
	Volume childEnvLog = ns.volume(myns + ee.envName + std::to_string(isctype));
	quaLog.placeVolume(childEnvLog,
			   100 * isctype + 10 * (icol - 1) + (irow - 1),
			   Transform3D(scrys.rotation(),
				       scrys.centrePos()));
      }
    }
  }

  return 1;
}

DECLARE_DDCMS_DETELEMENT(DDCMS_ecal_DDEcalEndcapAlgo, algorithm)
