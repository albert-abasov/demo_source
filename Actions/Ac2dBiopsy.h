#pragma once

///////////////////////////////////////////////////////////////////////////
// COPYRIGHT KONINKLIJKE PHILIPS ELECTRONICS N.V. 2011
// All rights are reserved. Reproduction in whole or in part is
// prohibited without the written consent of the copyright owner.
///////////////////////////////////////////////////////////////////////////

#include <SimpleAction.h>
#include <EPMData.h>
#include <ActionData.h>

BEGIN_NS_EVTMGR

class CAc2DBiopsy : 
    public CSimpleAction,
    public CActionData1<bool>,
    public CEPMData
{
public:

    explicit CAc2DBiopsy( CActionId actionId );
    virtual ~CAc2DBiopsy();

    CAc2DBiopsy(const CAc2DBiopsy&) = delete;
    CAc2DBiopsy& operator=(const CAc2DBiopsy&) = delete;

    CAc2DBiopsy(const CAc2DBiopsy&&) = delete;
    CAc2DBiopsy& operator=(const CAc2DBiopsy&&) = delete;

    virtual void updateEPMData() override;
    virtual void perform( const CEMActionParams& params,  CEMTSEventData &) override;
    virtual void perform( const CEMActionParams& params,  CEMEventData &) override;
    virtual void perform( const CEMActionParams& params,  CEMInternalEventData &) override;

    virtual bool getInitialValue(CTSCtrlAttributes& tsCtrlAttrib) override;
    virtual bool getInitialValue(CCPCtrlAttributes& cpCtrlAttrib) override;
    bool getCtrlAvailability(bool& isCtrlVisilbe, bool& isCtrlAvailable);

private:

    // This will handle all action to apply after a biopsy guide press once a guide type is selected
    void doWorkForDelayedHandling( const CEMActionParams& params);

    // This will handle actions for scanhead reconnect or PercuNav/AIBreast procedure start.
    // bool bProcStarting: this is true when we are transition into PercuNav.On but the state hasn't been updated yet
    // The scenario here is when we want to force the user to select bracket type on procedure start or SH 
    //      connect immediately and not delayed until biopsy is turned on
    void doWorkForImmediateHandling(const CEMActionParams &params, bool bProcStarting);

    //pop up any dialogs needed due to biopsy guide button press, otherwise call doWorkForBiopsyButtonPress
    //The scenario here is to delay forcing the user to select bracket type until biopsy guide is tuned on
    void biopsyButtonPressed(const CEMActionParams& params);

    void syncInternalDataToULSIAndPercuNav(bool bTurningBiopsyGuideOff);

    // send current biopsy guide state over to PercuNav
    // if bTurningBiopsyGuideOff == true, then send false for bBiopsyGuideOn regardless of real state
    // if bDontCheckState == true then only check if PN or AI breast is licensed instead of active/paused
    void SendDataToPercuNav(bool bTurningBiopsyGuideOff, bool bDontCheckState);

    void resetEpmDataFlags();
    static bool isOptionedForPercuNavMultiBracket();

    // Method which removes BIOPSY_POSITION  from the trackball list
    //if Biopsy is enabled when Live loop capture is in progress
    void updateTrackballListForOCI2D();

    int m_nBiopsyIsAllowed;    

    //if transducer supports 2 biopsy guides
    int m_nNum1Angles; 
    int m_nNum2Angles; 
    int m_biopsyType1;
    int m_biopsyType2;
    int m_selectedBiopsyType;
    bool m_bUserHasSelectedBiopsyType; // to avoid reasking between PercuNav and Voyager
    bool m_bSkipAngleSelection;        // Intermediate state to only apply biopsy type change
    bool m_bDisplayGuideSelectionDlg;
    bool m_bDisplayVerzaSpecificDlg;
    bool m_bDisableVerzaInSelection;   // disable the verza selection via internal capability

    int m_nNumAngles; // will have number of angles for the selected guide.
    int m_nAngleIndex; // 0,1,2
    bool m_bDisplayWarningPopup;//true is the warning popup must be displayed (C9-5ec selected)
    bool m_bIsInterventionTSP; // is it an intervention TSP
    EBiopsyMultipleGuideWarning     m_biopsyGuide;
    bool m_bIsScanheadMotorized;
};

END_NS_EVTMGR
 