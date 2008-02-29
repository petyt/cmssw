#ifndef EcalSelectiveReadoutValidation_H
#define EcalSelectiveReadoutValidation_H

/*
 * \file EcalSelectiveReadoutValidation.h
 *
 * $Date: 2007/05/21 13:21:51 $
 * $Revision: 1.2 $
 *
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"


#include "Geometry/CaloTopology/interface/EcalTrigTowerConstituentsMap.h"
#include "DataFormats/EcalDetId/interface/EcalScDetId.h"
#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
//#include "DataFormats/EcalDetId/interface/EEDetId.h"
//#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

#include "Validation/EcalDigis/src/CollHandle.h"

#include <string>
#include <utility>
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

class DQMStore;
class MonitorElement;
class EBDetId;
class EEDetId;
class EcalTPParameters;
class EcalElectronicsMapping;

class EcalSelectiveReadoutValidation: public edm::EDAnalyzer{

public:

  /// Constructor
  EcalSelectiveReadoutValidation(const edm::ParameterSet& ps);

  /// Destructor
  ~EcalSelectiveReadoutValidation();

protected:

  /// Analyzes the event.
  void analyze(const edm::Event& e, const edm::EventSetup& c);

  /// Calls at job start.
  void beginJob(const edm::EventSetup& c);

  /// Calls at job end.
  void endJob(void);

private:
  ///distinguishes barral and endcap of ECAL.
  enum subdet_t {EB, EE};

  /** Accumulates statitics for data volume analysis. To be called for each
   * ECAL digi. See anaDigiInit().
   */
  template<class T, class U>
  void anaDigi(const T& frame, const U& srFlagColl);

  /** Initializes statistics accumalator for data volume analysis. To
   * be call at start of each event analysis.
   */
  void anaDigiInit();

  /** Data volume analysis. To be called for each event.
   * @param event EDM event
   * @param es event setup
   */
  void analyzeDataVolume(const edm::Event& e, const edm::EventSetup& es);

  /** ECAL barrel data analysis. To be called for each event.
   * @param event EDM event
   * @param es event setup
   */
  void analyzeEB(const edm::Event& event, const edm::EventSetup& es);

  /** ECAL endcap data analysis. To be called for each event.
   * @param event EDM event
   * @param es event setup
   */
  void analyzeEE(const edm::Event& event, const edm::EventSetup& es);

  /** Trigger primitive analysis. To be called for each event.
   * @param event EDM event
   * @param es event setup
   */
  void analyzeTP(const edm::Event& event, const edm::EventSetup& es); 

  /** Energy reconstruction from ADC samples.
   * @param frame the ADC sample of an ECA channel
   */
  double frame2Energy(const EcalDataFrame& frame) const;

  /** Energy reconstruction from ADC samples to be used for trigger primitive
   * estimate.
   * @param frame the ADC sample of an ECA channel
   * @param offset time offset. To be used to evaluate energy of the event
   * previous (offset=-1) and next (offset=+1) to the triggered one.
   */
  template<class T>
  double frame2EnergyForTp(const T& frame, int offset = 0) const;
  
//   double getEcalEventSize(double nReadXtals) const{
//     return getDccOverhead(EB)*nEbDccs+getDccOverhead(EE)*nEeDccs
//       + nReadXtals*getBytesPerCrystal()
//       + (nEeRus+nEbRus)*8;
//   }

  /** Computes the size of an ECAL barrel event fragment.
   * @param nReadXtals number of read crystal channels
   * @return the event fragment size in bytes
   */
  double getEbEventSize(double nReadXtals) const{
    return getDccOverhead(EB)*nEbDccs + nReadXtals*getBytesPerCrystal()
      + nEbRus*8;
  }

  /** Computes the size of an ECAL endcap event fragment.
   * @param nReadXtals number of read crystal channels
   * @return the event fragment size in bytes
   */
  double getEeEventSize(double nReadXtals) const{
    return getDccOverhead(EE)*nEeDccs + nReadXtals*getBytesPerCrystal()
      + nEeRus*8;
  }

  /** Gets the size in bytes fixed-size part of a DCC event fragment.
   * @return the fixed size in bytes.
   */
  double getDccOverhead(subdet_t subdet) const{
    //  return (subdet==EB?34:25)*8;
    return (subdet==EB?33:51)*8;
  }

  /** Gets the number of bytes per crystal channel of the event part
   * depending on the number of read crystal channels.
   * @return the number of bytes.
   */
  double getBytesPerCrystal() const{
    return 3*8;
  }

  /** Gets the size of an DCC event fragment.
   * @param iDcc0 the DCC logical number starting from 0.
   * @param nReadXtals number of read crystal channels.
   * @return the DCC event fragment size in bytes.
   */
  double getDccEventSize(int iDcc0, double nReadXtals) const{
    subdet_t subdet;
  if(iDcc0<9 || iDcc0>=45){
    subdet = EE;
  } else{
    subdet = EB;
  }
  return getDccOverhead(subdet)+nReadXtals*getBytesPerCrystal()
    + getRuCount(iDcc0)*8;
  }

  /** Gets the number of readout unit read by a DCC. A readout unit
   * correspond to an active DCC input channel.
   * @param iDcc0 DCC logical number starting from 0.
   */
  int getRuCount(int iDcc0) const;

  /** Reads the data collections from the event. Called at start
   * of each event analysis.
   * @param event the EDM event.
   */
  void readAllCollections(const edm::Event& e);

  /** Computes trigger primitive estimates. A sum of crystal deposited
   * transverse energy is performed.
   * @param es event setup
   * @param ebDigis the ECAL barrel unsuppressed digi to use for the
   * computation
   * @param ebDigis the ECAL endcap unsuppressed digi to use for the
   * computation
   */
  void setTtEtSums(const edm::EventSetup& es,
		  const EBDigiCollection& ebDigis,
		  const EEDigiCollection& eeDigis);

  /** Retrieve the logical number of the DCC reading a given crystal channel.
   * @param xtarId crystal channel identifier
   * @return the DCC logical number starting from 1.
   */
  unsigned dccNum(const DetId& xtalId) const;

  /** Converts a std CMSSW crystal eta index to a c-array index (starting from
   * zero and without hole).
   */
  int iEta2cIndex(int iEta) const{
    return (iEta<0)?iEta+85:iEta+84;
  }

  /** Converts a std CMSSW crystal phi index to a c-array index (starting from
   * zero and without hole).
   */
  int iPhi2cIndex(int iPhi) const{
    return iPhi-1;
  }

  /** Converts a std CMSSW crystal x or y index to a c-array index (starting
   * from zero and without hole).
   */
  int iXY2cIndex(int iX) const{
    return iX-1;
  }

  /** converse of iXY2cIndex() method.
   */
  int cIndex2iXY(int iX0) const{
    return iX0+1;
  }

  /** converse of iEta2cIndex() method.
   */
  int cIndex2iEta(int i) const{
    return (i<85)?i-85:i-84;
  }


  /** converse of iPhi2cIndex() method.
   */
  int cIndex2iPhi(int i) const {
    return i+1;
  }

  /**Transforms CMSSW eta ECAL TT indices to indices starting at 0
   * to use for c-array or vector.
   * @param iEta CMSSW eta index (numbering -28...-1,28...56)
   * @return index in numbering from 0 to 55
   */
  int iTTEta2cIndex(int iEta) const{
    return (iEta<0)?iEta+28:iEta+27;
  }
  
  /**Transforms CMSSW phi ECAL crystal indices to indices starting at 0
   * to use for c-array or vector.
   * @param iPhi CMSSW phi index (numbering 1...72)
   * @return index in numbering 0...71
   */
  int iTTPhi2cIndex(int iPhi) const{
    return iPhi-1;
  }

  /** Retrieves the endcap supercrystal containing a given crysal
   * @param xtalId identifier of the crystal
   * @return the identifier of the supercrystal
   */
  EcalScDetId superCrystalOf(const EEDetId& xtalId) const;

  //@{
  /** Retrives the readout unit, a trigger tower in the barrel case,
   * and a supercrystal in the endcap case, a given crystal belongs to.
   * @param xtalId identifier of the crystal
   * @return identifer of the supercrystal or of the trigger tower.
   */
  EcalTrigTowerDetId readOutUnitOf(const EBDetId& xtalId) const;
  
  EcalScDetId readOutUnitOf(const EEDetId& xtalId) const;
  //@}

  //@{
  /** Wrappers to the book method of the DQMStore DQM
   *  histogramming interface.
   */
  MonitorElement* book1D(const std::string& name,
			 const std::string& title, int nbins,
			 double xmin, double xmax);
 
  MonitorElement* book2D(const std::string& name,
			 const std::string& title,
			 int nxbins, double xmin, double xmax,
			 int nybins, double ymin, double ymax);

  MonitorElement* bookProfile(const std::string& name,
			      const std::string& title,
			      int nbins, double xmin, double xmax);
  //@}

private:
  /** Used to store barrel crystal channel information
   */
  struct energiesEb_t{
    double simE;     ///sim hit energy sum
    double noZsRecE; ///energy reconstructed from unsuppressed digi
    double recE;     ///energy reconstructed from zero-suppressed digi
    //    EBDigiCollection::const_iterator itNoZsFrame; //
    int simHit;      ///number of sim hits
    double phi;      ///phi crystal position in degrees
    double eta;      ///eta crystal position           
  };

  /** Used to store endcap crystal channel information
   */
  struct energiesEe_t{
    double simE;     ///sim hit energy sum			      
    double noZsRecE; ///energy reconstructed from unsuppressed digi   
    double recE;     ///energy reconstructed from zero-suppressed digi
    //    EEDigiCollection::const_iterator itNoZsFrame;
    int simHit;      ///number of sim hits	       
    double phi;	     ///phi crystal position in degrees
    double eta;	     ///eta crystal position
  };

  /// number of bytes in 1 kByte:
  static const int kByte_ = 1024;
  
  ///Total number of DCCs
  static const unsigned nDccs = 54;

  /// number of DCCs for EB
  static const int nEbDccs = 36;

  /// number of DCCs for EE
  static const int nEeDccs = 18;

  ///number of RUs for EB
  static const int nEbRus = 36*68;

  ///number of RUs for EE
  static const int nEeRus = 2*(34+32+33+33+32+34+33+34+33);

  ///number of endcaps
  static const int nEndcaps = 2;

  ///number of crystals along Eta in EB
  static const int nEbEta = 170;

  ///number of crystals along Phi in EB
  static const int nEbPhi = 360;

  ///EE crystal grid size along X
  static const int nEeX = 100;

  ///EE crystal grid size along Y
  static const int nEeY = 100;

  ///Number of crystals along a supercrystal edge
  static const int scEdge = 5;

  ///Number of Trigger Towers along Eta
  static const int nTtEta = 56;

  ///Number of Trigger Towers along Phi
  static const int nTtPhi = 72;

  ///Conversion factor from radian to degree
  static const double rad2deg;
  
  ///Verbosity switch
  bool verbose_;

  ///Histogramming interface
  DQMStore* dbe_;

  ///Output file for histograms
  std::string outputFile_;

  //@{
  /** The event product collections.
   */
  CollHandle<EBDigiCollection>           ebDigis_;
  CollHandle<EEDigiCollection>           eeDigis_;
  CollHandle<EBDigiCollection>           ebNoZsDigis_;
  CollHandle<EEDigiCollection>           eeNoZsDigis_;
  CollHandle<EBSrFlagCollection>         ebSrFlags_;
  CollHandle<EESrFlagCollection>         eeSrFlags_;
  CollHandle<std::vector<PCaloHit> >     ebSimHits_;
  CollHandle<std::vector<PCaloHit> >     eeSimHits_;
  CollHandle<EcalTrigPrimDigiCollection> tps_;
  CollHandle<EcalRecHitCollection>       ebRecHits_;
  CollHandle<EcalRecHitCollection>       eeRecHits_;
  //@}

  //@{
  /** The histograms
   */
  MonitorElement* meDccVol_;
  MonitorElement* meVol_;
  MonitorElement* meVolB_;
  MonitorElement* meVolE_;
  MonitorElement* meVolBLI_;
  MonitorElement* meVolELI_;
  MonitorElement* meVolLI_;
  MonitorElement* meVolBHI_;
  MonitorElement* meVolEHI_;
  MonitorElement* meVolHI_;
  MonitorElement* meChOcc_;
  
  MonitorElement* meTp_;
  MonitorElement* meTtf_;
  MonitorElement* meTtfVsTp_;
  MonitorElement* meTtfVsEtSum_;
  MonitorElement* meTpVsEtSum_;

  MonitorElement* meEbRecE_;
  MonitorElement* meEbEMean_;
  MonitorElement* meEbNoise_;
  MonitorElement* meEbSimE_;
  MonitorElement* meEbRecEHitXtal_;
  MonitorElement* meEbRecVsSimE_;
  MonitorElement* meEbNoZsRecVsSimE_;
  
  MonitorElement* meEeRecE_;
  MonitorElement* meEeEMean_;
  MonitorElement* meEeNoise_;
  MonitorElement* meEeSimE_;
  MonitorElement* meEeRecEHitXtal_;
  MonitorElement* meEeRecVsSimE_;
  MonitorElement* meEeNoZsRecVsSimE_;
  //@}

  /** ECAL trigger tower mapping
   */
  const EcalTrigTowerConstituentsMap * triggerTowerMap_;

  /** Interface to access trigger primitive parameters,
   * especially to convert Et in compressed formart into natural unit.
   */
  const EcalTPParameters* tpParam_;

  /** Ecal electronics/geometrical mapping.
   */
  const EcalElectronicsMapping* elecMap_;
  
  /** Local reconstruction switch: true to reconstruct locally the amplitude
   * insted of using the Rec Hits.
   */
  bool localReco_;
  
  /** Weights for amplitude local reconstruction
   */
  std::vector<double> weights_;

  /** ECAL barrel read channel count
   */
  int nEb_;

  /** ECAL endcap read channel count
   */
  int nEe_;

  /** ECAL endcap low interest read channel count
   */
  int nEeLI_;

  /** ECAL endcap high interest read channel count
   */
  int nEeHI_;

  /** ECAL barrel low interest read channel count
   */
  int nEbLI_;

  /** ECAL barrel high interest read channel count
   */
  int nEbHI_;

  /** ECAL endcap read channel count
   */
  int nPerDcc_[nDccs];

  /** Event sequence number
   */
  int ievt_;

  /** Trigger tower Et computed as sum the crystal Et. Indices
   * stands for the eta and phi TT index starting from 0 at eta minimum and
   * at phi=0+ in std CMS coordinate system.
   */
  double ttEtSums[nTtEta][nTtPhi];

  /** Energy deposited in ECAL barrel crystals. Eta index starts from 0 at
   * eta minimum and phi index starts at phi=0+ in CMS std coordinate system.
   */
  energiesEb_t ebEnergies[nEbEta][nEbPhi];

  /** Energy deposited in ECAL endcap crystals. Endcap index is 0 for EE- and
   * 1 for EE+. X and Y index starts at x and y minimum in std CMS coordinate
   * system.
   */
  energiesEe_t eeEnergies[nEndcaps][nEeX][nEeY];
  
private:
  /** Used to sort crystal by decrasing simulated energy.
   */
  class Sorter{
    EcalSelectiveReadoutValidation* validation;
  public:
    Sorter(EcalSelectiveReadoutValidation* v): validation(v){};
    bool operator()(std::pair<int,int>a, std::pair<int,int>b){
      return (validation->ebEnergies[a.first][a.second].simE
	      > validation->ebEnergies[b.first][b.second].simE);
    }
  };
};

#endif //EcalSelectiveReadoutValidation_H not defined

